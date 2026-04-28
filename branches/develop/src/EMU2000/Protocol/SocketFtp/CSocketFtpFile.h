/*
 * =====================================================================================
 *
 *       Filename:  CSocketFtpFile.h
 *
 *    Description:  关于socket ftp 文件的处理类 
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时56分26秒
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

#ifndef  CSOCKETFTPFILE_INC
#define  CSOCKETFTPFILE_INC

/* #####   HEADER FILE INCLUDES   ################################################### */
#include "../../share/md5.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h> 
#include <sys/stat.h> 
#include <stdlib.h>
#include <ftw.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>


/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ################################### */

#define			FTP_FILE_LIST				(char *)"/mynand/filelist.tmp"			/* 临时的文件列表 */
#define			FTP_DOWN_CFG_FILE			(char *)"/mynand/downprgm/"				/* 配置文件 */
#define			FTP_DOWN_PRGM_FILE			(char *)"/mynand/downprgm/"				/* 程序文件 */
#define			FTP_DOWN_USER_FILE			(char *)"/myapp/downuser/user.sh"						/* user.sh */


typedef enum _FTP_FILE_TYPE 
{
	FTP_NONE_TYPE,
	FTP_CFG_TYPE,
	FTP_PRGM_TYPE
}FTP_FILE_TYPE;				/* ----------  end of enum _FTP_FILE_TYPE  ---------- */

/* #####   TYPE DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ######################### */

/* #####   DATA TYPES  -  LOCAL TO THIS SOURCE FILE   ############################### */

/*
 * =====================================================================================
 *        Class:  CSocketFtpFile
 *  Description:  关于文件的处理
 * =====================================================================================
 */
class CSocketFtpFile
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CSocketFtpFile ();                             /* constructor      */
		~CSocketFtpFile ();                            /* destructor       */

	public:
		/* ====================  Method		   ======================================= */
		//写数据
		unsigned int WriteFile ( char *pchFileName, 
				unsigned char *puchBuf,
				int iLen);
		//读文件
		unsigned int ReadFile ( char *pchFileName,
				unsigned char *puchBuf, 
				int iLen,
				unsigned int &uiReadpos) ;

		// 创建文件目录
		int CreateDir ( const char *pchDirPath );
		//获取文件大小
		unsigned long GetFileSize (  const char *cpchFileName);
		//目录内所有文件包括子文件的大小
		unsigned int GetDirSize ( const char *pchDirPath );
		//查看文件是否存在
		bool IsFileExist ( const char *cpchFileName );
		//查看文件夹是否存在
		bool IsDirExist ( const char *cpchDirPath );
		//查看是否是特殊目录
		bool IsSpecialDir ( const char *cpchDirPath );
		//获取完整路径
		void GetTotalFilePath ( const char *cpchPath, 
				const char *cpchFileName,
				char *cpchFilePath);
		// 修改文件权限
		bool ChangeFileMode ( char *pszFileName,
				int imode );
		// 修改目录内所有文件权限
		bool ChangeDirFilesMode ( char *pchDirPath, 
				int imode );
		//删除目录内所有文件
		bool DeleteDirFiles ( const char *pchDirPath );
		//移动文件或目录
		bool MoveDirFiles ( char *pchSrcPath, char *pchDirPath );
		//写文件夹中所有文件
		bool WriteFileListFile (  char *pchDirPath, char *pchFileName );
		//获得第n行数据
		char * GetFileLineBuf ( char *pchFileName, 
				int iLine,
				char *pchLineBuf );

	public:
		//检查文件列表 
		bool CheckFileList (void  );
		//获得下载的文件类型
		FTP_FILE_TYPE GetDownType ( char *pchFileName );
		//获得下载的文件名字
		char * GetDownFileName ( FTP_FILE_TYPE fType,
				char *pchFileName );

		//更新配置
		bool UpdateCfg ( void );
		//更新程序
		bool UpdatePrgm ( void );

	protected:
		/* ====================  DATA MEMBERS  ======================================= */
	private:
		//备份文件
		bool BakDirFiles ( char* pchSrcPath,
				char *pchDirPath );
	private:
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CSocketFtpFile  ----- */


/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ################################ */

/* #####   PROTOTYPES  -  LOCAL TO THIS SOURCE FILE   ############################### */

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################ */

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ##################### */







#endif   /* ----- #ifndef CSOCKETFTPFILE_INC  ----- */
