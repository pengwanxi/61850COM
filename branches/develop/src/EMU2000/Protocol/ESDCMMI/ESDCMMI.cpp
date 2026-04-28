#include "ESDCMMI.h" 
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

#define ERROR_CONST			5
#define COMSTATUS_ONLINE	1
#define COMSTATUS_FAULT		0

#define ERROR_FUN			1
#define ERROR_REGISTER		2
#define ERROR_DATA			3
#define ERROR_CONFIG		4

#define WORD_LENGTH 				7

#define YK_TIME_INTERVAL			6		/*��,ң�س�ʱʱ��*/
#define DZ_TIME_INTERVAL			10		/*��,��ֵ��ʱʱ��*/

#define FRAMEBYTE_CONTROLBYTE		0x71	/*��վ����վ*/
#define FRAMEKIND_YC		 		0x61
#define FRAMEKIND_YX				0xF4
#define FRAMEKIND_YM				0x85
#define FRAMEKIND_SOE				0x26
#define FRAMEKIND_FAIL				0x29

#define FRAMEKIND_DOWN_SETTIME		0x7A	/*��վ����վ*/
#define FRAMEKIND_DOWN_BECKONTIME	0x4C
#define FRAMEKIND_DOWN_YKRESET		0x61
#define FRAMEKIND_DOWN_YKEXECUTE	0xC2
#define FRAMEKIND_DOWN_YKCANCEL		0xB3
#define FRAMEKIND_DOWN_RESET		0x3D
#define FRAMEKIND_DOWN_DZ			0xA9

#define SEND_SOE_FRAME				1
#define SEND_YX_FRAME				2
#define SEND_YC_FRAME				3
#define SEND_YM_FRAME				4
#define SEND_FAIL_FRAME				5

#define YK_PROTOCOL_CLOSE			0XCC
#define YK_PROTOCOL_OPEN			0X33
#define YK_PROTOCOL_ERR				0XFF
#define YK_MODULE_CLOSE				1
#define YK_MODULE_OPEN				0
#define YK_PROTOCOL_EXECUTE			0XAA
#define YK_PROTOCOL_CANCEL			0X55
#define YK_TYPE_NORMAL				0X3A
#define YK_TYPE_MUTIL				0X35

#define YK_VALID					1
#define YK_INVALID					0
#define YK_PROCESS_SEND				0
#define YK_PROCESS_RECEIVE			1
#define YK_PROCESS_OVERTIME			2
#define YK_PROCESS_SUCCESS			3

#define YX_OPEN						0
#define YX_CLOSE					1


#define ESDCMMIPACKETLEN			512

extern "C" void GetCurrentTime( REALTIME *pRealTime );
extern "C" int  SetCurrentTime( REALTIME *pRealTime );


ESDCMMI::ESDCMMI()
{
	FirstFlag = TRUE;
	m_wErrorTimer = 0;
	m_byPortStatus = 0;	
	curSendType_normal = SEND_YX_FRAME;
	m_bRetTime = FALSE;
	memset(mt_sMasterAddr, 0, sizeof(mt_sMasterAddr));
}

ESDCMMI::~ESDCMMI()
{
	
}

BOOL ESDCMMI::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg ) 
{
	if( buf == NULL )
		return FALSE ;

	if( FirstFlag )
	{
		FirstFlag = FALSE;
		sleep(10);
	}
	
	
	if( pBusMsg )
	{
		DealBusMsgInfo( pBusMsg );
	}

	BYTE bySendType;

	if(m_iSOE_rd_p != m_iSOE_wr_p)
		bySendType = SEND_SOE_FRAME;
	else
		bySendType = curSendType_normal;

	switch (bySendType)
	{
	case SEND_YX_FRAME:/*��ʼ֡*/
		ESDCMMI_assembleYxFrame(buf,len);
		break;
	case SEND_YC_FRAME:
		ESDCMMI_assembleYcFrame(buf,len);
		break;
	case SEND_YM_FRAME:
		ESDCMMI_assembleYmFrame(buf,len);
		break;
	case SEND_SOE_FRAME:
		ESDCMMI_assembleSoeFrame(buf,len);
		break;
	case SEND_FAIL_FRAME:
		ESDCMMI_assembleFailFrame(buf,len);
		break;
	default:
		ESDCMMI_assembleYxFrame(buf,len);
		break;
	}

	m_wErrorTimer++;
	if( m_wErrorTimer > ERROR_CONST )
	{
		m_wErrorTimer = ERROR_CONST + 1 ;
		m_byPortStatus = COMSTATUS_FAULT;
	}
	else
	{
		m_byPortStatus = COMSTATUS_ONLINE;
	}

	return TRUE;

}

BOOL ESDCMMI::ProcessProtocolBuf( BYTE * buf , int len ) 
{

	BOOL Rtn = FALSE;
	if( len == 0 || buf == NULL )
		return FALSE ;

	//BYTE	ctrlByte=0,frameKind=0,infoWords=0,cmdType=0,crc=0;
	BYTE frameKind=0,infoWords=0,cmdType=0,crc=0;
	//WORD  iClearCount=0;
	WORD i=0;
	BYTE * pointer = NULL,*pData;
	BOOL bInfoOk;
	//BOOL bReturn=TRUE;
	
	pointer=buf;

	if(len < WORD_LENGTH*2)
		return FALSE;

	while(len>=WORD_LENGTH*2)
	{
		if(!ESDCMMI_isSyncWord(pointer))
		{
			len--;
			pointer++;
			//iClearCount++;
			continue;
		}

		crc = ESDCMMI_GetCRC(pointer+WORD_LENGTH, WORD_LENGTH-1);	/* crc ctrl_word*/
		if(crc!=*(pointer+WORD_LENGTH*2-1))
		{
			len-=WORD_LENGTH*2;
			pointer+=WORD_LENGTH*2;
			//iClearCount +=WORD_LENGTH*2;
			continue;
		}

		//ctrlByte = *(pointer+WORD_LENGTH);
		frameKind= *(pointer+WORD_LENGTH+1);
		infoWords= *(pointer+WORD_LENGTH+2);

		if( len < (infoWords+2)*WORD_LENGTH)
			break;

		len-=WORD_LENGTH*2;
		pointer+=WORD_LENGTH*2;
		//iClearCount+=WORD_LENGTH*2;

		bInfoOk = TRUE;
		pData=pointer;
		for(i=0;i<infoWords;i++)/*info all right*/
		{
			crc = ESDCMMI_GetCRC( pointer, WORD_LENGTH-1);
			if(crc != *(pointer+WORD_LENGTH-1) )
			{
				bInfoOk = FALSE;
			}
			len -= WORD_LENGTH;
			pointer += WORD_LENGTH;
			//iClearCount +=WORD_LENGTH;
		}
		
		if(!bInfoOk)
		{
			//bReturn = FALSE;
			break;
		}

		switch(frameKind)
		{
		case FRAMEKIND_DOWN_SETTIME:/*��������ʱ������*/
			Rtn = ESDCMMI_setSubstationTime(pData);
			break;
		case FRAMEKIND_DOWN_BECKONTIME:/*�����ٻ���վʱ������*/
			m_bRetTime = TRUE;
			break;
		case FRAMEKIND_DOWN_YKRESET:/*ң��Ԥ��*/
			cmdType = YK_SEL;
			Rtn = ESDCMMI_handleYkCmd(pData,cmdType);
			break;
		case FRAMEKIND_DOWN_YKEXECUTE:/*ң��ִ��*/
			cmdType = YK_EXCT;
			Rtn = ESDCMMI_handleYkCmd(pData,cmdType);
			break;
		case FRAMEKIND_DOWN_YKCANCEL:/*ң�س���*/
			cmdType = YK_CANCEL;
			Rtn = ESDCMMI_handleYkCmd(pData,cmdType);
			break;
		case FRAMEKIND_DOWN_RESET:/*��������*/
			//bReturn=FALSE;
			break;
		case FRAMEKIND_DOWN_DZ:/*read or write a group of DZ*/
			//bReturn=FALSE;
			break;
		default:
			//bReturn=FALSE;
			break;
		}
	}

	if( Rtn == TRUE )
	{
		m_wErrorTimer = 0;
		return TRUE ;
	}
	return FALSE ;
}

BOOL ESDCMMI::Init( BYTE byLineNo )
{
	m_byLineNo = byLineNo ;
	m_byProID    = 4;
    m_byEnable   = 1;
	m_wObjNum  = 1;
	sprintf( m_szObjName, "%s", m_sDevName );
	sprintf( m_ComCtrl1, "%s", mt_sMasterAddr );
	m_wRtuAddr = m_wDevAddr ;
	char szFileName[256] = "";

	sprintf( szFileName, "%s%s", ESDCMMIPREFIXFILENAME, m_sTemplatePath );
	//��ȡ��Ҫת�������ݵ���ģ��
    ReadMapConfig( szFileName );

	//��ʼ����ģ��
	InitRtuBase() ;

	//��ʼ���ϴ�װ��״̬ʱ��
	//time( &m_DevStateTime ) ;
	return TRUE ;
}

BOOL ESDCMMI::InitRtuBase( )
{
    UINT uPort;
    BOOL bOk = FALSE;
    char szCtrl[32];

    //OutPromptText("****CRtu104.Init()****");

    CBasePort::GetCommAttrib(m_ComCtrl1, szCtrl, uPort);

    m_wPortNum = (WORD)uPort ;

	//��ȡת�����
    CreateTransTab();

	//���ڴ����ݿ���--��ȡת����Ĭ������
	m_pMethod->ReadAllYcData(&m_fYCBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwYMBuf[0]);
	m_pMethod->ReadAllYxData( &m_byYXbuf[ 0 ] ) ;

	m_bTaskRun = TRUE;
    return bOk;
}

BOOL ESDCMMI::DealBusMsgInfo( PBUSMSG pBusMsg )
{
	YK_DATA *pData = (YK_DATA *)(pBusMsg->pData);

	switch ( pBusMsg->byMsgType )
	{
		case YK_PROTO:
			switch ( pBusMsg->dwDataType )
			{
				case YK_SEL_RTN:
					if( pBusMsg->DataNum != 1 || pBusMsg->DataLen != sizeof(YK_DATA) )
					{
						printf("ESDCMMI Yk DataNum err\n");
						return -1;
					}
					if( (ESDCMMI_Yk_Data.m_byValid == YK_VALID) && (ESDCMMI_Yk_Data.m_byYkCmd == YK_SEL) )
					{
						if( (ESDCMMI_Yk_Data.m_byLineNo == pBusMsg->SrcInfo.byBusNo)
								&& (ESDCMMI_Yk_Data.m_byAddress == pBusMsg->SrcInfo.wDevNo)
								&& (ESDCMMI_Yk_Data.m_byPointNo == pData->wPnt) 
								&& (ESDCMMI_Yk_Data.m_byYkCmd == YK_SEL) 
								&& (ESDCMMI_Yk_Data.m_byStatus == YK_PROCESS_SEND) )
						{	
							if( pData->byVal != ESDCMMI_Yk_Data.m_byYkAction )
							{
								ESDCMMI_Yk_Data.m_byYkAction = YK_ERROR;
							}
							ESDCMMI_Yk_Data.m_byStatus = YK_PROCESS_RECEIVE;
							ESDCMMI_Yk_Data.m_byYkCmd = YK_SEL_RTN;
						}
					}
					break;
				default:
					//printf("ESDCMMI can't find the YK_DATATYPE %d\n", (int)pBusMsg->dwDataType);
					break;
			}				/* -----  end switch  ----- */
			break;

		default:
			break;
	}				/* -----  end switch  ----- */
	return TRUE;
}


/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_addSyncWord
// Description	    : Modefied by YangBin	2003-12-17 17:28:10
// Return type		: void
// Argument         : UINT8 *pBuf
----------------------------------------------------------------------- */
void ESDCMMI::ESDCMMI_addSyncWord(BYTE *pBuf)
{
	pBuf[0] = 0xEB;		/*ESDCMMIЭ��涨*/
	pBuf[1] = 0x90;
	pBuf[2] = 0xEB;
	pBuf[3] = 0x90;
	pBuf[4] = 0xEB;
	pBuf[5] = 0x90;
	pBuf[6] = 0xFF;
}

/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_addControlWord
// Description	    : Modefied by YangBin	2003-12-17 17:33:44
// Return type		: void
// Argument         : UINT8 *pBuf
// Argument         : BYTE frameKind
// Argument         : BYTE infoWords
----------------------------------------------------------------------- */
void ESDCMMI::ESDCMMI_addControlWord(BYTE *pBuf,BYTE frameKind,BYTE infoWords)
{
	pBuf[0] = FRAMEBYTE_CONTROLBYTE;
	pBuf[1] = frameKind;
	pBuf[2] = infoWords;
	pBuf[3] = 0x00;
	pBuf[4] = 0x00;
	pBuf[5] = 0x00;
	pBuf[6] = ESDCMMI_GetCRC(pBuf,6);

	return;
}


BOOL ESDCMMI::ESDCMMI_assembleYxFrame( BYTE * pBuf , int &len )
{
	BYTE *pCtrlWord=NULL;
	WORD wYxNum,wSendOrder=0,sendBytes=0,priorBytes=0;
	BYTE byInfoWords=0,byTemp;
	BYTE funcCode=0;
	BOOL bChange=FALSE;
	DWORD dwYxData;
	
	ESDCMMI_addSyncWord(pBuf);
	pBuf += WORD_LENGTH;
	sendBytes = WORD_LENGTH;
	
	pCtrlWord = pBuf;
	pBuf += WORD_LENGTH;
	sendBytes += WORD_LENGTH;

	priorBytes=ESDCMMI_addPriorWord(pBuf,ESDCMMIPACKETLEN-sendBytes);
	pBuf+=priorBytes;
	sendBytes+=priorBytes;
	byInfoWords+=(priorBytes/WORD_LENGTH);

	wYxNum = GetPntSum(YX_SUM); 
	wSendOrder = ESDCMMI_sendOrder;
	funcCode=(BYTE)(wSendOrder/32);

	while(sendBytes+WORD_LENGTH<=ESDCMMIPACKETLEN && wSendOrder<wYxNum)
	{
		if(wSendOrder<wYxNum-32)
			byTemp=32;
		else
			byTemp=wYxNum-wSendOrder;
		ESDCMMI_generateYxDWord(wSendOrder,&dwYxData,byTemp);
		wSendOrder+=byTemp;

		pBuf[0]=0x00;
		pBuf[1]=funcCode;
		pBuf[2]=LOBYTE(LOWORD(dwYxData));
		pBuf[3]=HIBYTE(LOWORD(dwYxData));
		pBuf[4]=LOBYTE(HIWORD(dwYxData));
		pBuf[5]=HIBYTE(HIWORD(dwYxData));
		pBuf[6]=ESDCMMI_GetCRC(pBuf,6);

		pBuf+=WORD_LENGTH;
		sendBytes+=WORD_LENGTH;
		byInfoWords++;

		if(funcCode>=0xFF || wSendOrder>=wYxNum)
		{
			curSendType_prior = SEND_YM_FRAME;
			curSendType_normal = SEND_YC_FRAME;
			ESDCMMI_sendOrder = 0;
			bChange=TRUE;
			break;
		}

		funcCode++;
	}
	
	if(!bChange)
		ESDCMMI_sendOrder = wSendOrder;

	ESDCMMI_addControlWord(pCtrlWord,FRAMEKIND_YX,byInfoWords);

	if(!wYxNum)					  /*��ң��*/
	{
		curSendType_prior = SEND_YM_FRAME;
		curSendType_normal = SEND_YC_FRAME;
		ESDCMMI_sendOrder = 0;
	}
	len = sendBytes;
	return TRUE;
}

BOOL ESDCMMI::ESDCMMI_assembleYcFrame( BYTE * pBuf , int &len )
{
	BYTE *pCtrlWord=NULL;
	WORD wYcNum,wSendOrder=0,sendBytes=0,priorBytes=0;
	BYTE byInfoWords=0;
	float fVal = 0.0f;
	BOOL bChange=FALSE;
	BYTE byBuffer[4];
	
	ESDCMMI_addSyncWord(pBuf);
	pBuf += WORD_LENGTH;
	sendBytes = WORD_LENGTH;
	
	pCtrlWord = pBuf;
	pBuf += WORD_LENGTH;
	sendBytes += WORD_LENGTH;

	priorBytes=ESDCMMI_addPriorWord(pBuf,ESDCMMIPACKETLEN-sendBytes);
	sendBytes+=priorBytes;
	pBuf+=priorBytes;
	byInfoWords+=(priorBytes/WORD_LENGTH);
	
	wYcNum = GetPntSum(YC_SUM); 
	wSendOrder = ESDCMMI_sendOrder;
	
	while(sendBytes+WORD_LENGTH<=ESDCMMIPACKETLEN && wSendOrder<wYcNum)
	{
		pBuf[0] = HIBYTE(wSendOrder);
		pBuf[1] = LOBYTE(wSendOrder);
		
		fVal = m_fYCBuf[wSendOrder];
        if( m_pAIMapTab[wSendOrder].wStn>0 && m_pAIMapTab[wSendOrder].wPntNum>0 )
            fVal = CalcAIRipeVal(m_pAIMapTab[wSendOrder].wStn,m_pAIMapTab[wSendOrder].wPntNum,m_fYCBuf[wSendOrder]);
		
		memcpy(&(byBuffer[0]),&fVal,4);

		pBuf[2] = byBuffer[ 3 ] ;
		pBuf[3] = byBuffer[ 2 ] ;
		pBuf[4] = byBuffer[ 1 ] ;
		pBuf[5] = byBuffer[ 0 ] ;
		pBuf[6] = ESDCMMI_GetCRC(pBuf,6);
	

		wSendOrder++;
		pBuf+=WORD_LENGTH;
		sendBytes+=WORD_LENGTH;
		byInfoWords++;

		if(wSendOrder>0x7FF || wSendOrder>=wYcNum)/*����β�����������*/
		{
			curSendType_normal = curSendType_prior;
			ESDCMMI_sendOrder = 0;
			bChange=TRUE;
			break;
		}
	}
	if(!bChange)
		ESDCMMI_sendOrder = wSendOrder;
	
	ESDCMMI_addControlWord(pCtrlWord,FRAMEKIND_YC,byInfoWords);

	if(!wYcNum)                      /*��ң��*/
	{
		curSendType_normal = curSendType_prior;
		ESDCMMI_sendOrder = 0;
	}
	len = sendBytes;
	
	return TRUE;
}

BOOL ESDCMMI::ESDCMMI_assembleYmFrame( BYTE * pBuf , int &len )	
{
	BYTE *pCtrlWord=NULL;
	WORD wYmNum,wSendOrder=0,sendBytes=0,priorBytes=0;
	BYTE byInfoWords=0;
	float fVal = 0.0f;
	BOOL bChange=FALSE;
	BYTE byBuffer[4];
	BOOL flag ;
	
	ESDCMMI_addSyncWord(pBuf);
	pBuf += WORD_LENGTH;
	sendBytes = WORD_LENGTH;
	
	pCtrlWord = pBuf;
	pBuf += WORD_LENGTH;
	sendBytes += WORD_LENGTH;

	priorBytes=ESDCMMI_addPriorWord(pBuf,ESDCMMIPACKETLEN-sendBytes);
	sendBytes+=priorBytes;
	pBuf+=priorBytes;
	byInfoWords+=(priorBytes/WORD_LENGTH);
	
	wYmNum = GetPntSum(YM_SUM); 
	wSendOrder = ESDCMMI_sendOrder;

	
	while(sendBytes+WORD_LENGTH<=ESDCMMIPACKETLEN && wSendOrder<wYmNum)
	{
		pBuf[0] = HIBYTE(wSendOrder);
		pBuf[1] = LOBYTE(wSendOrder);

		fVal = ( float )GetPulseData ( m_pPIMapTab[wSendOrder].wStn,m_pPIMapTab[wSendOrder].wPntNum ,&flag );
		
		memcpy(&(byBuffer[0]),&fVal,4);

		pBuf[2] = byBuffer[ 3 ] ;
		pBuf[3] = byBuffer[ 2 ] ;
		pBuf[4] = byBuffer[ 1 ] ;
		pBuf[5] = byBuffer[ 0 ] ;
		pBuf[6] = ESDCMMI_GetCRC(pBuf,6);

		wSendOrder++;
		pBuf+=WORD_LENGTH;
		sendBytes+=WORD_LENGTH;
		byInfoWords++;

		if(wSendOrder>0x3FF || wSendOrder>=wYmNum)/*����β�����������*/
		{
			curSendType_prior = SEND_FAIL_FRAME;
			curSendType_normal = SEND_YC_FRAME;
			ESDCMMI_sendOrder = 0;
			bChange=TRUE;
			break;
		}
	}
	
	if(!bChange)
		ESDCMMI_sendOrder = wSendOrder;
	
	ESDCMMI_addControlWord(pCtrlWord,FRAMEKIND_YM,byInfoWords);
	
	if(!wYmNum)					  /*��ң��*/
	{
		curSendType_prior = SEND_FAIL_FRAME;
		curSendType_normal = SEND_YC_FRAME;
		ESDCMMI_sendOrder = 0;
	}
	len = sendBytes;
	return TRUE;
}

BOOL ESDCMMI::ESDCMMI_assembleSoeFrame( BYTE * pBuf , int &len )	
{
	BYTE *pCtrlWord=NULL;
	WORD sendBytes=0,priorBytes=0;
	WORD   wPnt, wVal, wMiSecond , wSerialNo;
	struct tm  tmStruct;
	int 	intV;
	BYTE  	byteV,byteV1;
	
	ESDCMMI_addSyncWord(pBuf);
	pBuf += WORD_LENGTH;
	sendBytes = WORD_LENGTH;
	
	pCtrlWord = pBuf;
	pBuf += WORD_LENGTH;
	sendBytes += WORD_LENGTH;
	
	priorBytes=ESDCMMI_addPriorWord(pBuf,ESDCMMIPACKETLEN-sendBytes);
	sendBytes+=priorBytes;
	pBuf+=priorBytes;

	while((sendBytes+WORD_LENGTH*2 <= ESDCMMIPACKETLEN)&&(m_iSOE_rd_p != m_iSOE_wr_p))
	{
		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond);
		/*��һ����*/
		pBuf[0] = 0x00;
		pBuf[1] = 0x80;
		intV = wMiSecond;
		byteV = intV&0xFF;
		pBuf[2] = byteV;
		intV = intV >> 8;
		byteV = intV&0x03;
		pBuf[3]= byteV;
		intV = tmStruct.tm_sec;
		byteV = intV&0x3F;
		pBuf[4] = byteV;
		intV = tmStruct.tm_min;
		byteV = intV&0x3F;
		pBuf[5] = byteV;
		pBuf[6] = ESDCMMI_GetCRC(pBuf,6);
		pBuf+=WORD_LENGTH;

		/*�ڶ�����*/
		pBuf[0] = 0x00;
		pBuf[1] = 0x81;
		intV = tmStruct.tm_hour;
		byteV = intV&0x1F;
		pBuf[2] = byteV;
		intV = tmStruct.tm_mday;
		byteV = intV&0x1F;
		pBuf[3] = byteV;
		intV = wPnt;
		byteV = intV&0xFF;
		pBuf[4] = byteV;
		intV = intV >> 8;
		byteV = intV&0x0F;
		byteV1 = wVal;
		if (byteV1)
			byteV |= 0x80;
		else
			byteV &= 0x7F;
		pBuf[5] = byteV;
		pBuf[6] = ESDCMMI_GetCRC(pBuf,6);
		sendBytes += WORD_LENGTH*2;
		pBuf+=WORD_LENGTH;
	}

	if(sendBytes>WORD_LENGTH*2)
		ESDCMMI_addControlWord(pCtrlWord,FRAMEKIND_SOE,sendBytes/WORD_LENGTH-2);
	else
		sendBytes=0;
	
	len = sendBytes;
	return TRUE;
}

BOOL ESDCMMI::ESDCMMI_assembleFailFrame( BYTE * pBuf , int &len )
{
	BYTE *pCtrlWord=NULL;
	WORD wSendOrder=0,sendBytes=0,priorBytes=0;
	BYTE byInfoWords=0;
	BOOL bChange=FALSE;
	BYTE byLine,bySlave;
	BYTE ByLineDevNum = 0;
	BOOL rtn;
	
	ESDCMMI_addSyncWord(pBuf);
	pBuf += WORD_LENGTH;
	sendBytes = WORD_LENGTH;
	
	pCtrlWord = pBuf;
	pBuf += WORD_LENGTH;
	sendBytes += WORD_LENGTH;

	priorBytes=ESDCMMI_addPriorWord(pBuf,ESDCMMIPACKETLEN-sendBytes);
	sendBytes+=priorBytes;
	pBuf+=priorBytes;
	byInfoWords+=(priorBytes/WORD_LENGTH);

	wSendOrder=ESDCMMI_sendOrder;
	byLine=HIBYTE(wSendOrder);
	bySlave=LOBYTE(wSendOrder);

	while(byLine<=m_pMethod->GetToTalBusNum( ))
	{
		BYTE ProtocolType = m_pMethod->GetBusLineProtocolType( byLine );
#if 0					//by cyz 2017-06-27	�����һ������ΪPAUSEʱ�޷�����װ��״̬����!
		if( ProtocolType != PROTOCO_GATHER && ProtocolType != PROTOCO_TRANSPROT )
			byLine = m_pMethod->GetToTalBusNum( );
		if(byLine==m_pMethod->GetToTalBusNum( ))
		{
			curSendType_prior = SEND_YX_FRAME;
			curSendType_normal = SEND_YC_FRAME;
			ESDCMMI_sendOrder = 0;
			bChange=TRUE;
			break;
		}
#endif
		if( ProtocolType != PROTOCO_GATHER && ProtocolType != PROTOCO_TRANSPROT ){		//by cyz! 2017-06-27 �滻��������!
			if(byLine == 22){
			curSendType_prior = SEND_YX_FRAME;
			curSendType_normal = SEND_YC_FRAME;
			ESDCMMI_sendOrder = 0;
			bChange=TRUE;
			break;
			}else{
				byLine++;
				continue;
			}
		}
		if(sendBytes+WORD_LENGTH*3>ESDCMMIPACKETLEN)
			break;

		ByLineDevNum=m_pMethod->GetDevNum(byLine);

		if(ByLineDevNum==0)
		{
			byLine++;
			bySlave=0;
			continue;
		}

		rtn = ESDCMMI_InsertFail(byLine, bySlave, pBuf);
		if( rtn )
		{
			pBuf+=WORD_LENGTH*3;
			sendBytes+=WORD_LENGTH*3;
			byInfoWords+=3;
		}
		if(bySlave==0)
			bySlave=1;
		else if(bySlave>=ByLineDevNum)
		{
			byLine++;
			bySlave=0;
		}
		else
			bySlave++;
	}

	if(!bChange)
	{
		wSendOrder=MAKEWORD(bySlave, byLine);
		ESDCMMI_sendOrder = 0;
	}
	ESDCMMI_addControlWord(pCtrlWord,FRAMEKIND_FAIL,byInfoWords);
	len = sendBytes;
	return TRUE;
}

/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_addPriorWord
// Description	 : 
// Return type	 : WORD
// Argument         : BYTE *pBuf
// Argument         : WORD nByte
----------------------------------------------------------------------- */
WORD ESDCMMI::ESDCMMI_addPriorWord( BYTE *pBuf , WORD nByte )
{
	WORD bytes=0;
	int len;
	WORD   wPnt, wVal, wSerialNo;
	
	while((bytes+WORD_LENGTH*3) <= nByte)/*ң�ű�λ*/
	{
		if(!GetDigitalEvt( wSerialNo , wPnt, wVal))
			break;
		if(!ESDCMMI_insertYxChp( wPnt , pBuf ))
			break;
		pBuf += WORD_LENGTH*3;/*��Ҫ��������*/
		bytes += WORD_LENGTH*3;
		
	}

	if((ESDCMMI_Yk_Data.m_byYkCmd == YK_SEL_RTN)&&(ESDCMMI_Yk_Data.m_byValid == YK_VALID)&&(WORD_LENGTH*3 <=(nByte-bytes)))
	{
	char chDebugBuf[100]="";
	sprintf(chDebugBuf , "%s","insert YK_SEL_RTN\n");
	
		len=ESDCMMI_assembleYkRevise(pBuf);
		if (len>0)/*�Ƿ���װ��ң��Ԥ�÷�У������ִ�з�У*/
		{
			pBuf += len;
			bytes += len;/*��Ҫ��������*/
		}
	
		OutBusDebug(m_byLineNo, (BYTE *)chDebugBuf, strlen( chDebugBuf ), 2);
	}

	if ((m_bRetTime == TRUE)&&(bytes+WORD_LENGTH*2 <= nByte)) /*�Ƿ���վҪ��Ҫ����վ����ʱ��*/
	{
		ESDCMMI_assemSubRetTime(pBuf);
		pBuf += WORD_LENGTH*2;
		bytes += WORD_LENGTH*2;
		m_bRetTime = FALSE;
	}

	return bytes;
}


/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_InsertWord
// Description	    : 
// Return type		: void
// Argument         : BYTE *pBuf
// Argument         : BYTE func_high
// Argument         : BYTE func_low
// Argument         : BYTE module_actionNo
// Argument         : BYTE byLineNo
// Argument         : BYTE module_addr
// Argument         : BYTE module_ykNo
----------------------------------------------------------------------- */
void ESDCMMI::ESDCMMI_InsertWord(BYTE *pBuf,BYTE func_high,BYTE func_low,BYTE module_actionNo,BYTE byLineNo,BYTE module_addr,BYTE module_ykNo)
{
	pBuf[0] = func_high;
	pBuf[1] = func_low;
	pBuf[2] = module_actionNo;
	pBuf[3] = byLineNo;
	pBuf[4] = module_addr;
	pBuf[5] = module_ykNo;
	pBuf[6] =ESDCMMI_GetCRC(pBuf,6);

	return;
}



/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_assembleYkRevise
// Description	: 
// Return type	: BYTE
// Argument		: BYTE* pBuf
// Argument		: struct STRUCT_YK_DATA *pYkData
// Argument		: BYTE ykType
----------------------------------------------------------------------- */
BYTE ESDCMMI::ESDCMMI_assembleYkRevise(BYTE* pBuf)
{
	BYTE 	bytes=0;
	time_t	time0,time1;
	BYTE	*pBuf0=pBuf;
	int 	intSec=0;
	BYTE 	protocol_ykAction,module_ykNo,module_addrNo,yk_lineNo;

	if(ESDCMMI_Yk_Data.m_byYkCmd == YK_SEL_RTN)/*����ң��Ԥ��*/
	{
		module_ykNo = ESDCMMI_Yk_Data.m_byPointNo;
		yk_lineNo=ESDCMMI_Yk_Data.m_byLineNo;
		module_addrNo=ESDCMMI_Yk_Data.m_byAddress;
		if(ESDCMMI_Yk_Data.m_byYkAction==YK_ERROR)/*��Ч���*/
		{
			ESDCMMI_Yk_Data.m_byValid=YK_INVALID;
			ESDCMMI_InsertWord(pBuf0,0xE1,YK_TYPE_NORMAL,YK_PROTOCOL_ERR,yk_lineNo,module_addrNo,module_ykNo);
			pBuf0 += WORD_LENGTH;
			memcpy(pBuf0,pBuf0-WORD_LENGTH,WORD_LENGTH);
			pBuf0 += WORD_LENGTH;
			memcpy(pBuf0,pBuf0-WORD_LENGTH,WORD_LENGTH);
			bytes+=WORD_LENGTH*3;

			return bytes;
		}
		
		protocol_ykAction = ESDCMMI_Yk_Data.m_byYkAction?YK_PROTOCOL_CLOSE:YK_PROTOCOL_OPEN;

		if(ESDCMMI_Yk_Data.m_byValid == YK_VALID)
		{
			if(ESDCMMI_Yk_Data.m_byStatus == YK_PROCESS_RECEIVE)/*Ԥ�óɹ�*/
			{
				ESDCMMI_Yk_Data.m_byValid=YK_INVALID;
				ESDCMMI_Yk_Data.m_byStatus=YK_PROCESS_SUCCESS;/*��������*/
				ESDCMMI_InsertWord(pBuf0,0xE1,YK_TYPE_NORMAL,protocol_ykAction,yk_lineNo,module_addrNo,module_ykNo);
				pBuf0 += WORD_LENGTH;
				memcpy(pBuf0,pBuf0-WORD_LENGTH,WORD_LENGTH);
				pBuf0 += WORD_LENGTH;
				memcpy(pBuf0,pBuf0-WORD_LENGTH,WORD_LENGTH);
				bytes+=WORD_LENGTH*3;

				return bytes;
			}
			time1 = time(NULL);
			time0 = ESDCMMI_Yk_Data.m_tm;
			intSec = (int)difftime(time1,time0);

			if(intSec > YK_TIME_INTERVAL)/*Ԥ�ó�ʱ*/
			{
				ESDCMMI_Yk_Data.m_byValid=YK_INVALID;
				ESDCMMI_Yk_Data.m_byStatus=YK_PROCESS_OVERTIME;/*��������*/
				ESDCMMI_InsertWord(pBuf0,0xE1,YK_TYPE_NORMAL,YK_PROTOCOL_ERR,yk_lineNo,module_addrNo,module_ykNo);
				pBuf0 += WORD_LENGTH;
				memcpy(pBuf0,pBuf0-WORD_LENGTH,WORD_LENGTH);
				pBuf0 += WORD_LENGTH;
				memcpy(pBuf0,pBuf0-WORD_LENGTH,WORD_LENGTH);
				bytes+=WORD_LENGTH*3;

				return bytes;
			}
		}
	}

	return bytes;
}


/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_assemSubRetTime
// Description	    : 
// Return type		: void
// Argument         : BYTE* pBuf
----------------------------------------------------------------------- */
void ESDCMMI::ESDCMMI_assemSubRetTime(BYTE* pBuf)
{
	BYTE	*pHead;
	REALTIME tm1;
	int 	intV;
	BYTE	byteV;

	GetCurrentTime(&tm1);
	pHead=pBuf;

	pBuf[0] = 0x00;/*��1����*/
	pBuf[1] = 0x84;
	intV = tm1.wMilliSec;
	byteV = intV&0xFF;/*�Ͱ�λ*/
	pBuf[2] = byteV;
	intV = intV >> 8;
	byteV = intV&0x03;/*�߰�λ��max-999*/
	pBuf[3] = byteV;
	intV = tm1.wSecond;
	byteV = intV&0x3F;
	pBuf[4] = byteV;
	intV = tm1.wMinute;
	byteV = intV&0x3F;
	pBuf[5] = byteV;
	pBuf[6] = ESDCMMI_GetCRC(pHead,6);

	pBuf += WORD_LENGTH;
	pHead += WORD_LENGTH;

	pBuf[0] = 0x00;/*��2����*/
	pBuf[1] = 0x85;
	intV =tm1.wHour;
	byteV = intV&0xFF;
	pBuf[2] = byteV;
	intV = tm1.wDay;
	byteV = intV&0xFF;
	pBuf[3] = byteV;
	intV = tm1.wMonth;
	byteV = intV&0xFF;
	pBuf[4] = byteV;
	intV = tm1.wYear%100;
	byteV = intV&0xFF;
	pBuf[5] = byteV;
	pBuf[6] = ESDCMMI_GetCRC(pHead,6);
}


/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_GetYxWordFromYxChP
// Description	    : 
// Return type		: BOOL
// Argument         : WORD YxOrder
// Argument         : BYTE* pBuf
----------------------------------------------------------------------- */
BOOL ESDCMMI::ESDCMMI_GetYxWordFromYxChP(WORD YxOrder,BYTE* pBuf)
{
	BYTE 	count=0;
	WORD 	funcCode = 0,maxYxOrder=0,yxLoWord=0,yxHiWord=0;
	WORD  baseYxOrder=0;
	DWORD   yxDWord=0;

	maxYxOrder=GetPntSum(YX_SUM)-1;
	if(YxOrder>maxYxOrder)
		return FALSE;

	funcCode = (YxOrder)/32;
	if(funcCode>0xFF)
		return FALSE;

	baseYxOrder=funcCode*32;/*ÿ��ң����ң����32Ϊ�����Ļ�����ʼ*/
	if((maxYxOrder-baseYxOrder+1)>=32)
		count=32;
	else
		count=maxYxOrder-baseYxOrder+1;

	pBuf[0] = HIBYTE(funcCode);
	pBuf[0]|=0x80;/*���룬���λ��1*/
	pBuf[1]=LOBYTE(funcCode);

	ESDCMMI_generateYxDWord(baseYxOrder,&yxDWord,count);
	yxLoWord=LOWORD(yxDWord);
	yxHiWord=HIWORD(yxDWord);
	pBuf[2] = LOBYTE(yxLoWord);
	pBuf[3] = HIBYTE(yxLoWord);
	pBuf[4] = LOBYTE(yxHiWord);
	pBuf[5] = HIBYTE(yxHiWord);
	pBuf[6] = ESDCMMI_GetCRC(pBuf,6);

	return TRUE;

}

/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_insertYxChp
// Description	    : 
// Return type		: BOOL
// Argument         : WORD YxChp
// Argument         : BYTE* pBuf
----------------------------------------------------------------------- */
BOOL ESDCMMI::ESDCMMI_insertYxChp(WORD YxChp,BYTE* pBuf)
{
	BYTE yxWord[WORD_LENGTH];

	if(!ESDCMMI_GetYxWordFromYxChP(YxChp,yxWord))
		return FALSE;

	memcpy(pBuf,yxWord,WORD_LENGTH);/*��������ң����3��*/
	pBuf += WORD_LENGTH;
	memcpy(pBuf,yxWord,WORD_LENGTH);
	pBuf += WORD_LENGTH;
	memcpy(pBuf,yxWord,WORD_LENGTH);

	return TRUE;
}


/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_generateYxDWord
// Description	    : Modefied by yangkaikai
// Return type		: BOOL
// Argument         : DWORD * pYxDWord
// Argument         : BYTE YxNum
----------------------------------------------------------------------- */

BOOL ESDCMMI::ESDCMMI_generateYxDWord(WORD wSendOrder,DWORD * pYxDWord,BYTE YxNum)
{
	DWORD dwYxWord=0;
	BYTE i;

	*pYxDWord=0;
	for (i = 0; i < YxNum; i++)
	{
		if(m_byYXbuf[wSendOrder]==YX_CLOSE)
			dwYxWord|=1<<i;

		wSendOrder++;
	}

	*pYxDWord=dwYxWord;
	return TRUE;
}


BOOL ESDCMMI::ESDCMMI_InsertFail(BYTE byLineNo,BYTE ModuleNoAddOne,BYTE *pBuf)
{
	REALTIME pRealTime;
	BYTE byType,byState;
	WORD bySlaveAddr;
	
	GetCurrentTime(&pRealTime);

	pBuf[0]=0x00;
	pBuf[1]=0x80;
	pBuf[2]=LOBYTE(pRealTime.wMilliSec);
	pBuf[3]=HIBYTE(pRealTime.wMilliSec);
	pBuf[4]=pRealTime.wSecond;
	pBuf[5]=pRealTime.wMinute;
	pBuf[6]=ESDCMMI_GetCRC(pBuf, WORD_LENGTH-1);
	pBuf+=WORD_LENGTH;

	pBuf[0]=0x00;
	pBuf[1]=0x81;
	pBuf[2]=pRealTime.wHour;
	pBuf[3]=pRealTime.wDay;
	pBuf[4]=pRealTime.wMonth;
	pBuf[5]=pRealTime.wYear%100;
	pBuf[6]=ESDCMMI_GetCRC(pBuf, WORD_LENGTH-1);
	pBuf+=WORD_LENGTH;

	byType=(ModuleNoAddOne==0);
	if(byType)
	{
		byType=1;
		byState=(m_pMethod->GetCommState(byLineNo)!=COM_DEV_NORMAL);
		bySlaveAddr = 0;
	}
	else
	{
		byType=2;
		bySlaveAddr = m_pMethod->GetAddrByLineNoAndModuleNo( byLineNo, ModuleNoAddOne-1 );
		if( 0 != bySlaveAddr )
			byState=(m_pMethod->GetDevCommState(byLineNo, bySlaveAddr)!=COM_DEV_NORMAL);
		else
			return FALSE;
	}

	pBuf[0]=0x00;
	pBuf[1]=0x82;
	pBuf[2]=byLineNo+1;
	pBuf[3]=bySlaveAddr & 0xFF;
	pBuf[4]=byState;
	pBuf[5]=byType;
	pBuf[6]=ESDCMMI_GetCRC(pBuf, WORD_LENGTH-1);
	return TRUE;
}

/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_isSyncWord
// Description	    : 
// Return type		: BOOL
// Argument         : BYTE* pWord
----------------------------------------------------------------------- */
BOOL ESDCMMI::ESDCMMI_isSyncWord( BYTE * pWord )
{
	return (0xEB==pWord[0] && 0x90==pWord[1] && 0xEB==pWord[2] && 0x90==pWord[3] && 0xEB==pWord[4] && 0x90==pWord[5] && 0xFF==pWord[6]);
}


/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_setSubstationTime
// Description	    : 
// Return type		: BOOL
// Argument         : BYTE* pBuf
----------------------------------------------------------------------- */
BOOL ESDCMMI::ESDCMMI_setSubstationTime(BYTE* pBuf)
{
	BYTE * pointer=NULL;
	WORD 	msec=0;
	BYTE	sec=0;
	BYTE	min=0;
	BYTE	hour=0;
	BYTE	day=0;
	BYTE	month=0;
	BYTE	year=0;
	REALTIME tm1;

	pointer=pBuf;

	if((pointer[0]!=0)||(pointer[1]!=0xEE))/*Э��涨*/
		return FALSE;
	msec = pointer[3];					/*MILLSECOND HI*/
	msec = msec << 8;
	msec |= (WORD)pointer[2];			/*MILLSECOND LOW*/
	sec	= pointer[4];
	min	= pointer[5];
	pointer+=WORD_LENGTH;

	/*process second info-word*/
	if((pointer[0]!=0)||(pointer[1]!=0xEF))/*Э��涨*/
		return FALSE;
	hour = pointer[2];
	day	= pointer[3];
	month	= pointer[4];
	year = pointer[5];

	tm1.wMilliSec = msec;
	tm1.wSecond = sec;
	tm1.wMinute = min;
	tm1.wHour = hour;
	tm1.wDay = day;
	tm1.wMonth = month;
	tm1.wYear = year+2000;

	return SetCurrentTime(&tm1);
}

/* -----------------------------------------------------------------------
// Function name	: ESDCMMI_handleYkCmd
// Description	    : 
// Return type		: BOOL
// Argument         : BYTE* pBuf
// Argument         : BYTE cmdType
----------------------------------------------------------------------- */
BOOL  ESDCMMI::ESDCMMI_handleYkCmd(BYTE* pBuf,BYTE cmdType)
{
	time_t	time0,time1;
	int 	intSec=0;
	BYTE module_action=0;
	BYTE func_hign=pBuf[0];
	BYTE func_low=pBuf[1];
	BYTE protocol_action=pBuf[2];
	BYTE line_no=pBuf[3];
	BYTE addr_no=pBuf[4];
	BYTE ctrl_no=pBuf[5];
	//BYTE port_no=line_no+1;
	
	time0 = ESDCMMI_Yk_Data.m_tm;
	time1 = time(NULL);
	intSec = (int)difftime(time1,time0);
	if(abs(intSec) > YK_TIME_INTERVAL)/*Ԥ�ó�ʱ*/
	{
		ESDCMMI_Yk_Data.m_byValid=YK_INVALID;
		ESDCMMI_Yk_Data.m_byStatus=YK_PROCESS_OVERTIME;
	}
	if( ESDCMMI_Yk_Data.m_byValid == YK_VALID )
	{
		printf("YK_VALID\n");
		return FALSE;
	}
	if((YK_SEL==cmdType&&func_hign!=0XE0)||(YK_EXCT==cmdType&&func_hign!=0XE2)||(YK_CANCEL==cmdType&&func_hign!=0XE3))
		return FALSE;/*������Ч���*/

	switch(func_low)
	{
	case YK_TYPE_NORMAL:
		break;
	case YK_TYPE_MUTIL:/*�ݲ�֧��*/
		return FALSE;
	default:
		return FALSE;
	}

	switch(protocol_action)
	{
	case YK_PROTOCOL_CLOSE:
		module_action=YK_MODULE_CLOSE;
		break;
	case YK_PROTOCOL_OPEN:
		module_action=YK_MODULE_OPEN;
		break;
	case YK_PROTOCOL_EXECUTE:
	case YK_PROTOCOL_CANCEL:
		module_action=ESDCMMI_Yk_Data.m_byYkAction;/*ִ�С�����*/
		break;
	default:
		return FALSE;/*�����������ݲ�֧��*/
	}
	
	switch( cmdType )
	{
	case YK_SEL:
		m_pMethod->SetYkSel(this, line_no, addr_no, ctrl_no, module_action);
		break;
	case YK_EXCT:
		m_pMethod->SetYkExe(this, line_no, addr_no, ctrl_no, module_action);
		break;
	case YK_CANCEL:
		m_pMethod->SetYkCancel(this, line_no, addr_no, ctrl_no, module_action);
	default:
		return FALSE;
	}
	ESDCMMI_Yk_Data.m_byLineNo = line_no;
	ESDCMMI_Yk_Data.m_byAddress = addr_no;
	ESDCMMI_Yk_Data.m_byPointNo = ctrl_no;
	ESDCMMI_Yk_Data.m_byYkCmd = cmdType;
	ESDCMMI_Yk_Data.m_byYkAction = module_action;
	ESDCMMI_Yk_Data.m_tm = time(NULL);
	ESDCMMI_Yk_Data.m_byValid = YK_VALID;
	ESDCMMI_Yk_Data.m_byStatus = YK_PROCESS_SEND;

	if(YK_SEL!=cmdType)/*ң��ִ�С�ң�س���������У*/
	{
		ESDCMMI_Yk_Data.m_byValid=YK_INVALID;
	}

	return TRUE;
}



BOOL ESDCMMI::WriteAIVal(WORD wSerialNo ,WORD wPnt, float wVal)
{
    if(m_pwAITrans==NULL) return FALSE;
    WORD wNum = m_pwAITrans[wPnt];
    if(wNum>m_wAISum) return FALSE;
    if(wNum<ESDCMMIMAX_AI_LEN)
    {
        float nDelt = wVal - m_fYCBuf[wNum];
        if(abs((int)nDelt)>=m_wDeadVal)
        {
            m_fYCBuf[wNum] = wVal;
            AddAnalogEvt( wSerialNo , wNum , wVal );
        }
    }
    return TRUE ;
}

BOOL ESDCMMI::WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal)
{
    if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>m_wDISum) return FALSE;
    if( wNum<ESDCMMIMAX_DI_LEN)
    {
        if( m_byYXbuf[ wNum ] != wVal )
        {
            m_byYXbuf[ wNum ] = wVal ;
            
            AddDigitalEvt( wSerialNo , wNum , wVal );
        }
    }
    return TRUE ;
}
BOOL ESDCMMI::WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal)
{
    if(m_pwPITrans==NULL) return FALSE;
    WORD wNum = m_pwPITrans[wPnt];
    if(wNum>m_wPISum) return FALSE;
    if(wNum<ESDCMMIMAX_PI_LEN)
    {
        m_dwYMBuf[wNum] = dwVal;
    }
    return TRUE ;
}

BOOL ESDCMMI::WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{
    if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>=m_wDISum) return FALSE;
    if(wNum<ESDCMMIMAX_DI_LEN)
    {
        AddSOEInfo(wSerialNo , wNum, wVal, lTime, wMiSecond);
    }
    return TRUE ;
}

void ESDCMMI::TimerProc()
{
	//���ڴ��ж�ȡ�仯ң�ź�ң������
	ReadChangData();

}

BOOL ESDCMMI::GetDevCommState( )
{
	if(m_byPortStatus == COMSTATUS_ONLINE)
	{
		return COM_DEV_NORMAL ;
    }
	else
	{
		return COM_DEV_ABNORMAL ;
	}	
}

