#pragma once
#pragma once
#include "BaseWebPage.h"
class CTemplate_Manager:public CBaseWebPage
{
public:
	CTemplate_Manager();
	~CTemplate_Manager();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	void getProtocol_list(Json::Value &protoList, WORD wCurItem = 0);
	void getModule_list(Json::Value &moduleList, WORD wProtocolCurItem=0, WORD wTemplateCurItem =0 );
	void getModule_file_content(string &fileName, Json::Value &protoList);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *);

	string m_strRoot;
	vector<string> m_vProtocolKeyArray;
	vector<string> m_vModuleKeyArray;
};

