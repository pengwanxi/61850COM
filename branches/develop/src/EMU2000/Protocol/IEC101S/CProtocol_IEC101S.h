#ifndef CPROTOCOL_IEC101S_H
#define CPROTOCOL_IEC101S_H

#include "../../share/Rtu.h"
#include "../../share/CMethod.h"
#include "../../share/typedef.h"



// #define		IEC101S_PRINT			/* 打印到终端 */
#define		IEC101S_DEBUG			/* 打印到总线 */

#define	IEC101SPREFIXFILENAME			"/mynand/config/IEC101Slave/"	/* 104固定路径 */
#define MODULE_IEC101S_2002		1  //标准101 2002版   
#define MODULE_IEC101S_1997		2  //标准101 1997版   


#define	IEC101S_TOTAL_CALL		0x00000001				/* 总召唤 */
#define	IEC101S_TOTAL_YX		0x00000002				/* 遥信 */
#define	IEC101S_TOTAL_YC	    0x00000004				/* 遥测 */
#define	IEC101S_TOTAL_YM		0x00000008				/* 遥脉 */
#define	IEC101S_CALL_YM			0x00000010				/* 召唤遥脉 */
#define	IEC101S_TIME_SYNC		0x00000020				/* 时间同步 */
#define	IEC101S_YK_SEL			0x00000040				/* 遥控选择 */
#define	IEC101S_YK_EXE			0x00000080				/* 遥控执行 */
#define	IEC101S_YK_CANCEL		0x00000100				/* 遥控取消 */
#define	IEC101S_TOTAL_CALL_END  0x00000200				/* 总召唤结束 */
#define	IEC101S_CALL_YM_END	    0x00000400				/* 召唤遥脉结束 */
#define	IEC101S_TIME_SYNC_END   0x00000800				/* 对时结束 */
#define	IEC101S_SPECIAL_DATA	0x00001000				/* 特殊数据阶段 特指总召对时遥脉等阶段 */
#define	IEC101S_CHANGE_YX	    0x00002000				/* 变化遥信 */
#define	IEC101S_CHANGE_YC	    0x00004000				/* 变化遥测 */
#define	IEC101S_SOE_YX			0x00008000				/* 遥信事件 */


// typedef enum _IEC101SSENDFLAG
// {
	// NULL_FLAG,				//标识置空
	// TOTAL_CALL_BEGIN,       //总召唤开始
	// TOTAL_CALL_YX,			//总召唤遥信
	// TOTAL_CALL_YC,			//总召唤遥测
	// TOTAL_CALL_YM_BEGIN,    //总召唤遥脉开始
	// TOTAL_CALL_YM,			//总召唤遥脉

	// TIME_SYNC,				//时钟标识

	// YK_BEGIN,				//遥控标识
	// YK_RETURN,              //遥控返回
// }IEC101SSENDFLAG;

typedef enum _IEC101SSENDSTATUS
{	
	/* 自定义  */
	NULL_STATUS,            //无发送状态
	RESEND,					//重发状态
	LEVEL1_DATA,				//一级数据
	LEVEL2_DATA,				//二级数据
	TOTAL_CALL,				//总召唤
	TIME_SYNC,				//对时
	CALL_YM,				//召唤遥脉
	YK_RTN_DATA,			//遥控返回数据
	LINK_STATUS,		   //响应帧 链路状态或要求访问
	RECOGNITION,		   //确认帧 认可  0
	DENY_RECOGNITION,      //确认帧 否定认可:未收到报文， 链路忙 1
	USER_DATA,			   //响应帧 用户数据
	NONE_USER_DATA,		   //响应帧 否定认可 无所请求数据

	// /* 固定帧 */

	// /* 变化帧 */
	// M_SP_NA_1,			   //单点信息	ASDU1
	// M_SP_TA_1,			   //带时标的单点信息 ASDU2
	// M_DP_NA_1,			   //双点信息	ASDU3 
	// M_DP_TA_1,			   //带时标的双点信息 ASDU4
	// // M_ST_NA_1,			   //步位置ASDU5 
	// // M_ST_TA_1,			   //带时标的步位置信息 ASDU6 
	// // M_BO_NA_1,			   //32比特串 ASDU7
	// // M_BO_TA_1,			   //带时标的32比特串 ASDU8
	// M_ME_NA_1,			   //测量值 归一化值 ASDU9
	// M_ME_TA_1,			   //测量值 带时标的归一化值 ASDU10
	// M_ME_NB_1,			   //测量值 标度化值ASDU11
	// M_ME_TB_1,			   //测量值 带时标的标度化值ASDU12
	// M_ME_NC_1,			   //测量值 短浮点数ASDU13
	// M_ME_TC_1,			   //测量值 带时标的短浮点数ASDU14
	// M_IT_NA_1,			   //累计量 ASDU15
	// M_IT_TA_1,			   //带时标的累计量ASDU16
	// // M_EP_TA_1,			   //带时标的继电器事件ASDU17
	// // M_EP_TB_1,			   //带时标的继电器成组启动事件ASDU18
	// // M_EP_TC_1,			   //带时标的成组输出信息ASDU19
	// // M_PS_NA_1,			   //带变位检出成组单点信息ASDU20
	// M_ME_ND_1,			   //测量值,不带品质描述值得归一化值ASDU21
	// M_SP_TB_1,			   //带CP56Time2a时标的单点信息 ASDU30          
	// M_DP_TB_1,             //带CP56Time2a时标的双点信息 ASDU31         
	// // M_ST_TB_1,             //带CP56Time2a时标的步位置信息   ASDU32 
	// // M_BO_TB_1,             //带CP56Time2a时标的32比特串 ASDU33    
	// M_ME_TD_1,             //带CP56Time2a时标的测量值, 规一化值 ASDU34
	// M_ME_TE_1,             //带CP56Time2a时标的测量值, 标度化值 ASDU35
	// M_ME_TF_1,             //带CP56Time2a时标的测量值, 短浮点数 ASDU36
	// M_IT_TB_1,             //带CP56Time2a时标的累计量           ASDU37
	// // M_EP_TD_1,             //带CP56Time2a时标的继电保护设备事件ASDU38
	// // M_EP_TE_1,             //带CP56Time2a时标的继电保护设备成组启动事件 ASDU39
	// // M_EP_TF_1,             //带CP56Time2a时标的继电保护设备成组输出电路信息ASDU40 
	// // 
	// //
	
}IEC101SSENDSTATUS;				/* ----------  end of enum IEC101SSENDSTATUS  ---------- */

class CProtocol_IEC101S : public CRtuBase
{
    public:
    CProtocol_IEC101S();
    virtual ~CProtocol_IEC101S();
	BOOL GetDevData( );
	BOOL ProcessFileData( CProfile &profile );
	BOOL CreateModule( int iModule ,char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath ) ;
	BOOL InitIEC101S_Module( CProtocol_IEC101S * pProtocol , int iModule , char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath );
	virtual BOOL Init( BYTE byLineNo );
	//获取校验和
	virtual BYTE GetCs( const BYTE *pBuf, int len );
	//判断报文合理性
	virtual BOOL WhetherBufValid( const BYTE *buf, int &len );
	//判断接收标识位
	virtual BOOL ProcessJudgeFlag( BYTE c );


	protected:
	char m_sMasterAddr[ 200 ] ;//网络参数保存
	void print( const char *szBuf, int len=0 );   //内部打印函数

	public:
		char m_szPrintBuf[256];
		BOOL m_bFcb;
		IEC101SSENDSTATUS m_SendStatus;
		// IEC101SSENDFLAG m_SendFlag;
		DWORD m_dwSendFlag;
    private:
};

#endif // CPROTOCOL_IEC101S_H
