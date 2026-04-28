#pragma once
#include "../share/global.h"
#include <vector>
#include <string>
#include "json/json.h"
using namespace  std ;
class CGetlogContent
{
public:
	CGetlogContent();
	~CGetlogContent();
	//获取日志列表
	BOOL getLogList();
	void getLogListFromBackup();
	void generateLogListJsonString();

	void getFileContent(string &strFilePath);
	void getFileContentJSonString(string &strFilePath);
	vector<string> m_logList;
	string m_strFileContent;
	string m_logPath;
	string m_JSonString;
	Json::Value JVal;
};

