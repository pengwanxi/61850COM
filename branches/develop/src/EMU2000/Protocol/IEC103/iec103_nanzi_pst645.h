
#ifndef  _IEC103_NANZI_PST645_H__INC
#define  _IEC103_NANZI_PST645_H__INC


#include <vector>

#include "../../share/typedef.h"
#include "../../share/CProtocol.h"
#include "../../share/gDataType.h"
#include "CProtocol_IEC103.h"

using namespace std;

#define IEC103PREFIXFILENAME				"/mynand/config/IEC103/template/"  /* 103固定名字前缀 */

#define	IEC103_MAX_YC_NUM			256			/* 遥测最大数量 */
#define	IEC103_MAX_YX_NUM			512			/* 遥信最大数量 */
#define	IEC103_MAX_YK_NUM			256			/* 遥控最大数量 */
#define	IEC103_MAX_YM_NUM			256			/* 遥脉最大数量 */
#define	IEC103_TOTAL_CALL			10*60*1000	/* 总召时间间隔 */
#define	IEC103_TIME_SYN				10*60*1000	/* 对时时间间隔 */
#define	IEC103_LINK_TIMEOUT			15*1000		/* 通讯超时时间 */
#define	IEC103_YK_TIMEOUT			5*1000		/* 遥控超时时间 */
#define	IEC103_MAX_ERROR_COUNT		3			/* 错误报文次数 */
#define	IEC103_MAX_YKERROT_COUNT	3			/* 遥控错误次数 */
#define	IEC103_MAX_BUF_LEN			256			/* 最大缓存长度 */
#define	IEC103_MAX_RESEND_COUNT		3			/* 最大重发次数 */
#define	IEC103_MAX_DEBUG_BUF_LEN	1024		/* 最大打印长度 */



/*
* =====================================================================================
*        Class:  CfgInfo
*  Description:  配置信息
* =====================================================================================
*/

typedef enum _SENDSTATUS1
{
	/* 固定帧 */
	C_PL1_NA, //召唤一级数据


	/* 变化帧*/
	C_YC_NA_3, //召唤遥测数据
	C_YX_NA_3, //召唤遥信数据
}SENDSTATUS1;				/* ----------  end of enum SendStatus  ---------- */


							/*
							* =====================================================================================
							*        Class:  IEC103
							*  Description:  IEC103
							* =====================================================================================
							*/
class CIEC103_Nanzi_PST645 :public CProtocol_IEC103
{
public:
	/* ====================  LIFECYCLE     ======================================= */
	CIEC103_Nanzi_PST645();                             /* constructor      */
	~CIEC103_Nanzi_PST645();                            /* destructor       */

										   /* ====================  ACCESSORS     ======================================= */

										   /* ====================  VIRTUAL METHODS ===================================== */
	//初始化协议数据
	virtual BOOL Init(BYTE byLineNo);
	//获取协议数据缓存
	virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
	//处理收到的数据缓存
	virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);
	//更新通讯状态
	virtual BOOL GetDevCommState();

private:
	/* ====================  DATA MEMBERS  ======================================= */

	BOOL m_bLinkStatus;          //通讯状态
	BOOL m_bFcb;				//FCB
	BOOL m_bIsReSend;				//是否重发
	BOOL m_bIsSending;				//?是否正在发送 1正在发送
	BOOL m_bIsNeedResend;			//是否需要重发
	BOOL m_bIsYking;				//是否属于遥控状态
	BOOL m_bIsTotalCall;			//是否总召
	BOOL m_bIsYmCall;				//是否总召


	DWORD m_dwTotalCallTime;        //总召计时
	DWORD m_dwLinkTimeOut;			 //通讯超时
	DWORD m_dwYkTimeOut;            //遥控超时

	BYTE m_byResendNumber;        //重发次数
	BYTE m_byYkErrorCount;          //遥控错误计数
	BYTE m_byReSendBuf[IEC103_MAX_BUF_LEN];//重发缓存
	BYTE m_byYkSendBuf[IEC103_MAX_BUF_LEN];//遥控重发
	BYTE m_byResendCount;			//重发计数
	WORD m_wReSendLen;				//重发缓存长度
	BYTE m_byYkSendLen;				//遥控缓存长度
	BYTE m_byRemoteBusNo;			//转发总线号
	BYTE m_byRemoteAddr;			//转发地址

	SENDSTATUS1 m_SendStatus;     //发送状态

								 /* ====================  METHODS     ======================================= */
public:
	char DebugBuf[IEC103_MAX_DEBUG_BUF_LEN];
	//打印协议信息
	void print(char *buf, int len = 0);

private:
	/* **********************初始化协议 ***************************************/
	//初始化协议状态
	BOOL InitProtocolStatus();

	/* **********************发送 ***************************************/
	/* 总线消息 */
	BOOL ProcessBusMsg(PBUSMSG pBusMsg, BYTE *buf, int &len);
	/* 固定帧 */
	//获取协议缓存
	BOOL GetSendBuf(BYTE *buf, int &len);

	/* **********************接收处理 ***************************************/
	//固定帧
	BOOL ProcessHead10Buf(BYTE *buf, int len);
	//可变帧
	BOOL ProcessHead68Buf(BYTE *buf, int len);
	BOOL prcess0x0A(BYTE *buf, int &len);
	BOOL CallLevel1Data(BYTE *buf, int &len);
	BOOL CallYcData(BYTE *buf, int &len);
	BOOL CallYxData(BYTE *buf, int &len);
};



#endif   /* ----- #ifndef _IEC103_H__INC  ----- */
