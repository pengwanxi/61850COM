/*
 * =====================================================================================
 *
 *       Filename:  Cjt188_2004.h
 *
 *    Description:  Cjt188_2004版本协议
 *
 *        Version:  1.0
 *        Created:  2015年03月12日 10时30分56秒 
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

#ifndef  CJT188_2007_INC
#define  CJT188_2007_INC

#include "CProtocol_Cjt188.h"


/*
 * =====================================================================================
 *        Class:  CCjt188_2004
 *  Description:  
 * =====================================================================================
 */
class CCjt188_2004 : public CProtocol_Cjt188
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CCjt188_2004 ();                             /* constructor      */
		~CCjt188_2004 ();                            /* destructor       */
		//时间处理函数
		virtual void    TimerProc( void );
		//初始化协议数据
		virtual BOOL Init( BYTE byLineNo );
		//获取协议数据缓存
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
		//处理收到的数据缓存 
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
		//更新通讯状态 
		virtual BOOL GetDevCommState( void ) ;
		
	public:
		/* ====================  METHODS  ============================================ */


	public:
	protected:
		/* ====================  DATA MEMBERS  ======================================= */
		//请求读电表数据
		virtual BOOL RequestReadData( BYTE *buf, int &len );
		//10-19h
		virtual BOOL ProcessDataT1 ( const BYTE *buf, int len  );
		//20-29h
		BOOL ProcessDataT2 ( const BYTE *buf, int len  );
		//30-39h
		BOOL ProcessDataT3 ( const BYTE *buf, int len  );
		//处理计量数据
		virtual BOOL ProcessReadData( const BYTE *buf, int len );
		//对时报文
		virtual BOOL TimeSync( BYTE *buf, int &len );
		//初始化发送信息
		virtual void InitSendCfgInfo( void );

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
		//获取装置名称为本地地址
		BOOL GetDevNameToAddr( void );
	
	private:
		BOOL m_bLinkTimeSyn;	//当链接上时对时一次
		BOOL m_bLinkStatus;		//链接状态
		BOOL m_bIsSending;		//是否正在发送
		BOOL m_bIsReSend;		//是否重发
		BOOL m_bIsNeedResend;	//是否需要重发
		BOOL m_bTimeSynFlag;	//对时标识


		BYTE m_byResendCount;			//重发计数
		BYTE m_byReSendBuf[CJT188_MAX_BUF_LEN];//重发缓存
		BYTE m_byReSendLen;				//重发缓存长度

		BYTE m_byRecvErrorCount;        //接收错误计数
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CCjt188_2004  ----- */

#endif   /* ----- #ifndef CJT188_2004_INC  ----- */
