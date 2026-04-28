#pragma once
#pragma once
#include "BaseWebPage.h"

class CCommunication:public CBaseWebPage
{
public:
	CCommunication();
	~CCommunication();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	void getBusLineVal(Json::Value & busJsonVal);
	void getProtocolParam(char * pszVal, WORD wBusNo, Json::Value &portData);
	void getCommunicationParam(char * pszVal, WORD wBusNo, Json::Value &portData);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *);

	string m_strProjectDirectoryPath;
};

