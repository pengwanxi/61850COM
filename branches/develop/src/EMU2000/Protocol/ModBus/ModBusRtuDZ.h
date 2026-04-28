#ifndef MODBUSRTUDZ_H_
#define MODBUSRTUDZ_H_

#define MSGERROR (1)
#define MSGTRUE (0)

#if _MSC_VER > 1000
#pragma once
#endif 

#include <sys/time.h>
#include <vector>
#include "CProtocol_ModBus.h"
#include "ModBusRTU.h"
#include "../../share/rdbFun.h"
#include "../../share/gDataType.h"

using namespace std;

#pragma pack(1)

class CModBusRtuDz : public CModBusRTU
{/*{{{*/
public:
	CModBusRtuDz();

	virtual ~CModBusRtuDz();

	virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);

	virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);

	virtual BOOL ResolveQuicklyBuf(BYTE * buf, int len);

	virtual BOOL ResolveRtuBuf(BYTE * buf, int len);

	virtual BOOL GetDzWriteProcess(BYTE * buf, int &len, PBUSMSG pBusMsg);

	virtual void ProcessPatchDz(BYTE * buf, int &len, PBUSMSG pBusMsg);

	virtual void ProcessDzRead(BYTE * buf, int &len, PBUSMSG pBusMsg);

	virtual void ProcessDzWrite(BYTE * buf, int &len, PBUSMSG pBusMsg);

	virtual void ProcessSingleDz(BYTE * buf, int &len, PBUSMSG pBusMsg);

	virtual void GetDzWriteBuffer(BYTE byRowNum, BYTE * buf, int &len);
	
	virtual void ModBusWriteDz_10(unsigned char *buffer, MODBUSCONF modbusconf);

	virtual BOOL Init(BYTE byLineNo);

	virtual BOOL GetYKBuffer(BYTE * buf, int &len, PBUSMSG pBusMsg);

	virtual void TimerProc();

	virtual void DefaultValConfig(MODBUSCONF * mc);

	//字符串转十六进制
	virtual unsigned int Atoh(char *buf);

	//读取配置文件
	virtual int ReadConf(char *filename);

	//时间组包 毫秒低位在后，高位在前
	virtual int TimePackMsecBigEdian(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec);

	//时间组包 毫秒低位在前，高位在后
	virtual int TimePackMsecLittleEdian(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec);

	virtual int TimePackIEC(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec);

	//时间组包
	virtual int SysLocalTime(MODBUSCONF modbusconf, unsigned char *buffer, int i);

	//遥信，遥测，对时等发送报文
	virtual void SendBuf(MODBUSCONF modbusconf, BYTE * buf, int *len);

	//YZ202遥控预置
	virtual void YkPresetSendBuf(MODBUSCONF modbusconf, YK_DATA *yk_data, BYTE * buf, int *len);

	//发送遥控报文
	virtual void YkJ05SendBuf(MODBUSCONF modbusconf, YK_DATA *yk_data, BYTE * buf, int *len);

	//发送遥控报文
	virtual void YkSendBuf(MODBUSCONF modbusconf, YK_DATA *yk_data, BYTE * buf, int *len);

	//esl 411 SOE组包
	virtual void Esl411SoeSendBuf(BYTE * buf, int *len);

	virtual short TwoByteValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	virtual int FourByteValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	virtual unsigned short TwoByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	virtual unsigned int FourByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	virtual unsigned long long EightByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	virtual double DoubleValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	virtual int GetBCD(BYTE byData);

	virtual float FloatValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	//求值  数据格式、有符号
	virtual double ModBusValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

	//遥信处理
	virtual void ModBusYxDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//要信一次处理的值
	virtual UINT ModBusYXTempValue(unsigned char *buffer, int pos, MODBUSCONF modbusconf);

	//遥信安位处理
	virtual void ModBusYxBitDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//遥信安字处理
	virtual void ModBusYxByteDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	////大于1的遥信值处理
	virtual void ModBusYxByteDeal_new(unsigned char *buffer, MODBUSCONF modbusconf);

	virtual void ModBusRsl_411YxYcDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//遥测处理
	virtual void ModBusYcDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//J05电流屏遥控处理
	virtual void ModBusJ05YkDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//标准遥控处理
	virtual void ModBusRtuYkDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//遥控处理
	virtual void ModBusYkDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//遥脉处理
	virtual void ModBusYmDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//对时处理
	virtual void ModBusSetTime(unsigned char *buffer, MODBUSCONF modbusconf);

	//EsdTek SOE处理
	virtual void ModBusRtuSoeDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//YZ202 SOE处理
	virtual void ModBusYZ202SoeDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//Esl 411 SOE处理
	virtual void ModBusEsl_411SoeDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//SOE处理
	virtual void ModBusSoeDeal(unsigned char *buffer, MODBUSCONF modbusconf);

	//发送读定值 
	virtual void ReadvalSendBuf(MODBUSCONF modbusconf, BYTE * buf, int *len);

	//发送写定值
	virtual void WritevalSendBuf(MODBUSCONF modbusconf, unsigned int * val, BYTE * buf, int *len);

	//读定值
	virtual void ModBusReadVal(unsigned char *buffer, MODBUSCONF modbusconf);

	//写定值
	virtual void ModBusWriteVal(unsigned char *buffer, MODBUSCONF modbusconf);

	//获得装置通讯状态 by	zhanghg 2014-9-23
	 virtual BOOL GetDevCommState();

public:
	virtual BOOL SendQuicklyBuf(MODBUSCONF modbusconf,BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);//发送快速报文
	 
	virtual BOOL SendRtuBuf(MODBUSCONF modbusconf,BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);//发送普通的Rtu 报文

	virtual BOOL WhetherQuicklyBufValue(BYTE *buf, int &len);//判断接收的快速报文正确与否

public:
	BYTE DzStartOrder;// 新添加的TCI 下发的定值的起始序号
	BYTE DzMount;//新添加的TCI下发的定值的数量

	void ProcessUnvarnish(BYTE * buf, int &len, PBUSMSG pBusMsg);
	void ProcessUnvarnishRtn(BYTE * buf, int len);

};/*}}}*/
#pragma pack( )
#endif 


