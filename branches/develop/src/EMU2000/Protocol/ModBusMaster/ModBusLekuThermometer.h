/*
 * =====================================================================================
 *
 *       Filename:  ModBusLekuThermometer.h
 *
 *    Description:  上海乐库变压器用电子温度计
				 .cpp有大概相应说明
 *
 *        Version:  1.0
 *        Created:  2016年04月21日 09时45分18秒
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


#ifndef  MODULE_LEKUTHERMOMETER_INC
#define  MODULE_LEKUTHERMOMETER_INC

#include "CProtocol_ModBusMaster.h"

/*
 * =====================================================================================
 *        Class:  ModBusLekuThermometer
 *  Description:  xH 
 * =====================================================================================
 */
class ModBusLekuThermometer : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		ModBusLekuThermometer ();                             /* constructor      */
		~ModBusLekuThermometer ();                            /* destructor       */

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


}; /* -----  end of class ModBusLekuThermometer  ----- */

#endif   /* ----- #MODULE_LEKUTHERMOMETER_INC  ----- */
