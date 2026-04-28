/*
 * =====================================================================================
 *
 *       Filename:  ModBusXiaoDianLiu.h
 *
 *    Description:  小电流接地选线ModBus协议 .cpp有大概相应说明
 *
 *        Version:  1.0
 *        Created:  2015年04月22日 11时13分42秒 
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


#ifndef  MODBUSXIAODIANLIU_INC
#define  MODBUSXIAODIANLIU_INC

#include "CProtocol_ModBusMaster.h"

#define		XDL_PRINT		1			/*  */

/*
 * =====================================================================================
 *        Class:  CModBusXDL
 *  Description:  干式温控仪类 
 * =====================================================================================
 */
class CModBusXDL : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CModBusXDL ();                             /* constructor      */
		~CModBusXDL ();                            /* destructor       */

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
	BOOL WhetherBufValue ( BYTE *buf, int &len , int &pos);
	// 处理报文
	BOOL ProcessRecvBuf ( BYTE *buf, int len );
	// 打印报文
	void print( char *buf );
	private:
		/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;

}; /* -----  end of class CModBusXDL  ----- */

#endif   /* ----- #ifndef MODBUSXIAODIANLIU_INC  ----- */
