/******************************************************************************
 *
 *  BasePort.h: interface for the BasePort class.
 *
 ******************************************************************************/
#ifndef _BASEPORT_H_
#define _BASEPORT_H_

#include "typedef.h"

/******************************************************************************
 * This class is base for the base port.
 */
class CBasePort
{/*{{{*/
	/*Constructors / Destructors*/
	public:
		CBasePort(void);
		virtual ~CBasePort(void);
		virtual char* ClassName(){return (char *)"CBasePort";}

		/* Attributes */
	protected:
		BOOL    m_bValid;
		HANDLE  m_hComm;						//file descriptor!
		WORD    m_wPortState;

	public:
		BYTE    m_byEnable;    /*使用/备用*/
		UINT	m_uThePort;
		char    m_szAttrib[32];
		char	m_szLocalAddr[24];
		char	m_szRemoteAddr[24];

		/* Implementation */
	public:
		static int GetCommAttrib( const char* lpszCtrl, char* lpszAttr, UINT& uPort );
		static int GetCommAttrib_original( const char* lpszCtrl, char* lpszAttr, UINT& uPort );
		void ReceiveProc( void );

		virtual BOOL   IsPortValid( void );
		virtual BOOL   OpenPort( char* lpszError=NULL );
		virtual void   ClosePort( void );
		virtual BOOL   SetQueue( DWORD dwInQueueSize, DWORD dwOutQueueSize );
		virtual int	   GetInQueue( void );
		virtual int    GetOutQueue( void );
		virtual int	   ReadPort( BYTE *pBuf, int nRead );
		virtual int	   WritePort( BYTE *pBuf, int nWrite );
		virtual int	   AsyReadData( BYTE *pBuf, int nRead );
		virtual int    AsySendData( BYTE *pBuf, int nWrite );
		virtual BOOL Connect( ){ return FALSE ;}
		virtual BOOL Ping( char * cIp ){ return FALSE ;}
};/*}}}*/
/*****************************************************************************/
#endif
