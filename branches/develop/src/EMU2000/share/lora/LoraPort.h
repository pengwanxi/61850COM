#pragma once

#ifndef _CLORAPORT_
#define _CLORAPORT_

#include "../BasePort.h"

class CLoraPort :public CBasePort
{
public:
	CLoraPort();
	~CLoraPort();
	virtual BOOL   IsPortValid(void);
	virtual BOOL   OpenPort(char* lpszError = NULL);
	virtual void   ClosePort(void);
	virtual int	   AsyReadData(BYTE *pBuf, int nRead);
	virtual int	   WritePort(BYTE *pBuf, int nWrite);
};

#endif
