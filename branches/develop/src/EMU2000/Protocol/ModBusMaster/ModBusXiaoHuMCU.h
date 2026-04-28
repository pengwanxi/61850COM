/*
 * =====================================================================================
 *
 *       Filename:  ModBusXiaoHuMCU.h
 *
 *    Description:  消弧控制器 .cpp有大概相应说明
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


#ifndef  MODBUSXIAOHUMCU_INC
#define  MODBUSXIAOHUMCU_INC

#include "CProtocol_ModBusMaster.h"

#define		XHMCU_NONE_TROUBLE_DATATYPE		1			/* 无故障 */
#define		XHMCU_TROUBLE_DATATYPE			2			/*  故障*/



#define		XHMCU_PRINT						1			/* 是否打印 屏蔽此行 不打印非报文信息 */

/*
 * =====================================================================================
 *        Class:  CModBusXHMCU
 *  Description:  xH 
 * =====================================================================================
 */
class CModBusXHMCU : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CModBusXHMCU ();                             /* constructor      */
		~CModBusXHMCU ();                            /* destructor       */

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
	//计算和校险
	WORD GetWordSumCheck ( BYTE *pBuf, int len  );
	BYTE GetSumCheck( BYTE * pBuf , int len );
	//处理故障数据
	BOOL ProcessTroubleData( BYTE *buf, int len );
	//处理非故障数据
	BOOL ProcessNoneTroubleData( BYTE *buf, int len );
	//打印信息
	void print( char *buf ) const;
	private:
		/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;
	BYTE m_bySrcAddr;
	BYTE m_byDataType;

}; /* -----  end of class CModBusXHMCU  ----- */

#endif   /* ----- #ifndef MODBUSXIAOHUMCU_INC  ----- */
