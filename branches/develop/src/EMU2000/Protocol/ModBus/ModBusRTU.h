// ModBusRTU.h: interface for the CModBusRTU class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_)
#define AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_
#define MSGERROR (1)
#define MSGTRUE (0)


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <sys/time.h>
#include "CProtocol_ModBus.h"
#include "../../share/rdbFun.h"
#include "../../share/gDataType.h"

// #include "../../share/semObj.h"
#include <vector>
using namespace std;

#pragma pack(1)

typedef struct
{/*{{{*/
	BYTE type;
	BYTE func;
	WORD registe;
	WORD registe_num;
	BYTE skew_byte;
	WORD get_num;
	WORD start_num;
	BYTE data_len;
	UINT mask_code;
	BYTE data_form;
	BYTE sign;
	BYTE yk_form;
	BYTE cir_flag;
	WORD YkClose;
	WORD YkOpen;
	BYTE SetTimeFlag;
	BYTE SoeFlag;
	BYTE YkSelFlag;
	BYTE YkExctFlag;
	/*lel*/
	BYTE YxProcessMethod;
	BYTE YxByBitValue;
	/*end*/

	//为快速报文添加如下:
	BYTE typesec;
	BYTE registenumsec;
	WORD getnumsec;
	WORD startpossec;
	BYTE typethi;
	BYTE registenumthi;
	WORD getnumthi;
	WORD startposthi;
}MODBUSCONF;/*}}}*/

typedef struct
{/*{{{*/
	BYTE pos;
	WORD start_num;
	WORD get_num;
	BYTE type;
}INFO;/*}}}*/

class CModBusRTU  : public CProtocol_ModBus
{/*{{{*/
	public:
		CModBusRTU();
		virtual ~CModBusRTU();

		float store_buf[2][128];
		int pos_flag;
		int line;
		int yk_flag;
		int readval_flag;
		int writeval_flag;
		int val_flag;
		int pos;
		int settime_pos;
		int yk_pos_num;
		int readval_pos_num;
		int writeval_pos_num;
		long last_settime;
		int m_wErrorTimer;
		int m_byPortStatus;
		int timeflag;
		BYTE bySrcBusNo;
		WORD wSrcDevAddr;
		int YkNo;
		BYTE YkVal;
		BYTE YkBuf[256];
		int YkLen;
		BYTE MsgErrorFlag;
		BYTE MsgRegisteAndData[4];

		BYTE ESL411SOEFlag;
		BOOL DevCirFlag;
		BOOL m_bUnvanish;


		CSemObj  m_hSem;

		vector < INFO > yk_info;
		vector < INFO > readval_info;
		vector < INFO > writeval_info;
		vector < MODBUSCONF > modbus_conf;

		BYTE m_quest_id[10];//为保存xml协议中questid

		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		void ProcessUnvarnish(BYTE * buf, int &len, PBUSMSG pBusMsg);
		BOOL GetDzWriteProcess(BYTE * buf, int &len, PBUSMSG pBusMsg);
		void ProcessPatchDz(BYTE * buf, int &len, PBUSMSG pBusMsg);
		void ProcessDzRead(BYTE * buf, int &len, PBUSMSG pBusMsg);
		void ProcessDzWrite(BYTE * buf, int &len, PBUSMSG pBusMsg);
		void ProcessSingleDz(BYTE * buf, int &len, PBUSMSG pBusMsg);
		void GetDzWriteBuffer(BYTE byRowNum, BYTE * buf, int &len);
		virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);
		void ProcessUnvarnishRtn(BYTE * buf, int len);
		void ModBusWriteDz_10(unsigned char *buffer, MODBUSCONF modbusconf);
		virtual BOOL Init(BYTE byLineNo);
		BOOL GetYKBuffer( BYTE * buf , int &len , PBUSMSG pBusMsg );
		virtual void TimerProc();

		void DefaultValConfig(MODBUSCONF * mc);
		//字符串转十六进制
		unsigned int Atoh(char *buf);

		//读取配置文件
		int ReadConf(char *filename);
		//时间组包 毫秒低位在后，高位在前
		int TimePackMsecBigEdian(MODBUSCONF modbusconf,unsigned char *buffer,int i,struct tm *p,WORD msec);
		//时间组包 毫秒低位在前，高位在后
		int TimePackMsecLittleEdian(MODBUSCONF modbusconf,unsigned char *buffer,int i,struct tm *p,WORD msec);

		int TimePackIEC(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec);
		//时间组包
		int SysLocalTime(MODBUSCONF modbusconf,unsigned char *buffer,int i);

		//遥信，遥测，对时等发送报文
		void SendBuf( MODBUSCONF modbusconf , BYTE * buf ,int *len);

		//YZ202遥控预置
		void YkPresetSendBuf( MODBUSCONF modbusconf , YK_DATA *yk_data , BYTE * buf ,int *len );

		//发送遥控报文
		void YkJ05SendBuf( MODBUSCONF modbusconf , YK_DATA *yk_data , BYTE * buf ,int *len);

		//发送遥控报文
		void YkSendBuf( MODBUSCONF modbusconf , YK_DATA *yk_data , BYTE * buf ,int *len);

	
		//esl 411 SOE组包
		void Esl411SoeSendBuf(BYTE * buf ,int *len);

		short TwoByteValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		int FourByteValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		unsigned  short TwoByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		unsigned int FourByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		unsigned long long EightByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		int GetBCD(BYTE byData);

		float FloatValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		double DoubleValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		//求值  数据格式、有符号
		double ModBusValue(unsigned char *buffer, int a, MODBUSCONF modbusconf);

		//遥信处理
		void ModBusYxDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//要信一次处理的值
		UINT ModBusYXTempValue(unsigned char *buffer, int pos, MODBUSCONF modbusconf);

		//遥信安位处理
		void ModBusYxBitDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//遥信安字处理
		void ModBusYxByteDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		////大于1的遥信值处理
		void ModBusYxByteDeal_new(unsigned char *buffer, MODBUSCONF modbusconf);

		void ModBusRsl_411YxYcDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//遥测处理
		void ModBusYcDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//J05电流屏遥控处理
		void ModBusJ05YkDeal(unsigned char *buffer,MODBUSCONF modbusconf);
		//标准遥控处理
		void ModBusRtuYkDeal(unsigned char *buffer,MODBUSCONF modbusconf);
		//遥控处理
		void ModBusYkDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//遥脉处理
		void ModBusYmDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//对时处理
		void ModBusSetTime(unsigned char *buffer,MODBUSCONF modbusconf);

		//EsdTek SOE处理
		void ModBusRtuSoeDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//YZ202 SOE处理
		void ModBusYZ202SoeDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//Esl 411 SOE处理
		void ModBusEsl_411SoeDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//SOE处理
		void ModBusSoeDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//发送读定值 
		void ReadvalSendBuf(MODBUSCONF modbusconf, BYTE * buf, int *len);

		//发送写定值
		void WritevalSendBuf(MODBUSCONF modbusconf, unsigned int * val, BYTE * buf, int *len);

		//读定值
		void ModBusReadVal(unsigned char *buffer,MODBUSCONF modbusconf);

		//写定值
		void ModBusWriteVal(unsigned char *buffer,MODBUSCONF modbusconf);

		//recv报文
		void RecvBuf(MODBUSCONF modbusconf);

		//处理遥控消息
		BOOL YK_MsgPorcess( YK_DATA * pYkData ) ;

		//获得装置通讯状态 by	zhanghg 2014-9-23
		virtual BOOL GetDevCommState( ) ;

};/*}}}*/
#pragma pack( )

#endif // !defined(AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_)
