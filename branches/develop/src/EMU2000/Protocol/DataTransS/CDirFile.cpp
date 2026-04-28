/*
 * =====================================================================================
 *
 *       Filename:  CDirFile.cpp
 *
 *    Description:	关于数据传输协议文件操作
 *
 *        Version:  1.0
 *        Created:  2015年07月01日 15时21分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp
 *   Organization:
 *
 *		  history:
 * =====================================================================================
 */

#include "CDirFile.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <ftw.h>
#include <sys/time.h>
#include <sys/resource.h>

static DWORD sdwDirSize;
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  CalDirSize
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
static int CalDirSize ( const char *pszPath,
		const struct stat *s,
		int typeflag)
{
	sdwDirSize += s->st_size;
	return 0;
}		/* -----  end of method CDirFile::CalDirSize  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  CDirFile
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDirFile::CDirFile ()
{
	struct rlimit r;
	r.rlim_cur = 10240;
	r.rlim_max = 10240;
	if( setrlimit( RLIMIT_NOFILE, &r ) < 0 )
	{
		printf ( "CDirFile setrlimit error\n" );
	}

	system( "ulimit -n 102400" );

	sleep( 1 );
}  /* -----  end of method CDirFile::CDirFile  (constructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  ~CDirFile
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDirFile::~CDirFile ()
{
}  /* -----  end of method CDirFile::~CDirFile  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  CreateDir
 * Description:  创建文件夹
 *       Input:  pszPath 文件夹路径名称
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDirFile::CreateDir ( char *pszPath )
{
	if( IsDir( pszPath ) )
	{
		printf ( "%s is exist\n", pszPath );
		return TRUE;
	}

	//创建权限为755的文件夹
	if( 0 == ( mkdir( pszPath, 0755 ) ) )
	{
		printf ( "CreateDir %s OK\n", pszPath );
		return TRUE;
	}

	printf ( "CreateDir %s Error\n", pszPath );
	return FALSE;
}		/* -----  end of method CDirFile::CreateDir  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  IsDir
 * Description:  检测文件夹是否存在
 *       Input:  pszPath文件夹路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDirFile::IsDir ( char *pszPath )
{
	if( NULL == pszPath )
	{
		return FALSE;
	}

	struct stat statbuf;
	//lstat返回文件的信息，文件信息存放在stat结构中
	if(lstat(pszPath, &statbuf) ==0)
	{
		//S_ISDIR宏，判断文件类型是否为目录
		return S_ISDIR(statbuf.st_mode) != 0;
	}
	return FALSE;
}		/* -----  end of method CDirFile::IsDir  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  WriteToFile
 * Description:	 将数据追加写入文件
 *       Input:  pszFileName:文件名字 带路径
 *				 pszBuf:要写入的数据
 *				 len:要写入数据的长度
 *		Return:  成功：数量
 *				 失败：0
 *--------------------------------------------------------------------------------------
 */
DWORD CDirFile::WriteToFile ( char *pszFileName,
		BYTE *pszBuf,
		int len )
{
	FILE *pFile = NULL;
	DWORD dwFileLen;

	//二进制打开文件
	pFile = fopen( pszFileName, "ab+" );
	if( NULL == pFile )
	{
		char szBuf[256];
		sprintf( szBuf, "WriteToFile %s:", pszFileName );
		perror( szBuf );
		return 0;
	}

	//写入数据
	dwFileLen = fwrite( pszBuf, 1, len, pFile  );

	fclose( pFile );

	return dwFileLen;
}		/* -----  end of method CDirFile::WriteToFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  ReadFromFile
 * Description:  从文件的相应位置读出数据
 *       Input:  pszFileName:文件名字 带路径
 *				 pszBuf:要写入的数据
 *				 len:要写入数据的长度
 *		Return:  成功：数量
 *				 失败：0
 *--------------------------------------------------------------------------------------
 */
DWORD CDirFile::ReadFromFile ( char *pszFileName,
		BYTE *pszBuf,
		int len,
		DWORD dwReadPos)
{
	FILE *pFile = NULL;
	DWORD dwFileLen=0;

	//二进制打开文件
	pFile = fopen( pszFileName, "rb" );
	if( NULL == pFile )
	{
		char szBuf[256];
		sprintf( szBuf, "ReadFromFile %s:", pszFileName );
		perror( szBuf );
		return 0;
	}

	if( -1 == fseek( pFile, dwReadPos, SEEK_SET ) )
	{
		fclose( pFile );
		return 0;
	}

	//读出数据
	dwFileLen = fread( pszBuf, 1, len, pFile  );

	fclose( pFile );

	return dwFileLen;
}		/* -----  end of method CDirFile::ReadFromFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  IsFile
 * Description:  是否有该文件
 *       Input:  pszFileName : 文件路径名字
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDirFile::IsFile ( char *pszFileName )
{
	struct dirent    *dp;
	DIR              *dfd;
	char szPath[128] = "";
	char szFile[64] = "";

	if( NULL == pszFileName )
	{
		return FALSE;
	}

	char *pszFile = strrchr( pszFileName, '/' );
	if( NULL == pszFile )
	{
		//当前文件夹下的文件
		strcpy( szPath, "./" );
		strcpy( szFile, pszFileName );
	}
	else
	{
		//去掉'/' 复制
		strcpy( szFile, pszFile + 1 );
		//只复制前面的部分的长度
		int iPathLen = strlen( pszFileName ) -  strlen( pszFile ) + 1;
		strncpy( szPath, pszFileName, iPathLen );
	}

	dfd = opendir( szPath );
	if(  NULL == dfd )
	{
		char szBuf[256];
		sprintf( szBuf, "IsFile %s:", pszFileName );
		perror( szBuf );
		return FALSE;
	}

	for(dp = readdir(dfd); NULL!=dp; dp = readdir(dfd))
	{
		// printf ( "dp->d_name=%s  %s\n", dp->d_name, szFile );
		if(  0 == strcmp(dp->d_name, szFile) )
		{
			closedir(dfd);
			return TRUE;
		}
	}

	closedir(dfd);

	return FALSE;
}		/* -----  end of method CDirFile::IsFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  GetLatestDriFile
 * Description:  获取最新的文件(文件名)
 *       Input:  pszPath:路径
 *		Return:  成功：文件名
 *				 失败：NULL
 *--------------------------------------------------------------------------------------
 */
char * CDirFile::GetLatestDriFile ( char *pszPath )
{
	struct dirent    *dp ;
	DIR              *dfd;
	char *pRtn = NULL;
	struct stat statbuf, statTmp;
	char szFileName[128];

	if( !IsDir(pszPath) )
	{
		return NULL;
	}

	if( (dfd = opendir(pszPath)) == NULL )
	{
		char szBuf[256];
		sprintf( szBuf, "GetLatestDriFile %s:", pszPath );
		perror( szBuf );
		return NULL;
	}

	for(dp = readdir(dfd); NULL!=dp; dp = readdir(dfd))
	{
		if( NULL != strstr( dp->d_name, ".log" ) )
		{
			sprintf( szFileName, "%s/%s", pszPath, dp->d_name );
			//lstat返回文件的信息，文件信息存放在stat结构中
			if(lstat(szFileName, &statbuf) ==0)
			{
				if( NULL == pRtn )
				{
					pRtn = dp->d_name;
					memcpy( &statTmp, &statbuf, sizeof( struct stat ) );
				}

				if( statbuf.st_mtime >= statTmp.st_mtime )
				{
					pRtn = dp->d_name;
					memcpy( &statTmp, &statbuf, sizeof( struct stat ) );
				}
			}

		}
	}

	closedir(dfd);
	return pRtn;
}		/* -----  end of method CDirFile::GetLatestDriFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  GetOldestDriFile
 * Description:  获取最旧的文件(文件名)
 *       Input:  pszPath:路径
 *		Return:  成功：文件名
 *				 失败：NULL
 *--------------------------------------------------------------------------------------
 */
char * CDirFile::GetOldestDriFile ( char *pszPath )
{
	struct dirent    *dp ;
	DIR              *dfd;
	char *pRtn = NULL;
	struct stat statbuf, statTmp;
	char szFileName[128];

	if( !IsDir(pszPath) )
	{
		return NULL;
	}

	if( (dfd = opendir(pszPath)) == NULL )
	{
		char szBuf[256];
		sprintf( szBuf, "GetOldestDriFile %s:", pszPath );
		perror( szBuf );
		return NULL;
	}

	for(dp = readdir(dfd); NULL!=dp; dp = readdir(dfd))
	{
		if( NULL != strstr( dp->d_name, ".log" ) )
		{
			sprintf( szFileName, "%s/%s", pszPath, dp->d_name );
			//lstat返回文件的信息，文件信息存放在stat结构中
			if(lstat(szFileName, &statbuf) ==0)
			{
				if( NULL == pRtn )
				{
					pRtn = dp->d_name;
					memcpy( &statTmp, &statbuf, sizeof( struct stat ) );
				}

				if( statbuf.st_mtime <= statTmp.st_mtime )
				{
					pRtn = dp->d_name;
					memcpy( &statTmp, &statbuf, sizeof( struct stat ) );
				}
			}
		}
	}

	closedir(dfd);
	return pRtn;
}		/* -----  end of method CDirFile::GetOldestDriFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  GetDirFile
 * Description:  随机得到一个文件
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
char * CDirFile::GetDirFile ( char *pPath  )
{
	struct dirent    *dp;
	DIR              *dfd;

	if( !IsDir(pPath) )
	{
		return NULL;
	}

	if( (dfd = opendir(pPath)) == NULL )
	{
		char szBuf[256];
		sprintf( szBuf, "GetDirFile %s:", pPath );
		perror( szBuf );
		return NULL;
	}

	for(dp = readdir(dfd); NULL!=dp; dp = readdir(dfd))
	{
		if( NULL != strstr( dp->d_name, ".log" ) )
		{
			closedir(dfd);
			return dp->d_name;
		}
	}

	closedir(dfd);
	return NULL;
}		/* -----  end of method CDirFile::GetDriFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  ReadDirSize
 * Description:  获取文件路径大小
 *       Input:  pszPath:路径
 *		Return:  失败：0
 *				 成功：字节数(B)
 *--------------------------------------------------------------------------------------
 */
DWORD CDirFile::GetDirSize ( char *pszPath )
{
	sdwDirSize = 0;

	int iFlag = ftw( pszPath, &CalDirSize, 200 );
	if( 0 == iFlag  )
	{
		return sdwDirSize;
	}

	return 0;
}		/* -----  end of method CDirFile::ReadDirSize  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  DeleteFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDirFile::DeleteFile ( char *pszFileName )
{
	if( !IsFile( pszFileName ) )
	{
		return FALSE;
	}

	remove( pszFileName );

	return TRUE;
}		/* -----  end of method CDirFile::DeleteFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  DeleteOldestFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDirFile::DeleteOldestFile ( char *pszPath )
{
	char *pFile;
	char szFile[128];
	if( !IsDir( pszPath ) )
	{
		return FALSE;
	}

	pFile = GetOldestDriFile( pszPath );
	if( NULL == pFile )
	{
		return FALSE;
	}

	sprintf( szFile, "%s/%s", pszPath, pFile  );
	DeleteFile( szFile );

	return FALSE;
}		/* -----  end of method CDirFile::DeleteOldestFile  ----- */
