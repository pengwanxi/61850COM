#ifndef CTCPPORTSERVER_H
#define CTCPPORTSERVER_H

#include "TcpPort.h"
#include "TcpListen.h"
#include "profile.h"
#include "cmsg.h"

class CTcpPortServer ;

class CTcpPortServerDelegate : public CDelegateBase
{
	public:
		CTcpPortServerDelegate( )
		{
			m_pServer = NULL ;
		}
		virtual ~CTcpPortServerDelegate()
		{
		}

		virtual BOOL AcceptProc(int hSocket, unsigned short nPort,
				char* lpszRemote, char* lpszLocal) ;

		CTcpPortServer * m_pServer ;
};

class CTcpPortServer : public CTcpPort
{
	public:
		CTcpPortServer();
		virtual ~CTcpPortServer();
		virtual BOOL   OpenPort(char* lpszError = NULL);
		BOOL InitPortOtherPara( char * path ) ;
		BOOL sendMsg(void * pVoid);
		BOOL recvMsg(void * pMsg);
		BOOL ProcessAcceptProc(int hSocket, unsigned short nPort,
				char* lpszRemote, char* lpszLocal) ;
		CTcpListen m_Listen ;
		CTcpPortServerDelegate m_delegate ;
		void setTcpParam(const char * pIP, UINT uPort);
		char    m_szLocalGateWay[24];
		char    m_szLocalSubNetMask[24];
		char    m_szLocalDNS[24];
		enum { eexit = 1 };
private:
		BOOL initMsgQueue();

private:
		CMsg m_msg;
};

#endif // CTCPPORTSERVER_H

