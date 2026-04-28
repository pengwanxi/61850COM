#include "RunStatusPage.h"


CRunStatusPage::CRunStatusPage()
{
	s_dns = "114.114.114.114";
	s_memtotal = "0M";
	s_memfree = "0M";
	s_membuffer = "0M";
	s_memswap = "0M";
	s_memcache = "0M";
	s_diskram = "0K/0K";
	s_diskextend = "0K/0K";
}


CRunStatusPage::~CRunStatusPage()
{
}

BOOL CRunStatusPage::getJSONStructFromWebPage(Json::Value &root)
{
	int i;
	Json::Value NetStatus;
	Json::Value ComStatus;
	Json::Value MemStatus;
	Json::Value DiskStatus;
	
	root["version"] = GetVerSion();
	root["systime"] = GetSysTime();
	root["uptime"] = GetUptime();

	////网卡状态
	GetNetStatInformation(NetStatus);
	root["dns"] = s_dns;
	root["netstatus"] = NetStatus;

	GetComStatInformation(ComStatus);
	root["comstatus"] = ComStatus;

	//CPU状态
	root["cpustatus"] = GetCpuInformation();

	////内存状态
	GetMemInformation(MemStatus);
	MemStatus["total"] = s_memtotal;
	MemStatus["free"] = s_memfree;
	MemStatus["buffer"] = s_membuffer;
	MemStatus["cache"] = s_memcache;
	MemStatus["swap"] = s_memcache;
	root["memstatus"] =MemStatus;

	//硬盘空间
	GetDiskInformation(DiskStatus);
	DiskStatus["ram"] = s_diskram;
	DiskStatus["extend"] = s_diskextend;
	root["disk"] = DiskStatus;

	return FALSE;
}

BOOL CRunStatusPage::procCmd(BYTE byCmd)
{

	return FALSE;
}

void CRunStatusPage::Init()
{
	
}

void CRunStatusPage::setLog(Clog * pLog )
{
	m_log = pLog;
}

void CRunStatusPage::GetComStatInformation(Json::Value &ComStatus)
{
	Json::Value comdata;
	FILE *fp;
	string temp;
	string port ;
	char buf[300] = { 0 };
	string strFmt = "sed -n '02p' /proc/tty/driver/uartch |awk -F'[: ]' '{print $10,$12}'|awk '{OFS=RS}NF=NF'";
	BYTE byLineNum = 0;
	char sBusLine[] = BUS_PATH;
	CProfile Profile(sBusLine);

	byLineNum = (BYTE)Profile.GetProfileInt((char *)"LINE-NUM", (char *)"NUM", 0);
	
	for (BYTE i = 0; i < byLineNum; i++)
	{
		string temp;
		char sPort[] = "port";
		char sPara[] = "para";
		char sInterval[] = "internal";
		char sSect[] = "PORT";
		int iInterval = 0;
		int sizebuff = 0;
		char sbuffer[200] = { 0 };
		sizebuff = sizeof(sbuffer);
		char sDllPath[200];
		BOOL bPause = FALSE;
		memset(sDllPath, 0, sizeof(sDllPath));
		char sTemp[200];
		memset(sTemp, 0, sizeof(sTemp));

		memset(sbuffer, 0, sizebuff);

		//获取通讯口类型
		sprintf(sTemp, "%s%02d", sPort, i + 1);
		Profile.GetProfileString(sSect, sTemp, (char *)"NULL", sbuffer, sizebuff);		
		if ((sbuffer[0] == 'C') && (sbuffer[1] == 'O') && (sbuffer[2] == 'M'))//只统计串口
		{
			temp = sbuffer;
			temp.erase(0 , 3);
			
			char buf[50];
			int num = 0;
			memset(buf,'\0',sizeof(buf));
			strcpy(buf,temp.c_str());
			char *token;
			token = strtok(buf,"_:,");
			while (token != NULL)
			{
				if (num == 0)
				{
					comdata["style"] = token;

				}
				else if (num == 1)
				{
					comdata["name"] = token;
				
					memset(sTemp, 0, sizeof(sTemp));
					sprintf(sTemp, "%02d",atoi(token)+1);
					port = sTemp;

				}
				else if (num == 2)
				{
					comdata["baudrate"] = token;

				}
				else if (num == 3)
				{
					comdata["paritybits"] = token;

				}
				else if (num == 4)
				{
					comdata["databits"] = token;

				}
				else if (num == 5)
				{
					comdata["stopbits"] = token;

				}
				token = strtok(NULL, "_:,");
				num++;
			}
			strFmt.replace(8,2,port);//打开proc/tty/driver/uartch文件第几行
			if ((fp = popen(strFmt.c_str(), "r")) == NULL)
			{
				perror("Fail to popen\n");
				exit(1);
			}

			float fcpuPercent = 0.0;
			int linenum = 0;
			
			memset(buf, '\0', sizeof(buf));
			while (fgets(buf, 200, fp) != NULL)
			{
				temp = buf;
				temp.erase(temp.end()-1);
				if (linenum==0)
				comdata["rx"] = temp;
				if (linenum==1)
				comdata["tx"] =temp;
				linenum++;
			}
			pclose(fp);

			comdata["status"] = "connected";

			ComStatus.append(comdata);

		}
		else
		{
			continue;
		}
		

	}


}
string CRunStatusPage::GetCpuInformation()
{
	FILE *fp;
	string temp;
	char str[10] = { 0 };
	char buf[200] = { 0 };
	string strFmt = "mpstat 1 1 | grep all | grep -v Average | awk '{ print $3  \"\\n\" $5}'  ";

	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}

	float fcpuPercent = 0.0;
	while (fgets(buf, 200, fp) != NULL)
	{
		fcpuPercent += atof(buf);
	}
	sprintf(str, "%.2f%s", fcpuPercent,"%");
	temp = str;
	pclose(fp);
	return  temp;
}
void CRunStatusPage::GetDiskInformation(Json::Value &DiskStatus)
{
	FILE *fp;
	FILE *fp_used;
	char buf[200] = { 0 };
	char buf_used[200] = {0};
	if ((fp = popen(" df | awk '{ print $2}'| grep -v \"1K- *\" ", "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}

	int size = 0;
	while (fgets(buf, 200, fp) != NULL)
	{
		size += atoi(buf);
	}


	if ((fp_used = popen("  df | awk '{ print $3}'| grep -v \"Used\"", "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}

	int size_used = 0;
	while (fgets(buf_used, 200, fp_used) != NULL)
	{
		size_used += atoi(buf_used);
	}


	s_diskram = std::to_string(size)+"KB"+"/"+std::to_string(size_used)+"KB";

	pclose(fp);
	
}
void CRunStatusPage::GetMemInformation(Json::Value &MemStatus)
{

	FILE *fp;
	string temp;
	int linenum = 0;
	char str[10] = { 0 };
	char buf[200] = { 0 };
	string strFmt = " head -5 /proc/meminfo|awk '{print $2}'  ";

	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}
	float fcpuPercent = 0.0;
	while (fgets(buf, 200, fp) != NULL)
	{
		if (linenum == 0)
		{
			s_memtotal = buf;
			s_memtotal.erase(s_memtotal.end() - 1);
		}			
		else if (linenum == 1)
		{
			s_memfree = buf;
			s_memfree.erase(s_memfree.end()-1);
		}
		else if (linenum == 2)
		{
			s_membuffer = buf;
			s_membuffer.erase(s_membuffer.end() - 1);
		}
		else if (linenum == 3)
		{
			s_memcache = buf;
			s_memcache.erase(s_memcache.end()-1);
		}
		else if (linenum == 4)
		{
			s_memswap = buf;
			s_memswap.erase(s_memswap.end()-1);
		}
		linenum++;
	}
	pclose(fp);
	

}
void  CRunStatusPage::GetNetStatInformation(Json::Value &NetStatus)
{
	FILE *fp;
	int i;
	string temp;
	
	string strFmt = "/sbin/ifconfig eth3|sed -n '3p'|awk '{print $3}'";
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

	char sKey[200] = { 0 };

	for (BYTE byNo = 1; byNo < 5; byNo++)
	{
		Json::Value netdata;
		int num = 0;
		
		memset(sKey, 0, sizeof(sKey));

		////获取网卡名字
		sprintf(sKey, "%s%02d", sNetCard, byNo);
		memset(sTemp, 0, sizeof(sTemp));
		Profile.GetProfileString(sSect, sKey, (char *)"NULL", sTemp, sizebuff);
		netdata["name"] = sTemp;
		strFmt.replace(15,4,sTemp);

		//获取网卡规格
		netdata["style"] = "10/100M";

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

		char buf[200] = { 0 };
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
		netdata["status"] =temp;
		NetStatus.append(netdata);

	}

}
string CRunStatusPage::GetUptime()
{
	FILE *fp;
	string temp;
	char buf[200] = {0};
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
string CRunStatusPage::GetVerSion()
{
	string temp;
	DWORD ver = EMU2000_VERSION;
	FILE * pFile = NULL;
	pFile = fopen("/myapp/2000_version", "r");
	char szSvnVersion[10] = {0};
	memset(szSvnVersion, '\0', sizeof(szSvnVersion));
	fread(szSvnVersion, sizeof(szSvnVersion), 1, pFile);
	fclose(pFile);
	char szVersion[30] = {0};
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
string CRunStatusPage::GetSysTime()
{
	FILE *fp;
	string temp;
	char buf[200] = {0};
	memset(buf,'\0',sizeof(buf));
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
