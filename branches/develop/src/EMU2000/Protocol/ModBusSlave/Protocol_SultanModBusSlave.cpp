// SultanModBusSlave.cpp: implementation of the CSultanModBusSlave class.
//
//////////////////////////////////////////////////////////////////////

#include "Protocol_SultanModBusSlave.h"
#define ERR_FUNC			0x01
#define ERR_ADDR			0x02
#define ERR_DATA			0x03

#define MBS_YK_COUNT  10000

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProtocol_SultanModBusSlave::CProtocol_SultanModBusSlave()
{

}

CProtocol_SultanModBusSlave::~CProtocol_SultanModBusSlave()
{

}

BOOL CProtocol_SultanModBusSlave::ykProcess(BYTE byAddr, BYTE *pBuf, WORD len)
{
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
	
	if( wOrder == 0 ) //ֻ����0xFF00
		return FALSE ; 

	WORD wStn = m_pDOMapTab[ wYkNo].wStn ;
    WORD wPnt  = m_pDOMapTab[ wYkNo].wPntNum ;
	if( wStn == 0 || wPnt == 0 )
		return FALSE ;

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
	default:
		SetErrorMsg( 0x05,ERR_DATA );
		return FALSE;
	}

	wStn = wStn - 1 ;
	wPnt = wPnt - 1 ;

	BYTE byBusNo = 0 ;
	WORD wDevAddr = 0 ;
	m_wYkAddr = wRegAddr ;
	m_wYkNum = wOrder ;

	if(m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkExe(this, byBusNo, wDevAddr, wPnt , byYkAction );
	else
		printf("[ModBusSlave]:Serialno error!!!\n");
	
	return TRUE ;
}

BOOL CProtocol_SultanModBusSlave::ykMessage( BYTE byAddr , BYTE * pbuf , int &len , PBUSMSG pBusMsg )
{
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
}


