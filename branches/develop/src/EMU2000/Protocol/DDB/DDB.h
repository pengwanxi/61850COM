/*
 * =====================================================================================
 *
 *       Filename:  DDB.h
 *
 *    Description:	双机冗余协议
 *
 *        Version:  1.0
 *        Created:  2014年10月15日 13时35分06秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *   Organization:
 *
 *		  history:
 *
 * =====================================================================================
 */


#ifndef  __DDB_H__
#define  __DDB_H__

#include "../../share/typedef.h"
#include "../../share/CProtocol.h"
#include "CProtocol_DDB.h"


#define	DDBPREFIXFILENAME			"/mynand/config/DDB/template/"	/* 配置文件固定路径 */

#define	DDB_YX_DATATYPE		1				/* 遥信数据类型 */
#define	DDB_YC_DATATYPE		2				/* 遥测数据类型 */
#define	DDB_YM_DATATYPE		3				/* 遥脉数据类型 */
#define	DDB_YK_DATATYPE		4				/* 遥控数据类型 */
#define	DDB_TIME_DATATYPE   5				/* 时间数据类型 */
#define	DDB_LINKBUSSTATUS_DATATYPE   6				/* 总线通讯状态数据类型 */
#define	DDB_LINKSTNSTATUS_DATATYPE   7				/* 装置通讯状态数据类型 */


#define	DDB_MAX_BUF_LEN     256				/* 报文最大长度 */


#define	IDENTITY_A			0				/* 身份A机 */
#define	IDENTITY_B			1				/* 身份B机 */
#define	IDENTITY_SINGLE		2				/* 身份独自 */

// #define	STATUS_MASTER		1				[> 主机 <]
// #define	STATUS_SLAVE		0				[> 从机 <]
#define	DDB_LINK_TIMEOUT	30*1000	//30s	/* 设置超时时间 */
#define	DDB_MAX_ERROR_COUNT   10				/* 最大错误报文 时间为 *间隔 */
#define	DDB_MAX_RESEND_COUNT  10				/* 最大重发次数 时间为 *间隔*/

/*-----------------------------------------------------------------------------
 *  发送状态
 *-----------------------------------------------------------------------------*/
typedef enum _DDBSENDSTATUS
{
	REQUEST_SYN,	//请求同步
	RESPONSE_SYN,	//响应同步
	REQUEST_DATA,	//请求数据
	RESPONSE_DATA,	//响应数据
	REQUEST_SWITCH,//切换请求
	RESPONSE_SWITCH,//切换响应
	NONESTATUS,		//无状态 等待

}DDBSENDSTATUS;				/* ----------  end of enum SendStatus  ---------- */


/*-----------------------------------------------------------------------------
 *  YK  状态 新增加状态， 填写在数据类型之后
 *-----------------------------------------------------------------------------*/

typedef enum _DDBYKSTATUS
{
	DDB_YK_NONE_STATUS,
	DDB_YK_SEL,
	DDB_YK_SEL_CONFIRM,
	DDB_YK_EXE,
	DDB_YK_EXE_CONFIRM,
	DDB_YK_CANCEL,
	DDB_YK_CANCEL_CONFIRM,
	DDB_YK_SEL_RTN,
	DDB_YK_SEL_RTN_CONFIRM,
	DDB_YK_EXE_RTN,
	DDB_YK_EXE_RTN_CONFIRM,
	DDB_YK_CANCEL_RTN,
	DDB_YK_CANCEL_RTN_CONFIRM,
	DDB_YK_ERROR
}DDBYKSTATUS;

typedef enum _DDBLINKSTATUS
{
	DDB_BUS_LINK_STATUE,  //总线通讯状态
	DDB_STN_LINK_STATUS   //站点通讯状态

}DDBLINKSTATUS;

/*
 * =====================================================================================
 *        Class:  CDDB
 *  Description:  双机冗余类
 * =====================================================================================
 */
class CDDB : public CProtocol_DDB
{
  public:
	/* ====================  LIFECYCLE     ======================================= */
	CDDB ();                             /* constructor      */
	~CDDB ();                            /* destructor       */

	/* ====================  VIRTUAL METHODS ===================================== */
	//时钟处理
	virtual void    TimerProc();
	//初始化函数
	virtual BOOL Init( BYTE byLineNo );
	//获取协议数据缓存
	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	//处理协议数据缓存
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;



  protected:
	/* ====================  DATA MEMBERS  ======================================= */

  private:
	/* ====================  DATA MEMBERS  ======================================= */

	//cfginfo
	char m_byMachineId;
	// char m_szLocalIp[16];
	// char m_szRemoteIp[16];
	// DWORD m_dwPort;

	//状态参数
	BOOL m_bSyn;					//是否同步
	BOOL m_bRemoteSyn;				//对方是否同步
	BOOL m_byLocalStatus;			//本地状态
	BOOL m_byRemoteStatus;			//远程状态
	BOOL m_bLinkStatus;				//通讯状态
	BOOL m_bIsReSend;				//是否需要重发
	BOOL m_bIsSending;				//发送后置1 其他置0

	BOOL m_bSwitchState;				//是否发送切换请求
	BYTE m_bTimeProcCount;			//TimeProc调用次数

	DWORD m_dwLinkTimeOut;			//通讯超时

	BYTE m_byRecvErrorCount;        //接收报文错误计数
	BYTE m_byReSendBuf[DDB_MAX_BUF_LEN];//重发缓存区
	BYTE m_byResendCount;			//重发计数
	WORD m_wReSendLen;				//重发缓存区长度
	WORD m_wDataPos;				//数据位置

	DDBSENDSTATUS m_SendStatus;		//发送状态
	BYTE m_byDataType;				//数据类型

	//yk相关
	DDBYKSTATUS m_YkStatus;         //遥控状态
	BYTE m_bySaveDataType;		//遥控时保存正在通讯的数据状态
	BYTE m_bySaveSrcBusNo;			//保存源总线号
	WORD m_wSaveSrcDevAddr;			//保存源地址
	DDBYK_DATA m_SaveDestYkData;	//保存目的数据
	BOOL m_bIsYking;				//是否正在yk

	// 通讯状态相关数据
	// BYTE m_byLinkStatusType;        //通讯状态类型　总线或装置

	//各类型数据总数量
	WORD m_wAllYcNum;
	WORD m_wAllYxNum;
	WORD m_wAllYmNum;

	//各类型数据首地址
	ANALOGITEM *m_pYcHeadAddr;
	DIGITALITEM *m_pYxHeadAddr;
	PULSEITEM *m_pYmHeadAddr;

	time_t m_tmLastSwitchTime;
	time_t m_tmNowSwitchTime;
	BYTE m_byQuickSwitchNum;
	int m_iDelayedSynSecond;
	int m_iDelayedSwitchMinute;
	BOOL m_bRecvResponseSwitch;			//是否收到切换请求
	//

	/* ====================  METHODS  ======================================= */
	/* **********************其他 ***************************************/
	//获取共享内存数据类型的起始位置和数量
	BOOL GetStnStartPosAndNum(BYTE byDataType, STNPARAM *pStn, DWORD &dwStartPos, WORD &wCount );
	//根据内存点号位置获取装置的顺序号和点号
	BOOL GetSerialNoAndPnt(BYTE byDataType, WORD wPos, WORD &wSerialNo, WORD &wPnt, STNPARAM *pStn );
	//切换主从 1为从换主 0为主换从
	BOOL SwitchStatus( BOOL bStatus );
	//切换报文(判断总线装置状态的情况)切换主从 1为从换主 0为主换从
	BOOL DevStateSwitchStatus( BOOL bStatus );
	//获取类型数据总数量
	WORD GetDataNum ( BYTE byDataType );
	//获取数据首地址
	DWORD GetDataHeadAddr ( BYTE byDataType );
	//获取公共帧
	void GetCommonFrame ( BYTE *buf, BYTE byFuncCode );
	//帧校验
	WORD GetCrc ( BYTE *pBuf, int nLen );
	//打印
	void print( char* szBuf );
	BOOL JudgeYkMsg ( PBUSMSG pBusMsg );
	void SaveYkMsgInfo ( PBUSMSG pBusMsg );
	void SetYkDataStatus ( PBUSMSG pBusMsg );
	/* **********************初始化协议 ***************************************/
	//读取配置信息
	BOOL ReadCfgInfo();
	//初始化协议状态
	BOOL InitProtocolStatus( );
	//初始化数据
	void InitProtocolData ( BOOL bStatus );
	/* **********************发送部分 ***************************************/
	/* 消息处理 */
	BOOL ProcessBusMsg(PBUSMSG pBusMsg, BYTE *buf, int &len);

	/* 发送类型处理 */
	BOOL GetSendTypeBuf( BYTE *buf, int &len );
	//添加发送校验位
	BOOL AddSendCrc(BYTE *buf, int &len);
	//请求同步
	BOOL RequestSyn( BYTE *buf, int &len );
	//响应同步
	BOOL ResponseSyn( BYTE *buf, int &len );
	//请求数据
	BOOL RequestData( BYTE *buf, int &len );
	//响应数据
	BOOL ResponseData( BYTE *buf, int &len  );
	//请求切换
	BOOL RequestSwitch( BYTE *buf, int &len );
	//响应切换
	BOOL ResponseSwitch( BYTE *buf, int &len );
	//yk请求
	BOOL RequestYkData ( BYTE *buf, int &len );
	//yk响应
	BOOL ResponseYkData ( BYTE *buf, int &len  );

	/* 发送数据 */
	BOOL GetSendDataBuf( BYTE *buf, int &len );
	//遥测数据
	BOOL YcDataSend( BYTE *buf, int &len );
	//遥信数据
	BOOL YxDataSend( BYTE *buf, int &len );
	//遥脉数据
	BOOL YmDataSend( BYTE *buf, int &len );
	//对时
	BOOL TimeSyncSend( BYTE *buf, int &len );
	// 通讯状态发送
	BOOL LinkBusStatusSend( BYTE *buf, int &len );
	BOOL LinkStnStatusSend( BYTE *buf, int &len );

	//主从判断
	void JudgeStatus ( BYTE byRemoteByte );
	//主从判断
	void JudgeStatus ( void );


	/* **********************处理部分 ***************************************/
	// 检测接收报文是否有效
	BOOL WhetherBufValue( BYTE *buf, int &len, int &pos );
	// 处理类型数据
	BOOL ProcessRecvTypeBuf( BYTE *buf, int len );
	//请求响应
	BOOL ProcessRequestSyn( BYTE *buf, int len );
	//响应同步
	BOOL ProcessResponseSyn( BYTE *buf, int len );
	//请求数据
	BOOL ProcessRequestData( BYTE *buf, int len );
	//响应数据
	BOOL ProcessResponseData( BYTE *buf, int len  );
	//请求切换
	BOOL ProcessRequestSwitch( BYTE *buf, int len );
	//响应切换
	BOOL ProcessResponseSwitch( BYTE *buf, int len );
	//发送yk消息
	BOOL PackSendYkMsg ( BYTE *buf );

	/* 处理数据 */
	BOOL ProcessRecvDataBuf(BYTE *buf, int len);
	//遥测数据
	BOOL YcDataDeal( BYTE *buf, int len );
	//遥信数据
	BOOL YxDataDeal( BYTE *buf, int len );
	//遥脉数据
	BOOL YmDataDeal( BYTE *buf, int len );
	//对时
	BOOL TimeSyncDeal( BYTE *buf, int len );
	//通讯状态
	BOOL LinkBusStatusDeal( BYTE *buf, int len );
	BOOL LinkStnStatusDeal( BYTE *buf, int len );

	//遍历底层装置状态
	BOOL ErgodicDevState();
	
	BOOL ProcessYK( PBUSMSG pBusMsg ) ;
}; /* -----  end of class CDDB  ----- */

#endif   /* ----- #ifndef __DDB_H__  ----- */
