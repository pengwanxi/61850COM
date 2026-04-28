#include "DataProto_JSon.h"

#include <string>
#include <time.h>
#include "Protocol_Xin_ao_Slave_MQTT_JSON_Module.h"
extern "C" int  SetCurrentTime(REALTIME *pRealTime);
extern "C" void GetCurrentTime(REALTIME *pRealTime);
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

CDataProto_JSon_Xin_ao::CDataProto_JSon_Xin_ao()
{
	m_pMQTT_mosquitto = NULL;
	time(&m_begin_counter);
	m_stnCounter = 0xFFFF;
}


CDataProto_JSon_Xin_ao::~CDataProto_JSon_Xin_ao()
{
}

bool CDataProto_JSon_Xin_ao::getSendBuf(char * pDataSend, int &len)
{
	return false;
}

bool CDataProto_JSon_Xin_ao::processRecvBuf(char * ptopic, char * pPayload)
{
	if (!ptopic || !pPayload)
		return false;
	
	if (!strlen(pPayload))
		return false;
	
	Json::Reader reader;
	Json::Value rootVal;
	bool ret = reader.parse( pPayload, rootVal);
	if (!ret)
	{
		cout << reader.getFormatedErrorMessages() << endl;
		return false;
	}

	Json::Value::Members json_keys = rootVal.getMemberNames();

	bool bret = 0;
	if (!strcmp(ptopic, m_topic[etiming]))
		bret = processTiming(rootVal, json_keys);
	else if (!strcmp(ptopic, m_topic[edevinfo]))
		bret = processDevinfo(rootVal, json_keys);
	else if (!strcmp(ptopic, m_topic[estatus]))
		bret = processStatus(rootVal, json_keys);

	return  bret;
}

bool CDataProto_JSon_Xin_ao::processStatus(Json::Value &rootVal, Json::Value::Members  &json_keys)
{
	if (!rootVal.size() || !json_keys.size())
		return false;
	bool bRes = rootVal.isMember("ver") && rootVal.isMember("pKey") && rootVal.isMember("sn") &&
		rootVal.isMember("type") && rootVal.isMember("seq");

	if (!bRes)
	{
		printf("%s  %s %d this item doesn't parse\n", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	string strType = rootVal["type"].asString();
	if (strType != "status")
	{
		printf("%s  %s %d this item doesn't parse\n", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	bRes = rootVal.isMember("cpu");
	if (bRes)
	{
		printf("%s  %s %d this item doesn't parse\n", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}
	time_t sec = 0;
	time(&sec);

	Json::Value root;
	root["ver"] = rootVal["ver"];
	root["pKey"] = m_szPkey;
	root["sn"] = m_szSn;
	root["type"] = rootVal["status"];
	root["ts"] = (Json::Value::UInt)sec;
	root["seq"] = rootVal["seq"];
	root["name"] = "2104EEM";
	char szCpu[30] = { 0 };
	sprintf(szCpu, "%.2f%% ", GetCpuPercent());
	root["cpu"] = szCpu;

	char szMem[30] = { 0 };
	sprintf(szMem, "%.2f%%", GetMemoryPercent() * 100 );

	root["mem"] = szMem ;
	root["commFlow"] = 0;
	root["signal"] = 0;

	string strout = root.toStyledString();
	int len = strout.length() + 1;

	m_pMQTT_JsonModule->m_mqtt.publishMsg(m_topic[estatus_ack], strout.c_str(), len);
	return true;
}

float CDataProto_JSon_Xin_ao::GetMemoryPercent()
{
	FILE *fp;
	char buf[200] = { 0 };
	string strFmt = " free | grep Mem | awk '{ print $2 \"\\n\" $3 }'  ";
	printf(strFmt.c_str());
	printf("\n");

	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}

	float fTotal = 0.0, fUsed = 0.0;
	int index = 0;
	while (fgets(buf, 200, fp) != NULL)
	{
		if (index == 0)
			fTotal = atof(buf);
		else
			fUsed = atof(buf);
		index++;
	}

	printf("mem: %f\n", fUsed / fTotal );
	pclose(fp);

	float fmemPercent = fUsed / fTotal;
	return fmemPercent ;
}

float CDataProto_JSon_Xin_ao::GetCpuPercent()
{
	FILE *fp;
	char buf[ 200 ] = { 0 };
	string strFmt = "mpstat 1 1 | grep all | grep -v Average | awk '{ print $3  \"\\n\" $5}'  ";
	printf(strFmt.c_str());
	printf("\n");

	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}

	float fcpuPercent = 0.0;
	while (fgets(buf, 200 , fp) != NULL)
	{
		fcpuPercent += atof(buf);
	}

	printf("cpu: %f\n", fcpuPercent);
	pclose(fp);
	
	return fcpuPercent;
}

bool CDataProto_JSon_Xin_ao::processDevinfo(Json::Value &rootVal, Json::Value::Members  &json_keys)
{
	if (!rootVal.size() || !json_keys.size())
		return false;
	bool bRes = rootVal.isMember("ver") && rootVal.isMember("pKey") && rootVal.isMember("sn") &&
		rootVal.isMember("type") && rootVal.isMember("seq");

	if (!bRes)
	{
		printf("%s  %s %d this item doesn't parse\n", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	string strType = rootVal["type"].asString();
	if (strType != "info")
	{
		printf("%s  %s %d this item doesn't parse\n", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	bRes = rootVal.isMember("compId");
	if (bRes)
	{
		printf("%s  %s %d this item doesn't parse\n", __FILE__, __FUNCTION__, __LINE__);
		return false;
	}

	time_t sec = 0;
	time(&sec);

	Json::Value root;
	root["ver"] = rootVal["ver"];
	root["pKey"] = m_szPkey;
	root["sn"] = m_szSn;
	root["type"] = rootVal["type"];
	root["ts"] = (Json::Value::UInt)sec;
	root["seq"] = rootVal["seq"];
	root["compId"] = "EEM";
	root["name"] = "8122EEM";
	root["sVer"] = "0.0";
	root["hVer"] = "0.0";
	root["meid"] = "0.0";
	root["loc"] = "0.0";
	root["memSize"] = (Json::Value::UInt)getMemorySize();
	root["diskSize"] = (Json::Value::UInt)getDiskSize();
	root["encr"] = "TLS";
	root["connType"] = "G";
	root["mac"] = getMacAddr("eth0");
	
	string strout = root.toStyledString();
	int len = strout.length() + 1;

	m_pMQTT_JsonModule->m_mqtt.publishMsg(m_topic[edevinfo_ack], strout.c_str(), len);
	return true;
}

string CDataProto_JSon_Xin_ao::getMacAddr(string strNetcardName)
{
	FILE *fp;
	char buf[200] = { 0 };
	string strFmt("ifconfig | grep ");
	string strEnd("| awk '{ print $5 }' ");
	string strCard(strNetcardName);

	strFmt = strFmt + strNetcardName + strEnd;
	if ((fp = popen( strFmt.c_str() , "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}

	string str;
	while (fgets(buf, 200, fp) != NULL)
	{
		str = buf;
	}

	pclose(fp);
	
	return str;
}

DWORD CDataProto_JSon_Xin_ao::getDiskSize()
{
	FILE *fp;
	char buf[200] = { 0 };
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

	pclose(fp);
	return size;
}

DWORD CDataProto_JSon_Xin_ao::getMemorySize()
{
	FILE *fp;
	char buf[200] = { 0 };
	if (  (fp = popen(" free | awk '{print $2}'| grep -v \"used\" ","r") ) == NULL )
	{
		perror("Fail to popen\n");
		exit(1);
	}
	string str;
	while (fgets(buf, 200, fp) != NULL)
	{
		str = buf ;
		break;
	}
	pclose(fp);

	int i = atoi(str.c_str()); 
	return i;
}

bool CDataProto_JSon_Xin_ao::subscribeMsg()
{
	m_pMQTT_JsonModule->m_mqtt.subscribeMsg( m_topic[ etiming ] );
	m_pMQTT_JsonModule->m_mqtt.subscribeMsg(m_topic[estatus]);
	m_pMQTT_JsonModule->m_mqtt.subscribeMsg(m_topic[edevinfo]);
	return true;
}

bool CDataProto_JSon_Xin_ao::processTiming(Json::Value &rootVal , Json::Value::Members  &json_keys )
{
	if (!rootVal.size() || !json_keys.size())
		return false;
	bool bRes = rootVal.isMember("ver") && rootVal.isMember("pKey") && rootVal.isMember("sn") &&
		rootVal.isMember("type") && rootVal.isMember("ts") && rootVal.isMember("seq");

	printf("%s%s%d bRes = %d\n", __FILE__, __FUNCTION__, __LINE__, bRes);
	if (!bRes)
	{
		printf( "%s  %s %d this item doesn't parse\n" , __FILE__ , __FUNCTION__ , __LINE__ );
		return false;
	}

	string sVer = rootVal["ver"].asString();
	string sKey = rootVal["pKey"].asString();
	string sSn = rootVal["sn"].asString();
	string sType = rootVal["timing"].asString();
	long ltime = rootVal["ts"].asInt64();
	string strSeq = rootVal["seq"].asString();
	if (sKey != string(m_szPkey) || sSn != string(m_szSn))
		return false;

	struct tm * preSetTime = localtime((time_t*)&ltime);
	printf("%s %d %d %d %d %d %d %d\n", __FUNCTION__ , __LINE__ , 
		preSetTime->tm_year + 1900 , preSetTime->tm_mon + 1 , preSetTime->tm_mday, preSetTime->tm_hour, preSetTime->tm_min, preSetTime->tm_sec);

	REALTIME rtime;
	rtime.wYear = preSetTime->tm_year + 1900;
	rtime.wMonth = preSetTime->tm_mon + 1;
	rtime.wDay = preSetTime->tm_mday;
	rtime.wHour = preSetTime->tm_hour;
	rtime.wMinute = preSetTime->tm_min;
	rtime.wSecond = preSetTime->tm_sec;
	rtime.wMilliSec = 0;

	SetCurrentTime(&rtime);
	replayTiming(rootVal , json_keys );
}

void CDataProto_JSon_Xin_ao::replayTiming(Json::Value &rootVal, Json::Value::Members  &json_keys )
{
	map<string, unsigned char > timeMap = { { "ver" , 0 } ,{ "pKey" , 0 } ,{ "sn" , 0 } ,{ "type" , 0 } ,{ "ts" , 0 } ,{ "seq" , 0 } };
	if (!isReply(json_keys, timeMap))
		return ;
	if (!m_pMQTT_mosquitto)
		return ;

	Json::Value root;
	root["ver"] = rootVal["ver"];
	root["pKey"] = m_szPkey ;
	root["sn"] = m_szSn;
	root["type"] = rootVal["type"];
	root["valid"] = 1;
	root["remark"] = "timing OK";
	root["ts"] = rootVal["ts"];
	root["seq"] = rootVal["seq"];
	string strout = root.toStyledString();

	int len = strout.length() + 1;

	m_pMQTT_mosquitto->publishMsg(m_topic[etiming_ack], strout.data(), len);
}

bool CDataProto_JSon_Xin_ao::isReply( Json::Value::Members &json_keys , map<string , unsigned char >&timeMap )
{
	bool bRes = true;
	for (unsigned char i = 0; i < timeMap.size(); i++)
	{
		map<string, unsigned char >::iterator itor = timeMap.find(json_keys.at(i));
		if (itor == timeMap.end())
		{
			cout << " don't find specify element" << endl;
			bRes = false;
			break;
		}
	}
	return bRes;
}

void CDataProto_JSon_Xin_ao::setMqtt(CMQTT_mosquitto * mosquitto)
{
	m_pMQTT_mosquitto = mosquitto;
}

void CDataProto_JSon_Xin_ao::setSnAndPkey(char * pSn, char * pKey)
{
	if (!pSn || !pKey)
		return;

	strcpy(m_szSn, pSn);
	strcpy(m_szPkey, pKey);
}

void CDataProto_JSon_Xin_ao::setProtoDataObj( CProtocol_Xin_ao_Slave_MQTT_JSON_Module * pMJ )
{
	if (!pMJ)
		return;
	
	m_pMQTT_JsonModule = pMJ;
}

void CDataProto_JSon_Xin_ao::TimerProc()
{
	time(&m_end_counter);
	if (difftime(m_end_counter, m_begin_counter) > 30)
	{
		sendYcRealData();
		time(&m_begin_counter);
	}

}

void CDataProto_JSon_Xin_ao::sendYcRealData()
{

	if (!m_pMQTT_JsonModule)
		return;
	
	Json::Value root;
	AddJsonHeadData(root);
	Json::Value joneDevice;
	Json::Value allDevice;
	Json::Value jCollect;

	WORD wTotalYc = m_pMQTT_JsonModule->m_wAISum;
	MAPITEM *pYcItemHead = m_pMQTT_JsonModule->GetYcTable();
	WORD index = 0;

	for ( WORD i  = 0 ; i < wTotalYc ; i++ )
	{
		if (pYcItemHead->wPntNum == 0 || pYcItemHead->wStn == 0)
		{
			index++;
			pYcItemHead++;
			continue;
		}
		
		if (m_stnCounter != pYcItemHead->wStn)
		{
			AddDeviceToJSon(allDevice, joneDevice, jCollect , m_stnCounter );
			BYTE byBusNo = 0;
			WORD wDevNo = 0;
			char devName[60] = { 0 };
			m_pMQTT_JsonModule->m_pMethod->GetBusLineAndAddr(pYcItemHead->wStn, byBusNo , wDevNo , devName );
			joneDevice["dev"] = devName;
			time_t sec = 0;
			time(&sec);
			joneDevice["ts"] = sec;
			joneDevice["sysId"] = "";
			m_stnCounter = pYcItemHead->wStn;
		}
		
		Json::Value jdata;
		jdata["m"] = GetMapDataName(pYcItemHead->wDevId).c_str();
		jdata["v"] = m_pMQTT_JsonModule->m_wYCBuf[index++];
		
		time_t sec = 0;
		time(&sec);

		jdata["ts"] = sec;
		jdata["dq"] = 0;
		jCollect.append(jdata);

		pYcItemHead++;
	}

	m_stnCounter = -1;
	
	//�������һ��װ��
	AddDeviceToJSon(allDevice, joneDevice, jCollect, m_stnCounter);

	root["devs"] = allDevice;
	string strout = root.toStyledString();
	int len = strout.length();

	m_pMQTT_mosquitto->publishMsg(m_topic[ertg], strout.data(), len + 1 );

}

void CDataProto_JSon_Xin_ao::AddDeviceToJSon(Json::Value &allDevice , Json::Value &joneDevice, Json::Value &jCollect, WORD wStn)
{
	if (jCollect.size() == 0)
		return;
	
	//����װ��ͨѶ״̬
// 	bool comStatus = m_pMQTT_JsonModule->m_pMethod->GetDevCommState(wStn);
// 	Json::Value jcom;
// 	jcom["m"] = "CommStatus";
// 	jcom["v"] = !comStatus;
// 	time_t sec = 0;
// 	time(&sec);
// 	jcom["ts"] = sec;
// 	jcom["dq"] = 0;
// 	jCollect.append(jcom);

	joneDevice["d"] = jCollect;
	allDevice.append(joneDevice);
	
	joneDevice.clear();
	jCollect.clear();
}

void CDataProto_JSon_Xin_ao::AddJsonHeadData(Json::Value &root)
{
	time_t sec = 0;
	time(&sec);

	root["ver"] = "v1.0.0";
	root["pKey"] = m_szPkey;
	root["sn"] = m_szSn;
	root["ts"] = (Json::Value::UInt)sec;
}

void CDataProto_JSon_Xin_ao::readMapDataTable()
{
	char szFileName[256] = "";
	sprintf(szFileName, "%s%s", "/mynand/config/xinaoSlaveMQTTJSON/", "map_data.cfg");
	CProfile file(szFileName);
	if (!file.IsValid())
	{
		printf("xin ao mqtt server.cfg is error\n");
		return ;
	}

	char  sSec[ 10 ]  = "yc";
	WORD key = 0;
	while (true)
	{
		key++;
		char retBuf[20] = { 0 };
		WORD len = sizeof(retBuf);
		char szkey[20] = { 0 };
		sprintf(szkey, "%d", key);
		file.GetProfileString(sSec , szkey , "NULL", retBuf, len);
		if (!strcmp(retBuf, "NULL"))
			break;
		
		m_dataNameMap[key] = retBuf;
	}

	readTopic();
}

void CDataProto_JSon_Xin_ao::readTopic()
{
	string  topic[ ] = { "/cloud/pkey/sn/timing/call"  ,		//��ʱ
		"/cloud/pkey/sn/status/call"	,		//�ٶ�������Ϣ
		"/cloud/pkey/sn/info/call" , //�ٶ��豸��Ϣ
		"/edge/pkey/sn/rtg",			//����ʵʱ�����ϴ�
		"/edge/pkey/sn/info", //�����ظ��ٶ��豸��Ϣ
		"/edge/pkey/sn/status", //�����ظ��ٶ�������Ϣ
		"/edge/pkey/sn/timing/cack", //�����ظ���ʱ��Ϣ
	};
	

	for ( int i = 0 ; i < 7 ; i++ )
	{
		string str = topic[i];
		int  pos = str.find("pkey");
		str.replace(pos, 4, m_szPkey);

		pos = str.find("sn");
		str.replace(pos, 2, m_szSn);
		strcpy(m_topic[i], str.c_str());
	}

	for ( int m = 0 ; m < 7 ; m++ )
	{
		printf(m_topic[ m ]);
		printf("\n");
	}
}

string CDataProto_JSon_Xin_ao::GetMapDataName(WORD wkey)
{
	int size = m_dataNameMap.size();
	if (!size)
		return " ";

	map<WORD, string >::iterator itor = m_dataNameMap.find(wkey);
	if (itor == m_dataNameMap.end() )
		return "";

	return itor->second;
}