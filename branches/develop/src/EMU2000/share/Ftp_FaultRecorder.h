#pragma once
#include "CFtpClient.h"
#include "CFtpServer.h"
#include "CTcpPortServer.h"
#include <string>
#include <vector>
using namespace std;

struct st_ftpClientInfo
{
	char username[ 100 ];
	char passwd[ 100 ];
	char ip[ 100 ];
	UINT port;
};

class CFtp_FaultRecorder
{
public:
	CFtp_FaultRecorder();
	~CFtp_FaultRecorder();
	BOOL initListenServer(int port , char * pType , string strIp = "0.0.0.0");
	BOOL startUpFaultReorder();
	BOOL initThread();
	static void * threadFtp (void *);
	void ProcessFtp();
	void sendErrorStr(string strCode);
	void sendStr(BYTE * pstr, int size);
	BOOL getProto(BYTE * pBuf, int &len);
	BOOL getTransmitFileReady(BYTE *pBuf, int &len);
	BOOL initFtpServer();
	BOOL getSuccessProto(BYTE * pBuf, int &len);
	void setError(string strErr);
	void getErrString(char * pErr, int &szLen);
private:
	BOOL initFtp();
	BOOL ProcessProto(BYTE * pBuf, int len);
	BOOL deleteDevFileListFromFtpDirectory();
	BOOL deleteDevDirectoryFromFtpDirectory();
	BOOL ProcessDownLoad(BYTE * pBuf, int len);
	BOOL resetConfig();
	BOOL downloadFromDev();
	BOOL downloadLatestFaultFile();
	string getFaultRecorderReadiniPath();
	BOOL saveFileInfoToLocal(vector<string>& vec);
	BOOL isSameFileName(const FILE_PROPERTY * pOld, const FILE_PROPERTY * pNew);
	string getleftstring(string strDest, string strdelimiter);
	void GetDevIp(char * ip, int len);
	UINT GetDevPort();
	BOOL isNeedDownLoad(const FILE_PROPERTY * pFile, const char * pdevIp, CProfile * pProfile);
	CProfile * GetProfile();
	BOOL initFtpClient();
	BOOL split(vector<string>&vec, char * pStrDest);
	void closeThread();
	void sendCloseMsg();
	void deleteftpobj();
	void createFaultRecorderIni();

private:
	char FILE_DIRECTORY[100] ;
	char FILE_READED_NAME[100] ;
	CFtpClient * m_pFtpClient ;
	CFtpServer * m_pFtpServer ;
	CTcpPortServer  *m_pTcpPortServer ;
	string m_listenIp;
	UINT m_listenPort;
	string m_typeCom ;
	pthread_t m_threadId ;
	const BYTE VECTOR_SIZE ; // eip eport , euser , epasswd
	enum{ eip = 0 , eport , euser , epasswd };
	vector<string> m_vec; // save ip port username passwd
	string m_strErr;
	enum
	{
		p_ready, p_err ,p_invalid
	};
	BYTE m_byProtoType ;
	UINT m_ftpServerPort ;
	CFtpServer::CUserEntry *m_pftpServerUser;
	vector<string> m_vDownLoadDevRecord;
};



