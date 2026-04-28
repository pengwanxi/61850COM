#include "Protocol_Xin_ao_Slave_MQTT_JSON_Module.h"
#include <json/json.h>
#include <cmath>

CProtocol_Xin_ao_Slave_MQTT_JSON_Module::CProtocol_Xin_ao_Slave_MQTT_JSON_Module()
{
}


CProtocol_Xin_ao_Slave_MQTT_JSON_Module::~CProtocol_Xin_ao_Slave_MQTT_JSON_Module()
{
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::Init(BYTE byLineNo)
{
	m_byLineNo = byLineNo;
	sprintf(m_szObjName, "%s", m_sDevName);
	m_wRtuAddr = m_wDevAddr;
	char szFileName[256] = "";

	sprintf(szFileName, "%s%s", "/mynand/config/xinaoSlaveMQTTJSON/", m_sTemplatePath);
	//��ȡ��Ҫת�������ݵ���ģ��
	ReadMapConfig(szFileName);

	//��ʼ����ģ��
	InitRtuBase();

	//��ȡserver����
	InitServerCfg();
	//����mqtt
	InitMqttandJSon();

	return TRUE;
}

void CProtocol_Xin_ao_Slave_MQTT_JSON_Module::InitMqttandJSon()
{
	m_mqtt.SetCafiles(m_szCafile, m_szCertfile, m_szKeyfile);
	m_mqtt.SetLoginInfo(m_szUserName, m_szPasswd);
	m_mqtt.SetServerInfo(m_szServerIP, m_szPort);
	m_mqtt.setDataProto(&m_json);
	m_json.setMqtt( &m_mqtt );
	m_json.setSnAndPkey(m_sDevName, m_szPKey);
	m_json.setProtoDataObj(this);
	m_json.readMapDataTable();
	m_mqtt.Init();
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::InitServerCfg()
{
	char szFileName[256] = "";
	sprintf(szFileName, "%s%s", "/mynand/config/xinaoSlaveMQTTJSON/", "server.cfg");
	CProfile file(szFileName);
	if (!file.IsValid())
	{
		printf("xin ao mqtt server.cfg is error\n");
		return false;
	}
	char sSect[200] = "server";
	char sKey[20][50] = { "username" , "passwd" , "cafile" , "certfile" , "keyfile" };

	file.GetProfileString(sSect, sKey[0], (char *)"NULL", m_szUserName, sizeof( m_szUserName ));
	file.GetProfileString(sSect, sKey[1], ( char *)"NULL", m_szPasswd, sizeof(m_szPasswd));
	file.GetProfileString(sSect, sKey[2], ( char *)"NULL", m_szCafile, sizeof(m_szCafile));
	file.GetProfileString(sSect, sKey[3], (char *)"NULL", m_szCertfile, sizeof(m_szCertfile));
	file.GetProfileString(sSect, sKey[4], ( char *)"NULL", m_szKeyfile, sizeof(m_szKeyfile));
	printf("server config info: %s , %s , %s , %s ,%s\n" , m_szUserName, m_szPasswd, m_szCafile, m_szCertfile, m_szKeyfile);

	file.GetProfileString(sSect, ( char *)"ip", (char *)"NULL", m_szServerIP, sizeof(m_szServerIP));
	file.GetProfileString(sSect, (char *)"port", (char *)"NULL", m_szPort, sizeof(m_szPort ) );
	file.GetProfileString((char *)"json", (char *)"pkey", (char *)"NULL", m_szPKey, sizeof(m_szPKey));
	
	printf("server config info: %s , %s , %s \n", m_szServerIP, m_szPort, m_szPKey );

	return true;
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::InitRtuBase()
{/*{{{*/
	UINT uPort;
	BOOL bOk = FALSE;
	char szCtrl[32];

	CBasePort::GetCommAttrib(m_ComCtrl1, szCtrl, uPort);

	m_wPortNum = (WORD)uPort;

	//��ȡת�����
	CreateTransTab();

	//���ڴ����ݿ���--��ȡת����Ĭ������
	m_pMethod->ReadAllYcData(&m_wYCBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwYMBuf[0]);
	m_pMethod->ReadAllYxData(&m_byYXbuf[0]);

	m_bTaskRun = TRUE;
	return bOk;
}/*}}}*/


void CProtocol_Xin_ao_Slave_MQTT_JSON_Module::TimerProc()
{
	//���ڴ��ж�ȡ�仯ң�ź�ң������
	ReadChangData();
	m_json.TimerProc();
}


BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::WriteAIVal(WORD wSerialNo, WORD wPnt, float wVal)
{
	if (m_pwAITrans == NULL) return FALSE;
	WORD wNum = m_pwAITrans[wPnt];
	if (wNum > m_wAISum) return FALSE;
	if (wNum < MAX_AI_LEN)
	{
		float nDelt = wVal - m_wYCBuf[wNum];
		if ( abs(nDelt) >= m_wDeadVal )
		{
			m_wYCBuf[wNum] = wVal;
			AddAnalogEvt(wSerialNo, wNum, wVal);
		}
	}
	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::WriteDIVal(WORD wSerialNo, WORD wPnt, WORD wVal)
{
	if (m_pwDITrans == NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if (wNum > m_wDISum) return FALSE;
	if (wNum < MAX_DI_LEN)
	{
		if (m_byYXbuf[wNum] != wVal)
		{
			m_byYXbuf[wNum] = wVal;

			AddDigitalEvt(wSerialNo, wNum, wVal);
		}
	}
	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::WritePIVal(WORD wSerialNo, WORD wPnt, QWORD dwVal)
{
	if (m_pwPITrans == NULL) return FALSE;
	WORD wNum = m_pwPITrans[wPnt];
	if (wNum > m_wPISum) return FALSE;
	if (wNum < MAX_PI_LEN)
	{
		m_dwYMBuf[wNum] = dwVal;
	}
	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::WriteSOEInfo(WORD wSerialNo, WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{
	if (m_pwDITrans == NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if (wNum >= m_wDISum) return FALSE;
	if (wNum < MAX_DI_LEN)
	{
		AddSOEInfo(wSerialNo, wNum, wVal, lTime, wMiSecond);
	}
	return TRUE;
}


BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg)
{
	return false;
}

BOOL CProtocol_Xin_ao_Slave_MQTT_JSON_Module::ProcessProtocolBuf(BYTE * buf, int len)
{
	return false;
}

MAPITEM * CProtocol_Xin_ao_Slave_MQTT_JSON_Module::GetYcTable()
{
	return m_pAIMapTab;
}