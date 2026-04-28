/*
 * =====================================================================================
 *
 *       Filename:  CProtocol_DataTrans.h
 *
 *    Description:  关于ESD自定义历史数据上传 
 *
 *        Version:  1.0
 *        Created:  2015年06月09日 14时22分18秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp
 *   Organization:  
 *
 *		  history:	Time						Author			version			Description
 *					2015-06-09 14:25:52         mengqp			1.0				created
 *
 * =====================================================================================
 */


#ifndef  CPROTOCOL_DATATRANS_INC
#define  CPROTOCOL_DATATRANS_INC

/* #####   HEADER FILE INCLUDES   ################################################### */

#include "../../share/CMethod.h"
#include "../../share/Rtu.h"

/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ################################### */
#define		DATATRANSSPREFIXFILENAME			"/mynand/config/DataTrans/"	/* 104锟教讹拷路锟斤拷 */

// #define		ESD_DATATRANS_PRINT			/* 终端打印 */
//#define		ESD_DATATRANS_DEBUG			/* UDP端口打印 */   //undefined by cyz!

#define		DATATRANS_MAX_YC_NUM			4096			/* 最大遥测数量 */
#define		DATATRANS_MAX_YX_NUM			8192			/* 最大遥信数量 */
#define		DATATRANS_MAX_YM_NUM			1024			/* 最大遥脉数量 */
#define		DATATRANS_MAX_BUF_LEN			256				/* 最大缓冲数量 */
#define		DATATRANS_MAX_SEND_COUNT		3				/* 最大发送次数 */




#define			DATATRANS_LINK_STATE			0x00000001		/* 链接 */
#define			DATATRANS_RESEND_STATE			0x00000002		/* 重发 */
#define			DATATRANS_YC_STATE				0x00000004		/*  YC*/
#define			DATATRANS_YC_OVER_STATE			0x00000008		/*  YC结束*/
#define			DATATRANS_YX_STATE				0x00000010		/*  YX*/
#define			DATATRANS_YX_OVER_STATE			0x00000020		/*  YX结束*/
#define			DATATRANS_YM_STATE				0x00000040		/*  YM*/
#define			DATATRANS_YM_OVER_STATE			0x00000080		/*  YM 结束*/
#define			DATATRANS_CHANGE_YX_STATE		0x00000100		/*  变化YX*/
#define			DATATRANS_HEARTBEAT_STATE		0x00000200		/*  心跳 */
#define			DATATRANS_WRITEFILE_STATE		0x00000400		/*  写文件 */
#define			DATATRANS_WRITEFILE_OVER_STATE	0x00000800		/*  写文件结束 */
#define			DATATRANS_WRITEFILE_HEAD_STATE	0x00001000		/*  写文件HEAD */
#define			DATATRANS_WRITEFILE_YC_STATE	0x00002000		/*  写文件YC */
#define			DATATRANS_WRITEFILE_YX_STATE	0x00004000		/*  写文件YX */
#define			DATATRANS_WRITEFILE_YM_STATE	0x00008000		/*  写文件YM */

/* #####   TYPE DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ######################### */
//定义历史传送数据的类型
typedef enum 
{
	ESD_DATATRANS_YC_DATATYPE,
	ESD_DATATRANS_YX_DATATYPE,
	ESD_DATATRANS_YM_DATATYPE
}ESD_DATATRANS_DATATYPE;				/* ----------  end of enum ESD_DATATRANS_DATATYPE  ---------- */

/* #####   DATA TYPES  -  LOCAL TO THIS SOURCE FILE   ############################### */

/*
 * =====================================================================================
 *        Class:  CProtocol_DataTrans
 *  Description:  自定义历史数据类
 * =====================================================================================
 */
class CProtocol_DataTrans : public CRtuBase
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CProtocol_DataTrans ();                             /* constructor      */
		virtual ~CProtocol_DataTrans ();                            /* destructor       */

		/* ====================  METHODS	   ======================================= */
		//获取协议报文
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		//处理协议报文
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		//初始化总线模块
		virtual BOOL Init( BYTE byLineNo  ) ;
		//时间处理函数
		virtual void TimerProc(){ return;  } 
		//广播消息处理
		virtual BOOL BroadCast( BYTE * buf , int &len ){return FALSE;} 
		//判断报文合理性
		virtual BOOL WhetherBufValid( const BYTE *buf, int &len, int &pos );

		/* ====================  MUTATORS      ======================================= */

		/* ====================  OPERATORS     ======================================= */


	protected:
		/* ====================  METHODS  ======================================= */
		// 设置状态 
		void SetState ( DWORD dwState );
		// 取消状态
		void UnsetState ( DWORD dwState );
		// 查看是否有该类型
		BOOL IsHaveState ( DWORD dwState ) const;
		// 打开链接
		void OpenLink ( void );
		// 关闭链接
		void CloseLink ( void );
		
		/* ====================  DATA MEMBERS  ======================================= */
		char m_sMasterAddr[ 200 ] ;//网络参数保存
		char m_szPrintBuf[256];
		void print( const char *szBuf );   //内部打印函数
		DWORD m_ProtocolState;

	private:
		/* ====================  METHODS	   ======================================= */
		//获取总线中的装置数据
		BOOL GetDevData( void ) ;
		//处理配置文件数据
		BOOL ProcessFileData( CProfile &profile );
		//创建模块
		BOOL CreateModule( int iModule ,
				char * sMasterAddr ,
				int iAddr , 
				char * sName , 
				char * stplatePath);
		//初始化模块参数
		BOOL InitModule( CProtocol_DataTrans * pProtocol ,
				int iModule , 
				char * sMasterAddr ,
				int iAddr ,
				char * sName ,
				char * stplatePath );
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CProtocol_DataTrans  ----- */


#endif   /* ----- #ifndef CPROTOCOL_DATATRANS_INC  ----- */
