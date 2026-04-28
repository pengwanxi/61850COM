/*
 * =====================================================================================
 *
 *       Filename:  CBusDebug.h
 *
 *    Description:  设置总线端口打印数据
 *
 *        Version:  1.0
 *        Created:  2014年08月01日 11时55分11秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *        Company:  esdtek
 *
 * =====================================================================================
 */

#ifndef  __CBUSDEBUG_H__
#define  __CBUSDEBUG_H__


#include	<stdio.h>
#include	<string.h>
#include	"../share/BasePort.h"
#include	"../share/UdpPort.h"
#include	"../share/typedef.h"

#define START_PORT 50000
/*
 * =====================================================================================
 *        Class:  CBusDebug
 *  Description:  设置总线端口打印的类
 * =====================================================================================
 */
class CBusDebug
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CBusDebug ();                             /* constructor      */
		~CBusDebug ();                            /* destructor       */
		BOOL Init(BYTE byBusLine);
		BOOL Init ( BYTE byBusline, char *pNetCard, char *pRemoteIp, DWORD dzPortNum );
		/* ====================  ACCESSORS     ======================================= */

		/* ====================  MUTATORS      ======================================= */
		int SendDebugMsg (  BYTE *buf, int Len  );
		BOOL GetLocalIp( char *pNetCard );
	protected:
		BOOL SetDebugAttri(BYTE byBusline, int intrface, char *szLocalIp, char *szBroadIp);
		BOOL SetDebugNet(BYTE byBusline);
		/* ====================  DATA MEMBERS  ======================================= */
	private:
		/* ====================  DATA MEMBERS  ======================================= */
		CUdpPort *pUdpObj;
		// CUdpPort *pUdp1Obj;
		// CUdpPort *pUdp2Obj;
		// CUdpPort *pUdp3Obj;
		// char m_szRemoteAddr[4][24];
		char m_szRemoteAddr[24];
		char m_szLocalAddr[16];
		bool m_binit;
}; /* -----  end of class CBusDebug  ----- */

#endif   /* ----- #ifndef CBUSDEBUG_INC  ----- */
