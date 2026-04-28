// Protocol_ESD_ModBusSlave.cpp: implementation of the CProtocol_ESD_ModBusSlave class.
//
//////////////////////////////////////////////////////////////////////

#include "Protocol_ESD_ModBusSlave.h"
#include "../../BayLayer/main.h"

#define MAXLINE		50
#define  MODBUSSLAVE_PATH  "/mynand/config/ModBusSlave/"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define ERR_FUNC			0x01
#define ERR_ADDR			0x02
#define ERR_DATA			0x03

#define MBS_YX_COUNT  20001
#define MBS_YC_COUNT  36801
#define MBS_YK_COUNT  10000
#define MBS_YM_COUNT 37825
#define MBS_YX_DEVSTATE_COUNT	 21001
#define MBS_BUS_RSADDR	 50000  //���߽��շ���֡���Ĵ�����ַ

#define CLOSE_STATE		1

#define SETYX			 0x0001
#define SETYC			 0x0002
#define SETYM			0x0004
#define SETYK			 0x0008
#define SET_DEVSTATE_YX	 0x0010 //����װ��״̬ң��
#define SET_BUS_COUNT	0x0020  //����װ��ͨѶ����/���ͼ���
#define SETADDR		 0x0100
#define SETDATA		 0x0200
#define  SETFUC		   0x0400

enum { YC , YX , YM , YK };

CProtocol_ESD_ModBusSlave::CProtocol_ESD_ModBusSlave()
{
    m_busDevStateArray.reserve( 300 );
}

CProtocol_ESD_ModBusSlave::~CProtocol_ESD_ModBusSlave()
{/*{{{*/
	int size = m_busDevStateArray.size() ;
   for( int  i = 0 ; i < size ; i++ )
   {
	   PMBS_BUSDEV pState = m_busDevStateArray[ i ] ;
	   if( pState )
		   delete pState ;
   }
   m_busDevStateArray.clear() ;
}/*}}}*/

int CProtocol_ESD_ModBusSlave::GetRealVal(BYTE byType, WORD wPnt, void *v)
{/*{{{*/
    BYTE  byValue = 0 ;
    switch(byType)
    {
    case YC:
        {
			if( wPnt >= MSMAX_YC_LEN )
			return FALSE ;

			float fVal = 0.0f ;
			if( m_pAIMapTab[ wPnt ].wStn>0 && m_pAIMapTab[ wPnt ].wPntNum>0 )
				fVal = CalcAIRipeVal(m_pAIMapTab[ wPnt ].wStn, m_pAIMapTab[ wPnt ].wPntNum, m_wYcBuf[ wPnt ] );
			else
				return FALSE ;

			memcpy(v, &fVal , sizeof(float));
		}
        break;
    case YX:
        {
			if( wPnt >= MSMAX_YX_LEN )
				return FALSE ;

			if( m_pDIMapTab[ wPnt ].wStn>0 && m_pDIMapTab[ wPnt ].wPntNum>0 )
			{
				if( m_byYxBuf[ wPnt ] ==0 )
					byValue = 0;
				else
					byValue = 1;

				memcpy(v, &byValue, sizeof(BYTE));
			}
			else
				return FALSE ;
		}
        break;
    case YM:
        {
			if( wPnt >= MSMAX_YM_LEN )
			return FALSE ;

			WORD wStn = m_pPIMapTab[ wPnt ].wStn  ;
			WORD wPntNum = m_pPIMapTab[ wPnt ].wPntNum ;
			QWORD dwVal = 0 ;
			if( wStn >0 && wPntNum >0 )
				m_pMethod->GetYmData( wStn - 1 , wPntNum - 1 , dwVal );
			else
				return FALSE ;

			memcpy(v, &dwVal , sizeof(QWORD));
		}
        break;
    default:
        return FALSE ;
    }
    return TRUE ;
}/*}}}*/

void CProtocol_ESD_ModBusSlave::TimerProc()
{/*{{{*/
	ReadChangData();
	return ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::WriteAIVal(WORD wSerialNo, WORD wPnt, float fVal )
{/*{{{*/
    if(m_pwAITrans==NULL) return FALSE;
    WORD wNum = m_pwAITrans[wPnt];
    if(wNum>m_wAISum) return FALSE;
    if(wNum<MSMAX_YC_LEN)
    {
        float nDelt = fVal - m_wYcBuf[wNum];
        if(abs(( int )nDelt)>=m_wDeadVal)
        {
            m_wYcBuf[wNum] = fVal;
        }
    }
    return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::WriteDIVal(WORD wSerialNo, WORD wPnt, WORD wVal)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>m_wDISum) return FALSE;
    if( wNum<MSMAX_YX_LEN)
    {
        if( m_byYxBuf[ wNum ] != wVal )
        {
            m_byYxBuf[ wNum ] = wVal ;
        }
    }
    return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::Init( BYTE byLineNo )
{/*{{{*/
	m_byLineNo = byLineNo ;
	sprintf( m_szObjName, "%s", m_sDevName );
	m_wRtuAddr   = m_wDevAddr ;
	char szFileName[256] = "";

	sprintf( szFileName, "%s%s", MODBUSSLAVE_PATH, m_sTemplatePath );
	//��ȡ��Ҫת�������ݵ���ģ��
    ReadMapConfig( szFileName );

	//��ʼ����ģ��
	InitRtuBase() ;
	return TRUE ;
}/*}}}*/

//��ȡ�ɼ�������ÿ�����ߺ�
BOOL CProtocol_ESD_ModBusSlave::InitDevState( )
{/*{{{*/
	if( m_pMethod == NULL )
		return FALSE ;

	BYTE size = MAX_LINE;
	WORD wAddr = 0 ;
	BYTE byIndex = 0 ;
	for ( BYTE i = 0 ; i < size ; i++ )
	{
		wAddr = 0 ;
		byIndex = 0 ;
		if( m_pMethod->GetSingleGatherDevCount( i , byIndex++ , &wAddr ) )
		{
			PMBS_BUSDEV pState = new MBS_BUSDEV ;
			pState->busNo = i ;
			pState->wAddr = wAddr ;
			m_busDevStateArray.push_back( pState ) ;
		}
	}

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::InitRtuBase()
{/*{{{*/
    BOOL bOk = FALSE;
	//��ȡת�����
    CreateTransTab();

	//���ڴ����ݿ���--��ȡת����Ĭ������
	m_pMethod->ReadAllYcData(&m_wYcBuf[0]);
	m_pMethod->ReadAllYxData( &m_byYxBuf[ 0 ] ) ;

    return bOk;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::GetProtocolBuf( BYTE * pBuf , int &len , PBUSMSG pBusMsg  )
{/*{{{*/
	if( !IsCanSend( ) )
		return FALSE ;

	BOOL bVal = FALSE ;
	DWORD dwFlag = 0 ;

	if( GetFlag( SETYK ) && pBusMsg )
	{
		bVal = ykMessage( m_wDevAddr , pBuf , len , pBusMsg ) ;
		dwFlag = SETYK ;
	}
	else if( GetFlag( SETYC ) )
	{
		bVal = ycMessage( m_wDevAddr , pBuf , len ) ;
		dwFlag = SETYC;
	}
	else if( GetFlag( SETYX ) )
	{
		bVal = yxMessage( m_wDevAddr , pBuf , len ) ;
		dwFlag = SETYX;
	}
	else if( GetFlag( SETYM ) )
	{
		
		bVal = ymMessage( m_wDevAddr , pBuf , len ) ;
		dwFlag = SETYM ;
	}
	else if( GetFlag( SETFUC ) )
	{
		bVal = ErrMessage( m_wDevAddr , m_FuncError , ERR_FUNC , pBuf , len );
		dwFlag= SETFUC ;
	}
	else if( GetFlag( SETADDR ) )
	{
		
		bVal = ErrMessage( m_wDevAddr , m_FuncError , ERR_ADDR , pBuf , len );
		dwFlag = SETADDR ;
	}
	else if( GetFlag( SETDATA ) )
	{
		
		bVal = ErrMessage( m_wDevAddr , m_FuncError , ERR_DATA , pBuf , len );
		dwFlag = SETDATA ;
	}
	else if( GetFlag( SET_DEVSTATE_YX ) )
	{
		bVal = yxDevState( m_wDevAddr , pBuf , len ) ;
		dwFlag = SET_DEVSTATE_YX ;
	}
	else if( GetFlag(SET_BUS_COUNT ) )
	{
		bVal = busRSMessage( m_wDevAddr , pBuf , len ) ;
		dwFlag = SET_BUS_COUNT ;
	}
	else
		return FALSE ;

	SetFlag( dwFlag , FALSE ) ;

	WORD wCrc = GetCRC( pBuf , len ) ;
	pBuf[ len++ ] = LOBYTE( wCrc ) ;
	pBuf[ len++ ] = HIBYTE( wCrc ) ;

	return bVal ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::GetDevState( WORD wYxNo , BYTE *byVal )
{/*{{{*/
	CMethod * pMethod = m_pMethod ;
	if( pMethod == NULL )
		return FALSE ;

	WORD wCount = pMethod->GetGatherDevCount() ;
	if( wYxNo >= wCount )
		return FALSE ;

	int size = m_busDevStateArray.size() ;
	if( size != wCount )
		return FALSE ;

	PMBS_BUSDEV pBusState = m_busDevStateArray[ wYxNo ] ;
	if( pBusState == NULL )
		return FALSE ;

     *byVal = m_pMethod->GetDevCommState( pBusState->busNo , pBusState->wAddr ) ;
	if( *byVal  == COM_DEV_ABNORMAL )
		*byVal  = FALSE ;
	else
		*byVal  = TRUE ;

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::yxDevState( BYTE byAddr , BYTE * buf , int &len )
{/*{{{*/
	if( buf == NULL  )
		return FALSE ;

	int nByte = 0 , i = 0 ;
	WORD wYxNo = 0 ;
	WORD wRegNum = m_wRegNum ;
	WORD wRegAddr = m_wRegAddr ;
	BYTE byData = 0 ;

	buf[nByte++]= byAddr ;
	buf[nByte++]= m_FucCode ;
	buf[nByte++]=0x00;

	wYxNo = wRegAddr - 20001 ;
	BYTE byVal = 0 ;
	while( wRegNum > 8 )
	{
		byData=0;
		for(i=0;i<8;i++)
		{
			byVal = 0 ;
			if( !GetDevState( wYxNo++ , &byVal ) )
			{
				ErrMessage( byAddr , 0x02 , ERR_ADDR , buf , len ) ;
				return TRUE;
			}

			if( byVal == CLOSE_STATE)
				byData |=( 1 << i );
		}

		buf[nByte++]=byData;
		wRegNum-=8;
	}

	byData=0;
	byVal = 0 ;
	for(i=0;i<wRegNum;i++)
	{
		if( !GetDevState( wYxNo++ , &byVal ) )
		{
			ErrMessage( byAddr , 0x02 , ERR_ADDR , buf , len ) ;
			return TRUE;
		}

		if( byVal == CLOSE_STATE)
			byData |=( 1 << i);
	}

	buf[nByte++]=byData;
	buf[2]=nByte - 3 ;

	len = nByte ;
	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ProcessProtocolBuf( BYTE * pBuf , int len )
{/*{{{*/
	if( len < 5 )
		return FALSE ;

	BYTE * pt = &pBuf[0];
	WORD crc = GetCRC( pt, len-2);/*����У����*/
	if( !( LOBYTE(crc) == *(pt+len-2) && HIBYTE(crc) == *(pt+len-1)) )/*У�����ж��Ƿ���ȷ*/
		return FALSE ;

	BYTE byFuncCode;
	if( pBuf[ 0 ] != m_wDevAddr )
		return FALSE ;

	byFuncCode = pBuf[ 1 ];

	BOOL bVal  = FALSE ;
	DWORD dwFlag = 0 ;
	switch( byFuncCode )
	{
	case 0x02:
		  bVal = yxDataProcess( m_wDevAddr , pBuf , len );
		break;
	case 0x03:
	case 0x04:
		bVal = dataProcess( m_wDevAddr , pBuf , len ) ;
		break;
	case 0x05:
		bVal = ykProcess( m_wDevAddr , pBuf , len );
		dwFlag = SETYK ;
		break;
	case 0xFF:
		bVal = SetErrorMsg( byFuncCode , ERR_DATA ) ;
		dwFlag = SETDATA ;
	default:
		{
			bVal = SetErrorMsg( byFuncCode , ERR_FUNC ) ;
			dwFlag = SETFUC ;
		}
		break;
	}

	if( bVal )
		SetFlag( dwFlag ) ;

	return TRUE;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::yxDataProcess( BYTE byAddr , BYTE *pBuf , WORD len )
{/*{{{*/
	WORD wRegAddr;
	//WORD wRegNum;

	if(len<8)
		return FALSE;

	wRegAddr=MAKEWORD(pBuf[3],pBuf[2]);
	//wRegNum=MAKEWORD(pBuf[5],pBuf[4]);

	BOOL bFlag = FALSE ;
	if( wRegAddr >= 10001 && wRegAddr < MBS_YX_COUNT )
	{
		bFlag = yxProcess( byAddr , pBuf , len );
		if( bFlag )
			SetFlag( SETYX ) ;

		return bFlag ;
	}

	else if( wRegAddr >= MBS_YX_COUNT && wRegAddr <= MBS_YX_DEVSTATE_COUNT )
	{
		bFlag = yxDevStateProcess( byAddr , pBuf , len ) ;
		if( bFlag )
			SetFlag( SET_DEVSTATE_YX ) ;
	}
	else
		return FALSE ;

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::yxDevStateProcess( BYTE byAddr ,BYTE *pBuf , WORD len )
{/*{{{*/
	WORD wRegAddr;
	WORD wRegNum;
	WORD wYxNo = 0 ;
	BYTE *pData=NULL;
	pData=pBuf+2;
	if( len < 8 )
		return FALSE;

	int wYxTotalCount = m_pMethod->GetGatherDevCount() ;

	wRegAddr=MAKEWORD(pData[1],pData[0]);
	wRegNum=MAKEWORD(pData[3],pData[2]);

	wYxNo = wRegAddr - 20001 ;

	if( wRegAddr < 20001 || wRegAddr >= MBS_YX_DEVSTATE_COUNT )
	{
		SetErrorMsg( 0x02 , ERR_ADDR ) ;
		return FALSE ;
	}

	if( wRegNum<=0 || wRegNum > 1024 )
	{
		SetErrorMsg( 0x02 , ERR_DATA ) ;
		return FALSE ;
	}
	else if( ( wYxNo + wRegNum ) > ( WORD )wYxTotalCount )
	{
		SetErrorMsg( 0x02 , ERR_ADDR ) ;
		return FALSE ;
	}
	else
	{
		m_wRegAddr = wRegAddr ;
		m_wRegNum = wRegNum ;
		m_FucCode = pBuf[ 1 ] ;
	}

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::dataProcess( BYTE byAddr, BYTE *pBuf, WORD len )
{/*{{{*/
	WORD wRegAddr;
	//WORD wRegNum;

	if(len<8)
		return FALSE;

	wRegAddr=MAKEWORD(pBuf[3],pBuf[2]);
	//wRegNum=MAKEWORD(pBuf[5],pBuf[4]);

	BOOL bFlag = FALSE ;
	if( wRegAddr >= 30001 && wRegAddr < MBS_YC_COUNT )
	{
		bFlag = ycProcess( byAddr , pBuf , len );
		if( bFlag )
			SetFlag( SETYC ) ;

		return bFlag ;
	}

	else if( wRegAddr >= 36801 && wRegAddr <= MBS_YM_COUNT )
	{
		bFlag = ymProcess( byAddr , pBuf , len ) ;
		if( bFlag )
			SetFlag( SETYM ) ;
	}

	else if( wRegAddr >= MBS_BUS_RSADDR && wRegAddr <= MBS_BUS_RSADDR + MAXLINE * 2 )
	{
		bFlag = busRSProcess( byAddr , pBuf , len ) ;
		if( bFlag )
			SetFlag( SET_BUS_COUNT ) ;
	}
	else
		return FALSE ;

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::busRSMessage( BYTE byAddr , BYTE * pbuf , int &len )
{/*{{{*/
	if( pbuf == NULL )
		return FALSE ;

	int nByte=0;
	WORD wYcNo = 0 ;
	WORD wRegNum = m_wRegNum ;

	BYTE *buf = pbuf ;

	buf[nByte++]=byAddr;
	buf[nByte++]=m_FucCode ;
	buf[nByte++]=0x00;

	wYcNo = m_wRegAddr - MBS_BUS_RSADDR ;
	WORD wRx = 0 , wTx = 0 ;
	while( wRegNum > 0 )
	{
		PBUSMANAGER pBus = m_pMethod->GetBus( wYcNo++ ) ;
		if( !pBus )
		{
			ErrMessage( byAddr , m_FucCode , ERR_ADDR , buf , len ) ;
			return TRUE;
		}

		wRx = pBus->m_Rx ;
		wTx = pBus->m_Tx ;

		buf[ nByte++ ]=HIBYTE( wRx );
		buf[ nByte++ ]=LOBYTE( wRx );
		buf[ nByte++ ] = HIBYTE( wTx ) ;
		buf[ nByte++ ] = LOBYTE( wTx ) ;
		wRegNum -= 2 ;
	}

	buf[2]=nByte - 3;

	len = nByte ;
	return TRUE ;
}/*}}}*/

WORD CProtocol_ESD_ModBusSlave::GetCRC(BYTE *pBuf, WORD nLen)
{
	WORD Genpoly = 0xA001;
	WORD CRC = 0xFFFF;
	WORD index;
	while (nLen--)
	{
		CRC = CRC ^ (WORD)*pBuf++;
		for (index = 0; index < 8; index++)
		{
			if ((CRC & 0x0001) == 1)
				CRC = (CRC >> 1) ^ Genpoly;
			else
				CRC = CRC >> 1;
		}
	}
	return CRC;
}

BOOL CProtocol_ESD_ModBusSlave::busRSProcess( BYTE byAddr , BYTE * pBuf , WORD len )
{/*{{{*/
	WORD wRegAddr;
	WORD wRegNum;
	WORD wNo;
	if( len < 8 )
		return FALSE;

	BYTE *pData=NULL;
	pData=pBuf+2;
	BYTE byFunc = pBuf[ 1 ] ;
	int wBusTotalCount = MAXLINE * 2 ;

	wRegAddr=MAKEWORD(pData[1],pData[0]);
	wRegNum=MAKEWORD(pData[3],pData[2]);

	wNo=wRegAddr-MBS_BUS_RSADDR;
	WORD wMaxLen = MBS_BUS_RSADDR + MAXLINE * 2;
	if( ( wRegAddr < MBS_BUS_RSADDR ) ||  ( wRegAddr > wMaxLen) || ( wRegAddr % 2 ) )
	{
		SetErrorMsg( byFunc , ERR_ADDR ) ;
		return FALSE ;
	}
	else if(wRegNum<=0 || wRegNum> wBusTotalCount || ( wRegNum % 2 != 0 ) )
	{
		SetErrorMsg( byFunc , ERR_DATA ) ;
		return FALSE ;
	}
	else if(wNo+wRegNum> MAXLINE * 2 )
	{
		SetErrorMsg( byFunc , ERR_DATA ) ;
		return FALSE ;
	}
	else
	{
		m_wRegAddr = wRegAddr ;
		m_wRegNum = wRegNum ;
		m_FucCode = pBuf[ 1 ] ;

	}

		return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ymMessage( BYTE byAddr , BYTE * pbuf , int &len )
{/*{{{*/
	if( pbuf == NULL )
		return FALSE ;

	QWORD dwData;
	int nByte=0;
	WORD wYcNo = 0 ;
	WORD wRegNum = m_wRegNum ;

	BYTE *buf = pbuf ;

	buf[nByte++]=byAddr;
	buf[nByte++]=m_FucCode ;
	buf[nByte++]=0x00;

	wYcNo = (m_wRegAddr - 36801)/2 ;
	WORD hVal = 0 ,lVal = 0 ;
	while( wRegNum > 0 )
	{
		if( !GetRealVal( YM , wYcNo++ , &dwData ) )
		{
			ErrMessage( byAddr , m_FucCode , ERR_ADDR , buf , len ) ;
			return TRUE;
		}
		hVal = HIWORD( (DWORD)dwData ) ;
		lVal = LOWORD( (DWORD)dwData ) ;
		buf[ nByte++ ] = HIBYTE( hVal ) ;
		buf[ nByte++ ] = LOBYTE( hVal ) ;
		buf[ nByte++ ]=HIBYTE( lVal );
		buf[ nByte++ ]=LOBYTE( lVal );

		wRegNum -= 2 ;
	}

	buf[2]=nByte - 3;

	len = nByte ;
	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ymProcess( BYTE byAddr, BYTE *pBuf, WORD len )
{/*{{{*/
	WORD wRegAddr;
	WORD wRegNum;
	WORD wYmNo;
	if( len < 8 )
		return FALSE;

	BYTE *pData=NULL;
	pData=pBuf+2;
	BYTE byFunc = pBuf[ 1 ] ;
	//int wYmTotalCount = GetPntSum( YM ) ;
	int wYmTotalCount = MSMAX_YM_LEN ;

	wRegAddr=MAKEWORD(pData[1],pData[0]);
	wRegNum=MAKEWORD(pData[3],pData[2]);

	wYmNo=wRegAddr-36801;
	if( ( wRegAddr < 36801 ) || ( wRegAddr > MBS_YM_COUNT ) || ( wRegAddr % 2 == 0 ) )
	{
		SetErrorMsg( 0x02 , ERR_ADDR ) ;
		return FALSE ;
	}
	else if(wRegNum<=0 || wRegNum>127 || ( wRegNum % 2 != 0 ) )
	{
		SetErrorMsg( byFunc , ERR_DATA ) ;
		return FALSE ;
	}
	else if(wYmNo+wRegNum> wYmTotalCount )
	{
		SetErrorMsg( byFunc , ERR_DATA ) ;
		return FALSE ;
	}
	else
	{
		m_wRegAddr = wRegAddr ;
		m_wRegNum = wRegNum ;
		m_FucCode = pBuf[ 1 ] ;
	}

		return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::yxProcess( BYTE byAddr, BYTE *pBuf, WORD len )
{/*{{{*/
	WORD wRegAddr;
	WORD wRegNum;
	WORD wYxNo = 0 ;
	BYTE *pData=NULL;
	pData=pBuf+2;
	if( len < 8 )
		return FALSE;

	//int wYxTotalCount = GetPntSum( YX ) ;
	int wYxTotalCount = MSMAX_YX_LEN ;

	wRegAddr=MAKEWORD(pData[1],pData[0]);
	wRegNum=MAKEWORD(pData[3],pData[2]);

	wYxNo = wRegAddr - 10001 ;

	if( wRegAddr < 10001 || wRegAddr >= MBS_YX_COUNT )
	{
		SetErrorMsg( 0x02 , ERR_ADDR ) ;
		return FALSE ;
	}

	if( wRegNum<=0 || wRegNum > 1024 )
	{
		SetErrorMsg( 0x02 , ERR_DATA ) ;
		return FALSE ;
	}
	else if( ( wYxNo + wRegNum ) > ( WORD )wYxTotalCount )
	{
		SetErrorMsg( 0x02 , ERR_ADDR ) ;
		return FALSE ;
	}
	else
	{
		m_wRegAddr = wRegAddr ;
		m_wRegNum = wRegNum ;
		m_FucCode = pBuf[ 1 ] ;
	}

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ycProcess( BYTE byAddr, BYTE *pBuf, WORD len )
{/*{{{*/
	WORD wRegAddr;
	WORD wRegNum;
	WORD wYcNo;
	if( len < 8 )
		return FALSE;

	BYTE *pData=NULL;
	pData=pBuf+2;
	BYTE byFunc = pBuf[ 1 ] ;
	//int wYcTotalCount = GetPntSum( YC ) ;
	int wYcTotalCount = MSMAX_YC_LEN ;

	wRegAddr=MAKEWORD(pData[1],pData[0]);
	wRegNum=MAKEWORD(pData[3],pData[2]);
	wYcNo=wRegAddr-30001;

	if( wRegAddr < 30001 || wRegAddr >= MBS_YC_COUNT )
	{
		SetErrorMsg( byFunc , ERR_ADDR ) ;
		return FALSE ;
	}

	if(wRegNum<=0 || wRegNum>120 )
	{
		SetErrorMsg( byFunc , ERR_DATA ) ;
		return FALSE ;
	}
	else if( ( wYcNo + wRegNum ) >wYcTotalCount )
	{
		SetErrorMsg( byFunc , ERR_ADDR ) ;
		return FALSE ;
	}
	else
	{
		m_wRegAddr = wRegAddr ;
		m_wRegNum = wRegNum ;
		m_FucCode = byFunc ;
	}

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ykProcess(BYTE byAddr, BYTE *pBuf, WORD len)
{/*{{{*/
	WORD wRegAddr;
	WORD wOrder;
	WORD wYkNo;
	BYTE byYkAction;

	if( len < 8 )
		return FALSE;

	int wYkTotalCount = MSMAX_YK_LEN ;

	BYTE *pData=NULL;
	pData=pBuf+2;
	wRegAddr=MAKEWORD(pData[1],pData[0]);
	wOrder=MAKEWORD(pData[3],pData[2]);
	wYkNo=wRegAddr-1;

	if( wRegAddr < 1 || wRegAddr >= MBS_YK_COUNT )
	{
		SetErrorMsg( 0x05,ERR_ADDR );
		return FALSE;
	}

	if( wYkNo >= wYkTotalCount )
		return FALSE ;

	switch(wOrder)
	{
	case 0xFF00:
		byYkAction= 1;
		break;
	case 0x0000:
		byYkAction= 0;
		break;
	default:
		SetErrorMsg( 0x05,ERR_DATA );
		return FALSE;
	}

	m_wYkAddr = wRegAddr ;
	m_wYkNum = wOrder ;

	WORD wStn = m_pDOMapTab[ wYkNo].wStn - 1 ;
    WORD wPnt  = m_pDOMapTab[ wYkNo].wPntNum - 1 ;
	BYTE byBusNo = 0 ;
	WORD wDevAddr = 0 ;

	if(m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkExe(this, byBusNo, wDevAddr, wPnt , byYkAction );
	else
		printf("[ModBusSlave]:Serialno error!!!\n");

	return TRUE;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ErrMessage( BYTE byAddr, BYTE byFuncCode, BYTE byErrCode, BYTE * pBuf , int &len )
{/*{{{*/
	if( pBuf == NULL  )
		return FALSE ;

	int nByte = 0 ;
	BYTE * buf = pBuf ;

	buf[nByte++]= byAddr ;
	buf[nByte++]= byFuncCode | 0x80 ;
	buf[nByte++]=byErrCode ;

	len = nByte ;

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::yxMessage( BYTE byAddr , BYTE * buf , int &len )
{/*{{{*/
	if( buf == NULL  )
		return FALSE ;

	int nByte = 0 , i = 0 ;
	WORD wYxNo = 0 ;
	WORD wRegNum = m_wRegNum ;
	WORD wRegAddr = m_wRegAddr ;
	BYTE byData = 0 ;

	buf[nByte++]= byAddr ;
	buf[nByte++]= m_FucCode ;
	buf[nByte++]=0x00;

	wYxNo = wRegAddr - 10001 ;
	BYTE byVal = 0 ;
	while( wRegNum > 8 )
	{
		byData=0;
		byVal = 0 ;
		for(i=0;i<8;i++)
		{
			if( !GetRealVal( YX , wYxNo++, &byVal ) )
			{
				ErrMessage( byAddr , 0x02 , ERR_ADDR , buf , len ) ;
				return TRUE;
			}

			if( byVal == CLOSE_STATE)
				byData |=( 1 << i );
		}

		buf[nByte++]=byData;
		wRegNum-=8;
	}

	byData=0;
	byVal = 0 ;
	for(i=0;i<wRegNum;i++)
	{
		if( !GetRealVal( YX , wYxNo++, &byVal ) )
		{
			ErrMessage( byAddr , 0x02 , ERR_ADDR , buf , len ) ;
			return TRUE;
		}

		if( byVal == CLOSE_STATE)
			byData |=( 1 << i);
	}

	buf[nByte++]=byData;
	buf[2]=nByte - 3 ;

	len = nByte ;
	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ycMessage( BYTE byAddr , BYTE * pbuf , int &len )
{/*{{{*/
	if( pbuf == NULL )
		return FALSE ;

	WORD wYcData;
	float fltData;
	int nByte=0;
	WORD wYcNo = 0 ;
	WORD wRegNum = m_wRegNum ;

	BYTE *buf = pbuf ;

	buf[nByte++]=byAddr;
	buf[nByte++]=m_FucCode ;
	buf[nByte++]=0x00;

	wYcNo = m_wRegAddr - 30001 ;
	while( wRegNum > 0 )
	{
		if( !GetRealVal( YC , wYcNo++ , &fltData ) )
		{
			ErrMessage( byAddr , m_FucCode , ERR_ADDR , buf , len ) ;
			return TRUE;
		}

		if(fltData<0)
		{
			wYcData=(WORD)-fltData;
			wYcData= ~wYcData + 1 ;
		}
		else
			wYcData = ( WORD )fltData;

		buf[nByte++]=HIBYTE(wYcData);
		buf[nByte++]=LOBYTE(wYcData);
		wRegNum--;
	}

	buf[2]=nByte - 3;

	len = nByte ;

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::SetErrorMsg( BYTE byFuncCode, BYTE byErrCode )
{/*{{{*/
	switch( byErrCode )
	{
	case ERR_ADDR:
		SetFlag( SETADDR ) ;
		break;

	case ERR_DATA:
		SetFlag( SETDATA );
		break;

	case ERR_FUNC:
		SetFlag( SETFUC ) ;
		break;
	}

	m_FuncError = byFuncCode ;

	return TRUE ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::ykMessage( BYTE byAddr , BYTE * pbuf , int &len , PBUSMSG pBusMsg )
{/*{{{*/
	if( pBusMsg == NULL )
		return FALSE ;

	if( pBusMsg->byMsgType != YK_PROTO )
		return FALSE ;

	if( pBusMsg->dwDataType != YK_EXCT_RTN )
		return FALSE ;

	if( ( pBusMsg->DataNum != 1 ) || ( pBusMsg->DataLen != sizeof(YK_DATA) ) )
	{
		printf("ModBusSlave Yk DataNum err\n");
		return -1;
	}

	YK_DATA *pData = (YK_DATA *)(pBusMsg->pData);

	BOOL bReturn = FALSE ;
	int nByte=0;
	WORD wRegNum = m_wYkNum ;
	WORD wRegAddr = m_wYkAddr ;
	BYTE *buf = pbuf ;

	if( pData->byVal == YK_ERROR )
		bReturn = FALSE ;
	else
		bReturn = TRUE ;

	buf[nByte++]=byAddr;
	if( bReturn )
		buf[nByte++]= 0x05 ;
	else
		buf[ nByte++ ] = 0x85 ;

	buf[nByte++]=HIBYTE( wRegAddr );
	buf[nByte++]=LOBYTE( wRegAddr );
	buf[nByte++]=HIBYTE( wRegNum );
	buf[nByte++]=LOBYTE( wRegNum );

	len = nByte ;
	return TRUE ;
}/*}}}*/

void CProtocol_ESD_ModBusSlave::SetFlag( DWORD dwVal , BOOL bFlag )
{/*{{{*/
	if( bFlag )
		m_dwSendFlag |= dwVal ;
	else
		m_dwSendFlag = m_dwSendFlag & ( ~dwVal ) ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::GetFlag( DWORD dwVal )
{/*{{{*/
	BOOL bFlag = ( m_dwSendFlag & dwVal  ) ?1:0 ;
	return bFlag ;
}/*}}}*/

BOOL CProtocol_ESD_ModBusSlave::IsCanSend( )
{/*{{{*/
	return m_dwSendFlag ? 1:0 ;
}/*}}}*/
