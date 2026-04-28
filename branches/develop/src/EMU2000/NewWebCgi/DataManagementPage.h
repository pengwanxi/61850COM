#pragma once
#include "BaseWebPage.h"


#define TRANS_PROTOCOL  0x01
#define GATHER_PROTOCOL 0x02


class CDataManagerPage :
	public CBaseWebPage
{
public:
	CDataManagerPage();
	~CDataManagerPage();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *pLog);

public:

	virtual int ProtocolStyle(string str);
	virtual BOOL GetDataPar(Json::Value &pardata);
	virtual void GetYMDataProperty(char *str);
	virtual void GetYCDataProperty(char *str);
	virtual void GetDevInfo(char pathname[], Json::Value &devinfo);//获取数据属性
	

public:
	string protocol_name;
	string protocol_para;
	string protocol_netcard;


public:
	BYTE yc_order;
	BYTE ym_order; //序号

	BYTE DevSerialNumber;//装置序号
	BYTE CommunicationBankNumber;//通讯行号
	string StationAddress;//站址
	BYTE StationSerialNumber;//站内序号
	string data_label;//标签
	string basic_value;//基值
	string ratio_value;//系数
	string threshold_value;//门槛值
	










};

