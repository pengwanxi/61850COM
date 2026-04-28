// TcpClient.h: interface for the CTcpClient class.
//
//////////////////////////////////////////////////////////////////////


#ifndef _TCPCLIENT_H_
#define _TCPCLIENT_H_

#include <netinet/in.h>
#include "BasePort.h"
#include "PintTest.h"

class CTcpClient  : public CBasePort
{
	public:
		CTcpClient();
		virtual ~CTcpClient();
		virtual char* ClassName() { return ( char * )"CTcpClient"; }
		virtual BOOL   IsPortValid( void );
		virtual BOOL   OpenPort( char* lpszError=NULL );
		BOOL  setKeepAlive();
		virtual void   ClosePort(void);
		virtual int	   ReadPort( BYTE *pBuf, int nRead );
		virtual int	   WritePort( BYTE *pBuf, int nWrite );
		virtual int	   AsyReadData( BYTE *pBuf, int nRead );
		virtual int    AsySendData( BYTE *pBuf, int nWrite );
		virtual BOOL Connect( );
	protected:
		BOOL m_bConnet ;
		struct sockaddr_in m_RemoteAddr;
		BOOL Ping( char * cIp ) ;
		CPintTest m_Ping ;
	private:
    	time_t m_lastActivityTime; // 最后活动时间戳
};

#endif // !defined(AFX_TCPCLIENT_H__06DCF3A5_6F00_4CAD_8D07_3DDB75370453__INCLUDED_)

