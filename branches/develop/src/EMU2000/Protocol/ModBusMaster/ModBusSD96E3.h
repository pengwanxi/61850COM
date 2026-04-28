/*
 * =====================================================================================
 *
 *       Filename:  CModBusSD96E3.h
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  2017年1月9日 11时40分18秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ykk (), 
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */


#ifndef  MODBUSSD96E3_INC
#define  MODBUSSD96E3_INC

#include "CProtocol_ModBusMaster.h"


/*
 * =====================================================================================
 *        Class:  ModBusSD96-E3
 *  Description:  
 * =====================================================================================
 */
class CModBusSD96E3 : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CModBusSD96E3 ();                             /* constructor      */
		~CModBusSD96E3 ();                            /* destructor       */

	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
	virtual BOOL Init( BYTE byLineNo  ) ;
	
	virtual void TimerProc() ;
	//获得装置通讯状态
	virtual BOOL GetDevCommState( ) ;

	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	private:
	// 判断报文有效性
	BOOL WhetherBufValue ( BYTE *buf, int &len );
	// 处理报文
	BOOL ProcessRecvBuf ( BYTE *buf, int len );
	private:
		/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;

}; /* -----  end of class CModBusSD96E3  ----- */

#endif   /* ----- #ifndef MODBUSSD96E3_INC  ----- */
