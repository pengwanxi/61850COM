/*
 * =====================================================================================
 *
 *       Filename:  Dlt645_SVT4_2007.h
 *
 *    Description:  dlt645 2007版本协议
 *
 *        Version:  1.0
 *        Created:  2014年11月10日 14时12分46秒
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

#ifndef  DLT645_SVT4_2007_INC
#define  DLT645_SVT_2007_INC

#include "CProtocol_Dlt645.h"


/*
 * =====================================================================================
 *        Class:  CDlt645_SVT4
 *  Description:  
 * =====================================================================================
 */
class CDlt645_SVT4 : public CProtocol_Dlt645
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CDlt645_SVT4 ();                             /* constructor      */
		~CDlt645_SVT4 ();                            /* destructor       */
		//时间处理函数
		virtual void    TimerProc( void );
		//初始化协议数据
		virtual BOOL Init( BYTE byLineNo );
		//获取协议数据缓存
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
		//处理收到的数据缓存 
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		//更新通讯状态 
		virtual BOOL GetDevCommState( void ) ;
		//请求读电表数据
		virtual BOOL RequestReadData( BYTE *buf, int &len );
		//处理遥测数据
		virtual BOOL ProcessYcData( const BYTE *buf, int len );
		//处理遥脉数据
		virtual BOOL ProcessYmData( const BYTE *buf, int len );
		//处理遥信数据
		virtual BOOL  ProcessYxData(const BYTE *buf, int len);
		//对时报文
		virtual BOOL TimeSync( BYTE *buf, int &len );
		
	public:
		/* ====================  METHODS  ============================================ */


	public:
	protected:
		//获取装置名称为本地地址
		BOOL GetDevNameToAddr( void );
		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  METHODS  ============================================ */
		//是否对时
		BOOL IsTimeToSync( void );
		//初始化协议状态数据
		BOOL InitProtocolStatus( void );
		//获取报文
		BOOL GetSendBuf( BYTE *buf, int &len );
		//处理报文
		BOOL ProcessBuf( const BYTE *buf, int len );
	
	private:
		BOOL m_bLinkTimeSyn;	//当链接上时对时一次
		BOOL m_bLinkStatus;		//链接状态
		BOOL m_bIsSending;		//是否正在发送
		BOOL m_bIsReSend;		//是否重发
		BOOL m_bIsNeedResend;	//是否需要重发
		BOOL m_bTimeSynFlag;	//对时标识


		BYTE m_byResendCount;			//重发计数
		BYTE m_byReSendBuf[DLT645_MAX_BUF_LEN];//重发缓存
		BYTE m_byReSendLen;				//重发缓存长度

		BYTE m_byRecvErrorCount;        //接收错误计数

		int m_dayflag;//时间 天的标记
		BYTE m_firstsendflag;//周期内第一次发送的标记
		int num ;
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CDlt645_SVT4  ----- */

#endif   /* ----- #ifndef DLT645_2007_INC  ----- */
