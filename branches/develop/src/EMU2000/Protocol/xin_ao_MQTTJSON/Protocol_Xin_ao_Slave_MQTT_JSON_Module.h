#pragma once
#include "Protocol_Xin_ao_Slave.h"
#include "MQTT_mosquitto.h"
#include "DataProto_JSon.h"

#define MAX_AI_LEN (6800)
#define MAX_PI_LEN (1599)
#define MAX_DI_LEN (9999)

class CProtocol_Xin_ao_Slave_MQTT_JSON_Module : public CProtocol_Xin_ao_Slave
{
public:
	CProtocol_Xin_ao_Slave_MQTT_JSON_Module();
	virtual ~CProtocol_Xin_ao_Slave_MQTT_JSON_Module();
	virtual void TimerProc();
	virtual BOOL Init(BYTE byLineNo);
	void InitMqttandJSon();
	BOOL InitServerCfg();
	BOOL InitRtuBase();
	BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
	BOOL ProcessProtocolBuf(BYTE * buf, int len);
	MAPITEM * GetYcTable();
	virtual BOOL WriteAIVal(WORD wSerialNo, WORD wPnt, float wVal);
	virtual BOOL WriteDIVal(WORD wSerialNo, WORD wPnt, WORD wVal);
	virtual BOOL WritePIVal(WORD wSerialNo, WORD wPnt, QWORD dwVal);
	virtual BOOL WriteSOEInfo(WORD wSerialNo, WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond);

	float    m_wYCBuf[MAX_AI_LEN];
	QWORD   m_dwYMBuf[MAX_PI_LEN];
	BYTE	m_byYXbuf[MAX_DI_LEN];
	char m_szUserName[50];
	char m_szPasswd[50];
	char m_szCafile[50];
	char m_szCertfile[50];
	char m_szKeyfile[50];
	char m_szServerIP[50];
	char m_szPort[50];
	char m_szPKey[50];
	CMQTT_mosquitto m_mqtt;
	CDataProto_JSon_Xin_ao m_json;
};

