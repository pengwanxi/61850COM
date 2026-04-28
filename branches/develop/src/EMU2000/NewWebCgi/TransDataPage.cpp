#include "TransDataPage.h"


CTransDataPage::CTransDataPage()
{
	g_nExtendSize = 0;
	trans_channel = 0;
}


CTransDataPage::~CTransDataPage()
{

}

BOOL CTransDataPage::getJSONStructFromWebPage(Json::Value &root)
{
	Json::Value comdata;

	Init();
	root["trans_page_version"] = GetVerSion();
	root["trans_page_systime"] = GetSysTime();
	root["trans_page_uptime"] = GetUptime();

	GetComPar(comdata);

	root["trans_page_protocol"] = comdata;

	return FALSE;
}
BOOL CTransDataPage::GetComPar(Json::Value &compardata)
{
	BYTE byLineNum = 0;

	char sBusLine[] = BUS_PATH;
	trans_channel = 0;
	CProfile Profile(sBusLine);

	byLineNum = (BYTE)Profile.GetProfileInt((char *)"LINE-NUM", (char *)"NUM", 0);
	for (BYTE i = 0; i < byLineNum; i++)
	{
		string temp;
		Json::Value data;
		char sPort[] = "port";
		char sPara[] = "para";
		char sInterval[] = "internal";
		char sSect[] = "PORT";
		char sNetCard[] = "NetCard";
		int sizebuff = 0;
		char sbuffer[200] = { 0 };
		sizebuff = sizeof(sbuffer);
		
		char sTemp[400];
		memset(sTemp, 0, sizeof(sTemp));
		sprintf(sTemp, "%s%02d", sPort, i + 1);
		Profile.GetProfileString(sSect, sTemp, (char *)"NULL", sbuffer, sizebuff);
		
		temp = sbuffer;
		if (temp=="PAUSE")
		{
			continue;
		}
		else{
			protocol_para = temp;

			memset(sTemp, 0, sizeof(sTemp));
			sprintf(sTemp, "%s%02d", sNetCard, i + 1);
			Profile.GetProfileString(sSect, sTemp, (char *)"NULL", sbuffer, sizebuff);
			protocol_netcard= sbuffer;

		}
		
		memset(sTemp, 0, sizeof(sTemp));
		sprintf(sTemp, "%s%02d", sPara, i + 1);
		Profile.GetProfileString(sSect, sTemp, (char *)"NULL", sbuffer, sizebuff);
		temp = sbuffer;
		int ret = ProtocolStyle(temp);
		if (GATHER_PROTOCOL ==ret)
		{
			continue;
		}
		else if (TRANS_PROTOCOL ==ret)
		{
			for (int i = 0; i < MAX_STN_SUM; i++)
			{
				yc_trans[i].clear();
				yx_trans[i].clear();
				ym_trans[i].clear();
				yk_trans[i].clear();
				dz_trans[i].clear();
			}
			memset(sTemp, 0, sizeof(sTemp));
			sprintf(sTemp, "%02d", i + 1);
			data["protocol_commun_number"] = sTemp;
			data["protocol_name"] = protocol_name;
			data["protocol_para"] = protocol_para;
			if (protocol_netcard != "NULL")
			{
				data["protocol_netcard"] = protocol_netcard;
			}
    	
			memset(sTemp, 0, sizeof(sTemp));
			sprintf(sTemp, "/mynand/config/%s/rtu%02d.txt", protocol_name.c_str(),i + 1);
			
			if(FALSE==GetTransNum(sTemp, trans_channel))continue;
			trans_channel++;

			Json::Value ycdata;
			Json::Value ymdata;
			Json::Value ykdata;
			Json::Value yxdata;
			Json::Value dzdata;

			TransDataYc(ycdata);
			data["yc_data"] = ycdata;

			TransDataYm(ymdata);
			data["ym_data"] = ymdata;

			TransDataYx(yxdata);
			data["yx_data"] = yxdata;

			TransDataYk(ykdata);
			data["yk_data"] = ykdata;

			TransDataDz(dzdata);
			data["dz_data"] = dzdata;

			compardata["protocol"].append(data);

		}
		
	}

}
BOOL CTransDataPage::GetTransNum(string pathname,int transchannel)
{
	
	FILE *fp;
	char line[1000];
	int num = 0;
	int num2 = 0;;
	
	fp = fopen(pathname.c_str(), "r");
	if (fp == NULL)
	{
		return FALSE;
		
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
			if ((temp.length() == 0 || (JudgeCharacter(line,&num,&num2) == FALSE)))
			{
				continue;
			}
			else
			{
				
				if (line[0] == 'Y'&&line[1] == 'C')
				{
					int pos = temp.find("=");
					temp=temp.substr(2, pos-2);
				    yc_trans[num][num2] = temp;				
					
				}
				else if (line[0] == 'Y'&&line[1] == 'X')
				{
					int pos = temp.find("=");
					temp = temp.substr(2, pos - 2);
					yx_trans[num][num2] = temp;

				}
				else if (line[0] == 'D'&&line[1] == 'D')
				{
					int pos = temp.find("=");
					temp = temp.substr(2, pos - 2);
					ym_trans[num][num2] = temp;

				}
				else if (line[0] == 'Y'&&line[1] == 'K')
				{
					int pos = temp.find("=");
					temp = temp.substr(2, pos - 2);
					yk_trans[num][num2] = temp;

				}
				else if (line[0] == 'D'&&line[1] == 'Z')
				{
					int pos = temp.find("=");
					temp = temp.substr(2, pos - 2);
					dz_trans[num][num2] = temp;

				}
				
			}		
		
		}
	
	}
	fclose(fp);
	return TRUE;

}
void CTransDataPage::Getnumopt(char *str, char *delim, int *num1,int *num2)
{
	char *token=NULL;

	int  num = 0;
	/* 获取第一个子字符串 */
	token = strtok(str, delim);
	/* 继续获取其他的子字符串 */
	while (token != NULL) {
		num++;
		if (2 == num)
		{
			printf("%s+++\n",token);
			*num1 = atoi(token);	
			
		}
		else if (3 == num)
		{
			printf("%s+++\n", token);
			*num2 = atoi(token);

		}
		else if (num > 3)
		{
			break;

		}
		token = strtok(NULL, delim);
	}



}
BOOL CTransDataPage::JudgeCharacter(char str[],int *num,int *num2)
{
	int a, b;
	if( (str[0] == 'Y') && (str[1] == 'C') && (str[2] != '_'))
	{
       Getnumopt(str, "=,",&a,&b);
	   *num = a;
	   *num2 = b;
	   return TRUE;
	}
	else if ((str[0] == 'Y') && (str[1] == 'X') && (str[2] != '_'))
	{
		Getnumopt(str, "=,", &a, &b);
		*num = a;
		*num2 = b;
		return TRUE;
	}
	else if ((str[0] == 'D') && (str[1] == 'D') && (str[2] != '_'))
	{
		Getnumopt(str, "=,", &a, &b);
		*num = a;
		*num2 = b;
		
		return TRUE;
	}
	else if ((str[0] == 'Y') && (str[1] == 'K') && (str[2] != '_'))
	{
		Getnumopt(str, "=,", &a, &b);
		*num = a;
		*num2 = b;
	
		return TRUE;
	}
	else if ((str[0] == 'D') && (str[1] == 'Z') && (str[2] != '_'))
	{
		Getnumopt(str, "=,", &a, &b);
		*num = a;
		*num2 = b;	
		return TRUE;
	}
	else{
		return FALSE;
	}
}
int CTransDataPage::ProtocolStyle(string str)
{
	if (str == "./lib/libModBusTcp_Transfer.so")//--
	{
		protocol_name = "ModBusTCP_Transfer";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libCdt.so")//***
	{
		protocol_name = "CDT";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libCjt188.so")//**
	{
		protocol_name = "Cjt188";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libDataTrans.so")//--
	{
		protocol_name = "DataTrans";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libDDB.so")//--
	{
		protocol_name = "DDB";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libDlt645.so")//**
	{
		protocol_name = "Dlt645";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libESDCMMI.so")//--
	{
		protocol_name = "ESDCMMI";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libesdYmBreakPoint.so")//
	{
		protocol_name = "esdBreakPoint";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libIEC101S.so")
	{
		protocol_name = "IEC101S";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libIEC103.so")//**
	{
		protocol_name = "IEC103";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libIEC104.so")//--
	{
		protocol_name = "IEC104Slave";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libIEC615.so")//**
	{
		protocol_name = "IEC61850";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libLFP.so")//**
	{
		protocol_name = "LFP";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libModBus.so")//**
	{
		protocol_name = "ModBus";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libModBusMaster.so")//**
	{
		protocol_name = "ModBusMaster";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libModBusSlave.so")//--
	{
		protocol_name = "ModBusSlave";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libModBusTcp.so")//--
	{
		protocol_name = "ModBusTcp";
		return TRANS_PROTOCOL;
	}
	else if (str == "./libModBusTcp_Gather.so")//**
	{
		protocol_name = "MBTcp";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libNanziPDS.so")//**
	{
		protocol_name = "nanzipds";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libSpa.so")//**
	{
		protocol_name = "SPA";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libUPS.so")//***
	{
		protocol_name = "UPS";
		return GATHER_PROTOCOL;
	}
	else if (str == "./lib/libxinaoSlave.so")//--
	{
		protocol_name = "xinaoSlave";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libxinaoSlaveMQTTJSON.so")//--
	{
		protocol_name = "xinaoSlaveMQTTJSON";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libXml.so")//--
	{
		protocol_name = "IMP_XML";
		return TRANS_PROTOCOL;
	}
	else if (str == "./lib/libesdMQTTJSON.so")//--
	{
		protocol_name = "esdMqttJSon";
		return TRANS_PROTOCOL;
	}

}
int CTransDataPage::ParseConfigItem(char *strItem, WORD *pwNum)
{
	char  strType[32];
	int   i, nLen;

	if (strstr(strItem, "station_sum")) return CONFIG_STATION_SUM;
	if (strstr(strItem, "rdbase_size")) return CONFIG_RDBASE_SIZE;
	if (strstr(strItem, "extend_size")) return CONFIG_EXTEND_SIZE;
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
	if (strcmp(strType, "STN") == 0) return CONFIG_STN_PARAM;
	if (strcmp(strType, "AI") == 0) return CONFIG_AI_PARAM;
	if (strcmp(strType, "DI") == 0) return CONFIG_DI_PARAM;
	if (strcmp(strType, "PI") == 0) return CONFIG_PI_PARAM;
	if (strcmp(strType, "DO") == 0) return CONFIG_DO_PARAM;
	return -1;
}
void CTransDataPage::ReadConfig(LPCSTR lpszFile)
{
	int  k;
	FILE *hFile;
	char strLine[192];
	char *pItem, *pParam;
	WORD wNum;
	//SetDefaultConfig();
	//读配置信息
	hFile = fopen(lpszFile, "r");
	if (hFile >0)
	{
		while (fgets(strLine, sizeof(strLine), hFile))
		{
			//ltrim(strLine);
			if (strLine[0] == ';' || strLine[0] == '#') continue;
			//分离文本行
			pItem = strtok(strLine, "=");
			if (pItem == NULL) continue;
			pParam = strtok(NULL, "=");
			if (pParam == NULL) continue;
			//解析配置行
			int nType = ParseConfigItem(pItem, &wNum);
			switch (nType)
			{
			case CONFIG_STATION_SUM:
				m_wStnSum = (WORD)atoi(pParam);
				if (m_wStnSum > MAX_STN_SUM)
					m_wStnSum = MAX_STN_SUM;
				break;
			case CONFIG_RDBASE_SIZE:
				k = atoi(pParam);
				break;
			case CONFIG_EXTEND_SIZE:
				k = atoi(pParam);
				if (k>0) g_nExtendSize = ONE_EXTEND_PAGE * k;
				break;
			case CONFIG_STN_PARAM:
				SetStnAttrib(wNum - 1, pParam);
				break;
			}
		}
		/*lel*/
		int j = 0;
		STNBUSADDR *pObjBusAddr = NULL;
		for (int i = 0; i < m_wStnSum; i++)
		{
			pObjBusAddr = &g_StnBusAddr[i];
			if (((i + 1) <= m_wStnSum) && (pObjBusAddr->byBusNo != (pObjBusAddr - 1)->byBusNo))
				j = 0;

			pObjBusAddr->wDevAddr = ++j;
		}
		/*end*/

		fclose(hFile);
	}
}
void CTransDataPage::SetStnAttrib(WORD wStn, char* szParam)
{
	int i = 0;
	if (wStn >= m_wStnSum) return;
	STNDEF *pObj = &g_StnDef[wStn];
	/*lel*/
	STNBUSADDR *pObjBusAddr = &g_StnBusAddr[wStn];
	/*end*/

	if (strlen(szParam) <= 0) return;
	char *p = strtok(szParam, ",");
	while (p)
	{
		switch (i)
		{
		case 0: //描述名称
			sprintf(pObj->szName, "%s", p);
			break;
		case 1: //遥测数量
			pObj->wAICount = (WORD)atoi(p);
			break;
		case 2: //遥信数量
			pObj->wDICount = (WORD)atoi(p);
			break;
		case 3: //遥控数量
			pObj->wDOCount = (WORD)atof(p);
			break;
		case 4: //电能数量
			pObj->wPICount = (WORD)atof(p);
			break;
		case 5: //定值数量
			pObj->wDZCount = (WORD)atof(p);
			break;
			/*lel*/
		case 6: //总线信息
			pObjBusAddr->byBusNo = (BYTE)atoi(p);
			break;
			/*end*/

		}
		p = strtok(NULL, ",");
		i++;
	}
}
BOOL CTransDataPage::TransDataYc(Json::Value &ycdata)
{
	int order = 0;
	int num = 0;

	for (int i = 0; i < m_wStnSum; i++)
	{
		for (int j = 0; j < g_StnDef[i].wAICount; j++)
		{
			Json::Value ycinfo;

			ycinfo["order_num"]=++order;//序号----

			ycinfo["wserial_num"]=i+1;//装置序号--
		
			ycinfo["station_num"] = g_StnDef[i].szName;//站址----

			ycinfo["station_serial_num"] =j+1;//装置内序号---

			if (yc_trans[i+1][j+1] == "")
				yc_trans[i+1][j+1] = "0";
			
			ycinfo["trans_num"] = yc_trans[i + 1][j + 1];//转发序号

			ycinfo["label"] =yc_label[num]; //标签名

			ycdata["yc_info"].append(ycinfo);

			num++;

		}
	}
	

}
BOOL CTransDataPage::TransDataYx(Json::Value &yxdata)
{
	int order = 0;
	int num = 0;
	for (int i = 0; i < m_wStnSum; i++)
	{
		for (int j = 0; j < g_StnDef[i].wDICount; j++)
		{
			Json::Value yxinfo;

			yxinfo["order_num"] = ++order;//序号----

			yxinfo["wserial_num"] = i + 1;//装置序号--

			yxinfo["station_num"] = g_StnDef[i].szName;//站址----

			yxinfo["station_serial_num"] = j + 1;//装置内序号---

			if (yx_trans[i + 1][j + 1] == "")
				yx_trans[i + 1][j + 1] = "0";

			yxinfo["trans_num"] = yx_trans[i + 1][j + 1];//转发序号
	
			yxinfo["label"] = yx_label[num]; //标签名

			yxdata["yx_info"].append(yxinfo);

			num++;

		}
	}
}
BOOL CTransDataPage::TransDataYm(Json::Value &ymdata)
{
	
	int order = 0;
	int num = 0;
	for (int i = 0; i < m_wStnSum; i++)
	{
		for (int j = 0; j < g_StnDef[i].wPICount; j++)
		{
			Json::Value yminfo;

			yminfo["order_num"] = ++order;//序号-----

			yminfo["wserial_num"] = i + 1;//装置序号----

			yminfo["station_num"] = g_StnDef[i].szName;//站址----

			yminfo["station_serial_num"] = j + 1;//装置内序号---

			if (ym_trans[i + 1][j + 1] == "")
				ym_trans[i + 1][j + 1] = "0";

			yminfo["trans_num"] = ym_trans[i + 1][j + 1];//转发序号

			yminfo["label"] = ym_label[num]; //标签名

			ymdata["ym_info"].append(yminfo);

			num++;

		}
	}
}
BOOL CTransDataPage::TransDataYk(Json::Value &ykdata)
{
	int order = 0;
	int num = 0;
	for (int i = 0; i < m_wStnSum; i++)
	{
		for (int j = 0; j < g_StnDef[i].wDOCount; j++)
		{
			Json::Value ykinfo;

			ykinfo["order_num"] = ++order;//序号----

			ykinfo["wserial_num"] = i + 1;//装置序号--

			ykinfo["station_num"] = g_StnDef[i].szName;//站址----

			ykinfo["station_serial_num"] = j + 1;//装置内序号---

				if (yk_trans[i+1][j+1] == "")
					yk_trans[i+1][j+1] = "0";

				ykinfo["trans_num"]=yk_trans[i+1][j+1];//转发序号

			
			ykinfo["label"] = "null"; //标签名

			ykdata["yk_info"].append(ykinfo);

			num++;

		}
	}

}
BOOL CTransDataPage::TransDataDz(Json::Value &dzdata)
{
	int order = 0;
	int num = 0;
	for (int i = 0; i < m_wStnSum; i++)
	{
		for (int j = 0; j < g_StnDef[i].wDZCount; j++)
		{

			Json::Value dzinfo;

			dzinfo["order_num"] = ++order;//序号----

			dzinfo["wserial_num"] = i + 1;//装置序号--

			dzinfo["station_num"] = g_StnDef[i].szName;//站址----

			dzinfo["station_serial_num"] = j + 1;//装置内序号---

			
			if (dz_trans[i+1][j+1] == "")
				dz_trans[i+1][j+1] = "0";

			dzinfo["trans_num"]=dz_trans[i+1][j+1];//转发序号


			dzinfo["label"] = "null";

			dzdata["dz_info"].append(dzinfo);

		}
	}

}
//string CTransDataPage::GetVerSion()
//{
//	string temp;
//	DWORD ver = EMU2000_VERSION;
//	FILE * pFile = NULL;
//	pFile = fopen("/myapp/2000_version", "r");
//	char szSvnVersion[10] = { 0 };
//	memset(szSvnVersion, '\0', sizeof(szSvnVersion));
//	fread(szSvnVersion, sizeof(szSvnVersion), 1, pFile);
//	fclose(pFile);
//
//	char szVersion[30] = { 0 };
//	memset(szVersion, '\0', sizeof(szVersion));
//	BYTE hh = ver >> 24;
//	BYTE hl = ver >> 16 & 0xFF;
//	BYTE lh = ver >> 8 & 0xFF;
//	BYTE ll = ver & 0xFF;
//
//	sprintf(szVersion, "EMU2000 Ver %d.%d.%d.%d:%s", hh, hl, lh, ll, szSvnVersion);
//	temp = szVersion;
//	temp.erase(temp.end() - 1);//去掉最后一个换行符
//	return temp;
//
//}
//string CTransDataPage::GetSysTime()
//{
//	FILE *fp;
//	string temp;
//	char buf[200] = { 0 };
//	memset(buf, '\0', sizeof(buf));
//	string strFmt = "date +%Y-%m-%d' '%H:%M:%M ";
//	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
//	{
//		perror("Fail to popen\n");
//		exit(1);
//	}
//
//	while (fgets(buf, 200, fp) != NULL)
//	{
//		temp = buf;
//	}
//	pclose(fp);
//	temp.erase(temp.end() - 1);//去掉最后一个换行符
//	return  temp;
//}
//string  CTransDataPage::GetUptime()
//{
//	FILE *fp;
//	string temp;
//	char buf[200] = { 0 };
//	memset(buf, '\0', sizeof(buf));
//	string strFmt = "cat /proc/uptime |awk '{print $1}' ";//系统从启动到现在运行的时间单位秒
//	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
//	{
//		perror("Fail to popen\n");
//		exit(1);
//	}
//	while (fgets(buf, 200, fp) != NULL)
//	{
//		temp = buf;
//	}
//	pclose(fp);
//	temp.erase(temp.end() - 1);//去掉最后一个换行符
//	return  temp;
//
//}
BOOL CTransDataPage::procCmd(BYTE byCmd)
{
	return FALSE;
}

void CTransDataPage::Init()
{
	WORD  i;
	char strFile[96];
	ReadConfig("/mynand/config/rtdb.conf");
	//初始化数据库参数
	for (i = 0; i<m_wStnSum; i++)
	{
		memset(strFile,'\0',sizeof(strFile));
		sprintf(strFile, "/mynand/config/Station/stn%02d.conf", i + 1);
		PntInit(i, strFile);
	}
	
}

void CTransDataPage::PntInit(WORD wStn, LPCSTR lpszFile)
{
	FILE *hFile;
	char strLine[256];
	char *pItem, *pParam;
	WORD wNum;
	string temp;
	if (wStn >= m_wStnSum) return;
	//读点表配置信息
	hFile = fopen(lpszFile, "r");
	if (hFile > 0)
	{
		while (fgets(strLine, sizeof(strLine), hFile))
		{
			//ltrim(strLine);
			if (strLine[0] == ';' || strLine[0] == '#') continue;
			//分离文本行
			pItem = strtok(strLine, "=");
			if (pItem == NULL) continue;
			pParam = strtok(NULL, "=");
			if (pParam == NULL) continue;
			//解析配置行
			temp = pParam;
			int nType = ParseConfigItem(pItem, &wNum);
			switch (nType)
			{
			//遥测处理
			case CONFIG_AI_PARAM:
				yc_label.push_back(temp.substr(0,temp.find_first_of(",")));		//将标签存入vector				
				break;
				//遥信处理
			case CONFIG_DI_PARAM:
				yx_label.push_back(temp.substr(0, temp.find_first_of(",")));		//将标签存入vector					
				break;
				//遥脉处理
			case CONFIG_PI_PARAM:
				ym_label.push_back(temp.substr(0, temp.find_first_of(",")));		//将标签存入vector					
				break;
				//遥控处理
			case CONFIG_DO_PARAM:
				yk_label.push_back(temp.substr(0, temp.find_first_of(",")));		//将标签存入vector				
				break;

			}
		}
		fclose(hFile);
	}
}

void CTransDataPage::setLog(Clog * pLog)
{
	m_log = pLog;
}



