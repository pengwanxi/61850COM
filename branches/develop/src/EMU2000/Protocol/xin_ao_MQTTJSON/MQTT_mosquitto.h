#pragma once
#include <mosquitto.h>
#include <pthread.h>
#include "DataProto.h"

class CMQTT_mosquitto
{
public:
	CMQTT_mosquitto();
	~CMQTT_mosquitto();
	void SetLoginInfo(char * pUser, char * pPasswd);
	void SetCafiles(char * pCafile, char * pCertFile, char * pKey);
	void SetServerInfo(char * pIP, char * pPort);
	bool Initmqtt();
	void SetMqttProperty();
	bool Connect();
	bool SetMqttCallbackFunc();
	bool Init();
	void setDataProto( CDataProto * pProto);
	static void * threadProc(void *);
	bool subscribeMsg(char * pTopic);
	void publishMsg(const char * pTopic, const char *pPayload, int len);
	static void  message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
	static void subscribeCallback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
	static void connectCallback(struct mosquitto * mosq, void *   obj, int result);
	static void publishCallback(struct mosquitto * mosq, void *   obj, int result);
	static void disconnectCallback(struct mosquitto * mosq, void *   obj, int result);

	struct mosquitto * m_mosq;
	char m_szUserName[50];
	char m_szPasswd[50];
	char m_szCafile[50];
	char m_szCertfile[50];
	char m_szKeyfile[50];
	char m_szServerIP[50];
	char m_szPort[50];
	bool m_bConn;

	CDataProto * m_dataProto;
};

