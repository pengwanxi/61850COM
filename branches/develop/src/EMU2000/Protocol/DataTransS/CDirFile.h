/*
 * =====================================================================================
 *
 *       Filename:  CDirFile.h
 *
 *    Description:	关于数据传输协议文件操作 
 *
 *        Version:  1.0
 *        Created:  2015年07月01日 15时21分56秒
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

#ifndef  CDIRFILE_INC
#define  CDIRFILE_INC

#include "../../share/typedef.h"

/*
 * =====================================================================================
 *        Class:  CDirFile
 *  Description:  
 * =====================================================================================
 */
class CDirFile
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CDirFile ();                             /* constructor      */
		~CDirFile ();                            /* destructor       */

		//创建文件夹
		BOOL CreateDir( char *pszPath );
		//是否有该文件夹
		BOOL IsDir( char *pszPath );
		//是否有该文件
		BOOL IsFile ( char *pszFileName );
		//写入文件
		DWORD WriteToFile ( char *pszFileName, BYTE *pszBuf, int len);
		//从文件读出
		DWORD ReadFromFile (  char *pszFileName, BYTE *pszBuf, int len , DWORD dwReadPos);
		//获取相应文件
		char *GetDirFile( char *pPath );
		//获取最新的文件
		char *GetLatestDriFile ( char *pszPath );
		//获取最旧的文件
		char *GetOldestDriFile ( char *pszPath );
		//删除文件
		BOOL DeleteFile( char *pszFileName );
		//删除最旧的文件
		BOOL DeleteOldestFile( char *pszPath );
		//读文件夹大小
		DWORD GetDirSize ( char *pszPath );


	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	private:

		DWORD m_dwDirSize;
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CDirFile  ----- */

#endif   /* ----- #ifndef CDIRFILE_INC  ----- */
