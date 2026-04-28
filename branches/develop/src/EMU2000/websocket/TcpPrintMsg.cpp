#include "UpdPrintMsg.h"
#include <stdio.h>



CTcpPrintMsg::CTcpPrintMsg()
{
	m_RecvPort = 0;
	m_brun = FALSE;
	m_threadID = 0;
	m_pWebSocket = 0;
}


CTcpPrintMsg::~CTcpPrintMsg()
{
}

void CTcpPrintMsg::Asleep(DWORD dwMilliSecd)
{/*{{{*/
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = dwMilliSecd * 1000;
	select(0, NULL, NULL, NULL, &tv);
}/*}}}*/

void CTcpPrintMsg::SetRecvPort(DWORD dwPort)
{
	m_RecvPort = dwPort;
}

void CTcpPrintMsg::Init(DWORD dwPort , CWebSocket * pWebSocket )
{
	SetRecvPort(dwPort);
	m_brun = TRUE;
	//m_pCTcpPort = new CTcpClient;
	m_pWebSocket = pWebSocket;

	int  hThread = pthread_create(&m_threadID, NULL, ThreadProc , this );
 	if (hThread < 0)
	{
		printf((char *)" ****CreateThread Fail!****");
	}
	
}

void CTcpPrintMsg::ClosePort()
{
	m_brun = FALSE;
	if (m_threadID != 0)
	{
		pthread_cancel(m_threadID);
		pthread_join(m_threadID, 0);
	}

		m_pCTcpPort.ClosePort();
}

 void * CTcpPrintMsg::ThreadProc(void * arg)
{
	 CTcpPrintMsg * pThis = (CTcpPrintMsg *)arg;

	CTcpClient * tcpPort = &pThis->m_pCTcpPort;
	tcpPort->m_uThePort = pThis->m_RecvPort;
	strcpy(tcpPort->m_szAttrib, "127.0.0.1");
	strcpy(tcpPort->m_szLocalAddr, "127.0.0.1");
	char error[100] = { 0 };
	bool b = tcpPort->OpenPort();

	while (pThis->m_brun)
	{
		const int size = 1024;
		char buf[size] = { 0 };	
		int nRead = tcpPort->ReadPort((BYTE*)buf, size);
		 // 处理连接关闭的情况
		 if(nRead == -3) {
			printf("[WebSocket] TCP connection closed, reconnecting...\n");
			tcpPort->ClosePort();
			b = tcpPort->OpenPort();
			pThis->Asleep(10);
			continue;
		}
		if(nRead > 0) {
			pThis->m_pWebSocket->send(buf, nRead);
		}
		else if(nRead<0)
		{
			tcpPort->ClosePort();
			b = tcpPort->OpenPort();
		}
		//pThis->m_pWebSocket->send(buf, nRead);
		pThis->Asleep(10);
	}

	return 0;
}