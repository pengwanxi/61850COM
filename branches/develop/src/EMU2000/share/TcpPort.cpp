/******************************************************************************
 *  TcpPort.cpp: implementation of the udp port class for Linux
 *  Copyright (C): 2010-2038 by houpeng
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

#include "TcpPort.h"

/*****************************************************************************/
CTcpPort::CTcpPort()
{/*{{{*/
	m_nState = 0;
	m_uThePort = 3066;
	m_hComm = ERROR;

	m_nAddrSize = sizeof(struct sockaddr_in);
	bzero((char*)&m_RemoteAddr, m_nAddrSize);
	m_RemoteAddr.sin_family = AF_INET;
}/*}}}*/

CTcpPort::~CTcpPort()
{ /*{{{*/
	ClosePort();
}/*}}}*/

BOOL CTcpPort::IsPortValid( void )
{/*{{{*/
	if(m_hComm <= 0) return FALSE;
	if(m_nState <= 0) return FALSE;
	return TRUE;
}/*}}}*/

void CTcpPort::ClosePort( void )
{/*{{{*/
	if(m_hComm==ERROR) return;
	shutdown(m_hComm, 2);
	close(m_hComm);
	m_hComm = ERROR;
	m_nState = 0;
}/*}}}*/

void CTcpPort::Attach( int hSocket )
{/*{{{*/
	if( m_hComm >= 0 )
	{
		shutdown(m_hComm, 2);
		close(m_hComm);
		m_hComm = ERROR;
	}
	m_hComm = hSocket;
	m_nState = 1;
}/*}}}*/

BOOL CTcpPort::Connect( int nTimeout )
{/*{{{*/
    //struct timeval tv;						//3 of mine!
    //tv.tv_sec = nTimeout;
    //tv.tv_usec = 0;

    if( m_hComm <= 0 ) return FALSE;
    if( connect( m_hComm,
                (struct sockaddr*)&m_RemoteAddr,
                sizeof(m_RemoteAddr) ) == ERROR )
    {
        return FALSE;
    }
	m_nState = 1;
    return TRUE;
}/*}}}*/

BOOL CTcpPort::OpenPort( char* lpszError )
{/*{{{*/
    int  nVal;
    struct sockaddr_in  localAddr;

    /* Create socket id */
    // sprintf(m_szRemoteAddr, "%s", m_szAttrib);
	//memcpy(m_szRemoteAddr, m_szAttrib, 24);
    memcpy(m_szRemoteAddr, m_szAttrib, 32);
    if( (m_hComm=socket(AF_INET, SOCK_STREAM, 0))==ERROR ) /*IPPROTO_TCP*/
    {
        if( lpszError )
            sprintf( lpszError, "CTcpPort: %s", "Create socket error!" );
        return FALSE;
    }
    /* Set SO_REUSEADDR */
	nVal = 1;
	setsockopt(m_hComm, SOL_SOCKET, SO_REUSEADDR, (char*)&nVal, sizeof(nVal));
    /* set up receive buffer sizes */
    nVal = MAX_TCP_SIZE;
    setsockopt(m_hComm, SOL_SOCKET, SO_RCVBUF, (char*)&nVal, sizeof(nVal));
    /* set up the local address */
    bzero((char*)&localAddr, m_nAddrSize);
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons((unsigned short)m_uThePort);
    localAddr.sin_addr.s_addr = inet_addr(m_szLocalAddr);
    /* bind socket to local address */
    if( bind(m_hComm, (struct sockaddr *)&localAddr, sizeof(struct sockaddr)) == ERROR )
    {
        if( lpszError )
            sprintf( lpszError, "CTcpPort: Bind(%s) error!", m_szLocalAddr );
        close(m_hComm);
        m_hComm = ERROR;
        return FALSE;
    }
	/*set no block*/
	/*fcntl(m_hComm, F_SETFL, O_NONBLOCK);*/
    /*nVal = 1; ioctl( m_hComm, FIONBIO, (int)&nVal) );*/
    /* set up the server address */
    m_RemoteAddr.sin_port = htons((unsigned short)m_uThePort);
    m_RemoteAddr.sin_addr.s_addr = inet_addr(m_szRemoteAddr);
    if( lpszError )
        sprintf( lpszError, "open tcp%d in %s ok.", m_uThePort, m_szLocalAddr );
    return TRUE;
}/*}}}*/

BOOL CTcpPort::SetQueue( DWORD dwInQueueSize, DWORD dwOutQueueSize )
{/*{{{*/
/*
    if( !IsPortValid() ) FALSE;
    int nVal = (int)dwInQueueSize;
    setsockopt(m_hComm, SOL_SOCKET, SO_RCVBUF, (char*)&nVal, sizeof(nVal));
    nVal = (int)dwOutQueueSize;
    setsockopt(m_hComm, SOL_SOCKET, SO_SENDBUF, (char*)&nVal, sizeof(nVal));
*/
    return TRUE;
}/*}}}*/

int CTcpPort::GetInQueue( void )
{/*{{{*/
    int nBytes = 0;
    if( !IsPortValid() ) return -1;
    ioctl(m_hComm, FIONREAD, (long)&nBytes);
    return nBytes;
}/*}}}*/

int CTcpPort::GetOutQueue( void )
{/*{{{*/
    return -1;
}/*}}}*/

int CTcpPort::ReadPort( BYTE *pBuf, int nRead )
{/*{{{*/
    int  nBytes = 0;

    if( !IsPortValid() || nRead<=0 ) return -1;
    nBytes = recv( m_hComm, (char*)pBuf, nRead, 0 );
    if( nBytes == ERROR )
	{
		m_nState = 0;
		return -2;
	}
    return nBytes;
}/*}}}*/
/*errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN*/
int CTcpPort::WritePort( BYTE *pBuf, int nWrite )
{/*{{{*/
    if( !IsPortValid() || nWrite<=0 ) return -1;
    int nBytes = send( m_hComm, (char*)pBuf, nWrite, MSG_NOSIGNAL );
	if( nBytes == ERROR )
	{
		m_nState = 0;
		return -2;
	}
	return nBytes;
}/*}}}*/

int CTcpPort::AsyReadData( BYTE *pBuf, int nRead )
{/*{{{*/
    int  nBytes = 0;
    fd_set rfds;
    struct timeval tv;

    if( !IsPortValid() || nRead<=0 ) return -1;
    FD_ZERO(&rfds);
    FD_SET(m_hComm, &rfds);
    tv.tv_sec  = 0;
    tv.tv_usec = 10000;
    switch( select(m_hComm+1, &rfds, NULL, NULL, &tv) )
    {
    case ERROR:
        m_nState = 0;
        return -2;
    case 0:
        break;
    case 1:
        if(FD_ISSET(m_hComm, &rfds))
        {
            nBytes = recv( m_hComm, (char*)pBuf, nRead, 0 );
            if( nBytes == ERROR )
				return 0;
			else if( nBytes == 0 )
				ClosePort() ;
        }
        break;
    }
    return nBytes;
}/*}}}*/

int CTcpPort::AsySendData( BYTE *pBuf, int nWrite )
{/*{{{*/
    fd_set wfds;
    struct timeval tv;

    if( !IsPortValid() || nWrite<=0 ) return -1;
    FD_ZERO(&wfds);
    FD_SET(m_hComm, &wfds);
    tv.tv_sec  = 3;
    tv.tv_usec = 0;
    switch( select(m_hComm+1, NULL, &wfds, NULL, &tv) )
    {
    case ERROR:
        m_nState = 0;
        return -2;
    case 0:
        break;
    case 1:
        if(FD_ISSET(m_hComm, &wfds))
           return send(m_hComm, (char*)pBuf, nWrite, MSG_NOSIGNAL );
        break;
    }
    return 0;
}/*}}}*/
/*****************************************************************************/
