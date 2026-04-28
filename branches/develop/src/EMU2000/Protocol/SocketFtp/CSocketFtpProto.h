/*
 * =====================================================================================
 *
 *       Filename:  CSocketFtpProto.h
 *
 *    Description:  协议类 
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时55分51秒
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

#ifndef  CSOCKETFTPPROTO_INC
#define  CSOCKETFTPPROTO_INC

#include "../../share/typedef.h"
#include "CSocketFtpFile.h"
#include "../../share/md5.h"

#define					FTP_MAX_SEND_LEN			1028			/* 最大发送长度 */

/*
 * =====================================================================================
 *        Class:  CSocketFtpProto
 *  Description:  
 * =====================================================================================
 */
class CSocketFtpProto
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CSocketFtpProto ();                             /* constructor      */
		~CSocketFtpProto ();                            /* destructor       */

	public:
		//处理协议报文
		BOOL ProcessProtoSendBuf ( BYTE *buf, int len );
		//初始化协议
		BOOL Init ( void  );


	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	private:

		//设置文件名字
		void SetFileName ( const char *pchFileName );

		//设置错误位
		void SetErrorBit ( void );
		//设置回复位
		void SetResponseBit ( void );
		// 设置文件传送位 
		void SetFileBit ( void );
		// 是否是文件装态
		BOOL isFileState ( void ) const;
		//设置上传下载位
		void SetDownLoadBit ( void );
		//是否是上传装态
		BOOL IsLoad ( void ) const;
		// 设置功能位 
		void SetFuncCode ( BYTE byFunc );
		// 获得功能码 
		BYTE GetFuncCode ( void ) const;
		//增加文件头
		void AddFrameHead ( void );

		//是否符合帧格式
		BOOL IsFrameFormat ( BYTE *buf, int len );
		//处理装态字节
		BOOL ProcessStateByte ( BYTE byState );
		//处理上传数据
		BOOL ProcessLoadData ( BYTE *buf, int len );
		//处理下载数据
		BOOL ProcessDownData ( BYTE *buf, int len );
		//处理上传数据 开始传输发送
		BOOL PLD_BeginTrans ( void );
		//处理上传数据1 文件列表信息发送
		BOOL PLD_FileListInfo ( void );
		//处理上传数据1 文件信息发送
		BOOL PLD_FileInfo ( void );
		//处理上传数据文件
		BOOL PLD_File ( void );
		//处理上传数据md5
		BOOL PLD_MD5 ( void );
		//处理上传数据15
		BOOL PLD15 ( void );
		//处理上传数据6
		BOOL PLD6 ( void );
		//处理下载数据结束传输
		BOOL PDD_endTrans ( void );
		//处理下载数据文件列表
		BOOL PDD_FileListInfo ( void );
		//处理下载数据文件
		BOOL PDD_FileInfo ( void );
		//处理下载数据6
		BOOL PDD6 ( const char *chMd5Buf );


		/* ====================  DATA MEMBERS  ======================================= */

	public:
		/* ====================  DATA MEMBERS  ======================================= */
		BYTE m_bySendBuf[ FTP_MAX_SEND_LEN ];
		WORD m_wSendLen;
		CSocketFtpFile m_FtpFile;
		BOOL m_bReboot;                         /* 是否重启 */

	private:
		/* ====================  DATA MEMBERS  ======================================= */
		char m_chFileName[256];
		DWORD m_dwFileSize;

		FTP_FILE_TYPE m_FileType;
		UINT m_uiReadPos;
		WORD m_wFileNum;

		enum m_eFuncCode                        /* 功能码 */
		{
			NONE_FUNC = 0,                      /* 无效的功能 */
			START_TRANS = 1,                    /* 开始传输 */
			END_TRANS = 2,                      /* 结束传输 */
			FILE_LIST_INFO = 3,                 /* 文件列表信息 */
			FILE_INFO = 4,                      /* 文件信息 */
			START_DOWN = 5,                     /* 开始下载 */
			MD5 = 6,                            /* md5 */
			REBOOT = 7,                         /* 重启 */
			RECV_CONFIG = 15                    /* 确认接收 */
		};


}; /* -----  end of class CSocketFtpProto  ----- */


#endif   /* ----- #ifndef CSOCKETFTPPROTO_INC  ----- */
