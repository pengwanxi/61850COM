/*
 * =====================================================================================
 *
 *       Filename:  IEC103.h
 *
 *    Description:  ????IEC103???写???
 *
 *        Version:  1.0
 *        Created:  2014??10??09?? 09时42??56??
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *   Organization:  esdtek
 *
 *		  history:
 *
 * =====================================================================================
 */

/*ASDU10结构说明		by cyz!
 * 字段名								长度
 * -------------------------------------------------------------
 *  类型标识(ASDU10)					1
 *  传送原因							1
 *  公共地址							1
 *  功能类型							1
 *  信息序号							1
 *  返回信息标识符						1
 *  通用分类数据集数目					1----------bit[0-5]:数目(采集量数目)
 *  										|------bit6:	计数器位——具有相同返回信息标识符的应用服务数据单元的一位计数器位
 *  										|------bit7:	后续状态位——0:后面未跟着具有相同返回信息标识符的应用服务数据单元
 *  																	1:后面跟着...
 * =======================
 *  通用分类标识序号	|
 *  描述类别			n枚
 *  通用分类数据描述	|
 *  通用分类标识数据	|
 * =======================
 */
#ifndef  _IEC103_ZHONGNENG_H__INC
#define  _IEC103_ZHONGNENG_H__INC


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
 *        Class:  CfgInfo1
 *  Description:  配置信息
 * =====================================================================================
 */
class CfgInfo1
{
	public:
		CfgInfo1 ()
		{
			FunType = 0;
			InfoIndex = 0;
			AddInfo = 0;
			DataType = 0;
			StartIndex = 0;
			DataNum = 0;
			DataFormat = 0;
		}                             /* constructor */

		BYTE FunType;            //功能类型
		BYTE InfoIndex;          //信息序号
		BYTE AddInfo;			 //附加信息
		BYTE DataType;           //数据类型
		BYTE StartIndex;         //起始位置
		BYTE DataNum;            //数据数量
		BYTE DataFormat;         //数据格式
}; /* -----  end of class CfgInfo1  ----- */


/*-----------------------------------------------------------------------------
 *  ????状态
 *-----------------------------------------------------------------------------*/
typedef enum _SENDSTATUS2
{
	/* 固定帧 */
	C_RCU_NA_2, //初始化
	C_NAME_2,//装置名称
	C_PL1_NA_2, //召唤一级数据
	C_PL2_NA_2, //召唤二级数据
	C_RLK_NA_2, //请求链路状态
	C_GD_NA_2,  //通用分类数据 ASDU10	

}SENDSTATUS2;				/* ----------  end of enum SendStatus  ---------- */


/*
 * =====================================================================================
 *        Class:  IEC103
 *  Description:  IEC103
 * =====================================================================================
 */
class CIEC103_ZN:public CProtocol_IEC103
{
public:
    /* ====================  LIFECYCLE     ======================================= */
    CIEC103_ZN ();                             /* constructor      */
    ~CIEC103_ZN ();                            /* destructor       */

    /* ====================  ACCESSORS     ======================================= */

    /* ====================  VIRTUAL METHODS ===================================== */
    //时间处理函数
    virtual void    TimerProc();
    //初始化协议数据
    virtual BOOL Init( BYTE byLineNo );
    //获取协议数据缓存
    virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
    //处理收到的数据缓存
    virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
    //更新通讯状态
    virtual BOOL GetDevCommState( ) ;

protected:
    /* ====================  DATA MEMBERS  ======================================= */

private:
    /* ====================  DATA MEMBERS  ======================================= */

    BOOL m_bLinkStatus;          //通讯状态
    BOOL m_bFcb;				//FCB
    BOOL m_bIsReSend;				//是否重发
    BOOL m_bIsSending;				//?是否正在发送 1正在发送
    BOOL m_bIsNeedResend;			//是否需要重发
   
   
	BOOL m_bIsGDCall;				//召唤通用分组数据
	BOOL m_bIsSendName;             //是否发送装置名称（第一次为true）

    DWORD m_dwLinkTimeOut;			 //通讯超时
  

    BYTE m_byRecvErrorCount;        //接收错误计数
    BYTE m_byYkErrorCount;          //遥控错误计数
    BYTE m_byReSendBuf[IEC103_MAX_BUF_LEN];//重发缓存
  
    BYTE m_byResendCount;			//重发计数
    WORD m_wReSendLen;				//重发缓存长度
    BYTE m_byYkSendLen;				//遥控缓存长度
    BYTE m_byRemoteBusNo;			//转发总线号
    BYTE m_byRemoteAddr;			//转发地址





    vector < CfgInfo1 > m_IEC103_CfgInfo; //配置信息
    SENDSTATUS2 m_SendStatus;     //发送状态

    /* ====================  METHODS     ======================================= */
public:
    char DebugBuf[IEC103_MAX_DEBUG_BUF_LEN];
    //打印协议信息
    void print( char *buf, int len = 0 );
    
   

private:
    /* **********************初始化协议 ***************************************/
   
    //初始化协议状态
    BOOL InitProtocolStatus();
    /* **********************发送 ***************************************/
    /* 固定帧 */
    //获取协议缓存
    BOOL GetSendBuf( BYTE *buf, int &len );
	BOOL CallGDData(BYTE *buf, int &len);
	BOOL CallGDData_02(BYTE *buf, int &len);
	BOOL CallGDData_03(BYTE *buf, int &len);
	BOOL CallGDData_04(BYTE *buf, int &len);
	
    //复位通信单元
    BOOL ResetCommUnit(BYTE *buf, int &len);
    //召唤一级数据
    BOOL CallLevel1Data(BYTE *buf, int &len);
    //召唤二级数据
    BOOL CallLevel2Data(BYTE *buf, int &len);



   
    //通用分类数据
    BOOL GeneralClassData(BYTE *buf, int &len);
    //一般命令
    BOOL GeneralCommand(BYTE *buf, int &len);
    //通用命令
    BOOL CommonCommand(BYTE *buf, int &len);

    /* **********************接收处理 ***************************************/
    //固定帧
    BOOL ProcessHead10Buf(BYTE *buf, int len);
    //可变帧
    BOOL ProcessHead68Buf(BYTE *buf, int len);
    

  
  
    /* 遥信 */
    //带时标的报文 ASDU1
    BOOL M_TTM_TA_3_Frame(BYTE *buf, int len);
    //带相对时间的时标报文 ASDU2
    BOOL M_TMR_TA_3_Frame(BYTE *buf, int len);
    //总召唤时传输的单点信息状态帧 ASDU40
    BOOL M_SP_NA_3_Frame(BYTE *buf, int len);
    //总召唤时传输的双点信息状态帧 ASDU42
    BOOL M_DP_NA_3_Frame(BYTE *buf, int len);
    //带时标的单点信息状态变化帧 ASDU41
    BOOL M_SP_TA_3_Frame(BYTE *buf, int len);
    //带时标的双点信息状态变化帧 ASDU43
    BOOL M_DP_TA_3_Frame(BYTE *buf, int len);
    //总召唤时传输的单点状态和状态变化信息帧 ASDU44
    BOOL M_SS_NA_3_Frame( BYTE *buf, int len );
    //总召唤时传输的双点状态和状态变化信息帧 ASDU46
    BOOL M_DS_NA_3_Frame( BYTE *buf, int len );
    //状态变化时传输的单点状态和状态变化信息 ASDU45
    BOOL M_SS_TA_3_Frame( BYTE *buf, int len );
    //状态变化时传输的双点状态和状态变化信息 ASDU47
    BOOL M_DS_TA_3_Frame( BYTE *buf, int len );

    /* 遥测 */
    
    
    
    //通用分类数据 ASDU10		特殊开发 by cyz!-------
    BOOL M_GD_NA_3_Frame(BYTE *buf, int len);
    
  
   
    //响应总召唤的被测值VII 超过门限值得被测值II ASDU50
    BOOL M_MEVII_NA_3_Frame(BYTE *buf, int len );

 
public:
  
    float floatvalue(BYTE *);

	BYTE bysendpos;
	BYTE count;
    //???????? ASDU65
    //OL C_RC_NA_3_Frame(BYTE *buf, int len );
};			/* -----  end of class CIEC103_ZN  ----- */



#endif   /* ----- #ifndef _IEC103_H__INC  ----- */
