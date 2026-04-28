#pragma once

#include "../BasePort.h"

#define  INVALID_HANDLE		-1
class CCanNet :
	public CBasePort
{
public:
	CCanNet();
	~CCanNet();

	void modifyBaudrate();
	virtual BOOL   IsPortValid(void);
	virtual BOOL   OpenPort(char* lpszError = NULL);
	bool openSocket();
	virtual void   ClosePort(void);
	virtual int	   ReadPort(BYTE *pBuf, int nRead);
	virtual int	   WritePort(BYTE *pBuf, int nWrite);
	virtual int	   AsyReadData(BYTE *pBuf, int nRead);
	virtual int    AsySendData(BYTE *pBuf, int nWrite);

private:
	int m_socket = INVALID_HANDLE ;
	int m_port = 0 ;
	int m_baudrate = 0 ;
};

