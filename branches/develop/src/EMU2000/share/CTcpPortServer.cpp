#include "CTcpPortServer.h"

BOOL CTcpPortServerDelegate::AcceptProc(int hSocket, unsigned short nPort,
										char* lpszRemote, char* lpszLocal)
{
	if( m_pServer )
		m_pServer->ProcessAcceptProc( hSocket , nPort , lpszRemote , lpszLocal ) ;
	else
		CDelegateBase::AcceptProc( hSocket , nPort , lpszRemote , lpszLocal ) ;
	return TRUE ;
}


CTcpPortServer::CTcpPortServer()
{
    //ctor
	m_delegate.m_pServer = this ;
}

CTcpPortServer::~CTcpPortServer()
{
    //dtor
	printf( "TcpPortServer Destruct OK.\n" );
}

BOOL CTcpPortServer::initMsgQueue()
{
	 int msgID = m_msg.CreateMsgQueue(CMsg::msgType_tcpportServer);
	 if (msgID == -1)
		 return FALSE;

	 return TRUE;
}

BOOL CTcpPortServer::OpenPort( char* lpszError  )
{
	if( lpszError == NULL )
		return FALSE ;

	//给服务器赋值
	m_Listen.m_nThePort = m_uThePort ;
	strcpy( m_Listen.m_szLocalAddr , m_szAttrib ) ;

	//创建服务器
	char szError[ 200 ];
	memset( szError , 0 , sizeof( szError ) );
	m_Listen.CreateObj( szError ) ;
	m_Listen.StartRun( &m_delegate ) ;

	return TRUE ;
}

BOOL CTcpPortServer::ProcessAcceptProc(int hSocket, unsigned short nPort,
                       char* lpszRemote, char* lpszLocal)
{
    char szStyle[32];

    sprintf(szStyle, "TCP%d:%s", nPort, lpszRemote);
    LogPromptText("\n***CCtrlLayer::ProcessAccept(%s:%d)***", lpszRemote, nPort);

	Attach( hSocket ) ;

	return TRUE ;
}


void CTcpPortServer::setTcpParam(const char * pIP, UINT uPort)
{
	if (pIP == NULL)
		return;
	strcpy(m_szAttrib, pIP);
	m_uThePort = uPort;
}

BOOL CTcpPortServer::InitPortOtherPara( char * path )
{
	return TRUE ;
}

BOOL CTcpPortServer::sendMsg( void * pVoid )
{
	if (!m_msg.IsMsgQueue() || !pVoid )
		return FALSE;

	return m_msg.SendMsg(pVoid);
}

BOOL CTcpPortServer::recvMsg(void * pMsg)
{
	if (!m_msg.IsMsgQueue() || !pMsg )
		return FALSE;

	return m_msg.RecvMsg(pMsg);
}