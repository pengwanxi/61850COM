#include "Clog.h"
#include <time.h>
#include<stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../../share/typedef.h"
#include <errno.h>
#include "../../share/global.h"
Clog::Clog()
{
	strcpy(m_szLogkey, "reboot");
	pthread_mutex_init(&m_mutex_lock, NULL);
}


Clog::~Clog()
{
	pthread_mutex_destroy(&m_mutex_lock);
}


void Clog::writeLog(char * logbuf, int size)
{
	pthread_mutex_lock(&m_mutex_lock);
	char chTime[32] = { 0 };
	time_t now;
	struct tm timenow;
	time(&now);
	localtime_r(&now, &timenow);
	sprintf(chTime, "%04u-%02u-%02u %02u:%02u:%02u", 1900 + timenow.tm_year
		, timenow.tm_mon + 1, timenow.tm_mday, timenow.tm_hour, timenow.tm_min, timenow.tm_sec);

	system("mkdir -p /mnt/log; mkdir -p /mnt/log/backup");
	char fileName[100] = { "/mnt/log/curlog" };
	char szLogContent[200] = { 0 };
	sprintf(fileName, "%s%s.txt", "/mnt/log/curlog", m_szLogkey);
	sprintf(szLogContent, "%s : %s\n", chTime, logbuf);
	
	delFile();
	appendToFile(fileName, szLogContent, strlen(szLogContent));

	pthread_mutex_unlock(&m_mutex_lock);
//	printf("writelog end sharelib\n");
	return;
}

bool Clog::appendToFile(const char *fileName, const char *content, int fileLength)
{
	FILE *fp = NULL;
	try
	{
		fp = fopen(fileName, "ab");
		fseek(fp,0,SEEK_END);//定位到文件的最后面
		if (fp == NULL)
		{
			return false;
		}
		long file_length = ftell(fp);

		if (file_length < 100 * 1024 )
		{
			//printf(" %s log file length = %ld \n", fileName, file_length);
			fwrite(content, 1, fileLength, fp);
			fclose(fp);
		}
		else
		{
			//printf(" %s log file length = %ld \n", fileName, file_length);
			fwrite(content, 1, fileLength, fp);
			fclose(fp);
			backupFile(fileName);
		}
	}
	catch (...)
	{
		fclose(fp);
		return false;
	}
	return true;
}


void Clog::backupFile(const char * fileName)
{
	printf(" %s log file backup \n", fileName);
	char chTime[32] = { 0 };
	time_t now;
	struct tm timenow;
	now = time(NULL);
	localtime_r(&now, &timenow);
	sprintf(chTime, "%04u-%02u-%02u_%02u_log_%s", 1900 + timenow.tm_year
		, timenow.tm_mon + 1, timenow.tm_mday, timenow.tm_hour , m_szLogkey );

	char szCmd[300] = { 0 };
	sprintf(szCmd, "mv  -f  %s  /mnt/log/backup/%s.txt ; rm -rf %s ;sync", fileName, chTime, fileName);
	system(szCmd);
}

void Clog::delFile()
{
//	printf(" del log file \n");
	char szPath[100] = { "/mnt/log/" };
	BYTE byFileNum = 0;
	char szGetFileNumber[] = { "ls -l /mnt/log/backup/ | grep \"^-\" | wc -l" };
	FILE * p_file = NULL;
	p_file = popen(szGetFileNumber, "r");
	char szOutput_num[100] = { 0 };
	fgets(szOutput_num, 200, p_file);
	byFileNum = atoi(szOutput_num);
	pclose(p_file);
//	printf("/mnt/log/backup totalfiles = %d\n", byFileNum);

	if (byFileNum <= 10)
		return;

	char szGetlastfileCmd[] = { "cd /mnt/log/backup/ ; ls -ltr | awk \'{ print $9 }\'| head -n 2" };
	p_file = popen(szGetlastfileCmd, "r");
	if (!p_file)
	{
		printf("file %s line =%d error\n" , __FILE__,__LINE__);
		return;
	}

	char szOutput[100] = { 0 };
	while ( fgets(szOutput , 200 , p_file ) )
	{
		rtrim(szOutput);
		ltrim(szOutput);
		if( strlen(szOutput) == 0 )
			continue;

		char buf[100] = { 0 };
		sprintf(buf, "%s will del\n", szOutput);
		printf(buf);
		char szDelFile[100] = { 0 };
		sprintf(szDelFile, "rm -rf /mnt/log/backup/%s;sync", szOutput);
		system( szDelFile );
		//writeLog(buf);
	}
	pclose(p_file);
}

void Clog::setLogKey(char * sLogKey)
{
	strcpy(m_szLogkey, sLogKey);
}
