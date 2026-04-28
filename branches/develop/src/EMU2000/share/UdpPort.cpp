/******************************************************************************
 *  UdpPort.cpp: implementation of the udp port class for Linux
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

#include "UdpPort.h"

/*****************************************************************************/
CUdpPort::CUdpPort()
{/*{{{*/
	m_uThePort = 3069;
	m_hComm = ERROR;

	m_nAddrSize = sizeof(struct sockaddr_in);
	bzero((char*)&m_RemoteAddr, m_nAddrSize);
	m_RemoteAddr.sin_family = AF_INET;
}/*}}}*/

CUdpPort::~CUdpPort()
{ /*{{{*/
	ClosePort();
}/*}}}*/

BOOL CUdpPort::IsPortValid( void )
{/*{{{*/
	return (m_hComm > 0);
}/*}}}*/

void CUdpPort::ClosePort( void )
{/*{{{*/
	if( !IsPortValid() ) return;
	shutdown(m_hComm, 2);
	close(m_hComm);
}/*}}}*/

BOOL CUdpPort::OpenPort( char* lpszError )
{/*{{{*/
	int  nVal;
	struct sockaddr_in localAddr;

	/* create socket */
	memcpy(m_szRemoteAddr, m_szAttrib, 24);
	// sprintf(m_szRemoteAddr, "%s", m_szAttrib);
	if( (m_hComm=socket(AF_INET, SOCK_DGRAM, 0))==ERROR )
	{
		if( lpszError )
			sprintf( lpszError, "CUdpPort: %s", "Create socket error!" );
		return FALSE;
	}
	/* set up receive buffer sizes */
	nVal = MAX_UDP_SIZE;
	setsockopt(m_hComm, SOL_SOCKET, SO_RCVBUF, (char*)&nVal, sizeof(nVal));
	/* enable send data to more than one destination */
	nVal = 1;
	setsockopt(m_hComm, SOL_SOCKET, SO_BROADCAST, (char*)&nVal, sizeof(nVal));
	/* enable reuse address */
	setsockopt(m_hComm, SOL_SOCKET, SO_REUSEADDR, (char*)&nVal, sizeof(nVal));
	/* set up the local address */
	bzero((char*)&localAddr, m_nAddrSize);
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(m_uThePort);
	localAddr.sin_addr.s_addr = inet_addr(m_szLocalAddr);
	//	if(!inet_aton(m_szLocalAddr, (struct in_addr*)&localAddr.sin_addr.s_addr) )
	//	{
	//	}
	/* bind socket to local address */
	if( bind(m_hComm, (struct sockaddr *)&localAddr, sizeof(struct sockaddr)) == ERROR )
	{
		if( lpszError )
			sprintf( lpszError, "%s", "CUdpPort: Bind address failure!" );
		close(m_hComm);
		m_hComm = ERROR;
		return FALSE;
	}
	/*set no block*/
	/*fcntl(m_hComm, F_SETFL, O_NONBLOCK);*/
	/*nVal = 1; ioctl( m_hComm, FIONBIO, (int)&nVal) );*/
	/* set up the remote address */
	m_RemoteAddr.sin_port = htons(m_uThePort);
	m_RemoteAddr.sin_addr.s_addr = inet_addr(m_szRemoteAddr);
	if( lpszError )
		sprintf( lpszError, "open udp%d in %s ok.", m_uThePort, m_szLocalAddr );
	return TRUE;
}/*}}}*/

BOOL CUdpPort::OpenPortRead(char* lpszError)
{/*{{{*/
	int  nVal;
	struct sockaddr_in localAddr;

	/* create socket */
	memcpy(m_szRemoteAddr, m_szAttrib, 24);
	// sprintf(m_szRemoteAddr, "%s", m_szAttrib);
	if ((m_hComm = socket(AF_INET, SOCK_DGRAM, 0)) == ERROR)
	{
		if (lpszError)
			sprintf(lpszError, "CUdpPort: %s", "Create socket error!");
		return FALSE;
	}
	/* set up receive buffer sizes */
	nVal = MAX_UDP_SIZE;
	setsockopt(m_hComm, SOL_SOCKET, SO_RCVBUF, (char*)&nVal, sizeof(nVal));
	/* enable send data to more than one destination */
	nVal = 1;
	setsockopt(m_hComm, SOL_SOCKET, SO_BROADCAST, (char*)&nVal, sizeof(nVal));
	/* enable reuse address */
	setsockopt(m_hComm, SOL_SOCKET, SO_REUSEADDR, (char*)&nVal, sizeof(nVal));

	m_RemoteAddr.sin_port = htons(m_uThePort);
	m_RemoteAddr.sin_addr.s_addr = inet_addr(m_szRemoteAddr);
	if (lpszError)
		sprintf(lpszError, "open udp%d in %s ok.", m_uThePort, m_szLocalAddr);
	return TRUE;
}/*}}}*/


BOOL CUdpPort::SetQueue( DWORD dwInQueueSize, DWORD dwOutQueueSize )
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

int CUdpPort::GetInQueue( void )
{/*{{{*/
	int nBytes = 0;
	if( !IsPortValid() ) return -1;
	ioctl(m_hComm, FIONREAD, (long)&nBytes);
	return nBytes;
}/*}}}*/

int CUdpPort::GetOutQueue( void )
{/*{{{*/
	return -1;
}/*}}}*/

int CUdpPort::ReadPort( BYTE *pBuf, int nRead )
{/*{{{*/
	int  nBytes, nLen;
	struct sockaddr_in from;
	char inetAddr[32];

	if( !IsPortValid() || nRead<=0 ) return -1;
	nBytes = recvfrom( m_hComm, (char*)pBuf, nRead, 0,
			(struct sockaddr *)&from, (socklen_t*)&nLen );
	if( nBytes == ERROR ) return -2;
	sprintf(inetAddr, "%s", inet_ntoa(from.sin_addr));

	return nBytes;
}/*}}}*/

int CUdpPort::WritePort( BYTE *pBuf, int nWrite )
{/*{{{*/
	if( !IsPortValid() || nWrite<=0 ) return -1;
	return sendto( m_hComm, (char*)pBuf, nWrite, 0,
			(struct sockaddr *)&m_RemoteAddr, sizeof(struct sockaddr) );
}/*}}}*/

int CUdpPort::WriteTo( BYTE *pBuf, int nWrite, char* lpszIPAddr, short nPort )
{/*{{{*/
	struct sockaddr_in to;
	if( !IsPortValid() || nWrite<=0 ) return -1;
	memcpy((char*)&to, (char*)&m_RemoteAddr, m_nAddrSize);
	to.sin_port = htons(nPort);
	to.sin_addr.s_addr = inet_addr(lpszIPAddr);
	return sendto( m_hComm, (char*)pBuf, nWrite, 0,
			(struct sockaddr *)&to, sizeof(struct sockaddr) );
}/*}}}*/

int CUdpPort::WriteTo( BYTE *pBuf, int nWrite, char* lpszIPAddr )
{/*{{{*/
	return WriteTo( pBuf, nWrite, lpszIPAddr, m_uThePort );
}/*}}}*/

int CUdpPort::AsyReadData( BYTE *pBuf, int nRead )
{/*{{{*/
	int  nBytes=0, nLen;
	char inetAddr[32];
	struct sockaddr_in from;
	fd_set rfds;
	struct timeval tv;

	if( !IsPortValid() || nRead<=0 ) return -1;
	FD_ZERO(&rfds);
	FD_SET(m_hComm, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 10000;
	switch( select(m_hComm+1, &rfds, NULL, NULL, &tv) )
	{
	case ERROR:
		return -2;
	case 0:
		break;
	case 1:
		if( FD_ISSET(m_hComm, &rfds) )
		{
			nBytes = recvfrom( m_hComm, (char*)pBuf, nRead, 0,
					(struct sockaddr *)&from, (socklen_t*)&nLen );
			if( nBytes == ERROR ) return -2;
			sprintf(inetAddr, "%s", inet_ntoa(from.sin_addr));
		}
		break;
	}
	return nBytes;
}/*}}}*/

int CUdpPort::AsySendData( BYTE *pBuf, int nWrite )
{/*{{{*/
	fd_set wfds;
	struct timeval tv;

	if( !IsPortValid() || nWrite<=0 ) return -1;
	FD_ZERO(&wfds);
	FD_SET(m_hComm, &wfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	switch( select(m_hComm+1, NULL, &wfds, NULL, &tv) )
	{
	case ERROR:
		return -2;
	case 0:
		break;
	case 1:
		if( FD_ISSET(m_hComm, &wfds) )
			return sendto( m_hComm, (char*)pBuf, nWrite, 0,
					(struct sockaddr *)&m_RemoteAddr, sizeof(struct sockaddr) );
		break;
	}
	return 0;
}

int CUdpPort::AsySendTo( BYTE *pBuf, int nWrite, char* lpszIPAddr, short nPort )
{/*{{{*/
	struct sockaddr_in to;
	if( !IsPortValid() || nWrite<=0 ) return -1;
	memcpy((char*)&to, (char*)&m_RemoteAddr, m_nAddrSize);
	to.sin_port = htons(nPort);
	to.sin_addr.s_addr = inet_addr(lpszIPAddr);

	fd_set wfds;
	struct timeval tv;
	FD_ZERO(&wfds);
	FD_SET(m_hComm, &wfds);
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	switch( select(m_hComm+1, NULL, &wfds, NULL, &tv) )
	{
	case ERROR:
		return -2;
	case 0:
		break;
	case 1:
		if( FD_ISSET(m_hComm, &wfds) )
			return sendto( m_hComm, (char*)pBuf, nWrite, 0,
					(struct sockaddr *)&to, sizeof(struct sockaddr) );
		break;
	}
	return 0;
}/*}}}*/

int CUdpPort::AsySendTo( BYTE *pBuf, int nWrite, char* lpszIPAddr )
{/*{{{*/
	return AsySendTo( pBuf, nWrite, lpszIPAddr, m_uThePort );
}/*}}}*/
/*****************************************************************************/
