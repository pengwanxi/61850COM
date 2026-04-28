#pragma once
#include "../share/UdpPort.h"
#include "../share/global.h"
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include "WebSocket.h"
#include "../share/TcpClient.h"
class CTcpPrintMsg
{
public:
	CTcpPrintMsg();
	~CTcpPrintMsg();
	void Asleep(DWORD dwMilliSecd);
	void SetRecvPort(DWORD dwPort);
	void Init(DWORD dwPort, CWebSocket * pWebSocket);
	void ClosePort();
	static void * ThreadProc(void * arg);
	DWORD m_RecvPort;
	CTcpClient m_pCTcpPort;
	BOOL m_brun;
	pthread_t m_threadID;
	CWebSocket * m_pWebSocket;
};

