#pragma once
#include "CFtpClient.h"
#ifdef  ARM
#include "../libiec61850/.install/include/iec61850_client.h"
#endif

class C61850TransferFile : public CFtpClient
{
public:
	C61850TransferFile();
	virtual ~C61850TransferFile();
	virtual BOOL downLoad(const char * pFileName);
	virtual BOOL connect() { return TRUE; }
	virtual BOOL login() { return TRUE; }
	virtual BOOL chdir(string strPath) { return TRUE; }
	virtual BOOL fileList(string oFileName, string destDirect);
	BOOL split(vector<string>&vec, char * pStrDest);
	string getleftstring(string strDest, string strdelimiter);
#ifdef ARM

	void showDirectory(IedConnection con);
	void getFile(IedConnection con, const char * localFilename, const char * devPath);
#endif
	virtual FILE_PROPERTY * GetLast();
	virtual FILE_PROPERTY * GetPrev();
};

