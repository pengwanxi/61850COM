/*
 * =====================================================================================
 *
 *       Filename:  CSocketFtpFile.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时56分23秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */
#include <stdio.h>
#include "CSocketFtpFile.h"

static unsigned int gs_uiTotalSize = 0;	
/*
 *--------------------------------------------------------------------------------------
 *      Method:  CalDirSize
 * Description:  计算每个文件大小，最后累加为所有文件大小 被ftw调用
 *       Input:  
 *		Return:
 *--------------------------------------------------------------------------------------
 */
static int CalDirSize ( const char *path, 
		const struct stat *s,
		int flagtype)
{
	// printf ( "设备号:%llu 节点号:%lu 大小:%lu\n",
			// s->st_dev,  s->st_ino, s->st_size );
	//把每个文件的大小叠加，包括了点(.)和点点(..)
	gs_uiTotalSize += s->st_size;
	return 0;
}		/* -----  end of method CSocketFtpFile::CalDirSize  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  CSocketFtpFile
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CSocketFtpFile::CSocketFtpFile ()
{
	//初始化参数
	gs_uiTotalSize = 0;
}  /* -----  end of method CSocketFtpFile::CSocketFtpFile  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  ~CSocketFtpFile
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CSocketFtpFile::~CSocketFtpFile ()
{
}  /* -----  end of method CSocketFtpFile::~CSocketFtpFile  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  WriteFile
 * Description:  往文件中写入数据   文件是以追加的方式写的，在写入新文件时注意删除旧文件
 *       Input:  pchFileName:文件名称 
 *		Return:  写入的长度，写入失败或无数据，返回0
 *--------------------------------------------------------------------------------------
 */
unsigned int CSocketFtpFile::WriteFile ( char *pchFileName, 
		unsigned char *puchBuf,
		int iLen)
{
	FILE *pFile = NULL;
	unsigned int uiWriteNum;

	// if( NULL == pchFileName ||
			// 0 == iLen)
	// {
		// return 0;
	// }
	//查看文件目录是否存在，没有则创建
	if( !IsFileExist( pchFileName ) )
	{
		char pchDirPath[256];
		int len = strlen( pchFileName )-  strlen ( strrchr( pchFileName, '/' ));
		strncpy( pchDirPath, pchFileName, len ) ;
		pchDirPath[len] = 0;

		CreateDir( pchDirPath );
	}

	//以二进制追加的文式写文件
	pFile = fopen( pchFileName, "ab+" );
	if( NULL == pFile )
	{
		printf ( "pchFileName=%s\n", pchFileName );
		perror( "pszFileName" );
		return 0;
	}

	//按照实际写入的返加，没写入将返回0
	uiWriteNum = fwrite( puchBuf, 1, iLen, pFile  );
	if( 0 == uiWriteNum )
	{
	}

	fclose( pFile );

	return uiWriteNum;
}		/* -----  end of method CSocketFtpFile::WriteFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  ReadFile
 * Description:  读文件
 *       Input:  文件名称
 *				 缓存区
 *				 长度
 *				 文件位置
 *		Return:  读出的长度 
 *--------------------------------------------------------------------------------------
 */
unsigned int CSocketFtpFile::ReadFile ( char *pchFileName,
		unsigned char *puchBuf, 
		int iLen,
		unsigned int &uiReadpos) 
{
	FILE *pFile = NULL;
	int uiReadNum=0;

	if( 0 == puchBuf )
	{
		return 0;
	}

	pFile = fopen( pchFileName, "rb" );
	if( NULL == pFile )
	{
		printf ( "pchFileName=%s\n", pchFileName );
		perror( "pszFileName" );
		return 0;
	}

	if( -1 == fseek( pFile, uiReadpos, SEEK_SET ) )
	{
		fclose( pFile );
		return 0;
	}

	//返回实际数目
	uiReadNum = fread( puchBuf, 1, iLen, pFile  );

	uiReadpos += uiReadNum;

	fclose( pFile );

	return uiReadNum;
}		/* -----  end of method CSocketFtpFile::ReadFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  CreateDir
 * Description:  创建文件目录 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
int CSocketFtpFile::CreateDir ( const char *pchDirPath )
{
	char   DirName[256];  
	strcpy(DirName,   pchDirPath);  
	int   i,len   =   strlen(DirName);  
	if(DirName[len-1]!='/')  
		strcat(DirName,   "/");  

	len   =   strlen(DirName);  

	for(i=1;   i<len;   i++)  
	{  
		if(DirName[i]=='/')  
		{  
			DirName[i]   =   0;  
			if(  access(DirName,  F_OK)!=0    )  
			{  
				if(mkdir(DirName,   0755)==-1)  
				{   
					perror("mkdir   error");   
					return   -1;   
				}  
			}  
			DirName[i]   =   '/';  
		}  
	}  

	return   0;  
}		/* -----  end of method CSocketFtpFile::CreateDir  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  GetFileSize
 * Description:  获取文件大小
 *       Input:  pchFileName:文件名
 *		Return:  大小 0为失败
 *--------------------------------------------------------------------------------------
 */
unsigned long CSocketFtpFile::GetFileSize (  const char *cpchFileName)
{
	struct stat statbuff;


	if(stat(cpchFileName, &statbuff) < 0)
	{
		printf ( "%s %lu\n", cpchFileName, statbuff.st_size );
		return 0xffffffff; 
	}

	return statbuff.st_size;
}		/* -----  end of method CSocketFtpFile::GetFileSize  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  GetDirSize
 * Description:  获取文件夹下所有文件大小 包括. 和.. 和子文件夹 
 *       Input:  pchDirPath :目录
 *		Return:  目录内文件大小
 *--------------------------------------------------------------------------------------
 */
unsigned int CSocketFtpFile::GetDirSize ( const char *pchDirPath )
{
	int iRtn;
	gs_uiTotalSize = 0;

    if (NULL == pchDirPath || access(pchDirPath, R_OK))  
	{
		printf ( "GetDirSize:is not a dirpath\n" );
		return 0;	
	}

	//递归调用计算文件大小  ftw()成功返回0 意外中止-1
	iRtn = ftw( pchDirPath, CalDirSize, 20 );
	if( -1 ==  iRtn)
	{
		printf ( "GetDirSize error %s\n", pchDirPath );
	}

	return gs_uiTotalSize;
}		/* -----  end of method CSocketFtpFile::GetDirSize  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  isFileExist
 * Description:  查看文件是否存在
 *       Input:  文件名
 *		Return:  bool
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::IsFileExist ( const char *cpchFileName )
{
    struct stat statbuf;
    if(lstat(cpchFileName, &statbuf) ==0)
	{
        return S_ISREG(statbuf.st_mode) != 0;//判断文件是否为常规文件
	}

	return false;
}		/* -----  end of method CSocketFtpFile::isFileExist  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  isDirExist
 * Description:  查看文件夹是否存在
 *       Input:  文件夹路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::IsDirExist ( const char *cpchDirPath )
{
    struct stat statbuf;
    if(lstat(cpchDirPath, &statbuf) ==0)//lstat返回文件的信息，文件信息存放在stat结构中
    {
        return S_ISDIR(statbuf.st_mode) != 0;//S_ISDIR宏，判断文件类型是否为目录
    }

    return false;
}		/* -----  end of method CSocketFtpFile::isDirExist  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  isSpecialDir
 * Description:  查看是否是特殊目录 特指. ..
 *       Input:  文件目录路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::IsSpecialDir ( const char *cpchDirPath )
{
    return strcmp(cpchDirPath, ".") == 0 || strcmp(cpchDirPath, "..") == 0;
}		/* -----  end of method CSocketFtpFile::isSpecialDir  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  GetTotalFilePath
 * Description:  由目录和文件名组成完整路径 如../ 和 main.c 组在../main.c
 *       Input:  cpchPath 目录 "../"
 *				 cpchFileName 文件名 "main.c"
 *				 cpchFilePath 要组成的文件：成功返回"../main.c"
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpFile::GetTotalFilePath ( const char *cpchPath, 
		const char *cpchFileName,
		char *cpchFilePath)
{
    strcpy(cpchFilePath, cpchPath);
    if(cpchFilePath[strlen(cpchPath) - 1] != '/')
        strcat(cpchFilePath, "/");
    strcat(cpchFilePath, cpchFileName);
}		/* -----  end of method CSocketFtpFile::GetTotalFilePath  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  ChangeFileMode
 * Description:  修改文件的权限  修改整个文件的权限使用
 *       Input:  pszFileName:文件名 
 *               imode：权限在stat中宏定义
 *               S_ISGID 02000 文件的 (set group-id on execution)位
 *               S_ISVTX 01000 文件的sticky 位
 *               S_IRUSR (S_IREAD) 00400 文件所有者具可读取权限
 *               S_IWUSR (S_IWRITE)00200 文件所有者具可写入权限
 *               S_IXUSR (S_IEXEC) 00100 文件所有者具可执行权限
 *               S_IRGRP 00040 用户组具可读取权限
 *               S_IWGRP 00020 用户组具可写入权限
 *               S_IXGRP 00010 用户组具可执行权限
 *               S_IROTH 00004 其他用户具可读取权限
 *               S_IWOTH 00002 其他用户具可写入权限
 *               S_IXOTH 00001 其他用户具可执行权限
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::ChangeFileMode ( char *pszFileName, int imode )
{
	if( NULL == pszFileName )
	{
		return false;
	}

	if( 0 == chmod ( pszFileName, imode ))
	{
		return true;
	}

	return false;
}		/* -----  end of method CSocketFtpFile::ChangeFileMode  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  ChangeDirFilesMode
 * Description:  修改文件夹内所有文件的权限 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::ChangeDirFilesMode ( char *pchDirPath, int imode )
{
	DIR *dir;
	struct dirent *ptr;
	struct stat ;
	char base[1000];

	if ((dir=opendir(pchDirPath)) == NULL)
	{
		perror("Open dir error...");
		return false;

	}

	while ((ptr=readdir(dir)) != NULL)
	{
		if(IsSpecialDir( ptr->d_name ))    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8 ||     ///file
				ptr->d_type == 10)    ///link file
		{
			char chFileName[256];
			sprintf( chFileName, "%s/%s", pchDirPath, ptr->d_name );
			if(  !ChangeFileMode( chFileName, imode ) )	
			{
				closedir( dir );
				return false;
			}
		}
		else if(ptr->d_type == 4)    ///dir
		{
			memset(base,'\0',sizeof(base));
			strcpy(base,pchDirPath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			if( !ChangeDirFilesMode(base, imode))
			{
				closedir( dir );
				return false;
			}
		}
	}

	closedir(dir);
	return true ;
}		/* -----  end of method CSocketFtpFile::ChangeDirFilesMode  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  DeleteDirFiles
 * Description:  删除文件夹内所有文件
 *       Input:  目录
 *		Return:  
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::DeleteDirFiles ( const char *pchDirPath )
{
    DIR *dir = NULL;
    dirent *dir_info;
    char file_path[PATH_MAX];
    if(IsFileExist(pchDirPath))
    {
        remove(pchDirPath);
        return false;
    }

    if(IsDirExist(pchDirPath))
    {
        if((dir = opendir(pchDirPath)) == NULL)
            return false;
        while((dir_info = readdir(dir)) != NULL)
        {
            GetTotalFilePath(pchDirPath, dir_info->d_name, file_path);
            if(IsSpecialDir(dir_info->d_name))
                continue;
            DeleteDirFiles(file_path);
            rmdir(file_path);
        }
    }

	closedir( dir );

	return true;
}		/* -----  end of method CSocketFtpFile::DeleteDirFiles  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  MoveDirFiles
 * Description:  移动文件或目录， 利用rename实现
 *       Input:  源文件夹或目录
 *				 目的文件夹或目录
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::MoveDirFiles ( char *pchSrcPath, char *pchDirPath )
{
	if( NULL == pchSrcPath
		|| NULL == pchDirPath )
	{
		return false;	
	}

	if( 0 == rename( pchSrcPath, pchDirPath ) )
	{
		return true;	
	}
	perror( "rename" );
	
	return false;
}		/* -----  end of method CSocketFtpFile::MoveDirFiles  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  WriteFileListFile
 * Description:  将文件夹下的内容写进某文件中 
 *       Input:  pchDirPath 路径
 *				 pchFileName 文件名
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::WriteFileListFile (  char *pchDirPath, char *pchFileName )
{
	DIR *dir;
	struct dirent *ptr;
	struct stat ;
	char base[1000];

	if( NULL == pchFileName )
	{
		return false;
	}

	if ((dir=opendir(pchDirPath)) == NULL)
	{
		perror("Open dir error...");
		return false;

	}

	while ((ptr=readdir(dir)) != NULL)
	{
		if(IsSpecialDir( ptr->d_name ))    ///current dir OR parrent dir
			continue;
		else if(ptr->d_type == 8 ||     ///file
				ptr->d_type == 10)    ///link file
		{
			char chFileName[256];
			sprintf( chFileName, "%s/%s\n", pchDirPath, ptr->d_name );
			WriteFile( pchFileName, (unsigned char *)chFileName , strlen( chFileName ));
		}
		else if(ptr->d_type == 4)    ///dir
		{
			memset(base,'\0',sizeof(base));
			strcpy(base,pchDirPath);
			strcat(base,"/");
			strcat(base,ptr->d_name);
			if( !WriteFileListFile(base, pchFileName))
			{
				closedir( dir );
				return false;
			}
		}
	}

	closedir(dir);
	return true ;
}		/* -----  end of method CSocketFtpFile::WriteFileListFile  ----- */


/* 
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 上面的可以共用，下面的仅仅是针对这个协议使用的-.-2015年09月25日 14时20分41秒
 * ---------------------------------------------------------------------------
 *  */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  CheckFileList
 * Description:  检查程序文件列表是否正确 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::CheckFileList (  )
{
	const int iCheckNum = 5;
	const int iMaxFileNum = 50;
	bool bPrgm[iCheckNum] = {false, false, false, false, false};
	int iother = 0;
	char szCheckType[iCheckNum][64] = {"/mynand/bin/pman", "/mynand/bin/gather", "/mynand/bin/rtdbserver", ".so", "/myapp/user.sh"};
	char chLineBuf[256];

	for ( int i=1; NULL != GetFileLineBuf( FTP_FILE_LIST, i, chLineBuf ) && i<iMaxFileNum; i++)
	{
		if( '\n' == chLineBuf[0] )
		{
			continue;
		}

		int j = 0;
		for( j=0; j<iCheckNum; j++ )
		{
			if( NULL != strstr( chLineBuf, szCheckType[j] ) )
			{
				bPrgm[j] = true;
				break;
			}
		}

		if( iother )
		if( j >= iCheckNum )
		{
			return false;
		}
	}


	for ( int k=0; k<iCheckNum-1; k++)
	{
		if( !bPrgm[k] )
		{
			printf ( "CheckFileList none %s\n", szCheckType[k] );
			return false;	
		}
	}

	return true;
}		/* -----  end of method CSocketFtpFile::CheckFileList  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  GetDownType
 * Description:  从filelist中获得文件类型，进行下一步的存储
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
FTP_FILE_TYPE CSocketFtpFile::GetDownType ( char *pchFileName )
{
	FILE *fp = NULL;
	char chLineBuf[256];

	fp = fopen( pchFileName, "rb" );
	if( NULL == fp )
	{
		perror( "ftp_list" );
		return FTP_NONE_TYPE;
	}

	while( fgets( chLineBuf, sizeof( chLineBuf ), fp ) )
	{
		if( NULL != strstr( chLineBuf, "/mynand/config/" ) )
		{
			fclose( fp );
			return FTP_CFG_TYPE;
		}
		
		if( NULL != strstr( chLineBuf, "/mynand/bin" ) ||
				NULL != strstr( chLineBuf, "/mynand/lib" ) ||
				NULL != strstr( chLineBuf, "/myapp/user.sh" ))
		{
			fclose( fp );
			return FTP_PRGM_TYPE;
		}

		continue;
	}

	fclose( fp );
	return FTP_NONE_TYPE;
}		/* -----  end of method CSocketFtpFile::GetDownType  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  GetDownFileName
 * Description:  根椐传过来的文件名修改为需要的文件名
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
char * CSocketFtpFile::GetDownFileName ( FTP_FILE_TYPE fType,
		char *pchFileName )
{
	char chTmpBuf[256];
	memset( chTmpBuf, 0, 256 );

	if( NULL == pchFileName ||
			FTP_NONE_TYPE == fType)
	{
		return NULL;
	}

	if( FTP_CFG_TYPE == fType )
	{
		if( 0 == strncmp( pchFileName, "/mynand/", 8 ) )		
		{
			strcpy( chTmpBuf, FTP_DOWN_CFG_FILE );	
			strcat( chTmpBuf, pchFileName+8 );
			strcpy( pchFileName, chTmpBuf );
			return pchFileName;
		}
	}

	if( FTP_PRGM_TYPE == fType )
	{
		if( 0 == strncmp( pchFileName, "/mynand/", 8 ) )		
		{
			strcpy( chTmpBuf, FTP_DOWN_PRGM_FILE );	
			strcat( chTmpBuf, pchFileName+8 );
			strcpy( pchFileName, chTmpBuf );
			return pchFileName;
		}
		else if( 0 == strncmp( pchFileName, "/myapp/user.sh" , 14) )
		{
			strcpy( pchFileName, FTP_DOWN_USER_FILE );
			return pchFileName;
		}
	}

	return NULL;
}		/* -----  end of method CSocketFtpFile::GetDownFileName  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  GetFileLineBuf
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
char * CSocketFtpFile::GetFileLineBuf ( char *pchFileName, 
		int iLine,
		char *pchLineBuf )
{
	FILE *fp = NULL;
	char chLineBuf[256];
	int iCount = 0;

	if( NULL == pchFileName ||
			NULL == pchLineBuf)
	{
		return NULL;
	}

	fp = fopen( pchFileName, "rb" );
	if( NULL == fp )
	{
		perror( "ftp_list" );
		return NULL;
	}

	while( fgets( chLineBuf, sizeof( chLineBuf ), fp ) )
	{
		if( NULL == strstr( chLineBuf, "/" ) )
		{
			continue;
		}

		iCount ++ ;
		if( iCount == iLine )
		{
			strcpy( pchLineBuf, chLineBuf );	
			fclose( fp );
			return pchLineBuf;
		}
	}

	fclose( fp );
	return NULL;
}		/* -----  end of method CSocketFtpFile::GetFileLineBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  BakDirFiles
 * Description:  备份文件
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::BakDirFiles ( char* pchSrcPath,
		char *pchDirPath )
{
	if( NULL == pchSrcPath ||
			NULL == pchDirPath)
	{
		return false;
	}

	//备份config
	DeleteDirFiles( pchDirPath ) ;
	if( !MoveDirFiles( pchSrcPath, pchDirPath ) )
	{
		printf ( "BakDirFiles::%s bak %s fail\n", pchSrcPath, pchDirPath );
		return false;
	}

	return true;
}		/* -----  end of method CSocketFtpFile::BakDirFiles  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  UpdateCfg
 * Description:  更新配置文件
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::UpdateCfg ( void )
{
	char chCfgFile[256] = "/mynand/config/";
	char chBakCfgFile[256] = "/mynand/bak/config/";
	char chTmp[256] = "";

	sprintf( chTmp, "%s%s", FTP_DOWN_CFG_FILE, (char *)"config/" );
	if( !IsDirExist( chTmp ) )
	{
		printf ( "can't find %s\n", chTmp );
		return false;
	}
	printf ( "/mynand/config begin to update\n" );

	//备份config
	if( !BakDirFiles( chCfgFile, chBakCfgFile ) )
	{
		return false;
	}

	//移动downcfg
	if( !BakDirFiles( chTmp, chCfgFile ) )
	{
		if( !BakDirFiles( chBakCfgFile, chCfgFile ) )
		{
			return false;	
		}

		return false;
	}
	else
	{
		// DeleteDirFiles( chBakCfgFile );
	}

	//权限
	if( !ChangeDirFilesMode( chCfgFile, 
				S_IRUSR | S_IRGRP ) )
	{
		return false;
	}

	DeleteDirFiles( FTP_DOWN_CFG_FILE );
	rmdir( FTP_DOWN_CFG_FILE );
	printf ( "/mynand/config update success\n" );
	return true;
}		/* -----  end of method CSocketFtpFile::UpdateCfg  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpFile
 *      Method:  UpdatePrgm
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
bool CSocketFtpFile::UpdatePrgm ( void )
{
	char chPrgmBinFile[256] = "/mynand/bin/";
	char chBakPrgmBinFile[256] = "/mynand/bak/bin/";
	char chPrgmLibFile[256] = "/mynand/lib/";
	char chBakPrgmLibFile[256] = "/mynand/bak/lib/";
	char chPrgmUserFile[256] = "/myapp/user.sh";
	char chBakPrgmUserFile[256] = "/myapp/bak/user.sh";
	char chTmp[256] = "";

	if( !IsDirExist( FTP_DOWN_PRGM_FILE ) )
	{
		return false;
	}

	sprintf( chTmp, "%s%s", FTP_DOWN_PRGM_FILE, "bin/" );
	if( IsDirExist( chTmp ) )
	{
		printf ( "/mynand/bin begin to update\n" );
		//备份bin
		if( !BakDirFiles( chPrgmBinFile, chBakPrgmBinFile ) )
		{
			return false;
		}

		//移动bin
		if( !BakDirFiles( chTmp, chPrgmBinFile ) )
		{
			if( !BakDirFiles( chBakPrgmBinFile, chPrgmBinFile ) )
			{
				return false;	
			}

			return false;
		}

		// 权限
		if( !ChangeDirFilesMode( chPrgmBinFile, 
					S_IRUSR | S_IWUSR | S_IXUSR	| 
					S_IRGRP | S_IWGRP | S_IXGRP |
					S_IROTH | S_IWOTH) )
		{
			return false;	
		}
		printf ( "/mynand/bin update success\n" );
		DeleteDirFiles( chTmp );
		rmdir( chTmp );
	}
	else
	{
		printf ( "can't find %s\n", chTmp );
	}


	sprintf( chTmp, "%s%s", FTP_DOWN_PRGM_FILE, "lib/" );
	if( IsDirExist( chTmp ) )
	{
		printf ( "/mynand/lib begin to update\n" );
		//备份lib
		if( !BakDirFiles( chPrgmLibFile, chBakPrgmLibFile ) )
		{
			return false;
		}

		//移动bin
		if( !BakDirFiles( chTmp, chPrgmLibFile ) )
		{
			if( !BakDirFiles( chBakPrgmLibFile, chPrgmLibFile ) )
			{
				return false;	
			}

			return false;
		}

		// 权限
		if( !ChangeDirFilesMode( chPrgmLibFile, 
					S_IRUSR | S_IWUSR |  
					S_IRGRP | S_IWGRP | 
					S_IROTH | S_IWOTH) )
		{
			return false;	
		}

		DeleteDirFiles( chTmp );
		rmdir( chTmp );
		printf ( "/mynand/lib update success\n" );
	}
	else
	{
		printf ( "can't find %s\n", chTmp );
	}

	if( IsFileExist( FTP_DOWN_USER_FILE ) )
	{
		printf ( "/myapp/user.sh begin to update\n" );
		//备份user.sh
		if( !BakDirFiles( chPrgmUserFile, chBakPrgmUserFile ) )
		{
			return false;
		}

		//移动user.sh
		// sprintf( chTmp, "%s%s", FTP_DOWN_PRGM_FILE, "user.sh" );
		if( !BakDirFiles(FTP_DOWN_USER_FILE , chPrgmUserFile ) )
		{
			if( !BakDirFiles( chBakPrgmUserFile, chPrgmUserFile ) )
			{
				return false;	
			}

			return false;
		}

		// 权限
		if( !ChangeFileMode( chPrgmUserFile, 
					S_IRUSR | S_IWUSR | S_IXUSR	| 
					S_IRGRP | S_IWGRP | S_IXGRP |
					S_IROTH | S_IWOTH) )
		{
			return false;	
		}

		DeleteDirFiles( FTP_DOWN_USER_FILE );
		rmdir( FTP_DOWN_USER_FILE );
		printf ( "/myapp/usr.sh update success\n" );
	}
	else
	{
		printf ( "can't find %s\n", FTP_DOWN_USER_FILE );
	}

	return true	;
}		/* -----  end of method CSocketFtpFile::UpdatePrgm  ----- */



