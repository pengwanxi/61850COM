/*
 * =====================================================================================
 *
 *       Filename:  CSocketFtp.cpp
 *
 *    Description:  利用socket 模拟ftp进行处理配置
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时29分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */

#include "CSocketFtp.h"
#include "GroupBroad.h"

#define		CSOCKETFTP_PRINT_MSG	1		/*  */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printMsg
 *  Description:  
 *		  Param:
 *		 Return:
 * =====================================================================================
 */
static void printMsg ( const BYTE *buf, int len, BOOL bFlag )
{
	return;
	int i = 0;

	bFlag?printf ( "Rx%d::", len ):printf ( "Tx%d::" ,len);

	for( i=0; i <len; i++ )
	{
		printf ( "%.2x ", buf[i]);
	}
	printf ( "\n" );
}		/* -----  end of function printMsg  ----- */
/*
 *--------------------------------------------------------------------------------------
 *      Method:  Thread
 * Description:  线程  
 *       Input:  
 *		Return:
 *--------------------------------------------------------------------------------------
 */
static  void *Thread ( void *pVoid  )			//这应该是特殊开发的!
{
	CSocketFtp *pSocketFtp = (CSocketFtp *)pVoid;
	printf ( "CSocketFtp thread\n" );
	DWORD dwRecvErrorTime = 0;
	const DWORD dwErrorTime = 10000000;
	DWORD dwUsleepInterval = 500000;
	sleep( 5 );

	while( 1 )
	{
		BYTE buf[FTP_MAX_SEND_LEN] = "";

		//socket 读数据
		int iReadLen =pSocketFtp->m_pPort->AsyReadData( buf, FTP_MAX_SEND_LEN );
		if( iReadLen <= 0 )
		{
			dwRecvErrorTime += dwUsleepInterval;
			if ( dwErrorTime <= dwRecvErrorTime )
			{
				//BYTE byNetCard = pSocketFtp->m_pGroupProto->GetOneNetPort(  ); //- by cyz! unused variable 'byNetCard'
				//pSocketFtp->m_pGroupProto->SetRoutePort( byNetCard );
				//- by cyz!
				//这里设置了一个224.0.0.100的奇葩目标ip，根本无用!
				dwUsleepInterval = 500000;
				pSocketFtp->m_pProto->Init();
				dwRecvErrorTime = 0;
			}

			usleep( dwUsleepInterval );
			continue;
		}

		// printf ( "iReadLen=%d %d\n", iReadLen, MAKEWORD( buf[2], buf[1] ) );
		dwRecvErrorTime = 0;
		dwUsleepInterval = 10000;
		printMsg( buf, iReadLen , TRUE );
		//处理数据 组包数据
		pSocketFtp->m_pProto->ProcessProtoSendBuf( buf, iReadLen );

		if(pSocketFtp->m_pProto->m_wSendLen > 0 )
		{
			printMsg( pSocketFtp->m_pProto->m_bySendBuf, pSocketFtp->m_pProto->m_wSendLen , FALSE );
			//socket 写数据
			pSocketFtp->m_pPort->WritePort(
					pSocketFtp->m_pProto->m_bySendBuf, 
					pSocketFtp->m_pProto->m_wSendLen );

			if( pSocketFtp->m_pProto->m_bReboot )
			{
				sleep( 1 );
				system( "reboot" );
			}
		}
		
		
	}

	pthread_join( pSocketFtp->m_pthread_id, NULL );
}		/* -----  end of method CSocketFtp::Thread  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  GroupBroadThread
 *  Description:  组播线程 用于处理组播数据
 *		  Param:  pVoid CSocketFtp 类
 *		 Return:
 * =====================================================================================
 */
static void *GroupBroadThread ( void *pVoid )
{
	CSocketFtp *pSocketFtp = (CSocketFtp *)pVoid;
	printf ( "CSocketFtp GroupBroadThread\n" );
	const int GROUPSLEEPTIME = 500000;
	int len;
	sleep( 10 );

	while ( 1 )
	{
		BYTE buf[GROUP_BROAD_PROTO_MAX_LEN] = "";
		len = 0;
		int iRtn = GroupBroadRecv( buf, &len );
		if( 0 == iRtn && len > 0)
		{
			printMsg( buf,len, TRUE );
			BOOL bRtn = pSocketFtp->m_pGroupProto->ProcessProtoData( buf, len );
			if( bRtn && pSocketFtp->m_pGroupProto->m_wSendLen > 0 )
			{
				printMsg( pSocketFtp->m_pGroupProto->m_bySendBuf,
						pSocketFtp->m_pGroupProto->m_wSendLen, FALSE);
				GroupBroadSend( pSocketFtp->m_pGroupProto->m_bySendBuf,
						pSocketFtp->m_pGroupProto->m_wSendLen);
			}
		}
		else
		{
			usleep( GROUPSLEEPTIME );
		}
	}

	pthread_join( pSocketFtp->m_groupbroad_id, NULL );
}		/* -----  end of function GroupBroadThread  ----- */
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  CSocketFtp
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CSocketFtp::CSocketFtp ()
{
	// 初始化数据
	m_pPort = NULL;
	m_pProto = NULL;
	m_pGroupProto = NULL;
}  /* -----  end of method CSocketFtp::CSocketFtp  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  ~CSocketFtp
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CSocketFtp::~CSocketFtp ()
{
	//删除数据
	if( NULL != m_pPort )
	{
		m_pPort->ClosePort();
		delete m_pPort;
		m_pPort = NULL;
	}

	if( NULL != m_pProto )
	{
		delete m_pProto;
		m_pProto = NULL;
	}

	if( NULL != m_pGroupProto )
	{
		delete m_pGroupProto;
		m_pGroupProto = NULL;
	}

}  /* -----  end of method CSocketFtp::~CSocketFtp  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  CreateTcpServer
 * Description:  创建服务器
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtp::CreateTcpServer ( void )
{
	char chErrorBuf[256] = "";

	m_pPort = new CTcpPortServer;
	if( NULL == m_pPort )
	{
		printf ( "CSocketFtp::CreateTcpServer fail\n" );
		return FALSE;
	}

	m_pPort->m_uThePort = 65530;
	sprintf( m_pPort->m_szAttrib,"%u",  INADDR_ANY );

	//打开端口
	m_pPort->OpenPort( chErrorBuf );

	return TRUE;
}		/* -----  end of method CSocketFtp::CreateTcpServer  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  CreateProto
 * Description:  创建协议
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtp::CreateProto ( void )
{
	m_pProto = new CSocketFtpProto(  );
	if( NULL == m_pProto )
	{
		printf ( "CSocketFtp::CreateProto fail\n" );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CSocketFtp::CreateProto  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  CreateGroupBroad
 * Description:  创建组播协议及线程
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtp::CreateGroupBroad ( void )
{
	m_pGroupProto = new CGroupBroadProto(  );
	if( NULL == m_pGroupProto )
	{
		return FALSE;
	}

	if( -1 == GroupBroadInit(  ) )
	{
		printf ( "CreateGroupBroad error\n" );
		return FALSE;	
	}

	return ( 0 == pthread_create( &m_groupbroad_id, NULL, GroupBroadThread, this ) );
}		/* -----  end of method CSocketFtp::CreateGroupBroad  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  Init
 * Description:  创始化该协议
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtp::Init ( void )
{
	//创建服务器
	if( !CreateTcpServer(  ) )
	{
		return FALSE;	
	}
	
	//增加协议类
	if( !CreateProto(  ) )
	{
		return FALSE;
	}

	if( !CreateGroupBroad(  ) )
	{
		return FALSE;
	}


	return TRUE;
	
}		/* -----  end of method CSocketFtp::Init  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtp
 *      Method:  CreateThread
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtp::CreateThread (  )
{
	// return FALSE;
	return ( 0 == pthread_create( &m_pthread_id, NULL, Thread, this ) );
}		/* -----  end of method CSocketFtp::CreateThread  ----- */
