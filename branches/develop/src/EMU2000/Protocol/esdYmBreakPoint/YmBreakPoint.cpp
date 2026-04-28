// *       Filename:  Protocol_SelfDefine_Trans.c
// *
// *    Description : �Զ���Э��, ����ң���Ķϵ�����
// *    Э����������
// *
// *
// *   //����
// *   0   0x68	��ʼ
// * 1   Byte	վ��   ��ǰ�ͺ�
// * 2   Byte
// * 3   Byte	�������� 0xF0����
// * 4   Byte	�� ��ǰ�ͺ�
// * 5   Byte
// * 6   Byte	��
// * 7   Byte	��
// * 8   Byte	ʱ
// * 9   Byte	�� 5�ּ�� 13:05 13 : 10
// * 10  Byte	���ݿ�ʼ��� ��ǰ�ͺ�
// * 11  Byte
// * 12  Byte	��������
// * 13  Byte	����1���� 0��������, 1û�ҵ�����
// *       Byte	����1 ��һ��ֵ
// *       Byte
// *       Byte
// *       Byte
// *       Byte	����2����
// *       Byte	����2 �ڶ���ֵ
// *       Byte
// *       Byte
// *       Byte
// *       ...
// *
// *
// *   //����
// *       0x68	��ʼ
// *       Byte	�������� 0xF1�յ�����ȷ��.

#include "YmBreakPoint.h"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#define DATAFROMFILE 1	 /* �ļ����� */
#define DATAFROMMEM 2	 /*  �ڴ�����*/
#define DATAFROMRESEND 3 /*  �����ط�*/

/* ���ݽṹ */
typedef struct
{
	BYTE byStep;	/* ���͵Ĳ��� һ�����ݷ����� */
	WORD wDataType; /* �������� */
	DWORD dwData;
	WORD wMin;			   /* ���ӣ�ÿ�η��ͺ󱣴棬ͬһ����ֻ����һ�� */
	BYTE byLink;		   /* �Ƿ��ܽ���TCPͨѶ�����ܵĻ��Ͳ����ļ��еõ����� */
	BYTE byReSendBuf[256]; /* �ط������� */
	BYTE byResendLen;	   /* �ط����������� */
	BOOL bReSendFlag;	   /* �ط���־ */
	BOOL bIsSending;	   /* �Ƿ��Ƿ���״̬�� ����״̬�²������ط� */
} SELF_DATATYPE;		   /* ----------  end of struct SELF_DATATYPE  ---------- */

static SELF_DATATYPE self_data[200][1];
#define MODULE_SELDUNION(byLineNo, byModuleNo) self_data[byLineNo][byModuleNo].byStep
#define MODULE_SELDUNION_WORD(byLineNo, byModuleNo) self_data[byLineNo][byModuleNo].wDataType
#define MODULE_SELDUNION_DWORD(byLineNo, byModuleNo) self_data[byLineNo][byModuleNo].dwData
#define MODULE_SELDMIN_WORD(byLineNo, byModuleNo) self_data[byLineNo][byModuleNo].wMin
#define MODULE_SELDLINK(byLineNo, byModuleNo) self_data[byLineNo][byModuleNo].byLink

CYmBreakPoint::CYmBreakPoint()
{
}

CYmBreakPoint::~CYmBreakPoint()
{
}

void CYmBreakPoint::TimerProc(void)
{
	static time_t tBegin = 0;
	if (tBegin == 0)
		time(&tBegin);
	else
	{
		time_t tEnd;
		time(&tEnd);
		if (difftime(tEnd, tBegin) > 60)
		{
			m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);
			time(&tBegin);
		}
	}
}

BOOL CYmBreakPoint::Init(BYTE byLineNo)
{
	system("mkdir -p /mnt/data");

	m_wRtuAddr = m_wDevAddr;
	char szFileName[256] = "";

	sprintf(szFileName, "%s%s", "/mynand/config/esdBreakPoint/", m_sTemplatePath);
	// ��ȡ��Ҫת�������ݵ���ģ��
	ReadMapConfig(szFileName);

	UINT uPort;
	BOOL bOk = FALSE;
	char szCtrl[32];

	CBasePort::GetCommAttrib(m_ComCtrl1, szCtrl, uPort);

	m_wPortNum = (WORD)uPort;

	// ��ȡת�����
	CreateTransTab();

	// ���ڴ����ݿ���--��ȡת����Ĭ������
	m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);

	return TRUE;
}

BOOL CYmBreakPoint::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg /*= NULL*/)
{
	len = SelfDef_Trans_getSendMessage(m_byLineNo, 0, buf, 512);
	if (len > 0)
	{
		int uiSendLen = 0;
		/* Ҫ��������ʱ���� */
		if (!m_pMethod->IsPortValid())
		{
			m_pMethod->CloseSocket(m_byLineNo);

			m_pMethod->OpenSocket(m_byLineNo);
		}

		uiSendLen = m_pMethod->m_pPort->WritePort(buf, len);

		SelfDef_Trans_SetReSendBuf(m_byLineNo, (char *)buf, len);
		self_data[m_byLineNo][0].bIsSending = TRUE;
		if (uiSendLen == -1)
			SelfDef_Trans_SetResendFlag(m_byLineNo, TRUE);

		/* �鿴�ܷ��ͳɹ� */
		if (uiSendLen <= 0)
		{
			/* �Ƿ���Ҫ�洢 */
			if (DATAFROMMEM == MODULE_SELDUNION_WORD(m_byLineNo, 0))
			{
				BYTE bySlaveAddr = m_wDevAddr;
				/* �洢���� */
				SelfDef_Trans_saveSendBuf(m_byLineNo, bySlaveAddr, buf, len);
			}
		}
	}
	else
	{
		if (m_pMethod->IsPortValid())
		{
			m_pMethod->m_pPort->ClosePort();
		}
	}
	return FALSE;
}

BOOL CYmBreakPoint::ProcessProtocolBuf(BYTE *buf, int len)
{
	// 		char szBuf[2048] = { 0 };
	// 		int len_ret = 0;
	// 		int index = 0;
	// 		for (int i = 0; i < len; i++)
	// 		{
	// 			len_ret = sprintf(&szBuf[ index ], "%02x", buf[i]);
	// 			index = index + len_ret;
	// 		}
	//
	// 		printf("recv:%s len = %d\n", szBuf,len );

	/* �����ݳ��Ȼ����ж� */
	if (len <= 0)
	{
		if (self_data[m_byLineNo][0].bIsSending)
		{
			SelfDef_Trans_SetResendFlag(m_byLineNo, TRUE);
		}
		return FALSE;
	}

	/* �жϱ��ĳ��� */
	if (2 != len)
	{
		if (self_data[m_byLineNo][0].bIsSending)
		{
			SelfDef_Trans_SetResendFlag(m_byLineNo, TRUE);
		}
		return TRUE;
	}
	if (0x68 == buf[0] || 0xf1 == buf[1]) /* �Ա��ı��������ж� */
	{
		if (self_data[m_byLineNo][0].bIsSending)
		{
			SelfDef_Trans_SetResendFlag(m_byLineNo, FALSE);
			self_data[m_byLineNo][0].bIsSending = FALSE;
		}
	}
	else
	{
		if (self_data[m_byLineNo][0].bIsSending)
		{
			SelfDef_Trans_SetResendFlag(m_byLineNo, TRUE);
		}
	}
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  CreateDir
 * Description:  �����ļ���
 *       Input:  pszPath �ļ���·������
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CYmBreakPoint::CreateDir(char *pszPath)
{
	if (IsDir(pszPath))
	{
		printf("%s is exist\n", pszPath);
		return TRUE;
	}
	/* ����Ȩ��Ϊ755���ļ��� */
	if (0 == (mkdir(pszPath, S_IRWXU)))
	{
		/* chmod( pszPath, 0755  );  */
		printf("CreateDir %s OK\n", pszPath);
		return TRUE;
	}

	printf("CreateDir %s Error\n", pszPath);
	return FALSE;
} /* -----  end of method CreateDir  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  IsDir
 * Description:  ����ļ����Ƿ����
 *       Input:  pszPath�ļ���·��
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CYmBreakPoint::IsDir(char *pszPath)
{
	DIR *dfd;
	char buf[128];
	getcwd(buf, 128);

	if (NULL == (dfd = opendir(pszPath)))
	{
		perror("error");
		return FALSE;
	}

	closedir(dfd);
	return TRUE;
} /* -----  end of method IsDir  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  WriteToFile
 * Description:	 ������׷��д���ļ�
 *       Input:  pszFileName:�ļ����� ��·��
 *				 pszBuf:Ҫд�������
 *				 len:Ҫд�����ݵĳ���
 *		Return:  �ɹ�������
 *				 ʧ�ܣ�0
 *--------------------------------------------------------------------------------------
 */
DWORD CYmBreakPoint::WriteToFile(char *pszFileName,
								 BYTE *pszBuf,
								 int len)
{
	FILE *pFile = NULL;
	DWORD dwFileLen;

	/* �ļ�����Ʋ��ܳ� */
	/* //�����ƴ��ļ� */
	pFile = fopen(pszFileName, "ab+");
	if (NULL == pFile)
	{
		char szBuf[256];
		sprintf(szBuf, "WriteToFile %s error:", pszFileName);
		perror(szBuf);
		return 0;
	}

	/* //д������ */
	dwFileLen = fwrite(pszBuf, 1, len, pFile);

	fclose(pFile);

	return dwFileLen;
} /* -----  end of method WriteToFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  ReadFromFile
 * Description:  ���ļ�����Ӧλ�ö�������
 *       Input:  pszFileName:�ļ����� ��·��
 *				 pszBuf:Ҫд�������
 *				 len:Ҫд�����ݵĳ���
 *		Return:  �ɹ�������
 *				 ʧ�ܣ�0
 *--------------------------------------------------------------------------------------
 */
DWORD CYmBreakPoint::ReadFromFile(char *pszFileName,
								  BYTE *pszBuf,
								  int len,
								  DWORD dwReadPos)
{
	FILE *pFile = NULL;
	DWORD dwFileLen = 0;

	/* //�����ƴ��ļ� */
	pFile = fopen(pszFileName, "rb");
	if (NULL == pFile)
	{
		char szBuf[256];
		sprintf(szBuf, "ReadFromFile %s:", pszFileName);
		perror(szBuf);
		return 0;
	}

	if (-1 == fseek(pFile, dwReadPos, SEEK_SET))
	{
		fclose(pFile);
		return 0;
	}

	/* //�������� */
	dwFileLen = fread(pszBuf, 1, len, pFile);

	fclose(pFile);

	return dwFileLen;
} /* -----  end of method ReadFromFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  IsFile
 * Description:  �Ƿ��и��ļ�
 *       Input:  pszFileName : �ļ�·������
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CYmBreakPoint::IsFile(char *pszFileName)
{
	struct dirent *dp;
	DIR *dfd;
	char szPath[128];
	char szFile[64];
	char *pszFile = NULL;

	if (NULL == pszFileName)
	{
		return FALSE;
	}

	pszFile = strrchr(pszFileName, '/');
	if (NULL == pszFile)
	{
		/* //��ǰ�ļ����µ��ļ� */
		strcpy(szPath, "./");
		strcpy(szFile, pszFileName);
	}
	else
	{
		int iPathLen = 0;
		/* //ȥ��'/' ���� */
		strcpy(szFile, pszFile + 1);
		/* //ֻ����ǰ��Ĳ��ֵĳ��� */
		iPathLen = strlen(pszFileName) - strlen(pszFile) + 1;
		strncpy(szPath, pszFileName, iPathLen);
	}

	dfd = opendir(szPath);
	if (NULL == dfd)
	{
		char szBuf[256];
		sprintf(szBuf, "IsFile %s:", pszFileName);
		perror(szBuf);
		return FALSE;
	}

	for (dp = readdir(dfd); NULL != dp; dp = readdir(dfd))
	{
		/* // printf ( "dp->d_name=%s  %s\n", dp->d_name, szFile ); */
		if (0 == strcmp(dp->d_name, szFile))
		{
			closedir(dfd);
			return TRUE;
		}
	}

	closedir(dfd);

	return FALSE;
} /* -----  end of method IsFile  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  DeleteDir
 *  Description:
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
void CYmBreakPoint::DeleteDir(char *pchPath)
{
	struct dirent *dp;
	DIR *dfd;
	char tmpBuf[128];

	if (!IsDir(pchPath))
	{
		return;
	}

	if ((dfd = opendir(pchPath)) == NULL)
	{
		return;
	}

	for (dp = readdir(dfd); NULL != dp; dp = readdir(dfd))
	{
		if (NULL != strstr(dp->d_name, ".L"))
		{
			sprintf(tmpBuf, "%s/%s", pchPath, dp->d_name);
			remove(tmpBuf);
		}
	}

	closedir(dfd);
	remove(pchPath);
} /* -----  end of function DeleteDir  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  GetOldestDir
 *  Description:
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
char *CYmBreakPoint::GetOldestDir(char *pchPath, char *destPath)
{
	DIR *dir = NULL;
	struct dirent *dir_info = NULL;
	char file_path[128] = "";
	char tmp_dir_name[64] = "";
	char *p = tmp_dir_name;

	if (IsDir(pchPath))
	{
		if (NULL == (dir = opendir(pchPath)))
		{
			return NULL;
		}
		while (NULL != (dir_info = readdir(dir)))
		{
			if ((0 == strcmp(dir_info->d_name, ".")) || 0 == strcmp(dir_info->d_name, ".."))
			{
				continue;
			}

			strcpy(file_path, pchPath);
			if ('/' != file_path[strlen(file_path) - 1])
			{
				strcat(file_path, "/");
			}
			strcat(file_path, dir_info->d_name);

			if (IsDir(file_path))
			{
				if (0 == strlen(p))
				{
					strcpy(tmp_dir_name, dir_info->d_name);
				}

				if (0 >= strcmp(tmp_dir_name, dir_info->d_name))
				{
				}
				else
				{
					strcpy(tmp_dir_name, dir_info->d_name);
				}

				p = tmp_dir_name;
			}
		}
	}

	closedir(dir);

	if (0 == strlen(p))
	{
		return NULL;
	}
	strcpy(destPath, p);
	return destPath;
} /* -----  end of function GetOldestDir  ----- */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  GetLastestDir
 *  Description:  �������µ��ļ��� ֻ����һ��Ŀ¼
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
char *CYmBreakPoint::GetLastestDir(char *pchPath, char *destPath)
{
	DIR *dir = NULL;
	struct dirent *dir_info = NULL;
	char file_path[128] = "";
	char tmp_dir_name[64] = "";
	char *p = tmp_dir_name;

	if (IsDir(pchPath))
	{
		if (NULL == (dir = opendir(pchPath)))
		{
			return NULL;
		}
		while (NULL != (dir_info = readdir(dir)))
		{
			if ((0 == strcmp(dir_info->d_name, ".")) || 0 == strcmp(dir_info->d_name, ".."))
			{
				continue;
			}

			strcpy(file_path, pchPath);
			if ('/' != file_path[strlen(file_path) - 1])
			{
				strcat(file_path, "/");
			}
			strcat(file_path, dir_info->d_name);

			if (IsDir(file_path))
			{
				if (0 == strlen(p))
				{
					strcpy(tmp_dir_name, dir_info->d_name);
				}

				if (0 >= strcmp(tmp_dir_name, dir_info->d_name))
				{
					strcpy(tmp_dir_name, dir_info->d_name);
				}
				else
				{
				}

				p = tmp_dir_name;
			}
		}
	}

	closedir(dir);

	if (0 == strlen(p))
	{
		return NULL;
	}
	strcpy(destPath, p);
	return destPath;
} /* -----  end of function GetLastestDir  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  GetLatestDriFile
 * Description:  ��ȡ���µ��ļ�(�ļ���)
 *       Input:  pszPath:·��
 *		Return:  �ɹ����ļ���
 *				 ʧ�ܣ�NULL
 *--------------------------------------------------------------------------------------
 */
char *CYmBreakPoint::GetLatestDriFile(char *pchPath, char *filename)
{
	struct dirent *dp;
	DIR *dfd;
	char *pRtn = NULL;
	struct stat statbuf, statTmp;
	char szFileName[128];
	char tmpBuf[128];
	char path[128], path1[128];
	char *pszPath = path1;

	if (NULL == GetLastestDir(pchPath, path))
	{
		return NULL;
	}
	sprintf(pszPath, "%s%s", pchPath, path);

	if (!IsDir(pszPath))
	{
		return NULL;
	}

	if ((dfd = opendir(pszPath)) == NULL)
	{
		char szBuf[256];
		sprintf(szBuf, "GetLatestDriFile %s:", pszPath);
		perror(szBuf);
		return NULL;
	}

	for (dp = readdir(dfd); NULL != dp; dp = readdir(dfd))
	{
		if (NULL != strstr(dp->d_name, ".L"))
		{
			if (NULL == pRtn)
			{
				pRtn = dp->d_name;
				strcpy(tmpBuf, dp->d_name);
				/* memcpy( tmpBuf, dp->d_name, sizeof( 128 ) ); */
			}

			if (0 <= strcmp(tmpBuf, dp->d_name))
			{
				pRtn = dp->d_name;
				strcpy(tmpBuf, dp->d_name);
				/* memcpy( tmpBuf, dp->d_name, sizeof( 128 ) ); */
			}

			/* sprintf( szFileName, "%s/%s", pszPath, dp->d_name ); */
			/* [> //lstat�����ļ�����Ϣ���ļ���Ϣ�����stat�ṹ�� <] */
			/* if(stat(szFileName, &statbuf) ==0) */
			/* { */
			/* if( NULL == pRtn ) */
			/* { */
			/* pRtn = dp->d_name; */
			/* memcpy( &statTmp, &statbuf, sizeof( struct stat ) ); */
			/* } */

			/* if( statbuf.st_mtime >= statTmp.st_mtime )	 */
			/* { */
			/* pRtn = dp->d_name; */
			/* memcpy( &statTmp, &statbuf, sizeof( struct stat ) ); */
			/* } */
			/* } */
		}
	}

	closedir(dfd);

	if (NULL != pRtn)
	{
		sprintf(filename, "%s/%s", pszPath, tmpBuf);
		pRtn = filename;
	}
	return pRtn;
} /* -----  end of method GetLatestDriFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  GetOldestDriFile
 * Description:  ��ȡ��ɵ��ļ�(�ļ���)
 *       Input:  pszPath:·��
 *		Return:  �ɹ����ļ���
 *				 ʧ�ܣ�NULL
 *--------------------------------------------------------------------------------------
 */
char *CYmBreakPoint::GetOldestDriFile(char *pchPath, char *filename)
{
	struct dirent *dp;
	DIR *dfd;
	char *pRtn = NULL;
	struct stat statbuf, statTmp;
	char szFileName[128];
	char tmpBuf[128];
	char path[128], path1[128];
	char *pszPath = path1;

	if (NULL == GetOldestDir(pchPath, path))
	{
		return NULL;
	}
	sprintf(pszPath, "%s%s", pchPath, path);

	if (!IsDir(pszPath))
	{
		return NULL;
	}

	if ((dfd = opendir(pszPath)) == NULL)
	{
		char szBuf[256];
		sprintf(szBuf, "GetOldestDriFile %s:", pszPath);
		perror(szBuf);
		return NULL;
	}

	for (dp = readdir(dfd); NULL != dp; dp = readdir(dfd))
	{
		if (NULL != strstr(dp->d_name, ".l"))
		{
			if (NULL == pRtn)
			{
				pRtn = dp->d_name;
				strcpy(tmpBuf, dp->d_name);
				/* memcpy( tmpBuf, dp->d_name, sizeof( 128 ) ); */
			}

			if (0 <= strcmp(tmpBuf, dp->d_name))
			{
				pRtn = dp->d_name;
				strcpy(tmpBuf, dp->d_name);
				/* memcpy( tmpBuf, dp->d_name, sizeof( 128 ) ); */
			}
		}
	}
	closedir(dfd);

	if (NULL != pRtn)
	{
		sprintf(filename, "%s/%s", pszPath, tmpBuf);
		pRtn = filename;
	}
	else
	{
		remove(pszPath);
	}
	return pRtn;
} /* -----  end of method GetOldestDriFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method : GetDirFile
 * Description : ����õ�һ���ļ�
 *       Input :
 *Return :
 */
char *CYmBreakPoint::GetDirFile(char *pPath)
{
	struct dirent *dp;
	DIR *dfd;

	if (!IsDir(pPath))
	{
		return NULL;
	}

	if ((dfd = opendir(pPath)) == NULL)
	{
		char szBuf[256];
		sprintf(szBuf, "GetDirFile %s:", pPath);
		perror(szBuf);
		return NULL;
	}

	for (dp = readdir(dfd); NULL != dp; dp = readdir(dfd))
	{
		if (NULL != strstr(dp->d_name, ".l"))
		{
			closedir(dfd);
			return dp->d_name;
		}
	}

	closedir(dfd);
	return NULL;
} /* -----  end of method GetDriFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  DeleteFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CYmBreakPoint::DeleteFile(char *pszFileName)
{
	/* if( !IsFile( pszFileName ) ) */
	/* { */
	/* return FALSE; */
	/* } */

	remove(pszFileName);
	/* taskDelay( 60 );  */
	/* sleep( 1 );  */

	return TRUE;
} /* -----  end of method DeleteFile  ----- */

int CYmBreakPoint::daysum(int y, int m, int d)
{
	unsigned char x[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	int i, s = 0;

	if (m > 12)
	{
		return -1;
	}

	for (i = 1970; i < y; i++)
		if (i % 4 == 0 && i % 100 != 0 || i % 400 == 0)
			s += 366; /* ���� */
		else
			s += 365; /* ƽ�� */

	if (y % 4 == 0 && y % 100 != 0 || y % 400 == 0)
		x[2] = 29;
	for (i = 1; i < m; i++)
		s += x[i]; /* ���µ����� */
	s += d;		   /* �յ����� */
	return s;	   /* ����������,��Թ�Ԫ1�� */
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDirFile
 *      Method:  DeleteOldestFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CYmBreakPoint::DeleteOldestFile(char *pszPath)
{
	char FilePath[128], LastestFilePath[128];
	char *pFile = FilePath;
	char *pLastestFile = LastestFilePath;
	char szFile[128], szLastestFile[128];
	struct stat statbuf, lasteststatbuf;
	int o_year = 0, o_month = 0, o_day = 0;
	int l_year = 0, l_month = 0, l_day = 0;
	int difdays = 0;

	if (!IsDir(pszPath))
	{
		return FALSE;
	}

	pFile = GetOldestDir(pszPath, FilePath);
	/* strcpy( path,  GetLastestDir( pchPath ) ); */
	if (NULL == pFile)
	{
		return FALSE;
	}

	pLastestFile = GetLastestDir(pszPath, LastestFilePath);
	if (NULL == pLastestFile)
	{
		return FALSE;
	}

	sscanf(pFile, "%4d%2d%2d", &o_year, &o_month, &o_day);
	sscanf(pLastestFile, "%4d%2d%2d", &l_year, &l_month, &l_day);

	if (30 <= (daysum(l_year, l_month, l_day) - daysum(o_year, o_month, o_day)))
	{
		sprintf(szFile, "%s/%s", pszPath, pFile);
		DeleteDir(szFile);
	}

	return FALSE;
} /* -----  end of method DeleteOldestFile  ----- */

/*  ******************************************Э�鴦������***************************** */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_SetReSendBuf
 *  Description:  �����ط�����
 *     Argument:  ͨѶ��
 *     Argument:  �ط�����
 *     Argument:  �ط����ĳ���
 *		 Return:
 * =====================================================================================
 */
BOOL CYmBreakPoint::SelfDef_Trans_SetReSendBuf(BYTE byLineNo, char *chBuf, int len)
{
	if (len > 256)
	{
		return FALSE;
	}

	self_data[byLineNo][0].byResendLen = len;
	memcpy(self_data[byLineNo][0].byReSendBuf, chBuf, len);
	return TRUE;
} /* -----  end of function SelfDef_Trans_SetReSendBuf  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_GetReSendBuf
 *  Description:  ����ط�����
 *     Argument:  ͨѶ��
 *     Argument:  �ط�����
 *     Argument:  �ط����ĳ���
 *		 Return:
 * =====================================================================================
 */
int CYmBreakPoint::SelfDef_Trans_GetReSendBuf(BYTE byLineNo, char *chBuf)
{
	int len = 0;
	len = self_data[byLineNo][0].byResendLen;
	memcpy(chBuf, self_data[byLineNo][0].byReSendBuf, len);

	return len;
} /* -----  end of function SelfDef_Trans_GetReSendBuf  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_getSendBufFromMem
 *  Description:  ���ڴ��л�ñ���
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
WORD CYmBreakPoint::SelfDef_Trans_getSendBufFromMem(BYTE byLineNo,
													BYTE bySlaveAddr,
													BYTE *buf,
													WORD uiMaxLen)
{
	MAPITEM *pTrans = m_pPIMapTab; /* ת���ṹ�� */
	BYTE byStep = 0;			   /* �ֲ����� ÿ��ת��32������ */
	BYTE byModuleNo = 0;		   /* ģ��� */
	WORD wYmStart = 0;			   /* ���η��͵���ʼ��� */
	WORD wYmTotalNum = 0;		   /* ���õ���ң������ */
	WORD wSendNum = 0;			   /* ���η������� */
	WORD len = 0;				   /* Ҫ���ص����ݳ��� */
	int i;
	DWORD dwValue = 0;
	BYTE byValid = 0;
	struct tm *pTm;
	float fValue = 0.0;

	/* ���ģ��� */
	byModuleNo = 0;
	/* ���ת��ָ�� */
	pTrans = m_pPIMapTab;
	/* ��õ�ǰת������ */
	byStep = (MODULE_SELDUNION(byLineNo, byModuleNo)) & 0xff;
	/* ���㵱ǰң����� */
	wYmStart = byStep * (WORD)32;

	/* �鿴�����ȷ�� */
	wYmTotalNum = GetPntSum(2);
	if (wYmStart >= wYmTotalNum)
	{
		/* ���ִ���ⲽ����0 */
		MODULE_SELDUNION(byLineNo, byModuleNo) = (0x0);
		return 0;
	}

	/* ���㷢������ */
	wSendNum = wYmTotalNum - wYmStart;
	if (32 < wSendNum) /* ����32�����ݷ���32������ */
	{
		wSendNum = 32;
		/* �������ò��� */
		byStep++;
		MODULE_SELDUNION(byLineNo, byModuleNo) = byStep;
	}
	else /* ����32��˵������������ */
	{
		MODULE_SELDUNION(byLineNo, byModuleNo) = (0x0);
	}

	if (uiMaxLen < (wSendNum * 5 + 13))
	{
		MODULE_SELDUNION(byLineNo, byModuleNo) = (0x0);
		return 0;
	}

	time_t timep;
	timep = time(NULL);
	pTm = localtime(&timep);

	buf[len++] = 0x68;		  /* ��ʼ�ֽ� */
	buf[len++] = 0x00;		  /* ַַ�� */
	buf[len++] = bySlaveAddr; /* ��ַ�� ���Ϊ256 */
	buf[len++] = 0xf0;		  /* �������� */
	buf[len++] = HIBYTE(pTm->tm_year + 1900);
	buf[len++] = LOBYTE(pTm->tm_year + 1900); /* �� */
	buf[len++] = (BYTE)pTm->tm_mon + 1;		  /* �� */
	buf[len++] = (BYTE)pTm->tm_mday;		  /* �� */
	buf[len++] = (BYTE)pTm->tm_hour;		  /* ʱ */
	buf[len++] = (BYTE)pTm->tm_min;			  /* �� */
	buf[len++] = HIBYTE(wYmStart);
	buf[len++] = LOBYTE(wYmStart); /* ��ʼ���^ */
	buf[len++] = (BYTE)wSendNum;   /* ���� */

	pTrans += wYmStart;
	for (i = 0; i < wSendNum; i++)
	{
		if (pTrans[i].wStn <= 0 || pTrans[i].wPntNum <= 0) /*û��ʵ��ת������*/
		{
			dwValue = 0;
			byValid = 0x01;
			printf("no content i = %d wSendNum = %d\n", i + wYmStart, wSendNum);
			continue;
		}
		else
		{
			dwValue = static_cast<DWORD>(CalcPulseRipeVal(pTrans[i].wStn,
									   pTrans[i].wPntNum,
									   m_dwPIBuf[i]));

			byValid = 0x00;
			WORD wSerialNo = pTrans[i].wStn - 1;
			if (m_pMethod->GetDevCommState(wSerialNo) != COM_DEV_NORMAL)
			{
				byValid = 0x01;
				printf("device abnormal \n");
			}
		}

		/*Ʒ��������*/
		buf[len++] = byValid;

		fValue = (float)dwValue;
		memcpy(buf + len, &fValue, sizeof(float));
		/* memcpy(buf+len,&dwValue,sizeof(DWORD)); [> ֵ <] */
		len += 4;
	}

	return len;
} /* -----  end of function SelfDef_Trans_getSendBufFromMem  ----- */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_getSendBufFromFile
 *  Description:  ���ļ��л�ñ���
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
WORD CYmBreakPoint::SelfDef_Trans_getSendBufFromFile(BYTE byLineNo,
													 BYTE bySlaveAddr,
													 BYTE *buf,
													 WORD uiMaxLen)
{
	MAPITEM *pTrans = m_pPIMapTab; /* ת���ṹ�� */
	BYTE byStep = 0;			   /* �ֲ����� ÿ��ת��32������ */
	BYTE byModuleNo = 0;		   /* ģ��� */
	WORD wYmStart = 0;			   /* ���η��͵���ʼ��� */
	WORD wYmTotalNum = 0;		   /* ���õ���ң������ */
	WORD wSendNum = 0;			   /* ���η������� */
	WORD len = 0;				   /* Ҫ���ص����ݳ��� */
	BYTE tmpBuf[13];
	int tmplen = 0;
	char chFileName[128];

	char chPath[256];
	char *pchFile = NULL;

	sprintf(chPath, "/mnt/data/bus%.2d/", byLineNo);
	pchFile = GetOldestDriFile(chPath, chFileName);
	if (NULL == pchFile)
	{
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		return 0;
	}

	/* ���ģ��� */
	byModuleNo = 0;
	/* ���ת��ָ�� */
	pTrans = m_pPIMapTab;
	/* ��õ�ǰת������ */
	byStep = (MODULE_SELDUNION(byLineNo, byModuleNo)) & 0xff;
	/* ���㵱ǰң����� */
	wYmStart = byStep * (WORD)32;

	/* �鿴�����ȷ�� */
	wYmTotalNum = GetPntSum(2);
	if (wYmStart >= wYmTotalNum)
	{
		/* ���ִ���ⲽ����0 */
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		return 0;
	}

	tmplen = ReadFromFile(pchFile,
						  tmpBuf,
						  13,
						  (32 * 5 + 13) * byStep);
	if (tmplen != 13)
	{
		DeleteFile(pchFile);
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		return 0;
	}

	/* ���㷢������ */
	wSendNum = tmpBuf[12];
	if (32 < wSendNum) /* ����32�����ݷ���32������ */
	{
		DeleteFile(pchFile);
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		return 0;
	}

	if (uiMaxLen < (wSendNum * 5 + 13))
	{
		DeleteFile(pchFile);
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		return 0;
	}

	len = ReadFromFile(pchFile,
					   buf,
					   (wSendNum * 5 + 13),
					   (32 * 5 + 13) * byStep);
	if (len != wSendNum * 5 + 13)
	{
		DeleteFile(pchFile);
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		return 0;
	}

	if (32 == wSendNum)
	{
		/* �������ò��� */
		byStep++;
		MODULE_SELDUNION(byLineNo, byModuleNo) = byStep;
		if (byStep * (WORD)32 == wYmTotalNum)
		{
			MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
			DeleteFile(pchFile);
		}
		printf("line = %d byStep = %d\n", __LINE__, byStep);
	}
	else /* ����32��˵������������ */
	{
		MODULE_SELDUNION(byLineNo, byModuleNo) = 0;
		DeleteFile(pchFile);
		printf("line = %d byStep = %d\n", __LINE__, byStep);
	}

	return len;
} /* -----  end of function SelfDef_Trans_getSendBufFromFile  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_getSendBuf
 *  Description:  ��֯���ͱ���
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
WORD CYmBreakPoint::SelfDef_Trans_getSendBuf(BYTE byLineNo,
											 BYTE bySlaveAddr,
											 BYTE *buf,
											 WORD uiMaxLen)
{

	switch (MODULE_SELDUNION_WORD(byLineNo, 0))
	{
		/* ���ļ��л�ñ��� */
	case DATAFROMFILE:
		return SelfDef_Trans_getSendBufFromFile(byLineNo,
												bySlaveAddr,
												buf,
												uiMaxLen);
		break;

	case DATAFROMMEM:
		return SelfDef_Trans_getSendBufFromMem(byLineNo,
											   bySlaveAddr,
											   buf,
											   uiMaxLen);
		break;

	case DATAFROMRESEND:
		return SelfDef_Trans_GetReSendBuf(byLineNo,
										  (char *)buf);
		break;

	default:
		break;
	} /* -----  end switch  ----- */

	return 0;
} /* -----  end of function SelfDef_Trans_getSendBuf  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_SetResendFlag
 *  Description:  �����ط���ʶ
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
void CYmBreakPoint::SelfDef_Trans_SetResendFlag(BYTE byLineNo, BOOL bFlag)
{
	self_data[byLineNo][0].bReSendFlag = bFlag;
} /* -----  end of function SelfDef_Trans_SetResendFlag  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_GetResendFlag
 *  Description:
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
BOOL CYmBreakPoint::SelfDef_Trans_GetResendFlag(BYTE byLineNo)
{
	return self_data[byLineNo][0].bReSendFlag;
} /* -----  end of function SelfDef_Trans_GetResendFlag  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_DataFileExist
 *  Description:  ���������ļ��Ƿ����
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
BOOL CYmBreakPoint::SelfDef_Trans_DataFileExist(BYTE byLineNo, BYTE bySlaveAddr)
{

	char chPath[256];
	char *pchFile = NULL;
	char chFileName[128];

	/* if( 0  == MODULE_SELDLINK( byLineNo,  0 ) ) */
	/* { */
	/* return FALSE; */
	/* } */

	sprintf(chPath, "/mnt/data/bus%.2d/", byLineNo);
	pchFile = GetOldestDriFile(chPath, chFileName);
	if (NULL == pchFile)
	{
		return FALSE;
	}

	MODULE_SELDUNION_WORD(byLineNo, 0) = DATAFROMFILE;
	return TRUE;
} /* -----  end of function SelfDef_Trans_DataFileExist  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_isTimeToSend
 *  Description:  �鿴�Ƿ񵽷���ʱ��
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
BOOL CYmBreakPoint::SelfDef_Trans_isTimeToSend(BYTE byLineNo, BYTE bySlaveAddr)
{
	time_t timep;
	timep = time(NULL);
	struct tm *tm = localtime(&timep);

	if (0 == tm->tm_min % 5)
	{
		if (tm->tm_min == MODULE_SELDMIN_WORD(byLineNo, 0))
		{
			if (DATAFROMMEM == MODULE_SELDUNION_WORD(byLineNo, 0) && 0 == MODULE_SELDUNION(byLineNo, 0))
			{
				return FALSE;
			}

			if (DATAFROMMEM != MODULE_SELDUNION_WORD(byLineNo, 0))
			{
				return FALSE;
			}
		}

		MODULE_SELDMIN_WORD(byLineNo, 0) = tm->tm_min;
		MODULE_SELDUNION_WORD(byLineNo, 0) = DATAFROMMEM;
		return TRUE;
	}

	return FALSE;

} /* -----  end of function SelfDef_Trans_isTimeToSend  ----- */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_isNeedSend
 *  Description:  ��ǰ�Ƿ���Ҫ��������
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
BOOL CYmBreakPoint::SelfDef_Trans_isNeedSend(BYTE byLineNo, BYTE bySlaveAddr)
{

	/* �ط�ʱΪ��ͨ״̬��HMIһֱ�ط��͵�ʱ��洢�����ط�ʱΪͨѶ״̬���������ȴ��ļ��еõ� */
	if (SelfDef_Trans_isTimeToSend(byLineNo, bySlaveAddr))
	{
		return TRUE;
	}

	if (SelfDef_Trans_GetResendFlag(byLineNo) && self_data[byLineNo][0].bIsSending)
	{
		MODULE_SELDUNION_WORD(byLineNo, 0) = DATAFROMRESEND;
		return TRUE;
	}
	else
	{
		/* �����ļ� */
		if (SelfDef_Trans_DataFileExist(byLineNo, bySlaveAddr))
		{
			return TRUE;
		}
	}

	/* �ļ�û�� ��ʱ�� */
	return FALSE;
} /* -----  end of function SelfDef_Trans_isNeedSend  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_saveSendBuf
 *  Description:  �洢���ݵ�buf
 *     Argument:
 *		 Return:
 * =====================================================================================
 */
void CYmBreakPoint::SelfDef_Trans_saveSendBuf(BYTE byLineNo,
											  BYTE bySlaveAddr,
											  BYTE *buf,
											  WORD len)
{
	char chPath[256], chPath1[256];
	char *pchFile = NULL;
	char filename[128] = "";

	/* �鿴�����ļ��� */
	sprintf(chPath, "/mnt/data/bus%.2d/", byLineNo);
	if (!IsDir(chPath))
	{
		CreateDir(chPath);
	}

	/* �鿴�������ļ��� */
	sprintf(chPath1, "%s/%.4d%.2d%.2d%",
			chPath,
			MAKEWORD(buf[5], buf[4]),
			buf[6],
			buf[7]);
	if (!IsDir(chPath1))
	{
		CreateDir(chPath1);
	}

	/* �����ļ� */
	sprintf(filename, "%s/%.2d%.2d.l",
			chPath1,
			buf[8],
			buf[9]);

	/* ɾ����õ��ļ��� */
	DeleteOldestFile(chPath);

	WriteToFile(filename, buf, len);
} /* -----  end of function SelfDef_Trans_saveSendBuf  ----- */
/*
 * ===  FUNCTION  ======================================================================
 *         Name:  SelfDef_Trans_getSendMessage
 *  Description:  ���ʹ���
 *     Argument:  ͨѶ��
 *                ģ���
 *                ����buf
 *                ������󳤶�
 *		 Return:  ʵ��buf����
 * =====================================================================================
 */
WORD CYmBreakPoint::SelfDef_Trans_getSendMessage(BYTE byLineNo, BYTE byModuleNo, BYTE *pBuf, WORD nMax)
{

	BYTE bySlaveAddr = m_wDevAddr;
	/* �Ƿ���Ҫ���� ��ʱ�䷢����Ϊ����1 ���ļ�����Ϊ����2*/
	if (!SelfDef_Trans_isNeedSend(byLineNo, bySlaveAddr))
	{
		return 0;
	}
	/* ���ͱ�����֯ */
	return SelfDef_Trans_getSendBuf(byLineNo, bySlaveAddr, pBuf, nMax);
}
