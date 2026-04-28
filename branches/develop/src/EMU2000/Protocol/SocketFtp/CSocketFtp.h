/*
 * =====================================================================================
 *
 *       Filename:  CSocketFtp.h
 *
 *    Description:  利用socket 模拟ftp进行处理配置
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时53分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */

#ifndef  CSOCKETFTP_INC
#define  CSOCKETFTP_INC


/* #####   HEADER FILE INCLUDES   ################################################### */
#include "../../share/CTcpPortServer.h"
#include "CGroupBroadProto.h"
#include "CSocketFtpProto.h"



/* #####   LOCAL CLASS DEFINITIONS   ################################################ */

/*
 * =====================================================================================
 *        Class:  CSocketFtp
 *  Description:  关于socket ftp的类
 * =====================================================================================
 */
class CSocketFtp
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CSocketFtp ();                             /* constructor      */
		~CSocketFtp ();                            /* destructor       */

	public:
		//初始化协议
		BOOL Init ( void );
		//线程
		BOOL CreateThread ( void );

	private:
		//创建tcp服务器
		BOOL CreateTcpServer ( void );
		//创建协议
		BOOL CreateProto ( void );

		//创建组播协议及线程
		BOOL CreateGroupBroad ( void );

	public:
		/* ====================  DATA MEMBERS  ======================================= */
		CTcpPortServer *m_pPort;
		CSocketFtpProto *m_pProto;
		CGroupBroadProto *m_pGroupProto;        /* 組播類 */

		pthread_t  m_pthread_id;
		pthread_t  m_groupbroad_id;

	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  DATA MEMBERS  ======================================= */
		

	private:
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CSocketFtp  ----- */





#endif   /* ----- #ifndef CSOCKETFTP_INC  ----- */
