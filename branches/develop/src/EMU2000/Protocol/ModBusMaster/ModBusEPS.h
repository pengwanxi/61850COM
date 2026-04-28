/*
 * =====================================================================================
 *
 *       Filename:  ModBusEPS.h
 *
 *    Description:  
 
 *        Version:  1.0
 *        Created:  2014年11月24日 09时40分24秒
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

#ifndef  MODBUSEPS_INC
#define  MODBUSEPS_INC

#include "CProtocol_ModBusMaster.h"


#define				MODBUSMASTER_EPS_MAX_POS			30		/* 最大位置 */

/*
 * =====================================================================================
 *        Class:  CModbusEPS
 *  Description:  ups和eps通讯类 
 * =====================================================================================
 */
class CModbusEPS : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CModbusEPS ();                             /* constructor      */
		~CModbusEPS ();                            /* destructor       */

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
	// 处理遥信报文
	BOOL ProcessYxBuf( BYTE *buf, int len );
	// 处理报文
	BOOL ProcessRecvBuf ( BYTE *buf, int len );

	private:
	//改变发送位置
	void ChangeSendPos( void );
	//获取发送位置
	BYTE GetSendPos( void );
	private:
		/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;
	BYTE m_bySendPos;

}; /* -----  end of class CModbusEPS  ----- */

#endif   /* ----- #ifndef MODBUSEPS_INC  ----- */
