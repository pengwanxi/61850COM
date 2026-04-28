/*
 * =====================================================================================
 *
 *       Filename:  ModBusLiuLiangJi.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2016年05月20日 09时45分18秒
 *       Revision:  none
 *       Compiler:  
 *
 *         Author:  ykk 
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */


#ifndef  MODULE_LIULIANGJI_INC
#define  MODULE_LIULIANGJI_INC

#include "CProtocol_ModBusMaster.h"

/*
 * =====================================================================================
 *        Class:  ModBusLiuLiangJi
 *  Description:  xH 
 * =====================================================================================
 */
class ModBusLiuLiangJi : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		ModBusLiuLiangJi ();                             /* constructor      */
		~ModBusLiuLiangJi ();                            /* destructor       */

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
	BOOL WhetherBufValue ( BYTE *buf, int &len ,int &pos);
	// 处理报文
	BOOL ProcessRecvBuf ( BYTE *buf, int len );
	//打印信息
	void print( char *buf ) const;
	private:
		/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;


}; /* -----  end of class ModBusLiuLiangJi  ----- */

#endif   /* ----- #MODULE_LIULIANGJI_INC  ----- */
