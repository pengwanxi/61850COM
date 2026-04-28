#pragma once
#include "BaseWebPage.h"


class CTransDataPage :
	public CBaseWebPage
{
public:
	CTransDataPage();
	~CTransDataPage();
	virtual BOOL getJSONStructFromWebPage(Json::Value &root);
	virtual BOOL procCmd(BYTE byCmd);
	virtual void  Init();
	virtual void setLog(Clog *pLog);

public:
	virtual BOOL TransDataYc(Json::Value &ycdata);
	virtual BOOL TransDataYx(Json::Value &yxdata);
	virtual BOOL TransDataYm(Json::Value &ymdata);
	virtual BOOL TransDataDz(Json::Value &dzdata);
	virtual BOOL TransDataYk(Json::Value &ykdata);
	virtual int ProtocolStyle(string str);
	virtual BOOL GetComPar(Json::Value &compardata);
	virtual void ReadConfig(LPCSTR lpszFile);
	virtual int ParseConfigItem(char *strItem, WORD *pwNum);
	virtual void SetStnAttrib(WORD wStn, char* szParam);
	virtual void PntInit(WORD wStn, LPCSTR lpszFile);
	virtual BOOL GetTransNum(string pathname,int transchannel);//通过文件获取转发序号
	
public:
	virtual BOOL JudgeCharacter(char str[],int *num,int *num2);
	virtual void Getnumopt(char *str, char *delim, int *num,int *num2);

public:
	string protocol_name;
	string protocol_para;
	string protocol_netcard;
	int g_nExtendSize;
	int m_wStnSum;
	STNDEF g_StnDef[MAX_STN_SUM];
	STNBUSADDR g_StnBusAddr[MAX_STN_SUM];	
	int trans_channel;
	
	vector<string>yc_label;
	vector<string>yx_label;
	vector<string>ym_label;
	vector<string>yk_label;
	vector<string>dz_label;

	map<int, string> yc_trans[MAX_STN_SUM];
	map<int, string> yx_trans[MAX_STN_SUM];
	map<int, string> ym_trans[MAX_STN_SUM];
	map<int, string> yk_trans[MAX_STN_SUM];
	map<int, string> dz_trans[MAX_STN_SUM];

};

