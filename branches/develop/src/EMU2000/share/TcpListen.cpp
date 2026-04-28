/******************************************************************************
 *  TcpListen.cpp: implementation of the CTcpListen class for Linux
 *  Copyright (C): 2010 by houpeng
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "TcpListen.h"

#define SERVER_MAX_CONNECTIONS  4      /* max clients connected at a time */
/*****************************************************************************/
#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */

	void  OutPromptText(char *lpszText);
	void  LogPromptText(const char *fmt, ...);
	void  OutMessageText(char *szSrc, unsigned char *pData, int nLen);

	void* TcpSvrTask(void *pListenObj)
	{
		((CTcpListen*)pListenObj)->WorkTaskProc();
		pthread_exit(0);
		return NULL;
	}

#ifdef	__cplusplus
}
#endif	/* __cplusplus */

/*****************************************************************************/
CTcpListen::CTcpListen()
{
	m_nThePort = 3066;
	m_hSocket = ERROR;
	m_pAcceptObj = NULL;
	m_nAddrSize = sizeof(struct sockaddr_in);
	m_tTaskID = 0;
	m_bTaskRun = FALSE;
}

CTcpListen::~CTcpListen()
{
	RunExit();
}

BOOL CTcpListen::IsObjValid( void )
{
	if(m_hSocket <= 0) return FALSE;
	return TRUE;
}

void CTcpListen::ReleaseObj( void )
{
	if(m_hSocket<=0) return;
	close(m_hSocket);
	m_hSocket = ERROR;
}

void CTcpListen::RunExit( void )
{
	m_bTaskRun = FALSE;
	if( m_tTaskID >= 0 )
	{
		//	    pthread_join(m_tThreadID, 0);
		//		m_tTaskID = -1;
	}
	ReleaseObj();
	m_pAcceptObj = NULL;
}

BOOL CTcpListen::CreateObj( char* lpszError )
{
	int  nVal=0;
	struct sockaddr_in  localAddr;

	/* Create socket id */
	if( (m_hSocket=socket(AF_INET, SOCK_STREAM, 0))==ERROR ) /*IPPROTO_TCP*/
	{
		if( lpszError )
			sprintf( lpszError, "CTcpListen: %s", "Create socket error!" );
		return FALSE;
	}
	/* set up the local address */
	m_nAddrSize = sizeof(struct sockaddr_in);
	bzero((char*)&localAddr, m_nAddrSize);
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(m_nThePort);
	localAddr.sin_addr.s_addr = inet_addr(m_szLocalAddr);
	/* enable reuse address */
	nVal = 1;
	setsockopt(m_hSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&nVal, sizeof(nVal));
	/* bind socket to local address */
	if( bind(m_hSocket, (struct sockaddr *)&localAddr, m_nAddrSize) == ERROR )
	{
		if( lpszError )
			sprintf( lpszError, "CTcpListen: bind(%s:%d) error!", m_szLocalAddr, m_nThePort );
		close(m_hSocket);
		m_hSocket = ERROR;
		return FALSE;
	}
	/* create queue for client connection requests */
	if( listen(m_hSocket, SERVER_MAX_CONNECTIONS) == ERROR )
	{
		if( lpszError )
			sprintf( lpszError, "CTcpListen: listen() error!" );
		close(m_hSocket);
		m_hSocket = ERROR;
		return FALSE;
	}
	if( lpszError )
		sprintf( lpszError, "CTcpListen::CreateObj(%s:%d) ok.", m_szLocalAddr, m_nThePort );
	return TRUE;
}

BOOL CTcpListen::StartRun( CDelegateBase* lpAcceptObj )
{
	m_pAcceptObj = lpAcceptObj;
	m_bTaskRun = TRUE;
	if( (m_tTaskID = pthread_create(&m_tThreadID, NULL, TcpSvrTask, (void*)this)) < 0 )
	{
		LogPromptText("****CTcpListen(%s:%d).StartRun() Failure****\n", m_szLocalAddr, m_nThePort);
		return FALSE;
	}
	LogPromptText("****CTcpListen(%s:%d).StartRun() success****\n", m_szLocalAddr, m_nThePort);
	return TRUE;
}

/******************************************************************************
 * 任务处理函数
 */
void CTcpListen::WorkTaskProc(void)
{
	int  hClientFd;     /* socket descriptor from accept */
	int  sockAddrSize;  /* size of socket address structure */

	OutPromptText((char *)"****CTcpListen::WorkTaskProc() Run****");
	sockAddrSize = sizeof(struct sockaddr_in);
	while(m_bTaskRun)
	{
		if( !IsObjValid() )
		{
			CreateObj();
		}
		else
		{
			/* accept new connect requests */
			if( (hClientFd = accept( m_hSocket,
							(struct sockaddr*)&m_ClientAddr,
							(socklen_t*)&sockAddrSize) ) == ERROR )
			{
				ReleaseObj();
				continue;
			}
			if( m_pAcceptObj )
			{
				sprintf(m_szRemoteAddr, "%s", inet_ntoa(m_ClientAddr.sin_addr) );
				m_pAcceptObj->AcceptProc( hClientFd, m_nThePort,
						m_szRemoteAddr, m_szLocalAddr );
			}
			else
			{
				close(hClientFd);
			}
		}
		usleep(500000);
	}
}
/*****************************************************************************/
