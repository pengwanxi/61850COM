#include "DataManagementPage.h"


CDataManagerPage::CDataManagerPage()
{
	yc_order = 0;
	ym_order = 0;
	DevSerialNumber = 0;
	StationAddress = "0";
	basic_value = "0";
	ratio_value = "0";
	threshold_value = "0";
	data_label = "0";

}

CDataManagerPage::~CDataManagerPage()
{

}

BOOL CDataManagerPage::getJSONStructFromWebPage(Json::Value &root)
{
	Json::Value datamanager;

	root["data_manager_page_version"] = GetVerSion();
	root["data_manager_page_systime"] = GetSysTime();
	root["data_manager_page_uptime"] = GetUptime();

	GetDataPar(datamanager);

	root["data_manager_page_protocol"] = datamanager;

	return FALSE;
}
BOOL CDataManagerPage::GetDataPar(Json::Value &pardata)
{
	BYTE byLineNum = 0;
	BYTE devnum = 0;

	yc_order = 0;
	ym_order = 0;
	devnum = 0;
	char sBusLine[] = BUS_PATH;
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

		if (TRANS_PROTOCOL == ret)
		{
			continue;
		}
		else if (GATHER_PROTOCOL == ret)
		{

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
			//获取装置数量
			sprintf(sTemp, "/mynand/config/%s/Bus%02d.ini", protocol_name.c_str(), i + 1);
			CProfile Profile_gather(sTemp);
			int DevNum = (BYTE)Profile_gather.GetProfileInt((char *)"DEVNUM", (char *)"NUM", 0);
			memset(sTemp, 0, sizeof(sTemp));
			sprintf(sTemp, "%03d", DevNum);
			data["data_mananger_dev_num"] = sTemp;

			for (int j = 0; j < DevNum; j++)
			{
				Json::Value devdata;
				Json::Value devinfo;
				int order = 0;
				memset(sTemp, 0, sizeof(sTemp));
				sprintf(sTemp, "DEV%03d", ++devnum);
				devdata["dev"] =sTemp;
				
				memset(sTemp, 0, sizeof(sTemp));
				sprintf(sTemp, "DEV%03d", j + 1);
				memset(sbuffer, '\0', sizeof(sbuffer));

				Profile_gather.GetProfileString(sTemp, "name", (char *)"NULL", sbuffer, sizeof(sbuffer));

				memset(sTemp, 0, sizeof(sTemp));
				sprintf(sTemp, "/mynand/config/Station/stn%02d.conf",  devnum);

				DevSerialNumber = devnum;//装置序号
				CommunicationBankNumber = i+1;//通讯行号				
				StationAddress = sbuffer;//站址

				GetDevInfo(sTemp, devinfo);

				devdata["dev_info"] = devinfo;
				data["data_mananger_dev"].append(devdata);
			}
			pardata["protocol"].append(data);

		}
		
	}

}

void CDataManagerPage::GetDevInfo(char pathname[], Json::Value &devinfo)
{
	FILE *fp;
	char line[100];
	Json::Value ycinfo;
	Json::Value yminfo;
	BYTE yc_serial_order = 0;
	BYTE ym_serial_order = 0;

	fp = fopen(pathname, "r");
	if (fp == NULL)
	{
		char error_temp[500] = {0};
		sprintf(error_temp,"open %s   fail\n", pathname);
		devinfo["error"].append(error_temp);
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
			if (line[0] == 'A'&&line[1] == 'I')
			{
				GetYCDataProperty(line);
				ycinfo["data_yc_order"] = ++yc_order;//序号
				ycinfo["device_serial_number"] = DevSerialNumber;//装置序号
				ycinfo["communication_bank_number"] = CommunicationBankNumber;//通讯行号
				ycinfo["data_station_address"] = StationAddress;//站址
				ycinfo["data_station_serial_number"] = ++yc_serial_order;//装置内序号
				ycinfo["data_ basic value"] = basic_value;//基值
				ycinfo["data_ coefficient "] = ratio_value;//系数
				ycinfo["data_threshold_value"] = threshold_value;//门槛值
				ycinfo["data_dev_label"] =data_label;//标签
				devinfo["yc_dev_data"].append(ycinfo);

			}
			else if (line[0] == 'P'&&line[1] == 'I')
			{
				GetYMDataProperty(line);
				yminfo["data_ym_order"] = ++ym_order;
				yminfo["device_serial_number"] = DevSerialNumber;//装置序号
				yminfo["communication_bank_number"] = CommunicationBankNumber;//通讯行号
				yminfo["data_station_address"] = StationAddress;//站址
				yminfo["data_station_serial_number"] =++ ym_serial_order;//装置内序号

				yminfo["data_ basic value"] ="0";//基值------遥脉不支持基值 原配置软件上有标注
				yminfo["data_ coefficient "] = ratio_value;//系数
				yminfo["data_threshold_value"] = "0";//门槛值-----遥脉不支持门槛值 原配置软件上有标注
				yminfo["data_dev_label"] = data_label;//标签
				devinfo["ym_dev_data"].append(yminfo);
			}
			else
			{
				continue;
			}
		}
	}
	fclose(fp);

}
void CDataManagerPage::GetYCDataProperty(char *str)
{
	char temp[16];
	int i = 0;
	int nLen = strlen(str);
	if (nLen <= 0) return;
	char *p = strtok(str, "=,");
	while (p)
	{
		memset(temp,'\0',sizeof(temp));
		switch (i)
		{
		case 0:
			break;
		case 1: //名称
			sprintf(temp, "%s", p);
			data_label = temp;
			break;
		case 2: //类型
			break;
		case 3: //单位
			break;
		case 4: //系数
			sprintf(temp, "%s", p);
			ratio_value = temp;
			break;
		case 5: //偏移---基值
			sprintf(temp, "%s", p);
			basic_value = temp;
			break;
		case 6: //点控制字
			break;
		case 7: //变化阈值----门槛值
			sprintf(temp, "%s", p);
			threshold_value = temp;
			break;
		}
		p = strtok(NULL, "=,");
		i++;
	}


}
void CDataManagerPage::GetYMDataProperty(char *str)
{
	int i = 0;
	char temp[16];
	int nLen = strlen(str);
	if (nLen <= 0) return;
	char *p = strtok(str, "=,");
	while (p)
	{
		memset(temp, '\0', sizeof(temp));
		switch (i)
		{
		case 0:
			break;
		case 1: //名称----标签
			sprintf(temp, "%s", p);
			data_label = temp;
			break;
		case 2: //类型
			break;
		case 3: //属性
			break;
		case 4: //系数
			sprintf(temp, "%s", p);
			ratio_value = temp;
			break;
		case 5: //控制字
			break;
		}
		p = strtok(NULL, "=,");
		i++;
	}
}

int CDataManagerPage::ProtocolStyle(string str)
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

BOOL CDataManagerPage::procCmd(BYTE byCmd)
{
	return FALSE;
}

void CDataManagerPage::Init()
{

	
}
void CDataManagerPage::setLog(Clog * pLog)
{
	m_log = pLog;
}



