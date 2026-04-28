#include "ParSetPage.h"


CparSetPage::CparSetPage()
{
	s_dns = "114.114.114.114";
	s_disturbrecordPort = "1212";
	s_disturbrecordType = "REF615";
	s_ntpip = "192.168.1.0";
	s_ntpmask = "255.255.255.0";
	
}
CparSetPage::~CparSetPage()
{
	
}

BOOL CparSetPage::getJSONStructFromWebPage(Json::Value &root)
{
	Json::Value NetData;
	Json::Value  Option_list;
	root["par_version"] = PartGetVerSion();
	root["par_systime"] = PartGetSysTime();
	root["par_tuptime"] = PartGetUptime();
	
	GetNetInformation(NetData);

	root["par_dns"] = s_dns;
	root["par_net"] = NetData;
	
	if (s_disturbrecordPort=="NULL" && s_disturbrecordType=="NULL")
	{
		root["Distu_yes_no"] = "no";
	}
	else{
		root["Distu_yes_no"] = "yes";
		root["Distu_port"] = s_disturbrecordPort;
		root["Distu_type_current_option"] = s_disturbrecordType;

	}
	root["Distu_type_list"].append("ABB615");
	root["Distu_type_list"].append("SIEMENS686");

	root["ntp_ip"] = s_ntpip;
	root["ntp_mask"] = s_ntpmask;	


	return FALSE;
}


void CparSetPage::GetNetInformation(Json::Value &NetInfo)
{
	FILE *fp;
	int i;
	string temp;

	char sBusLine[] = BUS_PATH;
	CProfile Profile(sBusLine);

	char sNetCard[] = "NetCard";
	char sSect[] = "NetCard";
	char sDNS[] = "DNS";
	char sGateWay[] = "GateWay";
	char sSubNetMask[] = "SubNetMask";
	char sIP[] = "IP";
	char sRouteIp[] = "RouteIp";

	char sTemp[200] = { 0 };
	int sizebuff = sizeof(sTemp);

	char sSysDNS[] = "SYSTEM-DNS";
	char sSysDNS_Key[] = "DNS";
	char sTemp_Dns[20] = { 0 };
	int sizebuff_Dns = sizeof(sTemp_Dns);

	//获取DNS
	memset(sTemp, 0, sizeof(sTemp));
	Profile.GetProfileString(sSysDNS, sSysDNS_Key, (char *)"NULL", sTemp_Dns, sizebuff_Dns);
	s_dns = sTemp_Dns;

	//获取故障录波设置	
	memset(sTemp, 0, sizeof(sTemp));
	Profile.GetProfileString("FaultRecorder", "Port", (char *)"NULL", sTemp, sizebuff);
	s_disturbrecordPort = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	Profile.GetProfileString("FaultRecorder", "Type", (char *)"NULL", sTemp, sizebuff);
	s_disturbrecordType = sTemp;

	//NtpServer
	memset(sTemp, 0, sizeof(sTemp));
	Profile.GetProfileString("NtpServer", "NetMask", (char *)"NULL", sTemp, sizebuff);
	s_ntpmask = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	Profile.GetProfileString("NtpServer", "IPField", (char *)"NULL", sTemp, sizebuff);
	s_ntpip = sTemp;

	char sKey[200] = { 0 };
	for (BYTE byNo = 1; byNo < 5; byNo++)
	{
		Json::Value netdata;
		int num = 0;

		memset(sKey, 0, sizeof(sKey));

		//获取网卡IP
		sprintf(sKey, "%s%02d", sIP, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		Profile.GetProfileString(sSect, sKey, (char *)"NULL", sTemp, sizebuff);
		netdata["ip"] = sTemp;

		//获取网关
		sprintf(sKey, "%s%02d", sGateWay, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		Profile.GetProfileString(sSect, sKey, (char *)"NULL", sTemp, sizebuff);
		netdata["gateway"] = sTemp;

		//获取子网掩码
		sprintf(sKey, "%s%02d", sSubNetMask, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		Profile.GetProfileString(sSect, sKey, (char *)"NULL", sTemp, sizebuff);
		netdata["mask"] = sTemp;

		//获取指定路由
		sprintf(sKey, "%s%02d", sRouteIp, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		Profile.GetProfileString(sSect, sKey, (char *)"NULL", sTemp, sizebuff);
		
		netdata["route"] = sTemp;

		NetInfo.append(netdata);

	}

}

string CparSetPage::PartGetUptime()
{
	FILE *fp;
	string temp;
	char buf[200] = { 0 };
	memset(buf, '\0', sizeof(buf));
	string strFmt = "cat /proc/uptime |awk '{print $1}' ";//系统从启动到现在运行的时间单位秒
	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}
	while (fgets(buf, 200, fp) != NULL)
	{
		temp = buf;
	}
	pclose(fp);
	temp.erase(temp.end() - 1);//去掉最后一个换行符
	return  temp;
}
string CparSetPage::PartGetVerSion()
{
	string temp;
	DWORD ver = EMU2000_VERSION;
	FILE * pFile = NULL;
	pFile = fopen("/myapp/2000_version", "r");
	char szSvnVersion[10] = { 0 };
	memset(szSvnVersion, '\0', sizeof(szSvnVersion));
	fread(szSvnVersion, sizeof(szSvnVersion), 1, pFile);
	fclose(pFile);
	char szVersion[30] = { 0 };
	memset(szVersion, '\0', sizeof(szVersion));
	BYTE hh = ver >> 24;
	BYTE hl = ver >> 16 & 0xFF;
	BYTE lh = ver >> 8 & 0xFF;
	BYTE ll = ver & 0xFF;
	sprintf(szVersion, "EMU2000 Ver %d.%d.%d.%d:%s", hh, hl, lh, ll, szSvnVersion);
	temp = szVersion;
	temp.erase(temp.end() - 1);//去掉最后一个换行符
	return temp;
}
string CparSetPage::PartGetSysTime()
{
	FILE *fp;
	string temp;
	char buf[200] = { 0 };
	memset(buf, '\0', sizeof(buf));
	string strFmt = "date +%Y-%m-%d' '%H:%M:%M ";
	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}
	while (fgets(buf, 200, fp) != NULL)
	{
		temp = buf;
	}
	pclose(fp);
	temp.erase(temp.end() - 1);//去掉最后一个换行符
	return  temp;
}
BOOL CparSetPage::procCmd(BYTE byCmd)
{
	GetMemData(SHMPARSETBKEY);
	
	return FALSE;
}

void CparSetPage::Init()
{
	
	
}

void CparSetPage::setLog(Clog * pLog )
{
	m_log = pLog;
}

