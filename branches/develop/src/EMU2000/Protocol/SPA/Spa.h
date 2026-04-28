/// \文件:	SPA.h
/// \概要:	ABB Spa协议头文件
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2017-09-25

#ifndef _SPA_H
#define _SPA_H

#include <vector>
#include "../../share/typedef.h"
#include "../../share/CProtocol.h"
#include "../../share/gDataType.h"
#include "CProtocol_Spa.h"
#include <map>

using namespace std;

#define ABBSPAPREFIXFILENAME	"/mynand/config/SPA/template/"

#define	SPA_MAX_YC_NUM			256				/* 遥测最大数量 */
#define	SPA_MAX_YX_NUM			512				/* 遥信最大数量 */
#define	SPA_MAX_YK_NUM			256				/* 遥控最大数量 */
#define	SPA_MAX_YM_NUM			256				/* 遥脉最大数量 */
#define	SPA_TOTAL_CALL			10 * 60 * 1000	/* 总召时间间隔 */
#define	SPA_TIME_SYN			10 * 60 * 1000	/* 对时时间间隔 */
#define	SPA_LINK_TIMEOUT		15 * 1000		/* 通讯超时时间 */
#define	SPA_YK_TIMEOUT			5 * 1000		/* 遥控超时时间 */
#define	SPA_MAX_ERROR_COUNT		3				/* 错误报文次数 */
#define	SPA_MAX_YKERROT_COUNT	3				/* 遥控错误次数 */
#define	SPA_MAX_BUF_LEN			256				/* 最大缓存长度 */
#define	SPA_MAX_RESEND_COUNT	3				/* 最大重发次数 */
#define	SPA_MAX_DEBUG_BUF_LEN	1024			/* 最大打印长度 */

#define QUERY_NULL				0x00
#define QUERY_YX				0x01			/*遥信*/
#define QUERY_YC				0x02			/*遥测*/
#define QUERY_YK				0x03			/*自动遥控*/
#define QUERY_YM				0x04			/*遥脉*/
#define QUERY_DZ_W				0x05			/*写定值*/
#define QUERY_DZ_R				0x06			/*读定值*/
#define QUERY_BHRESET			0x07			/*保护复归*/
#define QUERY_CLOCK				0x08			/*对时*/
#define QUERY_SOE				0x09			/*SOE*/
#define QUERY_PWD_OPEN			0x0a			/*打开口令*/
#define QUERY_PWD_CLOSE			0x0b			/*关闭口令*/
#define QUERY_REG_SAVE			0x0c			/*保存寄存器*/
#define QUERY_CLEAR				0x0d			/*清除状态*/
#define QUERY_YX1				0x0e			/*遥信1*/
#define QUERY_ENABLE_W			0x0f			/*使能*/
#define QUERY_DISABLE_W			0x10			/*禁止*/
#define QUERY_STATUS_R			0x11			/*读子站状态*/

#define SPALOOPOFF				0				/*循环周期 为0*/
#define SPALOOPON				1				/*循环周期 为1*/
#define SPATIMEOUT				60				/*对时周期*/

#define SPA_DATACLASS_YX		1				/*遥信数据类型*/
#define SPA_DATACLASS_YC		2				/*遥测数据类型*/
#define SPA_DATACLASS_YK		3				/*遥控数据类型*/
#define SPA_DATACLASS_YM		4				/*遥脉数据类型*/
#define SPA_DATACLASS_DZ		5				/*定值数据类型*/
#define SPA_DATACLASS_DZINT		6				/*整数型定值数据类型*/
#define SPA_DATACLASS_RESET		7				/*复归数据类型*/
#define SPA_DATACLASS_CLOCK		8				/*对时数据类型*/
#define SPA_DATACLASS_SOE		9				/*SOE数据类型*/

#define SPA_SOE_TIME_FIELD		0				/*SOE 时间域*/
#define SPA_SOE_EVENT_FIELD		1				/*SOE 事件域*/

#define SPA_YK_OPEN				1				/*遥控合闸*/
#define SPA_YK_CLOSE			0				/*遥控分闸*/

// --------------------------------------------------------
/// \概要:	CfgInfo 信息初始化
// --------------------------------------------------------
class CfgInfo
{
	public:
		CfgInfo(){
			bDataType   = 0;
			bDataNums   = 0;
			wStartNo    = 0;
			wCoverCode  = 0;
			memset(sZcode, 0, sizeof(sZcode));
			bEventCode1 = 0;
			bEventCode0 = 0;
			bLoop       = 0;
		}

		BYTE bDataType;   // 功能类型
		BYTE bDataNums;   // 采集个数1~127
		WORD wStartNo;    // 起始序号
		WORD wCoverCode;  // 屏蔽码
		char sZcode[12];  // 地址码  如：I1/112；
		BYTE bEventCode1; // 1事件码
		BYTE bEventCode0; // 0事件码
		map<WORD, WORD> mapEventCode0; //键:发生事件的编码,值:发生事件所对应的遥信点号
		map<WORD, WORD> mapEventCode1; //键:发生事件的编码,值:发生事件所对应的遥信点号
		BYTE bLoop;       // 发送循环周期
};

// --------------------------------------------------------
/// \概要:	ABB Spa 协议
// --------------------------------------------------------
class CSPA : public CProtocol_SPA
{
	public:
		CSPA();
		~CSPA();

		/*时间处理函数*/
		virtual void TimerProc();
		/*初始化协议数据*/
		virtual BOOL Init(BYTE byLineNo);
		/*获取协议数据缓存*/
		virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
		/*处理收到的数据缓存*/
		virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);
		/*更新通讯状态*/
		virtual BOOL GetDevCommState();
	protected:
		BOOL m_bLinkStatus;								//通讯状态
		BOOL m_bIsReSend;								//是否重复发
		BOOL m_bIsSending;								//是否正在发送，1正在发送
		BOOL m_bIsNeedResend;							//是否需要重发
		BOOL m_bIsYking;								//是否属于遥控状态
		BOOL m_bIsYmCall;								//是否是遥脉召唤

		DWORD m_dwLinkTimeOut;							//通讯超时
		DWORD m_dwYkTimeOut;							//遥控超时

		BYTE m_byRecvErrorCount;						//接收错误计数
		BYTE m_byYkErrorCount;							//遥控错误计数
		BYTE m_byReSendBuf[SPA_MAX_BUF_LEN];			//重发缓存
		BYTE m_byYkSendBuf[SPA_MAX_BUF_LEN];			//遥控重发
		BYTE m_byResendCount;							//重发计数
		WORD m_wReSendLen;								//重发缓存长度
		BYTE m_byYkSendLen;								//遥控缓存长度
		BYTE m_byRemoteBusNo;							//转发总线号
		BYTE m_byRemoteAddr;							//转发地址

		char DebugBuf[SPA_MAX_BUF_LEN];

		vector <CfgInfo> m_SPA_CfgInfo;					//配置信息
	//	SENDSTATUS m_SendStatus;						//发送状态
		int m_SendStatus;								//发送状态

		int iGLinePos;									//模板某一行
		int iGLineNum;									//模板总行数
		int iGLinePosLast;								//模板行数上一行
		int iGTimeFlag;

		/*打印协议信息*/
		void print(char *buf, int len = 0);
		/*获取配置信息*/
		BOOL GetModuleInfo(BYTE bDataType, WORD wPnt, CfgInfo &tCfgInfo);

		/*读取配置信息*/
		BOOL ReadCfgInfo();
		void addEventCodeToMap(CfgInfo &tCfgInfo, char * p, BYTE byCodeNo);
		/*读取默认配置信息*/
		BOOL DefaultCfgInfo();
		/*初始化协议状态*/
		BOOL InitProtocolStatus();

		/*总线消息*/
		BOOL ProcessBusMsg(PBUSMSG pBusMsg, BYTE *buf, int &len);
		/*固定帧*/
		/*获取协议缓存*/
		BOOL GetSendBuf(BYTE *buf, int &len);
		/*召唤遥信*/
		BOOL CallYxData(BYTE *buf, int &len);
		/*召唤遥测*/
		BOOL CallYcData(BYTE *buf, int &len);
		/*召唤遥脉*/
		BOOL CallYmData(BYTE *buf, int &len);
		/*读定值*/
		BOOL ReadDzData(BYTE *buf, int &len);
		/*设置使能*/
		BOOL WriteEnable(BYTE *buf, int &len);
		/*写定值*/
		BOOL WriteDzData(BYTE *buf, int &len);
		/*设置禁用*/
		BOOL WriteDisEnable(BYTE *buf, int &len);
		/*重设*/
		BOOL ResetData(BYTE *buf, int &len);
		/*读最新SOE事件*/
		BOOL ReadSoeData(BYTE *buf, int &len);
		/*读子站状态*/
		BOOL ReadStatus(BYTE *buf, int &len);
		/*清空状态*/
		BOOL ClearStatus(BYTE *buf, int &len);
		/*对时*/
		BOOL OnTime(BYTE *buf, int &len);


		/*遥控*/
		/*选择*/
		BOOL YkSel(PBUSMSG pBusMsg, YK_DATA *pYkData, BYTE *buf, int &len);
		/*执行*/
		BOOL YkExct(PBUSMSG pBusMsg, YK_DATA *pYkData, BYTE *buf, int &len);
		/*取消*/
		BOOL YkCancel(PBUSMSG pBusMsg, YK_DATA *pYkData, BYTE *buf, int &len);

		/*处理报文*/
		BOOL ProcessMessage(BYTE *buf, int len);
		/*处理遥信报文*/
		BOOL SPA_YX_Process(BYTE *buf, int len);
		/*处理遥测报文*/
		BOOL SPA_YC_Process(BYTE *buf, int len);
		/*处理遥脉报文*/
		BOOL SPA_YM_Process(BYTE *buf, int len);
		/*处理遥控报文*/
		BOOL SPA_YK_Process(BYTE *buf, int len);
		/*处理SOE报文*/
		BOOL SPA_SOE_Process(BYTE *buf, int len);
		BOOL IsHaveEventCode(map<WORD, WORD>& tEventCode, WORD bEventCode);
		/*得到完整的SOE时间*/
		BOOL Protocol_GetSoeTime(TIMEDATA *tSpaSoeTime, WORD wMilliSec, BYTE bSec, BYTE bMin, BYTE bHour, BYTE bDay, BYTE bMonth, WORD wYear);
		/*获得当前系统时间*/
		BOOL GetNewTime(TIMEDATA *NewSoeTime);
		/*判断是否是闰年*/
		BOOL CParSetting_IsLeapYear(UINT uYear);

};

#endif

