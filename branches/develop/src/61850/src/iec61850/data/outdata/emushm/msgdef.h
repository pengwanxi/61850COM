/********************************************************************
 *  消息常数、结构定义
 ********************************************************************/
#ifndef  _MSGDEF_H_
#define  _MSGDEF_H_

/*******************************************************************/
#define MSG_NAME_LEN  24  /*消息名称长度*/
#define MSG_BODY_LEN  512 /*消息内容长度*/
#define MSG_POOL_SUM  64  /*消息队列数量*/
#define MSG_SLOT_SUM  16  /*消息插槽数量*/

/*上行消息*/
#define MSGSET_RAW_DATA     0   /*原始数据, 遥测量、遥信量及SOE、电能量、保护数据等*/
#define MSGSET_CTRL_ECHO    1   /*控制响应, 遥控、升降、设点等                     */
#define MSGSET_DEVS_DATA    2   /*装置数据, 参数、定值、录波、报告等               */
#define MSGSET_FES_NOTIFY   3   /*前置(管理机)数据, 通道、设备通讯状态             */
#define MSGSET_FES_REQUEST  4   /*前置(管理机)请求                                 */

/*下行消息*/
#define MSGSET_CTRL_DATA    8   /*控制命令, 遥控、升降、设点等      */
#define MSGSET_DEVS_COMM    9   /*装置查询, 参数、定值、录波、报告等*/
#define MSGSET_FES_RECEIVE  10  /*前置(管理机)接收信息              */
#define MSGSET_FES_SWITCH   11  /*前置(管理机)切换信息              */

/*******************************************************************/
#pragma pack(1)

/*消息结构*/
typedef struct
{
	int   nSrcKey;        /*来源标识*/
	short nActive;        /*活动标志*/
	unsigned short  wLevel;         /*优先级别*/
	unsigned short  wTypes;         /*消息类型*/
	unsigned short  wMsgLen;        /*消息长度*/
	unsigned char   byBuff[MSG_BODY_LEN]; /*消息内容*/
} MSGITEM;

/*消息链表*/
typedef struct tagMSGLIST
{
	int    self;  /*本身的*/
	int    next;  /*后面的*/
} MSGLIST;

/*进程消息槽*/
typedef struct tagPROCSLOT
{
	char  szProcName[MSG_NAME_LEN]; /*进程名称*/
	int   nStatus;   /*通道状态*/
	int   nProcKey;  /*进程标识*/
	int   dwMsgCtrl; /*订阅控制*/
	int   nMsgPos;   /*消息位置*/
} PROCSLOT;

/*消息队列*/
typedef struct
{
	int  nQuality;	           /*空间状态*/
	int  nFreeNum;	           /*空闲数量*/
	int  nFreePos, nFreeTail;  /*空闲位置*/
	MSGLIST  vect[MSG_POOL_SUM]; /*消息索引空间*/
	MSGITEM  pool[MSG_POOL_SUM]; /*消息缓冲空间*/
	PROCSLOT slot[MSG_SLOT_SUM]; /*消息通道空间*/
} MSGSTORE;

#pragma pack()

/*******************************************************************/
#endif   /*_MSGDEF_H_*/
