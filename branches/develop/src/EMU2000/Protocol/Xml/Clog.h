#pragma once
#include <pthread.h>

class Clog
{
public:
	Clog();
	~Clog();
	void writeLog(char * buf, int size = 0 );
	bool appendToFile(const char *fileName, const char *content, int  fileLength);
	void backupFile(const char * fileName);
	void delFile();
	void setLogKey(char * sLogKey);
	char m_szLogkey[100];
	pthread_mutex_t   m_mutex_lock;
};

