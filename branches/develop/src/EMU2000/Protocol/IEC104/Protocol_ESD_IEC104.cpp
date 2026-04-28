// Protocol_ESD_IEC104.cpp: implementation of the CProtocol_ESD_IEC104 class.
//
//////////////////////////////////////////////////////////////////////

#include "Protocol_ESD_IEC104.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProtocol_ESD_IEC104::CProtocol_ESD_IEC104()
{/*{{{*/
	byStaticLineNo = 0 ;
	byStaticDevAddr = 0 ;
}/*}}}*/

CProtocol_ESD_IEC104::~CProtocol_ESD_IEC104()
{}

/* 上报通讯装置状态 */
//总线编号从0开始计数
//装置编号从1开始计数

BOOL CProtocol_ESD_IEC104::ReSetState( )
{/*{{{*/
	CRtu104::ReSetState( ) ;
	byStaticLineNo = 0 ;
	byStaticDevAddr = 0 ;
	return TRUE ;
}/*}}}*/

#if 0		//noted by cyz!
int CProtocol_ESD_IEC104::ComState_Message( WORD wUnitAddr )
{/*{{{*/
	BYTE byLen = 0 , byCount = 0 ;
	WORD wNS = 0 , wNR = 0 ;
	BYTE *pTXBuf = m_pTX_Buf;
	GetSendRecvNo( wNS , wNR ) ;


	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 41;				//41 自定义类型标识TYP
	pTXBuf[8] = 3;				// 自发传送 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	byLen = 12;

	BYTE byBusNum = m_pMethod->GetToTalBusNum() ;
	if( byBusNum == 0 )
		return 0 ;

	if( byStaticLineNo >= byBusNum )
		byStaticLineNo = 0 ;

	for( ; byStaticLineNo< byBusNum ; )
	{
		//如果是转发协议不传输通讯状态
		BYTE byType = m_pMethod->GetBusLineProtocolType( byStaticLineNo ) ;
		if( ( PROTOCO_TRANSPROT == byType ) || ( byType == 0xFF ) )
		{
			byStaticLineNo++ ;
			continue ;
		}

		BYTE byTotalDevNum = m_pMethod->GetDevNum( byStaticLineNo ) ;
		if( byStaticDevAddr >= byTotalDevNum )
			byStaticDevAddr = 0 ;

		for( int i = 0 ; byStaticDevAddr < byTotalDevNum ; byStaticDevAddr++ )
		{
			if( i == 0 )
			{
				if( PackComStateMsg( pTXBuf , byLen , byStaticLineNo , 0 , SEND_BUS_STATE ) )
					byCount++ ;

				// printf( "\t byLen = %d , Line:%d , Count = %d \n" , byLen , byStaticLineNo , byCount ) ;
				if( byLen > 239 )
					break;
			}

			WORD wDevAddr =  m_pMethod->GetAddrByLineNoAndModuleNo( byStaticLineNo, (WORD)byStaticDevAddr );
			if( 0 != wDevAddr )
			{
				if( PackComStateMsg( pTXBuf , byLen , byStaticLineNo , wDevAddr , SEND_DEV_STATE ) )
					byCount++ ;
			}
			else
			{
				continue;
			}


			//printf( "\t byLen = %d , Line:%d , DevAddr:%d , Count:%d \n" , byLen , byStaticLineNo , wDevAddr, byCount ) ;

			if( byLen > 239 )
			{
				if( byStaticDevAddr >= byTotalDevNum )
					byStaticLineNo++  ;
				break;
			}
			i++ ;
		}

		if( byLen > 239 )
			break;
		else
		{
			if( byStaticDevAddr >= byTotalDevNum )
			{
				byStaticDevAddr = 0 ;
				byStaticLineNo++  ;
			}
		}
	}

	pTXBuf[7] = byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;		 //长度L
	return byLen;
}/*}}}*/
#endif

int CProtocol_ESD_IEC104::ComState_Message( WORD wUnitAddr )
{/*{{{*/
	BYTE byLen = 0 , byCount = 0 ;
	WORD wNS = 0 , wNR = 0 ;
	BYTE *pTXBuf = m_pTX_Buf;
	GetSendRecvNo( wNS , wNR ) ;

	BYTE byTotalDevNum = 0;

#if 0
	pTXBuf[0] = 0x68;				//启动字符
	pTXBuf[2] = LOBYTE(wNS);
	pTXBuf[3] = HIBYTE(wNS);
	pTXBuf[4] = LOBYTE(wNR);
	pTXBuf[5] = HIBYTE(wNR);
	pTXBuf[6] = 41;				//41 自定义类型标识TYP
	pTXBuf[8] = 3;				// 自发传送 2 byte
	pTXBuf[9] = 0;
	pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
	pTXBuf[11] = HIBYTE(wUnitAddr);
	byLen = 12;
#endif

	BYTE byBusNum = m_pMethod->GetToTalBusNum() ;
	if( byBusNum == 0 )
		return 0 ;

	if( byStaticLineNo >= byBusNum )
		byStaticLineNo = 0 ;

	if(QueryAllDevStatus){
		pTXBuf[0] = 0x68;				//启动字符
		pTXBuf[2] = LOBYTE(wNS);
		pTXBuf[3] = HIBYTE(wNS);
		pTXBuf[4] = LOBYTE(wNR);
		pTXBuf[5] = HIBYTE(wNR);
		pTXBuf[6] = 0x29;				//41 自定义类型标识TYP
		pTXBuf[8] = 3;				// 自发传送 2 byte
		pTXBuf[9] = 0;
		pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
		pTXBuf[11] = HIBYTE(wUnitAddr);
		byLen = 12;
		for( ; byStaticLineNo< byBusNum ; )
		{
			//如果是转发协议不传输通讯状态
			BYTE byType = m_pMethod->GetBusLineProtocolType( byStaticLineNo ) ;
			if( ( PROTOCO_TRANSPROT == byType ) || ( byType == 0xFF ) )
			{
				byStaticLineNo++ ;
				continue ;
			}

			byTotalDevNum = m_pMethod->GetDevNum( byStaticLineNo ) ;						//byTotalDevNum:指定总线上设备数目!
			if( byStaticDevAddr >= byTotalDevNum ){
				//flag = byTotalDevNum;
				byStaticDevAddr = 0 ;
			}

			for( int i = 0 ; byStaticDevAddr < byTotalDevNum ; byStaticDevAddr++ )
			{
				//flag = byTotalDevNum;
				if( i == 0 )
				{
					if( PackComStateMsg( pTXBuf , byLen , byStaticLineNo , 0 , SEND_BUS_STATE ) )
						byCount++ ;

					// printf( "\t byLen = %d , Line:%d , Count = %d \n" , byLen , byStaticLineNo , byCount ) ;
					if( byLen > 239 )
						break;
				}

				WORD wDevAddr =  m_pMethod->GetAddrByLineNoAndModuleNo( byStaticLineNo, (WORD)byStaticDevAddr );
				if( 0 != wDevAddr )
				{
					//if( PackComStateMsg( pTXBuf , byLen , m_pMethod->GetSerialNo(byStaticLineNo, byStaticDevAddr), SEND_DEV_STATE ) )
					if( PackComStateMsg( pTXBuf , byLen , byStaticLineNo , wDevAddr , SEND_DEV_STATE ) )
						byCount++ ;
				}
				else
				{
					continue;
				}


				//printf( "\t byLen = %d , Line:%d , DevAddr:%d , Count:%d \n" , byLen , byStaticLineNo , wDevAddr, byCount ) ;

				if( byLen > 239 )
				{
					if( byStaticDevAddr >= byTotalDevNum )
						byStaticLineNo++  ;
					break;
				}
				i++ ;
			}

			if( byLen > 239 )
				break;
			else
			{
				if( byStaticDevAddr >= byTotalDevNum )
				{
					byStaticDevAddr = 0 ;
					byStaticLineNo++  ;
				}
			}
		}

		//printf("--------%d %d %d %d--------\n", byStaticLineNo, byBusNum, flag, byTotalDevNum);
		if((byStaticLineNo >= byBusNum))				//前者能保证遍历了所有总线，条件不足，只要最后一条总线不配置采集应该是没有问题的!
			QueryAllDevStatus = FALSE;
	}else{
		if(deque_stat.size() != 0){
			pTXBuf[0] = 0x68;				//启动字符
			pTXBuf[2] = LOBYTE(wNS);
			pTXBuf[3] = HIBYTE(wNS);
			pTXBuf[4] = LOBYTE(wNR);
			pTXBuf[5] = HIBYTE(wNR);
			pTXBuf[6] = 0x29;				//41 自定义类型标识TYP
			pTXBuf[8] = 3;				// 自发传送 2 byte
			pTXBuf[9] = 0;
			pTXBuf[10] = LOBYTE(wUnitAddr);	//数据单元地址 2 byte
			pTXBuf[11] = HIBYTE(wUnitAddr);
			byLen = 12;
		}
		while(deque_stat.size()){
			//WORD wDevAddr;
			//BYTE byLineNo;
//			m_pMethod->GetBusLineAndAddr(deque_stat.front().serialno, byLineNo, wDevAddr);				//顺序号从0开始!
//			if( PackComStateMsg( pTXBuf , byLen , byStaticLineNo , wDevAddr , SEND_DEV_STATE ) )
			if( PackComStateMsg( pTXBuf , byLen , deque_stat.front().serialno, SEND_DEV_STATE ) )
				byCount++ ;
			if(byLen > 239)
				break;
		}
	}

	pTXBuf[7] = byCount;		//可变结构限定词VSQ
	pTXBuf[1] = byLen-2;		 //长度L
	return byLen;
}/*}}}*/
