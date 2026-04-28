#include "MQTT_mosquitto.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>

using namespace std ;

CMQTT_mosquitto::CMQTT_mosquitto()
{
	m_mosq = NULL;
	m_bConn = false;
	m_dataProto = NULL;
}


CMQTT_mosquitto::~CMQTT_mosquitto()
{
	mosquitto_destroy(m_mosq);
	mosquitto_lib_cleanup();
}

void CMQTT_mosquitto::SetLoginInfo(char * pUser, char * pPasswd)
{
	if (!pUser || !pPasswd)
		return;

	strcpy(m_szUserName, pUser);
	strcpy(m_szPasswd, pPasswd);
}

void CMQTT_mosquitto::SetCafiles(char * pCafile, char * pCertFile, char * pKey)
{
	if (!pCafile || !pCertFile || !pKey)
		return;

	strcpy(m_szCafile, pCafile);
	strcpy(m_szCertfile, pCertFile);
	strcpy(m_szKeyfile, pKey);
}

void CMQTT_mosquitto::SetServerInfo(char * pIP, char * pPort)
{
	if (!pIP || !pPort)
		return;

	strcpy(m_szServerIP, pIP);
	strcpy(m_szPort, pPort);
}

bool CMQTT_mosquitto::SetMqttCallbackFunc()
{
	if (!m_mosq)
		return false;

	mosquitto_connect_callback_set(m_mosq, CMQTT_mosquitto::connectCallback);
	mosquitto_disconnect_callback_set(m_mosq, CMQTT_mosquitto::disconnectCallback);
	mosquitto_publish_callback_set(m_mosq, CMQTT_mosquitto::publishCallback);
	mosquitto_subscribe_callback_set(m_mosq, CMQTT_mosquitto::subscribeCallback);
	mosquitto_message_callback_set(m_mosq, CMQTT_mosquitto::message_callback);

	return true;
}

void CMQTT_mosquitto::message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	printf("mosquitto_message_callback_set\n");

	if (message->payloadlen) {
		printf("%s %s\n", message->topic, (char *)message->payload);
	}
	else {
		printf("%s (null)\n", message->topic);
	}
	fflush(stdout);

	CMQTT_mosquitto * pCls = (CMQTT_mosquitto *)obj;

	if (pCls->m_dataProto)
		pCls->m_dataProto->processRecvBuf(message->topic, ( char * )message->payload);

}

void CMQTT_mosquitto::subscribeCallback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
	printf("subscribeCallback\n");
	int i;
	printf("Subscribed (mid: %d): %d , qos_count=%d\n", mid, granted_qos[0], qos_count);
	for (i = 1; i < qos_count; i++) {
		printf(", %d", granted_qos[i]);
	}
}

void CMQTT_mosquitto::connectCallback(struct mosquitto * mosq, void * obj, int result)
{
	if (!obj)
	{
		printf("CMQTT_mosquitto::connectCallback-----user data is null\n");
		return;
	}

	CMQTT_mosquitto * pCls = (CMQTT_mosquitto *)obj;
	printf("m_bconn = %d %d %s , %s , %s\n", pCls->m_bConn , __LINE__ , __FUNCTION__ , pCls->m_szServerIP , pCls->m_szPort );
	if (!result)
	{
		pCls->m_bConn = true;
		printf("connectCallback\n");
		printf("m_bconn = %d %d %s\n", pCls->m_bConn, __LINE__, __FUNCTION__);
	}
	else if (result)
	{
		fprintf(stderr, "%s\n", mosquitto_connack_string(result));
		printf("CMQTT_mosquitto::connectCallback error \n");
	}
}

void CMQTT_mosquitto::publishCallback(struct mosquitto * mosq, void * obj, int result)
{
	printf("publishCallback\n");
}

void CMQTT_mosquitto::disconnectCallback(struct mosquitto * mosq, void * obj, int result)
{
	printf("CMQTT_mosquitto::disconnectCallback\n");
}

bool CMQTT_mosquitto::Initmqtt()
{
	mosquitto_lib_init();
	m_mosq = mosquitto_new(NULL, true, this);
	if (!m_mosq)
	{
		printf("m_mosq==NULL init mqtt lib failed \n");
		return false;
	}

	return true;
}

void CMQTT_mosquitto::SetMqttProperty()
{
	if (mosquitto_will_set(m_mosq, "/data", 16, "C client stoped", 0, 0))
	{
		printf("mosquitto_will_set\n");
	}

	if (mosquitto_username_pw_set(m_mosq, m_szUserName, m_szPasswd) == MOSQ_ERR_SUCCESS) {
		printf("username_pw_set success\n");
	}

	char path[256] = { "/mynand/config/xinaoSlaveMQTTJSON/" };
	string strCafile = string(path) + string(m_szCafile);
	string strCertfile = string(path) + string(m_szCertfile);
	string strkeyfile = string(path) + string(m_szKeyfile);

	if (mosquitto_tls_opts_set(m_mosq, 1, NULL, NULL) ||
		mosquitto_tls_set(m_mosq, strCafile.c_str(), NULL, strCertfile.c_str(), strkeyfile.c_str(), NULL) ||
		mosquitto_tls_insecure_set(m_mosq, true))
	{
		printf("mosquitto_tls_opts_set error\n");
	}

	int  value = MQTT_PROTOCOL_V31;
	if (mosquitto_max_inflight_messages_set(m_mosq, 20) ||
		mosquitto_opts_set(m_mosq, MOSQ_OPT_PROTOCOL_VERSION, &value))
	{
		printf("mosquitto_opts_set\n");
	}
}

bool CMQTT_mosquitto::Connect()
{
	int ret = mosquitto_connect(m_mosq, m_szServerIP, atoi( m_szPort ) , 100000);
	if (ret)
	{
		return false;
	}
	else {
		printf("mosquitto_connect success\n");
	}

	printf("%s %s %d : %s \n", __FILE__, __FUNCTION__, __LINE__ , "connecting server");
	while (m_bConn == false)
	{
		int value = mosquitto_loop(m_mosq, -1, 1);
		if (value)
		{
			//printf("%s %s %d : %s , errorno:%d\n", __FILE__ , __FUNCTION__ , __LINE__, mosquitto_strerror(value), value);
			usleep(1000 * 30 );
		}
		else
		{
			//printf("%s %s %d : %s , errorno:%d\n", __FILE__, __FUNCTION__, __LINE__, mosquitto_strerror(value), value);
			usleep(1000 * 30 );
		}
	}
	printf("%s %s %d : %s \n", __FILE__, __FUNCTION__, __LINE__, "connected server");

	if (m_bConn)
	{
		m_dataProto->subscribeMsg();
		return true;
	}

	return false;
}

bool CMQTT_mosquitto::Init()
{
	if ( !Initmqtt())
		return false;

	SetMqttCallbackFunc();
	SetMqttProperty();

	pthread_t  tid = 0;
	pthread_create(&tid, NULL, CMQTT_mosquitto::threadProc, this);

	return false;
}

void CMQTT_mosquitto::setDataProto(CDataProto * pProto)
{
	if (m_dataProto)
	{
		delete m_dataProto;
		m_dataProto = NULL;
	}

	m_dataProto = pProto;
}

void * CMQTT_mosquitto::threadProc(void * pData)
{
	if (!pData)
		return 0;
	CMQTT_mosquitto * pMqttClass = (CMQTT_mosquitto *)pData;
	
	while (!pMqttClass->Connect())
		usleep(1000* 1000 * 2 );

	mosquitto_loop_forever(pMqttClass->m_mosq, 100, 1);
	return 0;
}

bool CMQTT_mosquitto::subscribeMsg(char * pTopic)
{
	if (!pTopic)
		return false;

	if (!m_bConn)
	{
		printf("mosquitto doesn't connect server!\n");
		return false;
	}

	printf("CMQTT_mosquitto::subscribeMsg\n");
	int qos = 1;
	int ret = 0; 
	int id = 0;
	ret = mosquitto_subscribe(m_mosq, &id, pTopic, qos);
	if (ret == MOSQ_ERR_SUCCESS)
	{
		printf("mosquitto_subscribe invoke success\n");
	}
	else
		printf("mosquitto_subscribe invoke failed\n");
}

void CMQTT_mosquitto::publishMsg(const char * pTopic, const char *pPayload, int len)
{
	if (!pTopic || !pPayload || !len)
		return;

	if (!m_bConn)
	{
		printf("mosquitto doesn't connect server!\n");
		return;
	}

	int id = 0;
	mosquitto_publish(m_mosq, &id, pTopic, len, pPayload, 1, 0);
}