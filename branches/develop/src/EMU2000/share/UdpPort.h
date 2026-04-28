/******************************************************************************
 *
 *  UdpPort.h: interface for the UdpPort class for Linux
 *  Copyright (C): 2010 by houpeng
 *
 ******************************************************************************/
#ifndef _UDPPORT_H_
#define _UDPPORT_H_

#include <netinet/in.h>
#include "BasePort.h"

#define MAX_UDP_SIZE  1280
/*****************************************************************************/
/* This class is the udp port */
class CUdpPort : public CBasePort
{
	/*Constructors / Destructors*/
	public:
		CUdpPort(void);
		virtual ~CUdpPort(void);
		virtual char* ClassName(){return (char *)"CUdpPort";}

		/* Attributes */
	private:
		int m_nAddrSize;
		struct sockaddr_in m_RemoteAddr;

	protected:

	public:
		int  WriteTo( BYTE *pBuf, int nWrite, char* lpszIPAddr, short nPort );
		int  WriteTo( BYTE *pBuf, int nWrite, char* lpszIPAddr );
		int  AsySendTo( BYTE *pBuf, int nWrite, char* lpszIPAddr, short nPort );
		int  AsySendTo( BYTE *pBuf, int nWrite, char* lpszIPAddr );

		/* Implementation */
	public:
		virtual BOOL   IsPortValid( void );
		virtual BOOL   OpenPort( char* lpszError=NULL );
		BOOL OpenPortRead(char* lpszError);
		virtual void   ClosePort(void);
		virtual BOOL   SetQueue( DWORD dwInQueueSize, DWORD dwOutQueueSize );
		virtual int	   GetInQueue( void );
		virtual int    GetOutQueue( void );
		virtual int	   ReadPort( BYTE *pBuf, int nRead );
		virtual int	   WritePort( BYTE *pBuf, int nWrite );
		virtual int	   AsyReadData( BYTE *pBuf, int nRead );
		virtual int    AsySendData( BYTE *pBuf, int nWrite );	
};
/*****************************************************************************/
#endif
