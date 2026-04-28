#pragma once
#include "DataProto.h"
#include <json/json.h>
#include <iostream>
#include "MQTT_mosquitto.h"
class CProtocol_Xin_ao_Slave_MQTT_JSON_Module;

using namespace std;
#include <map>
class CDataProto_JSon_Xin_ao :
	public CDataProto
{
public:
	CDataProto_JSon_Xin_ao();
	~CDataProto_JSon_Xin_ao();

	virtual bool getSendBuf(char * pDataSend, int &len);
	virtual bool processRecvBuf(char * ptopic, char * pPayload);
	bool processStatus(Json::Value &rootVal, Json::Value::Members &json_keys);
	float GetMemoryPercent();
	float GetCpuPercent();
	bool processDevinfo(Json::Value &rootVal, Json::Value::Members &json_keys);
	string getMacAddr(string strNetcardName);
	DWORD getDiskSize();
	DWORD getMemorySize();
	virtual bool subscribeMsg();

	bool processTiming(Json::Value &rootVal, Json::Value::Members &json_keys);
	void replayTiming(Json::Value &rootVal, Json::Value::Members &json_keys);
	bool isReply(Json::Value::Members &json_keys, map<string, unsigned char >&timeMap);
	void setMqtt(CMQTT_mosquitto * mosquitto);
	void setSnAndPkey(char * pSn, char * pKey);
	void setProtoDataObj(CProtocol_Xin_ao_Slave_MQTT_JSON_Module * pMJ);
	void TimerProc();
	void sendYcRealData();

	void AddDeviceToJSon(Json::Value &root, Json::Value &joneDevice, Json::Value &jCollect, WORD wStn);
	void AddJsonHeadData(Json::Value &root);
	void readMapDataTable();
	void readTopic();
	string GetMapDataName(WORD wkey);

	char m_szSn[100];
	char m_szPkey[100];
	CMQTT_mosquitto * m_pMQTT_mosquitto;
	CProtocol_Xin_ao_Slave_MQTT_JSON_Module * m_pMQTT_JsonModule = NULL;
	char m_topic[100][200];
	            //��ʱ      ����        �豸��Ϣ    ʵʱ����
	enum { etiming  , estatus , edevinfo , ertg, edevinfo_ack , estatus_ack, etiming_ack};
	time_t m_begin_counter , m_end_counter;
	map<WORD, string >m_dataNameMap;
	WORD m_stnCounter;
};

