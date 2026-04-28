/*
 * =====================================================================================
 *
 *       Filename:  CGroupBroadProto.h
 *
 *    Description:  组播协议协议
 *
 *       Compiler:  gcc
 *
 *        Version:  1.0
 *        Created:  2015年10月13日 16时55分00秒
 *
 *         Author:  mengqp
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */

#ifndef  CGROUPBROADPROTO_INC
#define  CGROUPBROADPROTO_INC

#include "../../share/typedef.h"


#define			GROUP_BROAD_PROTO_MAX_LEN		1024		/*  */

/*
 * =====================================================================================
 *        Class:  CGroupBroadProto
 *  Description:  
 * =====================================================================================
 */
class CGroupBroadProto
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CGroupBroadProto ();                             /* constructor      */
		~CGroupBroadProto ();                            /* destructor       */

	public:
		// 处理接收到的协议报文
		BOOL ProcessProtoData( BYTE *buf, int len );

		//获得一个网线状态的端口 从eth0 - eht4 依次获得
		BYTE GetOneNetPort( void );
		// 设置该端口加入route 其它删除route
		void SetRoutePort( BYTE byPort );

	private:
		// 获得校验和
		BYTE GetCheckByte( BYTE *buf, int len );
		// 校验帧格式	
		BOOL CheckFrameFormat( BYTE *buf, int len );
		// 处理数据帧
		BOOL ProcessDataFrame( BYTE *buf, int len );
		// 处理询问ip数据 
		BOOL PDF_AskIp( BYTE *buf, int len );
		// 设置时间标识
		void SetTimeFlag( long lTimeFlag );
		// 校验时间标识
		BOOL CheckTimeFlag( long lTimeFlag );
		// 获得回应askip的报文
		BOOL GetAnsIpFrame( void );
		// 获得网卡数量
		BYTE GetNetCardNum(  );
		// 获得网线状态
		BOOL GetNetState ( const char *if_name, int phy_id, int reg_num );
		// 获得网卡ip和状态
		BOOL GetNetIpAndState(const BYTE byNetCard, char *pchBuf, BOOL &bState );
		// IP地址转BYTE报文
		void IpToByte( char *pchBuf, BYTE *byIpBuf );
		// 设置ip状态
		void SetIpState( BYTE &byStateByte , BOOL bState, BYTE byNetCard );
		// 添加固定帧格式
		void AddFrameFormat( BYTE *byDataBuf, WORD wDataLen );

	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	public:
		BYTE m_bySendBuf[GROUP_BROAD_PROTO_MAX_LEN]; /* 发送缓冲 */
		WORD m_wSendLen;                        /* 发送长度 */
	private:
		long m_lTimeFlag;                       /* 每次接收报文时唯一时间标识，若接收时间与此相同，则不处理 */
		enum m_eFrameByte                       /* 数据位 */
		{
			HEAD_START = 0,                     /* 0x68 数据头 */
			HEAD_HILEN,                         /* 长度位 高 */
			HEAD_LOLEN,                         /* 长度位 低 */
			HEAD_RESTART,                       /* 0x68 数据头 */
			DATA_FUNC                           /* 功能码 */
		};

		enum m_eFuncCode                        /* 功能码 */
		{
			FUNC_ASK_IP = 1	
		};

		BYTE m_RoutePort;                        /* 要设置的路由端口 */
		BOOL m_IsAddRoute[4];                    /* 要添加的route */

		char m_chPrjName[32];                   /* 工程名 */

		int m_iSocketFd;                        /* 文件套接字 */

		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CGroupBroadProto  ----- */


#endif   /* ----- #ifndef CGROUPBROADPROTO_INC  ----- */
