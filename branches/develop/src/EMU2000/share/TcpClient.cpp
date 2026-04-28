// TcpClient.cpp: implementation of the CTcpClient class.
//
//////////////////////////////////////////////////////////////////////
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/tcp.h>

#include "TcpClient.h"
#define MAX_TCP_SIZE 1280
#include <iostream>
using namespace std;

int keepAlive = 1;	  // ����keepalive����
int keepIdle = 60;	  // ���������60����û���κ���������,�����̽��
int keepInterval = 5; // ̽��ʱ������ʱ����Ϊ5 ��
int keepCount = 3;	  // ̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.

CTcpClient::CTcpClient()
{ /*{{{*/
	m_hComm = NULL;
	m_bConnet = FALSE;
	m_lastActivityTime = time(NULL); // 初始化时间戳
} /*}}}*/

CTcpClient::~CTcpClient()
{ /*{{{*/
	ClosePort();
} /*}}}*/

BOOL CTcpClient::IsPortValid(void) // main.cpp::ThreadScanSever:һֱ��ѯ�鿴ͨ������״̬��һ��FALSE��close�����´�!
{								   /*{{{*/
	if (m_hComm <= 0)
	{
		// printf( "m_hComm Error!\n") ;
		return FALSE;
	}

	if (m_bConnet <= 0)
	{
		// printf( "m_bConnet Error!\n" ) ;
		return FALSE;
	}

	return TRUE;
} /*}}}*/

BOOL CTcpClient::OpenPort(char *lpszError)
{ /*{{{*/
	int nVal;
	char buf[24];

	/* Create socket id */
	// sprintf(m_szRemoteAddr, "%s", m_szAttrib);
	memcpy(m_szRemoteAddr, m_szAttrib, 24);
	memcpy(buf, m_szAttrib, 24);
	if (m_hComm)
	{
		close(m_hComm);
		m_hComm = NULL;
	}
	if ((m_hComm = socket(AF_INET, SOCK_STREAM, 0)) == ERROR) /*IPPROTO_TCP*/
	{
		if (lpszError)
			sprintf(lpszError, "CTcpPort: %s", "Create socket error!");
		return FALSE;
	}

	int nAddrSize = sizeof(struct sockaddr_in);
	bzero((char *)&m_RemoteAddr, nAddrSize);
	m_RemoteAddr.sin_family = AF_INET;
	m_RemoteAddr.sin_port = htons(m_uThePort);
	m_RemoteAddr.sin_addr.s_addr = inet_addr(m_szAttrib);

	/* Set SO_REUSEADDR */
	nVal = 1;
	setsockopt(m_hComm, SOL_SOCKET, SO_REUSEADDR, (char *)&nVal, sizeof(nVal));

	/* set up receive buffer sizes */
	nVal = MAX_TCP_SIZE;
	setsockopt(m_hComm, SOL_SOCKET, SO_RCVBUF, (char *)&nVal, sizeof(nVal));

	// ��ʼ������һ�η��������Ժ�ÿ��ɨ�������Ƿ�Ͽ�
	Connect();

	// set keepalive
	setKeepAlive();

	return TRUE;
} /*}}}*/

BOOL CTcpClient::setKeepAlive()
{
	if (m_hComm <= 0)
		return FALSE;

	setsockopt(m_hComm, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive));
	setsockopt(m_hComm, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle, sizeof(keepIdle));
	setsockopt(m_hComm, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt(m_hComm, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount));

	return TRUE;
}

BOOL CTcpClient::Connect()
{ /*{{{*/
	if (m_hComm <= 0)
		return FALSE;
	if (connect(m_hComm,
				(struct sockaddr *)&m_RemoteAddr,
				sizeof(m_RemoteAddr)) == ERROR)
	{
        printf("[TCP] Connect to %s:%d failed: %s\n", inet_ntoa(m_RemoteAddr.sin_addr), ntohs(m_RemoteAddr.sin_port), strerror(errno));
		return FALSE;
	}

    printf("[TCP] Connected to %s:%d (fd:%d)\n", inet_ntoa(m_RemoteAddr.sin_addr), ntohs(m_RemoteAddr.sin_port), m_hComm);
	m_bConnet = TRUE;
	m_lastActivityTime = time(NULL); // 连接成功时记录时间
	return TRUE;
} /*}}}*/

void CTcpClient::ClosePort(void)
{ /*{{{*/
	if (m_hComm == ERROR)
		return;
	printf("Closing port (fd:%d)\n", m_hComm); // 增加关闭日志
	fflush(stdout);
	shutdown(m_hComm, 2); // �رն�дͨ�����������ͷ��ļ�������!
	close(m_hComm);		  // �ȿ��Թرն�дͨ����ͬʱҲ���ͷ��ļ�������!
	m_hComm = ERROR;
	m_bConnet = 0;
} /*}}}*/

int CTcpClient::ReadPort(BYTE *pBuf, int nRead)
{ /*{{{*/
	int nBytes = 0;

	if (!IsPortValid() || nRead <= 0)
		return -1;
	nBytes = recv(m_hComm, (char *)pBuf, nRead, 0);
	if (nBytes == ERROR)
	{
		m_bConnet = FALSE;
		return -2;
	}
    else if(nBytes == 0) {
        printf("[TCP] Connection gracefully closed (fd:%d)\n", m_hComm);
        ClosePort();
        return -3; // 新增返回码表示正常关闭
    }

	return nBytes;
} /*}}}*/

int CTcpClient::WritePort(BYTE *pBuf, int nWrite)
{ /*{{{*/
	if (!IsPortValid() || nWrite <= 0)
		return -1;
	// MSG_NOSIGNAL ��ֹ��ϵͳ�����쳣��Ϣ
	int nBytes = send(m_hComm, (char *)pBuf, nWrite, MSG_NOSIGNAL);
	if (nBytes == ERROR)
	{
		perror("send");
		m_bConnet = 0;
		return -2;
	}
	return nBytes;
} /*}}}*/

int CTcpClient::AsyReadData(BYTE *pBuf, int nRead)
{ /*{{{*/
	int nBytes = 0;
	fd_set rfds;
	struct timeval tv;

	//添加超时检查
	 time_t currentTime = time(NULL);
	 if (currentTime - m_lastActivityTime > 600) { // 10分钟=600秒
		 printf("[TCP] Connection timeout (fd:%d)\n", m_hComm);
		 ClosePort();
		 return -3;
	 }

	// 	if( !IsPortValid() || nRead<=0 ) return -1;
	// 	FD_ZERO(&rfds);
	// 	FD_SET(m_hComm, &rfds);
	// 	tv.tv_sec  = 0;
	// 	tv.tv_usec = 10000;
	// 	switch( select(m_hComm+1, &rfds, NULL, NULL, &tv) )
	// 	{
	// 	case ERROR:
	// 		m_bConnet = 0;
	// 		return -2;
	// 	case 0:
	// 		break;
	// 	case 1:
	// 		if(FD_ISSET(m_hComm, &rfds))
	// 		{
	// 			nBytes = recv( m_hComm, (char*)pBuf, nRead, 0 );
	// 			if( nBytes == ERROR )
	// 				return 0;
	// 			else if( nBytes == 0 )
	// 				ClosePort() ;
	// 		}
	// 		break;
	// 	}
	if (!IsPortValid() || nRead <= 0)
		return -1;

	char Recvbuf[102400] = {0};
	DWORD dwindex = 0;
	while (1)
	{
		FD_ZERO(&rfds);
		FD_SET(m_hComm, &rfds);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		int ret = select(m_hComm + 1, &rfds, NULL, NULL, &tv);
		if (ret < 0)
		{
			m_bConnet = 0;
			break;
		}
		if (ret == 0)
		{
			break;
		}
		if (FD_ISSET(m_hComm, &rfds) == 0)
		{
			printf("fd_isset == 0 \n");
			break;
		}
		nBytes = recv(m_hComm, Recvbuf, nRead - dwindex, 0);
		if (nBytes == ERROR)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
				printf("Fatal recv error: %s (fd:%d)\n", strerror(errno), m_hComm);
				fflush(stdout);
				ClosePort();
				return -2;
			}
			return 0;
		}
		else if (nBytes == 0)
		{
			printf("[TCP] Connection closed by peer (fd:%d, addr:%s:%d)\n", m_hComm, inet_ntoa(m_RemoteAddr.sin_addr), ntohs(m_RemoteAddr.sin_port));
			fflush(stdout); // 强制刷新输出缓冲区
			ClosePort();
			return 0;
		}
		else if (nBytes>0)
		{
			m_lastActivityTime = time(NULL); // 更新最后活动时间戳
		}

		DWORD dwCounter = 0;
		while (dwCounter < nBytes)
		{
			if (dwindex <= nRead)
				pBuf[dwindex++] = Recvbuf[dwCounter++];
			else
				break;
		}
		/*
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 50 * 1000;
		select(0, NULL, NULL, NULL, &tv);
		*/
		usleep(50 * 1000); // 使用更安全的微秒级延时
	}
	nBytes = dwindex;
	return nBytes;
} /*}}}*/

int CTcpClient::AsySendData(BYTE *pBuf, int nWrite)
{ /*{{{*/
	return 0;
} /*}}}*/

BOOL CTcpClient::Ping(char *cIp)
{ /*{{{*/
	if (cIp == NULL)
		return FALSE;

	m_Ping.Ping(cIp);

	return TRUE;
} /*}}}*/
