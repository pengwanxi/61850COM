#pragma once
#include "BaseWebPage.h"
#include "RunStatusPage.h"
class CSysOperatePage :
	public CBaseWebPage
{
public:
	CSysOperatePage();
	~CSysOperatePage();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *pLog);
public:

	void SysRebootOperation();
	void SysRebootAndNewProject();
};

