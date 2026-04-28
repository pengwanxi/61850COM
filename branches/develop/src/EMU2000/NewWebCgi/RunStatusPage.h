#pragma once
#include "BaseWebPage.h"
#define BUS_PATH	"/mynand/config/BusLine.ini"  //工程路径
#define MAX_LINE	200 //最大通讯行数据
#define		EMU2000_VERSION			0x01040100					/* EMU2000版本号 */
using namespace std;

class CRunStatusPage :public CBaseWebPage
{
public:
	CRunStatusPage();
	~CRunStatusPage();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *);


public:
	void GetNetStatInformation(Json::Value &NetStatus);
	void GetComStatInformation(Json::Value &ComStatus);
	void  GetMemInformation(Json::Value &MemStatus);
	void  GetDiskInformation(Json::Value &DiskStatus);
	string GetCpuInformation();
	string GetMemoryInformationt();
	string GetVerSion();
	string GetSysTime();
	string GetUptime();
	
public:
	string s_dns;
	string s_memtotal;
	string s_memfree;
	string s_membuffer;
	string s_memswap;
	string s_memcache;
	string s_diskram;
	string s_diskextend;




};

