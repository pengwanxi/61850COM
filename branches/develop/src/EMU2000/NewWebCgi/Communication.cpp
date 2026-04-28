#include "Communication.h"



CCommunication::CCommunication()
{
	m_strProjectDirectoryPath = "/mnt/project_config/config/";
}


CCommunication::~CCommunication()
{
}

BOOL CCommunication::getJSONStructFromWebPage(Json::Value &root)
{
	Json::Value busJsonVal;
	Json::Value templateJsonVal;
	Json::Value comTypeJsonVal;
	Json::Value devListJsonVal;
	getBusLineVal(busJsonVal);
	root["Communacation_busVal"] = busJsonVal;
	root["Communacation_template"] = templateJsonVal;
	root["Communacation_comType"] = comTypeJsonVal;
	root["Communacation_devlist"] = devListJsonVal;
	return TRUE;
}

void CCommunication::getBusLineVal(Json::Value & busJsonVal)
{

	Json::Value totalBus;
	string strBusLineFile = m_strProjectDirectoryPath + "BusLine.ini";
	CProfile pfile((char *)strBusLineFile.c_str());
	int line_num = pfile.GetProfileInt("LINE-NUM", "NUM", -1);
	if (line_num == -1)
	{
		printf("read busline.ini error!\n");
		return;
	}
	
	char szPortSect[20] = { "PORT" };
	char szLine[20] = { "port" };
	char sztemp[100] = { 0 };
	char szVal[200] = { 0 };
	for (int i = 0; i < line_num; i++)
	{
		Json::Value portJVal;
		sprintf(sztemp, "%s%02d", szLine, i + 1);
		portJVal["name"] = sztemp;

		pfile.GetProfileString(szPortSect, sztemp, "NULL", szVal, 200);
		if (strcmp(szVal, "PAUSE") == 0)
		{
			portJVal["type"] = szVal;
			totalBus.append(portJVal);
			continue;
		}
		getCommunicationParam(szVal, i,portJVal );

		sprintf(sztemp, "NetCard%02d", i + 1);
		pfile.GetProfileString(szPortSect, sztemp, "NULL", szVal, 200);
		portJVal["property"] = szVal;


		sprintf(sztemp, "para%02d", i + 1);
		pfile.GetProfileString(szPortSect, sztemp, "NULL", szVal, 200);
		getProtocolParam(szVal, i , portJVal);

		sprintf(sztemp, "internal%02d", i + 1);
		pfile.GetProfileString(szPortSect, sztemp, "NULL", szVal, 200);
		portJVal["interval"] = atoi(szVal);

		totalBus.append(portJVal);
	}
	busJsonVal["curItem"] = "0";
	busJsonVal["totalBus"] = totalBus;
}

void CCommunication::getProtocolParam(char * pszVal, WORD wBusNo, Json::Value &portData)
{
	if (pszVal == NULL)
		return;

	string strVal(pszVal);
	size_t begin =  strVal.find_last_of("lib");
	size_t end = strVal.find_last_of(".");
	string val = strVal.substr(begin + 1, end - begin - 1);
	portData["protocol"] = val;
}


void CCommunication::getCommunicationParam(char * pszVal, WORD wBusNo, Json::Value &portData)
{
	if (pszVal == NULL)
		return;

	string strVal(pszVal);
	size_t found = strVal.find("_");
	if (found == string::npos)
		return;
	
	string strPortNo;
	string strComType = strVal.substr(0, found);
	string strProperty;
	if (strComType == "TCP")
	{
		strPortNo = strVal.substr(found + 1);
		strProperty = "eth0"; //后面更新
	}
	else
	{
		size_t found_m = strVal.find(":");
		if (found_m == string::npos)
			return;
		strPortNo = strVal.substr(found + 1, found_m - found - 1);
		strProperty = strVal.substr(found_m + 1);
	}

	portData["type"] = strComType;
	portData["portNo"] = strPortNo;
	portData["property"] = strProperty;
}


BOOL CCommunication::procCmd(BYTE byCmd)
{
	return TRUE;
}

void CCommunication::Init()
{
}

void CCommunication::setLog(Clog *)
{
}
