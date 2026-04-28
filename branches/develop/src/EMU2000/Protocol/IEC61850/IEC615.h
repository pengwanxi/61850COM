// ModBusRTU.h: interface for the IEC615 class.
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
#include "CProtocol_IEC615.h"
#include "../../share/rdbFun.h"
#include "../../share/gDataType.h"

#include ".install/include/iec61850_client.h"
#include ".install/include/hal_thread.h"

#include ".install/include/mms_value.h"
#include ".install/include/mms_value_internal.h"
// #include "../../share/semObj.h"
#include <vector>
#include <iostream>
#include <string>
#include <stdio.h>
#include <fstream>
#include <map>
#include <pthread.h>
using namespace std;

#pragma pack(1)

class config
{/*{{{*/
	public:
		config(BYTE _1, BYTE _2, WORD _3, WORD _4, unsigned long long _5, unsigned long long _6, string _7, string _8):type(_1), count(_2), start_pos(_3), group(_4), mask(_5), dataset_mask(_6), field_mask(_7), report_path(_8){}
		config(){}
		BYTE type;					//0+1 for yx, 1+1 for yc, 2+1 for ym, 3+1 for yk, 4+1 for dz, 5+1 for soe!
		BYTE count;					//采集数量!
		WORD start_pos;				//起始序号!
		WORD group;					//定值组号!	只对定值有效!
		unsigned long long mask;	//遥测遥脉掩码!					0-0x7fffffffffffffff	暂不用!
		unsigned long long dataset_mask;	//数据集掩码!	0-0x7fffffffffffffff
		string field_mask;				//字段掩码!使用普通类型数据一位表示一个掩码，但是位数最大64位，这里使用字串的方式扩展!
		string fc_eg;						//功能约束! 读取遥脉或定值时使用
		string report_path;				//数据集或报告姓名/路径! 对于定值，路径具体到最底层!

		string sel;					//预置
		string oper;				//执行
		string cancle;				//取消!
};/*}}}*/

class IEC615  : public CProtocol_IEC615
{/*{{{*/
	public:
		IEC615();
		virtual ~IEC615();
		virtual bool mask_resolv(WORD, BYTE);
		struct tm *GetTime();
		virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
		virtual BOOL register_report();
		virtual BOOL iec615_inner_connection();
		virtual BOOL Init( BYTE byLineNo ) ;
		virtual void TimerProc();
		void writeLog(char * pContent, int len);
		virtual FunctionalConstraint get_fc(string);
		virtual BOOL yk_preset(YK_DATA *, PBUSMSG, BYTE);
		virtual BOOL yk_execute(YK_DATA *, PBUSMSG, BYTE);
		virtual BOOL yk_cancel(YK_DATA *, PBUSMSG, BYTE);
		virtual BOOL yk_deal(YK_DATA *, PBUSMSG);
		virtual BOOL dz_read(DZ_DATA *, PBUSMSG, WORD);
		virtual BOOL dz_preset(DZ_DATA *, PBUSMSG, WORD);
		virtual BOOL insert_dz(DZ_DATA *, WORD, MmsValue *);
		virtual BOOL dz_write(DZ_DATA *, PBUSMSG, WORD);
		virtual BOOL dz_deal(DZ_DATA *, PBUSMSG);
		virtual int read_conf(const char *filename);
		virtual BOOL GetDevCommState( ) ;
		virtual void default_config();
		friend void reportCallbackFunction(void *, ClientReport);
		friend void report_resolv(MmsValue *, WORD &, WORD &, BYTE, IEC615 *);
		void ReadYm();
private:
		IedConnection m_pConnect = nullptr;
		int tcp_port;
		ClientReportControlBlock m_pRcb = nullptr;
		char *report_name = nullptr;					//yc!
		char *report_name_yx = nullptr;			//yx!
		vector<config> vec_conf;			//yc-yx-ym!1
		vector<config> vec_conf_ym;			//遥脉容器!因为遥测遥信可以订阅报告，遥脉不可以，故分开处理!
		vector<config> vec_yk;				//遥控配置!
		BYTE count_flag;					//表示总的采集数目，现在做测试使用!
		bool yk_sel_flag;						//遥控预置标识!
		ControlObjectClient m_pControl = { nullptr };
		MmsValue *m_pCtlVal;
		unsigned int connection_flag;

		vector<ClientReportControlBlock > m_rcbVec;
		map<string, BYTE> report_line;		//报告名到配置行的映射!
		multimap<BYTE, BYTE> group_to_line;	//组号到配置行的映射，用于定值!
		time_t time_ym;
		time_t time_yk_sel;					//遥控预置超时恢复!是和yk_sel_flag相关联的参数!
		pthread_mutex_t mutex;

		DZ_DATA dz_write_array[64];
		BYTE dz_write_num;
		FILE * m_logFile;
		int m_countComm ;
		bool m_bstateOldComm ;

		bool m_bConnectIed ;
		bool m_bReadYm ;
		
};/*}}}*/
#pragma pack( )

#endif // !defined(AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_)
