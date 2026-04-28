/*
 * =====================================================================================
 *
 *       Filename:  ModBusWenKongyi.h
 *
 *    Description:  干式温控仪Modbus协议 .cpp有大概相应说明
 *
 *        Version:  1.0
 *        Created:  2014年11月19日 13时21分18秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */


#ifndef  MODBUSWENKONGYI_INC
#define  MODBUSWENKONGYI_INC

#include "CProtocol_ModBusMaster.h"


/*
 * =====================================================================================
 *        Class:  CModbusWenKongYi
 *  Description:  干式温控仪类 
 * =====================================================================================
 */
class CModbusWenKongYi : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CModbusWenKongYi ();                             /* constructor      */
		~CModbusWenKongYi ();                            /* destructor       */

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

}; /* -----  end of class CModbusWenKongYi  ----- */

#endif   /* ----- #ifndef MODBUSWENKONGYI_INC  ----- */
