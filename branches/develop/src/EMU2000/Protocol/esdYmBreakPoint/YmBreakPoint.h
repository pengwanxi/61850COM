#pragma once
#include "Protocol_esdYmBreakPoint.h"
class CYmBreakPoint : public CProtocol_esdYmBreakPoint
{
public:
	CYmBreakPoint();
	~CYmBreakPoint();
	//ʱ�䴦������
	virtual void    TimerProc(void);
	//��ʼ��Э������
	virtual BOOL Init(BYTE byLineNo);
	//��ȡЭ�����ݻ���
	virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
	//�����յ������ݻ��� 
	virtual BOOL ProcessProtocolBuf(BYTE * pBuf, int len);
	BOOL CreateDir(char *pszPath);
	BOOL IsDir(char *pszPath);
	DWORD WriteToFile(char *pszFileName, BYTE *pszBuf, int len);
	DWORD ReadFromFile(char *pszFileName, BYTE *pszBuf, int len, DWORD dwReadPos);
	BOOL IsFile(char *pszFileName);
	void DeleteDir(char *pchPath);
	char * GetOldestDir(char *pchPath, char *destPath);
	char * GetLastestDir(char *pchPath, char *destPath);
	char * GetLatestDriFile(char *pchPath, char* filename);
	char *GetOldestDriFile(char *pchPath, char *filename);
	char * GetDirFile(char *pPath);
	BOOL DeleteFile(char *pszFileName);
	int daysum(int y, int m, int d);
	BOOL DeleteOldestFile(char *pszPath);
	BOOL SelfDef_Trans_SetReSendBuf(BYTE byLineNo, char *chBuf, int len);
	int SelfDef_Trans_GetReSendBuf(BYTE byLineNo, char *chBuf);
	WORD SelfDef_Trans_getSendBuf(BYTE byLineNo, BYTE bySlaveAddr, BYTE *buf, WORD uiMaxLen);
	void SelfDef_Trans_SetResendFlag(BYTE byLineNo, BOOL bFlag);
	BOOL SelfDef_Trans_GetResendFlag(BYTE byLineNo);
	BOOL SelfDef_Trans_DataFileExist(BYTE byLineNo, BYTE bySlaveAddr);
	BOOL SelfDef_Trans_isTimeToSend(BYTE byLineNo, BYTE bySlaveAddr);
	BOOL SelfDef_Trans_isNeedSend(BYTE byLineNo, BYTE bySlaveAddr);
	void SelfDef_Trans_saveSendBuf(BYTE byLineNo, BYTE bySlaveAddr, BYTE *buf, WORD len);
	WORD SelfDef_Trans_getSendMessage(BYTE byLineNo, BYTE byModuleNo, BYTE* pBuf, WORD nMax);
	void SelfDef_Trans_workSend(BYTE byLineNo);
	WORD SelfDef_Trans_getSendBufFromFile(BYTE byLineNo, BYTE bySlaveAddr, BYTE *buf, WORD uiMaxLen);
	WORD SelfDef_Trans_getSendBufFromMem(BYTE byLineNo, BYTE bySlaveAddr, BYTE *buf, WORD uiMaxLen);
	QWORD   m_dwPIBuf[2048];	//ͬ�ϣ�����ң��!
};

