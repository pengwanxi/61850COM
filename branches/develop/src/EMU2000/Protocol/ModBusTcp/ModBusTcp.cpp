#include "ModBusTcp.h"
#include <stdlib.h>


#define ERROR_CONST			5
#define COMSTATUS_ONLINE	1
#define COMSTATUS_FAULT		0

#define ERROR_FUN			1
#define ERROR_REGISTER		2
#define ERROR_DATA			3
#define ERROR_CONFIG		4

#define MAX_LINE	200

ModBusTcp::ModBusTcp()
{/*{{{*/
	SendFlag = 0;
	MsgFlag = 0;
	FunNum = 0;
	m_wErrorTimer = 0;
	m_byPortStatus = 0;
	ErrorFlag = 0;
	yk_recv_flag = FALSE;
	time_flag = 0;
	Yk_FunNum = 0x05;
	memset(yk_bufSerial,0,sizeof(yk_bufSerial));
	memset(mt_sMasterAddr, 0, sizeof(mt_sMasterAddr));
	m_busDevStateArray.reserve(300);
}/*}}}*/

ModBusTcp::~ModBusTcp()
{
	int size = m_busDevStateArray.size();
	for (int i = 0; i < size; i++)
	{
		PMBS_BUSDEV pState = m_busDevStateArray[i];
		if (pState)
			delete pState;
	}
	m_busDevStateArray.clear();
}

BOOL ModBusTcp::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg )
{/*{{{*/

	if( buf == NULL )
		return FALSE ;

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

	if( pBusMsg )
	{
		DealBusMsgInfo( pBusMsg);
		//return FALSE;
	}


	if( MsgFlag == 2 )
	{
		return FALSE;
	}
	MsgFlag = 2;
	memcpy( buf , MsgBuf , MsgLen );
	len = MsgLen;
	memset( MsgBuf , 0 , MsgLen);
	MsgLen = 0;
	yk_recv_flag = FALSE;
	return TRUE;

}/*}}}*/

BOOL ModBusTcp::ProcessProtocolBuf( BYTE * buf , int len )
{/*{{{*/
	//printf("0\n");
	BOOL Rtn = FALSE;
	if( len == 0 || buf == NULL )
		return FALSE ;

	if(yk_recv_flag){
		time_t t;
		t = time(NULL);
		if((t - time_flag) < 5)
			return FALSE;
	}

	if(buf[7] == 0x05){
		yk_recv_flag = TRUE;
		time_t t;
		t = time(NULL);
		time_flag = t;
	}
	//	printf("m_wRtuAddr== %d\n",m_wRtuAddr);
	if( buf[2]==0 && buf[3]==0 && buf[4]==0
			&& (buf[5]+6)==len && buf[6]== m_wRtuAddr )//&& len == 12 )
			{//printf("1\n");
				FunNum = buf[7];
				MsgFlag = 0;
				switch( FunNum )
				{
				case 2:
					Rtn = YXPacket( buf , len );
					break;
				case 3:
					Rtn = YcYmPacket( buf , len );
					break;
				case 4:
					Rtn = YcYmPacket( buf , len );
					break;
				case 5:
					Rtn = YKMsg( buf , len );
					break;
				case 6:
					Rtn = DzPacket(buf, len);
					break;
				default:
					ErrorPacket( buf , ERROR_FUN );
					break;
				}
				if( Rtn == TRUE )
				{
					m_wErrorTimer = 0;
				}
				return TRUE ;
			}
	return FALSE ;
}/*}}}*/

BOOL ModBusTcp::DzPacket(BYTE * buf, int len)
{
	WORD mt_register = 256 * buf[8] + buf[9];

	if (  mt_register > 10000 || mt_register == 0  )
	{
		ErrorPacket(buf, ERROR_REGISTER);
		return FALSE;
	}
	BYTE byBusNo;
	WORD byStn;
	BYTE byStatus;
	WORD wDevAddr;
	WORD wPnt;
	WORD wVal = 0;

	byStn = m_pDZMapTab[mt_register - 1].wStn - 1;
	wPnt = m_pDZMapTab[mt_register - 1].wPntNum - 1;
	if (m_pDZMapTab[mt_register - 1].wStn > 0 && m_pDZMapTab[mt_register - 1].wPntNum > 0)
	{
		dz_buff[0] = buf[0];
		dz_buff[1] = buf[1];
		dz_FunCode = buf[7];
		wVal = MAKEWORD(buf[11], buf[10]);

		if (m_pMethod->GetBusLineAndAddr(byStn, byBusNo, wDevAddr))
		{
			DZ_DATA dzData;
			dzData.wPnt = wPnt;
			dzData.byVal[0] = LOBYTE(wVal);
			dzData.byVal[1] = HIBYTE(wVal);
			dzData.byType = 1;

			m_pMethod->SetDzWriteExct(this, byBusNo, wDevAddr, 0, (DZ_DATA *)&dzData, 1);
		}
		dzRegisterAddr = mt_register;
		dzRegisterNum = MAKEWORD(buf[11], buf[10]);
		return TRUE;
	}
	else
	{
		ErrorPacket(buf, ERROR_REGISTER);
		return FALSE;
	}
}

BOOL ModBusTcp::Init( BYTE byLineNo )
{/*{{{*/
	m_byLineNo = byLineNo ;
	m_byProID    = 4;
	m_byEnable   = 1;
	m_wObjNum  = 1;
	sprintf( m_szObjName, "%s", m_sDevName );
	sprintf( m_ComCtrl1, "%s", mt_sMasterAddr );
	m_wRtuAddr = m_wDevAddr ;
	char szFileName[256] = "";

	sprintf( szFileName, "%s%s", MODBUSTCPPREFIXFILENAME, m_sTemplatePath );
	//��ȡ��Ҫת�������ݵ���ģ��
	ReadMapConfig( szFileName );

	//��ʼ����ģ��
	InitRtuBase() ;

	//��ʼ���ϴ�װ��״̬ʱ��
	//time( &m_DevStateTime ) ;
	return TRUE ;
}/*}}}*/

BOOL ModBusTcp::InitRtuBase( )
{/*{{{*/
	UINT uPort;
	BOOL bOk = FALSE;
	char szCtrl[32];

	//OutPromptText("****CRtu104.Init()****");

	CBasePort::GetCommAttrib(m_ComCtrl1, szCtrl, uPort);

	m_wPortNum = (WORD)uPort ;

	//��ȡת�����
	CreateTransTab();

	//���ڴ����ݿ���--��ȡת����Ĭ������
	m_pMethod->ReadAllYcData(&m_wYCBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwYMBuf[0]);
	m_pMethod->ReadAllYxData( &m_byYXbuf[ 0 ] ) ;

	m_bTaskRun = TRUE;
	return bOk;
}/*}}}*/

BOOL ModBusTcp::DealBusMsgInfo( PBUSMSG pBusMsg )
{/*{{{*/
	int DeviceNo = 0;
	switch ( pBusMsg->byMsgType )
	{
	case YK_PROTO:
		switch (pBusMsg->dwDataType)
		{
			//case YK_SEL_RTN:
		case YK_EXCT_RTN:
		{
			//case YK_CANCEL_RTN:
			YK_DATA *pData = (YK_DATA *)(pBusMsg->pData);
			if (pBusMsg->DataNum != 1
				|| pBusMsg->DataLen != sizeof(YK_DATA))
			{
				printf("ModBusTcp Yk DataNum err\n");
				return -1;
			}

			DeviceNo = m_pMethod->GetSerialNo(pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo);
			if (DeviceNo == -1)
				return -1;


			YKPacket(m_wRelayNum, pData->byVal);
			MsgFlag = 0;
			if (pData->byVal == YK_ERROR && DeviceNo > 10000)
			{
				if (DeviceNo > 10000)
					ErrorPacket(yk_bufSerial, ERROR_REGISTER);
				if (pData->byVal == YK_ERROR)
					ErrorPacket(yk_bufSerial, ERROR_DATA);
			}
		}
		break;
		default:
			printf("ModBusTcp can't find the YK_DATATYPE %d\n", (int)pBusMsg->dwDataType);
			break;
		}				/* -----  end switch  ----- */
		break;
	case DZ_PROTO:
	{
		MsgFlag = 0;
		processDzRtn(pBusMsg);
	}
	break;
	default:
		break;
	}				/* -----  end switch  ----- */
	return TRUE;
}/*}}}*/

void ModBusTcp::processDzRtn(PBUSMSG pBusMsg)
{
	if (!pBusMsg || pBusMsg->dwDataType != DZ_WRITE_EXCT_RTN)
		return;

	WORD index = 0;
	WORD ModBusTcpByteLen = 6;

	if (dzRegisterAddr > 10000 || dzRegisterAddr == 0)
	{
		ErrorPacket(dz_buff, ERROR_REGISTER);
		return ;
	}
	MsgBuf[index++] = dz_buff[0];
	MsgBuf[index++] = dz_buff[1];
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = ModBusTcpByteLen >> 8;
	MsgBuf[index++] = ModBusTcpByteLen;
	MsgBuf[index++] = m_wRtuAddr;
	MsgBuf[index++] = FunNum;

	MsgBuf[index++] = HIBYTE(dzRegisterAddr );
	MsgBuf[index++] = LOBYTE(dzRegisterAddr);
	MsgBuf[index++] = HIBYTE(dzRegisterNum);
	MsgBuf[index++] = LOBYTE(dzRegisterNum);

	MsgLen = index;
	return ;
}

void ModBusTcp::RelayEchoProc(BYTE byCommand, WORD wIndex, BYTE byResult)
{}

BOOL ModBusTcp::YXPacket( BYTE * buf , int len )
{/*{{{*/
	WORD index = 0;
	WORD mt_register = 256 * buf[8] + buf[9];
	BOOL bFlag = FALSE;
	if (mt_register > 0 && mt_register < 10000)
		bFlag = ProcessYx(buf, len);
	else if (mt_register > 20000 && mt_register < 25000)
		bFlag = ProcessDevState(buf, len);

	return TRUE;
}/*}}}*/

BOOL ModBusTcp::ProcessDevState(BYTE * buf, int len)
{
	WORD index = 0;
	WORD mt_register = 256 * buf[8] + buf[9];
	WORD registernum = 256 * buf[10] + buf[11];
	WORD ModBusByteLen = ((registernum - 1) / 8) + 1;
	WORD ModBusTcpByteLen = 3 + ModBusByteLen;

	MsgBuf[index++] = buf[0];
	MsgBuf[index++] = buf[1];
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = ModBusTcpByteLen >> 8;
	MsgBuf[index++] = ModBusTcpByteLen;
	MsgBuf[index++] = m_wRtuAddr;
	MsgBuf[index++] = FunNum;
	MsgBuf[index++] = ModBusByteLen;		//ModBus�����ֽ�

	int i = 0;
	WORD wYxNo = 0;
	BYTE byData = 0;

	wYxNo = mt_register - 20001;
	BYTE byVal = 0;
	while (registernum > 8)
	{
		byData = 0;
		for (i = 0; i < 8; i++)
		{
			byVal = 0;
			if (!GetDevState(wYxNo++, &byVal))
			{
				ErrorPacket(buf, ERROR_REGISTER);
				return TRUE;
			}
			const BYTE CLOSE_STATE = 1;
			if (byVal == CLOSE_STATE)
				byData |= (1 << i);
		}

		MsgBuf[index++] = byData;
		registernum -= 8;
	}

	byData = 0;
	byVal = 0;
	for (i = 0; i < registernum; i++)
	{
		if (!GetDevState(wYxNo++, &byVal))
		{
			ErrorPacket(buf, ERROR_REGISTER);
			return TRUE;
		}
		const BYTE CLOSE_STATE = 1;
		if (byVal == CLOSE_STATE)
			byData |= (1 << i);
	}

	MsgBuf[index++] = byData;

	MsgLen = index;
}

BOOL ModBusTcp::ProcessYx(BYTE * buf, int len)
{
	WORD index = 0;
	WORD mt_register = 256 * buf[8] + buf[9];
	WORD registernum = 256 * buf[10] + buf[11];

	WORD ModBusByteLen = ((registernum - 1) / 8) + 1;
	WORD ModBusTcpByteLen = 3 + ModBusByteLen;

	if ((mt_register > 9999 || mt_register == 0) || (registernum > 9999 || registernum == 0))
	{
		if (mt_register > 9999 || mt_register == 0)
			ErrorPacket(buf, ERROR_REGISTER);
		return FALSE;
	}
	MsgBuf[index++] = buf[0];
	MsgBuf[index++] = buf[1];
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = ModBusTcpByteLen >> 8;
	MsgBuf[index++] = ModBusTcpByteLen;		//ModBusTcp�����ֽ�
	MsgBuf[index++] = m_wRtuAddr;
	MsgBuf[index++] = FunNum;
	MsgBuf[index++] = ModBusByteLen;		//ModBus�����ֽ�
	int j = 0;
	for (j = 0; j < ModBusByteLen; j++)
	{
		//�ӹ����ڴ���ȡ����
		BYTE i = 0;
		BYTE ByYXVal = 0;
		for (i = 0; i < 8; i++)
		{
			if (j == ModBusByteLen - 1 && i == registernum % 8 && 0 != registernum % 8)
				break;
			//printf("m_pDIMapTab[%d].wStn =%d\n",mt_register-1+(8*j+i),m_pDIMapTab[mt_register-1+(8*j+i)].wStn);
			if (m_pDIMapTab[mt_register - 1 + (8 * j + i)].wStn > 0 && m_pDIMapTab[mt_register - 1 + (8 * j + i)].wPntNum > 0)
				ByYXVal |= m_byYXbuf[mt_register - 1 + (8 * j + i)] << i;
			else
			{
				ErrorPacket(buf, ERROR_REGISTER);
				return FALSE;
			}
		}
		MsgBuf[index++] = ByYXVal;
	}
	MsgLen = index;
	return TRUE;
}

BOOL ModBusTcp::YcYmPacket( BYTE * buf , int len )
{/*{{{*/
	BOOL flag ;
	QWORD ymval=0 ;
	WORD index = 0,wVal = 0;;
	float fVal = 0.0f ;
	WORD mt_register = 256 * buf[8] + buf[9];
	WORD registernum = 256 * buf[10] + buf[11];

	WORD ModBusByteLen = 2*registernum;
	WORD ModBusTcpByteLen = 3+ModBusByteLen;
	//printf("mt_register = %d registernum = %d\n",mt_register,registernum);
	if( ( mt_register > 9999 || mt_register == 0 ) || ( registernum > 9999 || registernum == 0 )
			|| ( mt_register <= 6800 && mt_register+registernum > 6800 ) )
	{
		ErrorPacket( buf , ERROR_REGISTER );
		return FALSE;
	}
	MsgBuf[index++] = buf[0];
	MsgBuf[index++] = buf[1];
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = ModBusTcpByteLen >> 8;
	MsgBuf[index++] = ModBusTcpByteLen;		//ModBusTcp�����ֽ�
	MsgBuf[index++] = m_wRtuAddr;
	MsgBuf[index++] = FunNum;
	MsgBuf[index++] = ModBusByteLen;		//ModBus�����ֽ�
	int j = 0;
	for( j=0; j<registernum; j++ )
	{
		//�ӹ����ڴ���ȡ����
		if( mt_register >= 1 && mt_register <= 6800 )
		{
			if( m_pAIMapTab[mt_register-1+j].wStn > 0 && m_pAIMapTab[mt_register-1+j].wPntNum > 0 )
			{
				fVal = CalcAIRipeVal(m_pAIMapTab[mt_register-1+j].wStn, m_pAIMapTab[mt_register-1+j].wPntNum, m_wYCBuf[mt_register-1+j]);	//�ӹ����ڴ���ȡ����!
				if (fVal < 0)
				{
					wVal = -fVal;
					wVal = ~wVal + 1;
				}
				else
					wVal = fVal;

				MsgBuf[index++] = HIBYTE(wVal);
				MsgBuf[index++] = LOBYTE(wVal);
			}
			else
			{
				ErrorPacket( buf , ERROR_REGISTER );
				return FALSE;
			}
		}
		else if( mt_register >= 6801 && mt_register <= 9999 )
		{
			if( ( ( mt_register-6801 )%2 == 1 ) || ( registernum%2 == 1 ) )
			{
				ErrorPacket( buf , ERROR_REGISTER );
				return FALSE;
			}
			else
			{
				if( j%4 == 0 )
				{
					if( m_pPIMapTab[(mt_register-6801)/4+j/4].wStn > 0 && m_pPIMapTab[(mt_register-6801)/4+j/4].wPntNum > 0 )
					{
						ymval = GetPulseData ( m_pPIMapTab[(mt_register-6801)/4+j/4].wStn,
								m_pPIMapTab[(mt_register-6801)/4+j/4].wPntNum, &flag  );
						if( !flag )
						{
							ErrorPacket( buf , ERROR_DATA );
							return FALSE;
						}						
						// IEEE754双精度转换
                        BYTE byteBuffer[8];
                        memcpy(byteBuffer, &ymval, sizeof(QWORD));                        
                        // 按大端字节序写入8字节
                        MsgBuf[index++] = byteBuffer[7];  // 最高有效字节
                        MsgBuf[index++] = byteBuffer[6];
                        MsgBuf[index++] = byteBuffer[5];
                        MsgBuf[index++] = byteBuffer[4];
                        MsgBuf[index++] = byteBuffer[3];
                        MsgBuf[index++] = byteBuffer[2];
                        MsgBuf[index++] = byteBuffer[1];
                        MsgBuf[index++] = byteBuffer[0];  // 最低有效字节				
					}
					else
					{
						ErrorPacket( buf , ERROR_REGISTER );
						return FALSE;
					}
				}
				/*
				if( j%2 == 0 )
				{
					if( m_pPIMapTab[(mt_register-6801)/2+j/2].wStn > 0 && m_pPIMapTab[(mt_register-6801)/2+j/2].wPntNum > 0 )
					{
						ymval = GetPulseData ( m_pPIMapTab[(mt_register-6801)/2+j/2].wStn,
								m_pPIMapTab[(mt_register-6801)/2+j/2].wPntNum, &flag  );
						if( !flag )
						{
							ErrorPacket( buf , ERROR_DATA );
							return FALSE;
						}
						#if 0
						MsgBuf[index++] = HIBYTE(HIWORD((DWORD)ymval));
						MsgBuf[index++] = LOBYTE(HIWORD((DWORD)ymval));
						MsgBuf[index++] = HIBYTE(LOWORD((DWORD)ymval));
						MsgBuf[index++] = LOBYTE(LOWORD((DWORD)ymval));
						#endif
						// IEEE754双精度转换
                        BYTE byteBuffer[8];
                        memcpy(byteBuffer, &ymval, sizeof(double));
                        
                        // 按大端字节序写入8字节
                        MsgBuf[index++] = byteBuffer[7];  // 最高有效字节
                        MsgBuf[index++] = byteBuffer[6];
                        MsgBuf[index++] = byteBuffer[5];
                        MsgBuf[index++] = byteBuffer[4];
                        MsgBuf[index++] = byteBuffer[3];
                        MsgBuf[index++] = byteBuffer[2];
                        MsgBuf[index++] = byteBuffer[1];
                        MsgBuf[index++] = byteBuffer[0];  // 最低有效字节				
					}
					else
					{
						ErrorPacket( buf , ERROR_REGISTER );
						return FALSE;
					}
				}
					*/
			}
		}
	}
	MsgLen = index;
	return TRUE;
}/*}}}*/

BOOL ModBusTcp::YKMsg( BYTE * buf , int len )
{/*{{{*/
	WORD mt_register = 256 * buf[8] + buf[9];

	if( ( mt_register > 10000 || mt_register == 0 ) ||
			!( ( buf[10] == 0xff || buf[10] == 0x00 ) && buf[11] == 0x00 ) )
	{
		if( mt_register > 10000 || mt_register == 0 )
			ErrorPacket( buf , ERROR_REGISTER );
		if( !( ( buf[10] == 0xff || buf[10] == 0x00 ) && buf[11] == 0x00 ) )
			ErrorPacket( buf , ERROR_DATA );
		return FALSE;
	}
	BYTE byBusNo;
	WORD byStn;
	BYTE byStatus;
	WORD wDevAddr;
	WORD wPnt;

	switch( buf[10] )
	{
	case 0xff:
		byStatus = 1;
		break;
	case 0x00:
		byStatus = 0;
		break;
	}

	byStn = m_pDOMapTab[mt_register-1].wStn - 1 ;
	wPnt  = m_pDOMapTab[mt_register-1].wPntNum - 1 ;
	if( m_pDOMapTab[mt_register-1].wStn > 0 && m_pDOMapTab[mt_register-1].wPntNum > 0 )
	{
		yk_bufSerial[0]=buf[0];
		yk_bufSerial[1]=buf[1];
		Yk_FunNum = buf[7];
		if(m_pMethod->GetBusLineAndAddr(byStn, byBusNo, wDevAddr))
		{
			m_pMethod->SetYkSel(this, byBusNo, wDevAddr, wPnt, byStatus);
			m_pMethod->SetYkExe(this, byBusNo, wDevAddr, wPnt, byStatus);
		}
		m_wRelayNum = mt_register;
		return TRUE;
	}
	else
	{
		ErrorPacket( buf , ERROR_REGISTER );
		return FALSE;
	}
}/*}}}*/

BOOL ModBusTcp::YKPacket( int yk_register , int val )
{/*{{{*/

	WORD index = 0;
	WORD ModBusTcpByteLen = 6;

	if( yk_register > 10000 || yk_register == 0 )
	{
		ErrorPacket( yk_bufSerial , ERROR_REGISTER );
		return FALSE;
	}
	MsgBuf[index++] = yk_bufSerial[0];
	MsgBuf[index++] = yk_bufSerial[1];
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = ModBusTcpByteLen >> 8;
	MsgBuf[index++] = ModBusTcpByteLen;		//ModBusTcp�����ֽ�
	MsgBuf[index++] = m_wRtuAddr;
	//	MsgBuf[index++] = FunNum;
	MsgBuf[index++] = Yk_FunNum;

	MsgBuf[index++] = yk_register>>8;
	MsgBuf[index++] = yk_register;		//�Ĵ�����ַ

	if( val==1 )
		MsgBuf[index++] = 0xff;		//	��
	else if( val==0 )
		MsgBuf[index++] = 0x00;
	else
	{
		ErrorPacket( yk_bufSerial , ERROR_DATA );
		return TRUE;
	}

	MsgBuf[index++] = 0x00;

	MsgLen = index;
	return TRUE;
}/*}}}*/

BOOL ModBusTcp::ErrorPacket(BYTE * buf,BYTE errorflag)
{/*{{{*/
	MsgFlag = 1;

	BYTE index = 0;

	MsgBuf[index++] = buf[0];
	MsgBuf[index++] = buf[1];
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 0;
	MsgBuf[index++] = 3;		//ModBusTcp�����ֽ�
	MsgBuf[index++] = m_wRtuAddr;
	MsgBuf[index++] = FunNum | 0x80;;
	MsgBuf[index++] = errorflag;

	MsgLen = index;
	return TRUE;
}/*}}}*/

BOOL ModBusTcp::WriteAIVal(WORD wSerialNo ,WORD wPnt, float wVal)
{/*{{{*/
	if(m_pwAITrans==NULL) return FALSE;
	WORD wNum = m_pwAITrans[wPnt];
	if(wNum>m_wAISum) return FALSE;
	if(wNum<MODBUSTCPMAX_AI_LEN)
	{
		int nDelt = wVal - m_wYCBuf[wNum];
		if(abs(nDelt)>=m_wDeadVal)
		{
			m_wYCBuf[wNum] = wVal;
			AddAnalogEvt( wSerialNo , wNum , wVal );
		}
	}
	return TRUE ;
}/*}}}*/

BOOL ModBusTcp::WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if(wNum>m_wDISum) return FALSE;
	if( wNum<MODBUSTCPMAX_DI_LEN)
	{
		if( m_byYXbuf[ wNum ] != wVal )
		{
			m_byYXbuf[ wNum ] = wVal ;

			AddDigitalEvt( wSerialNo , wNum , wVal );
		}
	}
	return TRUE ;
}/*}}}*/

BOOL ModBusTcp::WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal)
{/*{{{*/
	if(m_pwPITrans==NULL) return FALSE;
	WORD wNum = m_pwPITrans[wPnt];
	if(wNum>m_wPISum) return FALSE;
	if(wNum<MODBUSTCPMAX_PI_LEN)
	{
		m_dwYMBuf[wNum] = dwVal;
	}
	return TRUE ;
}/*}}}*/

BOOL ModBusTcp::WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if(wNum>=m_wDISum) return FALSE;
	if(wNum<MODBUSTCPMAX_DI_LEN)
	{
		AddSOEInfo(wSerialNo , wNum, wVal, lTime, wMiSecond);
	}
	return TRUE ;
}/*}}}*/

void ModBusTcp::TimerProc()
{/*{{{*/
	//���ڴ��ж�ȡ�仯ң�ź�ң������
	ReadChangData();
}/*}}}*/

BOOL ModBusTcp::GetDevCommState( )
{/*{{{*/
	if(m_byPortStatus == COMSTATUS_ONLINE)
	{
		return COM_DEV_NORMAL ;
	}
	else
	{
		return COM_DEV_ABNORMAL ;
	}
}/*}}}*/


 //��ȡ�ɼ�������ÿ�����ߺ�
BOOL ModBusTcp::InitDevState()
{/*{{{*/
	if (m_pMethod == NULL)
		return FALSE;

	BYTE size = MAX_LINE;
	WORD wAddr = 0;
	BYTE byIndex = 0;
	for (BYTE i = 0; i < size; i++)
	{
		wAddr = 0;
		byIndex = 0;
		for( int m = 0 ; m < 256 ; m++ )
		{
            if (m_pMethod->GetSingleGatherDevCount(i, byIndex++, &wAddr))
            {
                PMBS_BUSDEV pState = new MBS_BUSDEV;
                pState->busNo = i;
                pState->wAddr = wAddr;
                m_busDevStateArray.push_back(pState);
            }
        }
	}

	return TRUE;
}/*}}}*/



BOOL ModBusTcp::GetDevState(WORD wYxNo, BYTE *byVal)
{/*{{{*/
	CMethod * pMethod = m_pMethod;
	if (pMethod == NULL)
		return FALSE;

	WORD wCount = pMethod->GetGatherDevCount();
	if (wYxNo >= wCount)
		return FALSE;

	int size = m_busDevStateArray.size();
	if (size != wCount)
		return FALSE;

	PMBS_BUSDEV pBusState = m_busDevStateArray[wYxNo];
	if (pBusState == NULL)
		return FALSE;

	*byVal = m_pMethod->GetDevCommState(pBusState->busNo, pBusState->wAddr);
	if (*byVal == COM_DEV_ABNORMAL)
		*byVal = FALSE;
	else
		*byVal = TRUE;

	return TRUE;
}/*}}}*/




