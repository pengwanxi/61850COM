/*
 * =====================================================================================
 *
 *       Filename:  ModBusDSE7320FDJ.h
 *
 *    Description:  俞乾现场发过来的DSE7320 Modbus 通讯地址点表.xls协议  项目是贵州松桃高速项目
 *					具体协议查看该文档
 *
 *        Version:  1.0
 *        Created:  2014年12月17日 09时55分18秒
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

#ifndef  MODBUSDSE7320FDJ_INC
#define  MODBUSDSE7320FDJ_INC


#include "CProtocol_ModBusMaster.h"


#define				MODBUSMASTER_DSE7320FDJ_MAX_POS			8		/* 最大位置 */
#define				MODBUSMASTER_DSE7320FDJ_YX_DATATYPE		1		/* 遥信数据 */
#define				MODBUSMASTER_DSE7320FDJ_YC_DATATYPE		2		/* 遥测数据类型 */

/*
 * =====================================================================================
 *        Class:  CModbusDSE7320FDJ
 *  Description:  ups和eps通讯类 
 * =====================================================================================
 */
class CModbusDSE7320FDJ : public CProtocol_ModBusMaster
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CModbusDSE7320FDJ ();                             /* constructor      */
		~CModbusDSE7320FDJ ();                            /* destructor       */

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
	// 处理遥测报文
	BOOL ProcessYcBuf( BYTE *buf, int len );
	// 处理报文
	BOOL ProcessRecvBuf ( BYTE *buf, int len );

	// 获取发送报文
	BOOL GetSendBuf( BYTE *buf, int &len );

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

	BYTE m_byDataType;
	BYTE m_byYcDealFlag;
	BYTE m_byYxDealFlag;

}; /* -----  end of class CModbusDSE7320FDJ  ----- */

#endif   /* ----- #ifndef MODBUSDSE7320FDJ_INC  ----- */
