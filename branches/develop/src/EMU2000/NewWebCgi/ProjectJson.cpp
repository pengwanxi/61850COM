#include "ProjectJson.h"

CProJson::CProJson()
{
		
}
CProJson::~CProJson()
{

}
BOOL CProJson::getJSONStructFromWebPage(Json::Value &root)
{
	Json::Value modbusrtu;
	DirfileInformation(PROJECT_PATH, modbusrtu);
	root = modbusrtu;

	SaveJsontoFile(SAVE_JSON_FILE,modbusrtu);//JSON报文保存至文件接口测试
	
	printf("***************导入工程接口测试*********************\n");

	SaveJSonInformationBus(modbusrtu);
	SaveJSonInformationZigBee(modbusrtu);
	SaveJSonInformationBusLine(modbusrtu);
	SaveJSonInformationModuld(modbusrtu);
	SaveJSonInformationServerCfg(modbusrtu);
	SaveJSonInformationCacert(modbusrtu);
	SaveJSonInformationRtdb(modbusrtu);
	SaveJSonInformationRtu(modbusrtu);
	SaveJSonInformationStation(modbusrtu);
	SaveJSonInformationProcman(modbusrtu);

	SaveMouldJSonInformationToSystem(modbusrtu);//JSOn数据模板部分导入模板库测试
	return FALSE;
}

/********************************************************/
//遍历目录下的所有文件包含子目录下的文件
/********************************************************/
BOOL CProJson::DirfileInformation(char *path, Json::Value &dirdata)
{
	readFileList(path);
	int size = pathfile.size();
	for (int i = 0; i < size; i++)
	{
		string temp;
		temp = pathfile[i];
		GetfileInformation(temp, filename[i], dirdata);	  
	}
	return FALSE;
}
/*******************************************************/
//将文件的内容转转化成JSON字符串
//pathname:文件的路径
//filename:具体的文件名称
//filedata:转化后存储的JSON串
/******************************************************/
BOOL CProJson::GetfileInformation(string pathname,string filename,Json::Value &filedata)
{
	if (filename.substr(0, 3) == "Bus"&&filename.substr(5, 4) == ".ini"&&filename.length() == 9)//bus01.ini类似文件
	{
		GetfilerPrBusInformation(pathname, filename, filedata);		
	}
	else if (pathname.substr(pathname.size()-9-filename.size())=="template/"+filename)//template里面的模板文件
	{
		GetfilerTemplateInformation(pathname, filename, filedata);	
	}
	else if (pathname.substr(pathname.size() - 8 - filename.size()) == "Station/" + filename)//Station里面文件类似stn文件相关文件格式
	{
		GetfilerStationStnInformation(pathname, filename, filedata);		
	}
	else if (filename.substr(0, 3) == "rtu"&&filename.substr(5, 4) == ".txt"&&filename.length()==9)//rtu01.txt类似文件转发协议
	{
		GetfilerRtuInformation(pathname, filename, filedata);
	}
	else if (filename == "BusLine.ini"&&filename.length() == 11)//BusLine.ini文件
	{
		GetfilerPrBusLineInformation(pathname, filename, filedata);		
	}
	else if (filename == "rtdb.conf"&&filename.length() == 9)//rtdb.conf文件
	{
		GetfilerRdbInformation(pathname, filename, filedata);
	}
	else if (filename == "zigbee_config"&&filename.length() == 13)//zigbee_config 文件
	{
		GetfilerZigBeeInformation(pathname, filename, filedata);
	}
	else if (filename == "procman.conf"&&filename.length() == 12)//procman.conf文件
	{
		GetfilerProcManInformation(pathname, filename, filedata);
	}
	else if (pathname=="/mynand/config/mqtt_transmit/server.cfg")//mqtt_transmit下的server.cfg 文件
	{
		GetfilerMqttServerInformation(pathname, filename, filedata);
	}
	else if (pathname == "/mynand/config/mqtt_transmit/cacert.pem")//mqtt_transmit下的cacert.pem 文件
	{
		GetfilerMqttCacertInformation(pathname, filename, filedata);
	}
	return FALSE;

}
BOOL CProJson::GetfilerMqttServerInformation(string pathname, string filename, Json::Value &filedata)
{
	char m_sDevPath[200];
	char sTemp[200] = { 0 };
	Json::Value datacontent;
	Json::Value datatransmit;

	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s", pathname.c_str());
	CProfile profile(m_sDevPath);

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "username", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_username"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "passwd", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_passwd"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "ip", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_ip"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "port", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_port"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "cafile", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_cafile"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "certfile", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_certfile"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "keyfile", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_keyfile"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "clientID", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_clientID"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("server", "sn", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["mqtt_server_sn"] = sTemp;

	datatransmit["mqtt_transmit_servercfg"] = datacontent;
	datatransmit["mqtt_transmit_path"] = pathname;
	filedata["mqtt_transmit_server_cfg"] = datatransmit;

	return FALSE;

}
BOOL CProJson::GetfilerMqttCacertInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		filedata["error_code"].append(pathname + " can not load file");

	}
	else{
		while (!feof(fp))
		{
			string temp;
			memset(line, '\0', sizeof(line));
			fgets(line, 1000, fp);
			int len = strlen(line);
			line[len - 1] = '\0';
			temp = line;
			if (temp.length() == 0)
				continue;
			datacontent["mqtt_cacert_content"].append(temp);
		}
		datacontent["mqtt_cacert__path"] = pathname;
		filedata["mqtt_cacert"]=datacontent;
	}
	fclose(fp);
	return FALSE;
}
BOOL CProJson::GetfilerRtuInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		filedata["error_code"].append(pathname + " can not load file");
		
	}
	else{
		while (!feof(fp))
		{
			string temp;
			Json::Value datartu;
			memset(line, '\0', sizeof(line));
			fgets(line, 1000, fp);
			int len = strlen(line);
			line[len - 1] = '\0';
			temp = line;
			if (temp.length() == 0)
				continue;
			if ((temp.find("YC_AMOUNT=") != temp.npos))
			{
				datacontent["rtu_yc_amount"] = temp.substr(10);
				continue;
			}
			else if ((temp.find("YX_AMOUNT=") != temp.npos))
			{
				datacontent["rtu_yx_amount"] = temp.substr(10);
				continue;
			}
			else if ((temp.find("YM_AMOUNT=") != temp.npos))
			{
				datacontent["rtu_ym_amount"] = temp.substr(10);
				continue;
			}
			else if ((temp.find("YK_AMOUNT=") != temp.npos))
			{
				datacontent["rtu_yk_amount"] = temp.substr(10);
				continue;
			}
			else if ((temp.find("PARAM_AMOUNT=") != temp.npos))
			{
				datacontent["rtu_param_amount"] = temp.substr(13);
				continue;
			}
			else if ((temp.find("YC_DEAD=") != temp.npos))
			{
				datacontent["rtu_yc_dead"] = temp.substr(8);
				continue;
			}
			else if ((temp.find("TIMING=") != temp.npos))
			{
				datacontent["rtu_timing"] = temp.substr(7);
				continue;
			}
			else if ((temp.find("YC_PROPERTY=") != temp.npos))
			{
				datacontent["rtu_yc_property"] = temp.substr(12);
				continue;
			}
			else
			{
				datacontent["rtu_content"].append(temp);
			}
		}
		datacontent["rtu_path"] = pathname;
		filedata["rtu"].append(datacontent);
	}
	fclose(fp);
	return FALSE;
}
BOOL CProJson::GetfilerStationStnInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		filedata["error_code"].append(pathname + " can not load file");
		
	}
	else{
		while (!feof(fp))
		{
			string temp;
			memset(line, '\0', sizeof(line));
			fgets(line, 1000, fp);
			int len = strlen(line);
			line[len - 1] = '\0';
			temp = line;
			if (temp.length() == 0)
				continue;
			datacontent["station_content"].append(temp);
		}
		datacontent["station_path"] = pathname;
		filedata["station"].append(datacontent);
	}	
	fclose(fp);
	return FALSE;
}

BOOL CProJson::GetfilerTemplateInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];
	char temp[10] = { 0 };
	int yc_mount, yx_mount, ym_mount, yk_mount;
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		filedata["error_code"].append(pathname + " can not load file");	
		fclose(fp);
	}
	else
	{
		while (!feof(fp))
		{
			string temp;
			memset(line, '\0', sizeof(line));
			fgets(line, 1000, fp);
			int len = strlen(line);
			line[len - 1] = '\0';
			temp = line;

			if (temp.length() == 0)
				continue;
			temp = get_string(temp);
			datacontent["mould_content"].append(temp);
		}
		fclose(fp);
		datacontent["mould_path"] = pathname;


		if (pathname.find("/mynand/config/ModBus") != pathname.npos)
		{
			FigureJsonInformationAmount(pathname, &yc_mount, &yx_mount, &ym_mount, &yk_mount);
			memset(temp, '\0', sizeof(temp));
			sprintf(temp, "%d", yc_mount);
			datacontent["yc_amount"] = temp;
			memset(temp, '\0', sizeof(temp));
			sprintf(temp, "%d", ym_mount);
			datacontent["ym_amount"] = temp;
			memset(temp, '\0', sizeof(temp));
			sprintf(temp, "%d", yx_mount);
			datacontent["yx_amount"] = temp;
			memset(temp, '\0', sizeof(temp));
			sprintf(temp, "%d", yk_mount);
			datacontent["yk_amount"] = temp;
		}

		filedata["mould"].append(datacontent);
	}	

	return FALSE;

}
BOOL CProJson::GetfilerPrBusInformation(string pathname, string filename, Json::Value &filedata)
{
	char m_sDevPath[200];
	char sTemp[200] = { 0 };
	char Title[10] = { 0 };
	Json::Value datacontent;
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s", pathname.c_str());
	CProfile profile(m_sDevPath);

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("DEVNUM", "NUM", (char *)"NULL", sTemp, sizeof(sTemp));

	datacontent["dev_num"] = sTemp;
	datacontent["bus_path"] = pathname;

	int dev_num = atoi(sTemp);
	for (int i = 1; i <= dev_num; i++)
	{
		Json::Value datasum;
		memset(Title, '\0', sizeof(Title));
		memset(sTemp, 0, sizeof(sTemp));
		sprintf(Title, "%s%03d", "DEV", i);
		datasum["bus_dev_name"] = Title;

		profile.GetProfileString(Title, "module", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_module"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString(Title, "serialno", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_serialno"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString(Title, "addr", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_addr"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString(Title, "name", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_name"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString(Title, "sysid", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_sysid"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString(Title, "template", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_template"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString(Title, "ModuleName", (char *)"NULL", sTemp, sizeof(sTemp));
		datasum["bus_ModuleName"] = sTemp;

		datacontent["bus_content"].append(datasum);

	}
	filedata["Bus"].append(datacontent);
	return FALSE;
}

BOOL CProJson::GetfilerRdbInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];
	char sTemp[200] = { 0 };
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		filedata["error_code"].append(pathname + " can not load file");
	}
	while (!feof(fp))
	{
		string temp;
		memset(line, '\0', sizeof(line));
		fgets(line, 1000, fp);
		int len = strlen(line);
		line[len - 1] = '\0';
		temp = line;
		if (temp.length() == 0)
			continue;
		datacontent["rtdb_content"].append(temp);
	}
	datacontent["rtdb_path"] = pathname;
	filedata["rtdb"]=datacontent;
	fclose(fp);
	return FALSE;
}

BOOL CProJson::GetfilerZigBeeInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];
	char sTemp[200] = { 0 };
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		filedata["error_code"].append(pathname + " can not load file");	
	}
	else
	{
		while (!feof(fp))
		{
			string temp;
			memset(line, '\0', sizeof(line));
			fgets(line, 1000, fp);
			int len = strlen(line);
			line[len - 1] = '\0';
			temp = line;
			if (temp.length() == 0)
				continue;
			datacontent["ZigBee_content"]=temp;

		}
		datacontent["ZigBee_path"] = pathname;
		filedata["ZigBee"] = datacontent;
	}	
	fclose(fp);
	return FALSE;
}
BOOL CProJson::GetfilerPrBusLineInformation(string pathname, string filename, Json::Value &filedata)
{
	char m_sDevPath[200];
	char sTemp[200] = { 0 };	
	Json::Value datacontent;

	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s", pathname.c_str());
	CProfile profile(m_sDevPath);

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("PRINT_PROTOCOL_MSG", "NetCard", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["PRINT_PROTOCOL_MSG_NetCard"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("PRINT_PROTOCOL_MSG", "RemoteIP", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["PRINT_PROTOCOL_MSG_RemoteIP"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("PRINT_PROTOCOL_MSG", "StartPortNum", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["PRINT_PROTOCOL_MSG_StartPortNum"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("PROJECT", "name", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["PROJECT_name"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("PROJECT", "transdelay", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["PROJECT_transdelay"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("LINE-NUM", "NUM", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["LINE-NUM"] = sTemp;

	int num = atoi(sTemp);
	char sPort[20] = { 0 };
	char sPara[20] = { 0 };
	char sInternal[20] = { 0 };
	char snetcard[20] = { 0 };
	for (int byNo = 1; byNo <= num; byNo++)
	{
		Json::Value busdata;

		memset(sPort, 0, sizeof(sPort));
		memset(sPara, 0, sizeof(sPara));
		memset(sInternal, 0, sizeof(sInternal));
		memset(snetcard, 0, sizeof(snetcard));

		sprintf(sPort, "%s%02d", "port", byNo);
		sprintf(sPara, "%s%02d", "para", byNo);
		sprintf(sInternal, "%s%02d", "internal", byNo);
		sprintf(snetcard, "%s%02d", "NetCard", byNo);

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("PORT", sPort, (char *)"NULL", sTemp, sizeof(sTemp));
		if (strcmp(sTemp, "NULL") != 0)
			busdata["PORT_port"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("PORT", sPara, (char *)"NULL", sTemp, sizeof(sTemp));
		if (strcmp(sTemp, "NULL") != 0)
			busdata["PORT_Para"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("PORT", sInternal, (char *)"NULL", sTemp, sizeof(sTemp));
		if (strcmp(sTemp, "NULL") != 0)
			busdata["PORT_Internal"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("PORT", sInternal, (char *)"NULL", sTemp, sizeof(sTemp));
		if (strcmp(sTemp, "NULL") != 0)
			busdata["PORT_Internal"] = sTemp;

		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("PORT", snetcard, (char *)"NULL", sTemp, sizeof(sTemp));
		if (strcmp(sTemp, "NULL") != 0)
			busdata["PORT_netcard"] = sTemp;

		datacontent["Bus_PORT"].append(busdata);

	}
	char sKey[200] = { 0 };
	for (BYTE byNo = 1; byNo < 5; byNo++)
	{
		Json::Value netdata;
		int num = 0;
		////获取网卡名字
		sprintf(sKey, "%s%02d", "NetCard", byNo);
		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("NetCard", sKey, (char *)"NULL", sTemp, sizeof(sTemp));
		netdata["name"] = sTemp;

		//获取网卡IP
		sprintf(sKey, "%s%02d", "IP", byNo);
		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("NetCard", sKey, (char *)"NULL", sTemp, sizeof(sTemp));
		netdata["ip"] = sTemp;

		//获取网关
		sprintf(sKey, "%s%02d", "GateWay", byNo);
		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("NetCard", sKey, (char *)"NULL", sTemp, sizeof(sTemp));
		netdata["gateway"] = sTemp;

		//获取子网掩码
		sprintf(sKey, "%s%02d", "SubNetMask", byNo);
		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("NetCard", sKey, (char *)"NULL", sTemp, sizeof(sTemp));
		netdata["mask"] = sTemp;

		//获取指定路由
		sprintf(sKey, "%s%02d", "RouteIp", byNo);
		memset(sTemp, 0, sizeof(sTemp));
		profile.GetProfileString("NetCard", sKey, (char *)"NULL", sTemp, sizeof(sTemp));
		netdata["route"] = sTemp;

		datacontent["Bus_Net"].append(netdata);
	}

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("SYSTEM-DNS", "DNS", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["Bus_dns"] = sTemp;


	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("FaultRecorder", "Port", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["FaultRecorder_Port"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("FaultRecorder", "Type", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["FaultRecorder_Type"] = sTemp;

	//NtpServer
	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("NtpServer", "NetMask", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["NtpServer_NetMask"] = sTemp;

	memset(sTemp, 0, sizeof(sTemp));
	profile.GetProfileString("NtpServer", "IPField", (char *)"NULL", sTemp, sizeof(sTemp));
	datacontent["NtpServer_IPField"] = sTemp;

	datacontent["Busline_path"] = pathname;
	//filedata["BusLine"].append(datacontent);
	filedata["BusLine"]=datacontent;

	return FALSE;

}
int CProJson::ParseConfigItem(char *strItem, WORD *pwNum)
{/*{{{*/
	char  strType[32];
	int   i, nLen;

	if (strstr(strItem, "environ")) return CONFIG_ENV_VAR;
	i = 0;
	nLen = strlen(strItem);
	while (!isdigit(strItem[i]) && i<(int)sizeof(strType))
	{
		strType[i] = toupper(strItem[i]);
		if (++i >= nLen) break;
	}

	strType[i] = '\0';
	if (i >= nLen) *pwNum = 0;
	else *pwNum = (WORD)atoi(&strItem[i]);

	if (strcmp(strType, "PROC") == 0)
		return CONFIG_PROC_STYLE;
	if (strcmp(strType, "PARA") == 0)
		return CONFIG_PROC_PARAM;
	return -1;
}/*}}}*/
BOOL CProJson::GetfilerProcManInformation(string pathname, string filename, Json::Value &filedata)
{
	FILE *hFile;
	char szText[160];
	char *pItem, *pParam;
	WORD wNum;
	Json::Value datacontent;

	//读配置信息
	hFile = fopen(pathname.c_str(), "r");
	if (hFile >0)
	{
	
		while (fgets(szText, sizeof(szText), hFile))
		{
			rtrim(szText);
			//ltrim(szText);
			if (szText[0] == ';' || szText[0] == '#') continue;
			//分离文本行
			pItem = strtok(szText, "=");
			if (pItem == NULL) continue;
			pParam = strtok(NULL, "@");

			if (pParam)
			{
				ltrim(pParam);
				//解析配置行
				int nType = ParseConfigItem(pItem, &wNum);
				switch (nType)
				{
				case CONFIG_ENV_VAR:
					datacontent["g_szEnviVar"].append(pParam);
					break;
				case CONFIG_PROC_STYLE:
					datacontent["config_proc_style"].append(pParam);
					break;
				case CONFIG_PROC_PARAM:
					datacontent["config_proc_param"].append(pParam);
					break;
				}
			}
		}
		datacontent["Procman_path"] = pathname;
		filedata["Procman"]=datacontent;
		fclose(hFile);
	}
	else
		return FALSE;

}
BOOL CProJson::SaveJsontoFile(string pathname, Json::Value &filedata)
{
	string buf;
	buf = filedata.toStyledString();
	//打开一个info.json文件，并写入json内容
	FILE *fp = fopen(pathname.c_str(), "w");
	fwrite(buf.c_str(),buf.length(), 1, fp);
	fclose(fp);//关闭文件
	return FALSE;
	
}
BOOL CProJson::procCmd(BYTE byCmd)
{
	return FALSE;
}

void CProJson::Init()
{
	
}
void CProJson::setLog(Clog * pLog )
{
	m_log = pLog;
}
/*********************************************/
//递归遍历文件目录存储名录名称及相应文件路径
/********************************************/
int CProJson::readFileList(char *basePath)
{
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	char pathname[508];
	int num = 0;

	if ((dir = opendir(basePath)) == NULL)
	{
		perror("Open dir error...");
		exit(1);
	}
	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    //current dir OR parrent dir
			continue;
		else if (ptr->d_type == 8)   ///file
		{
			sprintf(pathname,"%s/%s", basePath, ptr->d_name);
			pathfile.push_back(pathname);
			filename.push_back(ptr->d_name);
		}		
		else if (ptr->d_type == 10)   ///link file
		{
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		}
		else if (ptr->d_type == 4)   ///dir
		{
			memset(base, '\0', sizeof(base));
			strcpy(base, basePath);
			strcat(base, "/");
			strcat(base, ptr->d_name);
			readFileList(base);
		}	
	}
	closedir(dir);
	return 1;
}

size_t CProJson::Get_file_size(const char *filepath)
{
	if (NULL == filepath)
		return 0;
	struct stat filestat;
	memset(&filestat, 0, sizeof(struct stat));
	/*获取文件信息*/
	if (0 == stat(filepath, &filestat))
		return filestat.st_size;
	else
		return 0;
}

BOOL  CProJson::SaveJSonInformationBus(Json::Value &filedata)
{
	if (!filedata["Bus"].isNull()) 
	{
		Json::Value arrayBus = filedata["Bus"];
		for (unsigned int i = 0; i < arrayBus.size(); i++)
		{
			string bus_path = arrayBus[i]["bus_path"].asString();
			string  num = arrayBus[i]["dev_num"].asString();
			string buf ="[DEVNUM]";
		   Json::Value arrayBus_content = arrayBus[i]["bus_content"];

			FILE *fp = fopen(bus_path.c_str(), "w+");
			fprintf(fp,"%s",buf.c_str());
			fprintf(fp,"%s","\r\n");

			buf = "NUM=" + num;
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");
			
			for (unsigned int j = 0; j < arrayBus_content.size(); j++)
			{
				buf = "[" + arrayBus_content[j]["bus_dev_name"].asString() + "]";
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "module=" + arrayBus_content[j]["bus_module"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "serialno=" + arrayBus_content[j]["bus_serialno"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "addr=" + arrayBus_content[j]["bus_addr"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "name=" + arrayBus_content[j]["bus_name"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "sysid=" + arrayBus_content[j]["bus_sysid"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "template=" + arrayBus_content[j]["bus_template"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

				buf = "ModuleName=" + arrayBus_content[j]["bus_ModuleName"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");
			}
			fclose(fp);//关闭文件
		}

	}
	return FALSE;

}


BOOL CProJson::SaveJSonInformationModuld(Json::Value &filedata)
{
	string buf;
	if (!filedata["mould"].isNull())
	{

		Json::Value arraymould = filedata["mould"];
		for (unsigned int i = 0; i < arraymould.size(); i++)
		{
			string bus_path =  arraymould[i]["mould_path"].asString();
			FILE *fp = fopen(bus_path.c_str(), "w+");
			Json::Value arraymould_content = arraymould[i]["mould_content"];
			for (unsigned int j = 0; j < arraymould_content.size(); j++)
			{
				buf = arraymould_content[j].asString();
				fprintf(fp,"%s\n",buf.c_str());
			}
			fclose(fp);


		}

	}
	return FALSE;

}

BOOL CProJson::SaveJSonInformationZigBee(Json::Value &filedata)
{
	if (!filedata["ZigBee"].isNull())
	{
		string bus_path = filedata["ZigBee"]["ZigBee_path"].asString();
		string buf = filedata["ZigBee"]["ZigBee_content"].asString();
		FILE *fp = fopen(bus_path.c_str(), "w+");
		fprintf(fp, "%s", buf.c_str());
		fclose(fp);
	}
}
BOOL CProJson::SaveJSonInformationBusLine(Json::Value &filedata)
{
	char bufstr[20] = {0};
	string str;
	if (!filedata["BusLine"].isNull())
	{
		string bus_path = filedata["BusLine"]["Busline_path"].asString();
		string buf = "[PRINT_PROTOCOL_MSG]";
		FILE *fp = fopen(bus_path.c_str(), "w+");
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");


		buf = "NetCard=" + filedata["BusLine"]["PRINT_PROTOCOL_MSG_NetCard"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "RemoteIP=" + filedata["BusLine"]["PRINT_PROTOCOL_MSG_RemoteIP"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "StartPortNum=" + filedata["BusLine"]["PRINT_PROTOCOL_MSG_StartPortNum"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");
		fprintf(fp, "%s", "\r\n");

		buf = "[PROJECT]";
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "name=" + filedata["BusLine"]["PROJECT_name"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "transdelay=" + filedata["BusLine"]["PROJECT_transdelay"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");
		fprintf(fp, "%s", "\r\n");


		buf = "[LINE-NUM]";
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "NUM=" + filedata["BusLine"]["LINE-NUM"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");
		fprintf(fp, "%s", "\r\n");


		buf = "[PORT]";
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");
		Json::Value arrayBusPort = filedata["BusLine"]["Bus_PORT"];
		for (unsigned int i = 0; i < arrayBusPort.size(); i++)
		{
			memset(bufstr, '\0', sizeof(bufstr));
			sprintf(bufstr, "port%02d=", i + 1);
			str = bufstr;
			buf = str + arrayBusPort[i]["PORT_port"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");

			if (arrayBusPort[i].isMember("PORT_Para"))
			{
				memset(bufstr, '\0', sizeof(bufstr));
				sprintf(bufstr, "para%02d=", i + 1);
				str = bufstr;
				buf = str + arrayBusPort[i]["PORT_Para"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

			}
			if (arrayBusPort[i].isMember("PORT_Internal"))
			{
				memset(bufstr, '\0', sizeof(bufstr));
				sprintf(bufstr, "internal%02d=", i + 1);
				str = bufstr;
				buf = str + arrayBusPort[i]["PORT_Internal"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

			}

			if (arrayBusPort[i].isMember("PORT_netcard"))
			{
				memset(bufstr, '\0', sizeof(bufstr));
				sprintf(bufstr, "NetCard%02d=", i + 1);
				str = bufstr;
				buf = str + arrayBusPort[i]["PORT_netcard"].asString();
				fprintf(fp, "%s", buf.c_str());
				fprintf(fp, "%s", "\r\n");

			}
			fprintf(fp, "%s", "\r\n");
		}

		buf = "[NetCard]";
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");
		Json::Value arrayBusNet = filedata["BusLine"]["Bus_Net"];
		for (unsigned int j = 0; j < arrayBusNet.size(); j++)
		{
			memset(bufstr, '\0', sizeof(bufstr));
			sprintf(bufstr, "NetCard%02d=", j + 1);
			str = bufstr;
			buf = str + arrayBusNet[j]["name"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");

			memset(bufstr, '\0', sizeof(bufstr));
			sprintf(bufstr, "IP%02d=", j + 1);
			str = bufstr;
			buf = str + arrayBusNet[j]["ip"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");


			memset(bufstr, '\0', sizeof(bufstr));
			sprintf(bufstr, "SubNetMask%02d=", j + 1);
			str = bufstr;
			buf = str + arrayBusNet[j]["mask"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");


			
		
			memset(bufstr, '\0', sizeof(bufstr));
			sprintf(bufstr, "GateWay%02d=", j + 1);
			str = bufstr;
			buf = str + arrayBusNet[j]["gateway"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");

			memset(bufstr, '\0', sizeof(bufstr));
			sprintf(bufstr, "RouteIp%02d=", j + 1);
			str = bufstr;
			buf = str + arrayBusNet[j]["route"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");


			fprintf(fp, "%s", "\r\n");

		}

		buf = "[SYSTEM-DNS]";
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "DNS=" + filedata["BusLine"]["Bus_dns"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		fprintf(fp, "%s", "\r\n");
		buf = "[FaultRecorder]";
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");


		buf = "Port=" + filedata["BusLine"]["FaultRecorder_Port"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		buf = "Type=" + filedata["BusLine"]["FaultRecorder_Type"].asString();
		fprintf(fp, "%s", buf.c_str());
		fprintf(fp, "%s", "\r\n");

		if (filedata["BusLine"]["NtpServer_IPField"].asString() != "NULL"&&filedata["BusLine"]["NtpServer_NetMask"].asString() != "NULL")
		{
			buf = "[NtpServer]";
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");

			buf = "NetMask=" + filedata["BusLine"]["NtpServer_NetMask"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");

			buf = "IPField" + filedata["BusLine"]["FaultRecorder_Type"].asString();
			fprintf(fp, "%s", buf.c_str());
			fprintf(fp, "%s", "\r\n");

		}
		fclose(fp);
	}

}
BOOL CProJson::SaveJSonInformationServerCfg(Json::Value &filedata)
{
	string buf;
	if (!filedata["mqtt_transmit_server_cfg"].isNull())
	{
		Json::Value data;
		data = filedata["mqtt_transmit_server_cfg"];
		string bus_path = filedata["mqtt_transmit_server_cfg"]["mqtt_transmit_path"].asString();
		
		FILE *fp = fopen(bus_path.c_str(), "w+");

		buf = "[server]";
		fprintf(fp, "%s\n", buf.c_str());

		buf = "username=" + data["mqtt_transmit_servercfg"]["mqtt_server_username"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "passwd="+data["mqtt_transmit_servercfg"]["mqtt_server_passwd"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "ip=" + data["mqtt_transmit_servercfg"]["mqtt_server_ip"].asString();
		fprintf(fp, "%s\n", buf.c_str());


		buf = "port=" + data["mqtt_transmit_servercfg"]["mqtt_server_port"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "cafile=" + data["mqtt_transmit_servercfg"]["mqtt_server_cafile"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "certfile=" + data["mqtt_transmit_servercfg"]["mqtt_server_certfile"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "keyfile=" + data["mqtt_transmit_servercfg"]["mqtt_server_keyfile"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "clientID=" + data["mqtt_transmit_servercfg"]["mqtt_server_clientID"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		buf = "sn=" + data["mqtt_transmit_servercfg"]["mqtt_server_sn"].asString();
		fprintf(fp, "%s\n", buf.c_str());

		fclose(fp);
	}

}

BOOL CProJson::SaveJSonInformationCacert(Json::Value &filedata)
{
	string buf;
	if (!filedata["mqtt_cacert"].isNull())
	{
		string bus_path = filedata["mqtt_cacert"]["mqtt_cacert__path"].asString();
		FILE *fp = fopen(bus_path.c_str(), "w+");
		Json::Value arraycacert = filedata["mqtt_cacert"]["mqtt_cacert_content"];

		for (unsigned int i = 0; i < arraycacert.size(); i++)
		{
			buf = arraycacert[i].asString();
			fprintf(fp, "%s\n", buf.c_str());
		}
		fclose(fp);
	}

}

BOOL CProJson::SaveJSonInformationRtdb(Json::Value &filedata)
{
	string buf;
	if (!filedata["rtdb"].isNull())
	{
		string bus_path = filedata["rtdb"]["rtdb_path"].asString();
		FILE *fp = fopen(bus_path.c_str(), "w+");
		Json::Value arrayrtdb = filedata["rtdb"]["rtdb_content"];

		for (unsigned int i = 0; i < arrayrtdb.size(); i++)
		{
			buf = arrayrtdb[i].asString();
			fprintf(fp, "%s\n", buf.c_str());
		}
		fclose(fp);

	}
}
BOOL CProJson::SaveJSonInformationStation(Json::Value &filedata)
{
	string buf;
	if (!filedata["station"].isNull())
	{
		Json::Value arraystation = filedata["station"];
		for (unsigned int i = 0; i < arraystation.size(); i++)
		{
			
			string bus_path = arraystation[i]["station_path"].asString();

			FILE *fp = fopen(bus_path.c_str(), "w+");

			Json::Value arraystation_content = arraystation[i]["station_content"];
			for (unsigned int j = 0; j < arraystation_content.size(); j++)
			{
				buf = arraystation_content[j].asString();
				fprintf(fp, "%s\n", buf.c_str());
			}
			fclose(fp);
		}


	}

}

BOOL CProJson::SaveJSonInformationRtu(Json::Value &filedata)
{
	string buf;
	if (!filedata["rtu"].isNull())
	{
		Json::Value arrayrtu = filedata["rtu"];
		for (unsigned int i = 0; i < arrayrtu.size(); i++)
		{
			string bus_path = arrayrtu[i]["rtu_path"].asString();			
			FILE *fp = fopen(bus_path.c_str(), "w+");

			buf = "YC_AMOUNT=" + arrayrtu[i]["rtu_yc_amount"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			buf = "YX_AMOUNT=" + arrayrtu[i]["rtu_yx_amount"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			buf = "YM_AMOUNT=" + arrayrtu[i]["rtu_ym_amount"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			buf = "YK_AMOUNT=" + arrayrtu[i]["rtu_yk_amount"].asString();
			fprintf(fp, "%s\n", buf.c_str());
			
			buf = "PARAM_AMOUNT=" + arrayrtu[i]["rtu_param_amount"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			buf = "YC_DEAD=" + arrayrtu[i]["rtu_yc_dead"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			buf = "YC_PROPERTY=" + arrayrtu[i]["rtu_yc_property"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			buf = "TIMING=" + arrayrtu[i]["rtu_timing"].asString();
			fprintf(fp, "%s\n", buf.c_str());

			Json::Value arrayrtu_content = arrayrtu[i]["rtu_content"];
			for (unsigned int j = 0; j < arrayrtu_content.size(); j++)
			{
				buf = arrayrtu_content[j].asString();
				fprintf(fp, "%s\n", buf.c_str());

			}
			fclose(fp);

		}

	}

}

BOOL CProJson::SaveJSonInformationProcman(Json::Value &filedata)
{
	string buf;
	char str[10] = {0};
	string temp;
	if (!filedata["Procman"].isNull())
	{
		string bus_path = filedata["Procman"]["Procman_path"].asString();

		FILE *fp = fopen(bus_path.c_str(), "w+");

		buf = "#[system]";//----不知道有用没用
		fprintf(fp, "%s\n", buf.c_str());

		buf = "environ =";//----不知道有用没用
		fprintf(fp, "%s\n", buf.c_str());

		fprintf(fp, "%s", "\r\n");


		buf = "#[process]";
		fprintf(fp, "%s\n", buf.c_str());
		Json::Value arrayparam = filedata["Procman"]["config_proc_param"];
		for (unsigned int i = 0; i < arrayparam.size(); i++)
		{
			memset(str,'\0',sizeof(str));
			sprintf(str,"para%02d=",i+1);
			temp = str;
			buf = temp + arrayparam[i].asString();
			fprintf(fp, "%s\n", buf.c_str());


		}
		Json::Value arraystyle = filedata["Procman"]["config_proc_style"];
		for (unsigned int j = 0; j < arraystyle.size(); j++)
		{
			memset(str, '\0', sizeof(str));
			sprintf(str, "proc%02d=", j + 1);
			temp = str;
			buf = temp + arraystyle[j].asString();
			fprintf(fp, "%s\n", buf.c_str());

		}
		fclose(fp);
	}

}
std::vector<std::string> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<std::string> result;
	str += pattern;//扩展字符串以方便操作
	int size = str.size();

	for (int i = 0; i<size; i++)
	{
		pos = str.find(pattern, i);
		if (pos<size)
		{
			std::string s = str.substr(i, pos - i);
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	return result;
}

BOOL CProJson::SaveMouldJSonInformationToSystem(Json::Value &filedata)
{
	string buf;
	char line[1000];
	int pos;
	string tempname;
	if (!filedata["mould"].isNull())
	{
		Json::Value arraymould = filedata["mould"];
		for (unsigned int i = 0; i < arraymould.size(); i++)
		{
			int linenum = 0;
			Json::Value arraymould = filedata["mould"];
			string mould_path = arraymould[i]["mould_path"].asString();//"/mynand/config/MBTcp/template/XMLyx.txt"
			int pos = mould_path.find("template/");
			mould_path.erase(pos, 9);
			mould_path.replace(0, 14, "/mnt/config_system/System");//替换成模板库中文件路径

			Json::Value arraymould_content = arraymould[i]["mould_content"];
			FILE *fp;
			if (Get_file_size(mould_path.c_str()) == 0)//模板库中不存在该模板--》直接保存至模板库中就行
			{
				//将JSON串模板写入模板库
				fp = fopen(mould_path.c_str(), "w+");
				for (unsigned int j = 0; j < arraymould_content.size(); j++)
				{
					buf = arraymould_content[j].asString();
					fprintf(fp, "%s\n", buf.c_str());
				}
				fclose(fp);
				//修改manager.ini
				 tempname = mould_path;
				pos = mould_path.find_last_of("/");
				tempname.substr(pos+1);
				mould_path.replace(pos + 1, tempname.length(), "manager.ini");

				string New_Mould = tempname.substr(pos + 1);//模板文件的名称
				New_Mould.erase(New_Mould.end() - 4, New_Mould.end());//去掉结尾的.txt
				fp = fopen(mould_path.c_str(), "a+");
				

				int yc_mount = atoi(arraymould[i]["yc_amount"].asString().c_str());
				int yx_mount = atoi(arraymould[i]["yx_amount"].asString().c_str());
				int ym_mount = atoi(arraymould[i]["ym_amount"].asString().c_str());
				int yk_mount = atoi(arraymould[i]["yk_amount"].asString().c_str());


				fprintf(fp, "%s=%d,%d,%d,%d\n", New_Mould.c_str(), yc_mount, yx_mount, ym_mount, yk_mount);//
				fclose(fp);

			}
			else//模板库中已经存在该名称的模板-》下一步比较其内容---》如内容不一样---》重命名库中原模板名称
			{
				if (ComapareFileAndJson(mould_path, arraymould_content) == TRUE)
				{
					//重命名原原来模板的名称并将新模板写入					
					pos = mould_path.find_last_of("/");				
					tempname = mould_path;
					tempname.substr(pos + 1);
				
					string Old_Mould_Name = tempname.substr(pos + 1);//模板文件的名称
					Old_Mould_Name.erase(Old_Mould_Name.end() - 4, Old_Mould_Name.end());//去掉结尾的.txt
				

					string oldtemp_path = mould_path;
					char str_time[10] = { 0 };
					sprintf(str_time,"%ld",time(NULL));

					string newtemp_path = oldtemp_path.insert(oldtemp_path.length()-4, string(str_time));
					rename(mould_path.c_str(), newtemp_path.c_str());
				


					fp = fopen(mould_path.c_str(), "w+");
					for (unsigned int j = 0; j < arraymould_content.size(); j++)
					{
						buf = arraymould_content[j].asString();
						fprintf(fp, "%s\n", buf.c_str());
					}
					fclose(fp);

					//修改manager.ini---->
					tempname = mould_path;
					pos = mould_path.find_last_of("/");
					tempname.substr(pos + 1);
	
					mould_path.replace(pos + 1, tempname.length(), "manager.ini");

					string New_Mould_Name = tempname.substr(pos + 1);//模板文件的名称
					New_Mould_Name.erase(New_Mould_Name.end() - 4, New_Mould_Name.end());//去掉结尾的.txt

					
					char m_sDevPath[200];
					char sTemp[200] = { 0 };
					char Info[200] = {0};
					memset(m_sDevPath, 0, sizeof(m_sDevPath));
					sprintf(m_sDevPath, "%s", mould_path.c_str());
					CProfile profile(m_sDevPath);

					memset(sTemp, 0, sizeof(sTemp));
					profile.GetProfileString("Module", (char *)Old_Mould_Name.c_str(), (char *)"NULL", sTemp, sizeof(sTemp));
					
					memset(Info,'\0',sizeof(Info));
					sprintf(Info, "%s%s=%s", (char *)Old_Mould_Name.c_str(),str_time,sTemp);
					//修改原manager.ini中那一行	
					ModifyLineData((char *)mould_path.c_str(), Old_Mould_Name,Info);

					fp = fopen(mould_path.c_str(), "a+");

					int yc_mount = atoi(arraymould[i]["yc_amount"].asString().c_str());
					int yx_mount = atoi(arraymould[i]["yx_amount"].asString().c_str());
					int ym_mount = atoi(arraymould[i]["ym_amount"].asString().c_str());
					int yk_mount = atoi(arraymould[i]["yk_amount"].asString().c_str());

					fprintf(fp, "%s=%d,%d,%d,%d\n", New_Mould_Name.c_str(), yc_mount, yx_mount, ym_mount, yk_mount);//
					fclose(fp);		
				}
				else{

					//不用做任何处理-----JSON和模板库中一样

				}

			}


		}

	}
}

BOOL CProJson::ComapareFileAndJson(string pathname, Json::Value &filedata)
{
	FILE *fp;
	char line[1000];

	vector<string>mould_info_file;
	vector<string>mould_info_json;

	fp = fopen(pathname.c_str(), "r");
	while (!feof(fp))
	{
		string temp;
		memset(line, '\0', sizeof(line));
		fgets(line, 1000, fp);
		int len = strlen(line);
		line[len - 1] = '\0';
		temp = line;

		if (temp.length() == 0)
			continue;
		mould_info_file.push_back(temp);

	}
	fclose(fp);

	for (unsigned int i = 0; i < filedata.size(); i++)
	{
		
		mould_info_json.push_back(filedata[i].asString());
		
	}

	if (mould_info_file.size() != mould_info_json.size())
	{
		return TRUE;
	}
	else
	{
		for (unsigned int j = 0; j < mould_info_file.size(); j++)
		{
			if (mould_info_file[j] != mould_info_json[j])
			{
				return TRUE;
				break;
			}
		}
		return FALSE;

	}
}

//修改指定行的内容----
void CProJson::ModifyLineData(char* fileName, string str, char* lineData)
{
	ifstream in;
	in.open(fileName);
	string strFileData = "";
	int line = 1;
	char tmpLineData[1024] = { 0 };
	while (in.getline(tmpLineData, sizeof(tmpLineData)))
	{
		string temp = CharToStr(tmpLineData);
		//cout << temp.substr(0, temp.find("=")) << endl;
		if (temp.substr(0, temp.find("=")) == str)
		{
			strFileData += CharToStr(lineData);
			strFileData += "\n";
		}
		else
		{

			strFileData += CharToStr(tmpLineData);
			strFileData += "\n";
		}
		line++;
	}
	in.close();
	//写入文件
	ofstream out;
	out.open(fileName);
	out.flush();
	out << strFileData;
	out.close();

}
string CProJson::CharToStr(char * contentChar)
{
	string tempStr;
	for (int i = 0; contentChar[i] != '\0'; i++)
	{

		tempStr += contentChar[i];

			
	}
	return tempStr;

}

int CProJson::substrpoint(char str[], int point)
{
	char *token;
	int figure;

	int num = 0;
	token = strtok(str, ",");

	while (token != NULL) {
		num++;
		if (num == point)
		{
			figure = atoi(token);
			break;
		}
			token = strtok(NULL, ",");
	}
	return figure;
}

BOOL CProJson::FigureJsonInformationAmount(string pathname, int *yc_mount, int *yx_mount, int *ym_mount, int *yk_mount)
{
	FILE *fp;
	int yc=0, yx=0, ym=0, yk=0;
	char line[1000];
	Json::Value datacontent;
	fp = fopen(pathname.c_str(), "r");
	if (pathname.find("/mynand/config/ModBus") != pathname.npos)//统计ModBus模板中yc ym yx yk数量
	{
		while (!feof(fp))
		{
				string temp;
				memset(line, '\0', sizeof(line));
				fgets(line, 1000, fp);
				int len = strlen(line);
				line[len - 1] = '\0';
				temp = line;
				if (temp.length() == 0)
					continue;
				if (line[0] == '1')//遥信数量
				{
					yx += substrpoint(line, 6);
				}
				else if (line[0] == '2')//遥测数量
				{
					yc += substrpoint(line, 6);
				}
				else if (line[0] == '3')//遥控数量
				{
					yk += substrpoint(line, 6);
				}
				else if (line[0] == '4')//遥脉数量
				{
					ym += substrpoint(line, 6);
				}

		}		
		fclose(fp);
		*yc_mount = yc;
		*ym_mount = ym;
		*yk_mount = yk;
		*yx_mount = yx;
	}

	else{
		*yc_mount = yc;
		*ym_mount = ym;
		*yk_mount = yk;
		*yx_mount = yx;

	}

}

string CProJson::get_string(string res)
{
	int r = res.find('\r');
	while (r != string::npos)
	{
		if (r != string::npos)
		{
			res.replace(r, 1, "");
			r = res.find('\r');
		}
	}
	return res;
}









		








