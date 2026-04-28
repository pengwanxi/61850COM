#pragma once
#include "BaseWebPage.h"

#define BUS_PATH	"/mynand/config/BusLine.ini"  //工程路径
#define MAX_LINE	200 //最大通讯行数据
#define		EMU2000_VERSION			0x01040100					/* EMU2000版本号 */
using namespace std;

class CparSetPage :public CBaseWebPage
{
public:
	CparSetPage();
	~CparSetPage();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *);


public:
	void GetNetInformation(Json::Value &netdata);
	string PartGetUptime();
	string PartGetVerSion();
	string PartGetSysTime();

public:
	string s_disturbrecordPort;
	string s_disturbrecordType;
	string s_dns;
	string s_ntpip;
	string s_ntpmask;

	






};

