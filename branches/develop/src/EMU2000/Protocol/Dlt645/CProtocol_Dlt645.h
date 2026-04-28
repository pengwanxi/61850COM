
#ifndef CPROTOCOL_Dlt645_H
#define CPROTOCOL_Dlt645_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

#define DLT645PREFIXFILENAME				"/mynand/config/Dlt645/template/"  /* Dlt645固定名字前缀 */

//#define	DLT645PRINT			/* 打印到终端 */
//#define	DLT645DEBUG			/* 打印到总线 */

#define	DLT645_YC_DATATYPE		2				/* 遥测数据类型 */
#define	DLT645_YM_DATATYPE		4				/* 遥脉数据类型 */
#define	DLT645_TIME_DATATYPE	3				/* 遥脉数据类型 */
#define	DLT645_YX_DATATYPE	1				/* 遥信数据类型 */
#define DLT645_MAX_MIN_DATATYPE  5             /* 最大最小值数据类型*/
#define DLT645_SOE_DATATYPE      6            /* SOE事件记录*/


#define	DLT645_MAX_BUF_LEN			256			/* 最大缓存区长度 */
#define	DLT645_MAX_RESEND_COUNT		3			/* 最大重发次数*/
#define	DLT645_MAX_RECV_ERR_COUNT	3			/* 最大接收错误次数*/


#define BCD_TO_DEC(x)            ((x >> 4) * 10 + (x & 0x0F))                  /* BCD码转十进制 */
#define DEC_TO_BCD(x)            (((x / 10) << 4) + (x % 10))                  /* 十进制转BCD码 */



extern "C" void GetCurrentTime( REALTIME *pRealTime );

/*
 * =====================================================================================
 *        Class:  Dlt645CfgInfo
 *  Description:  配置信息
 * =====================================================================================
 */
class Dlt645CfgInfo
{
	public:
		Dlt645CfgInfo ()                             /* constructor */
		{
			byDataType = 0;
			byDI0 = 0;
			byDI1 = 0;
			byDI2 = 0;
			byDI3 = 0;
			byDataNum = 0;
			byStartIndex = 0;
			byDataFormat = 0;
			byDataLen = 0;
			byFENum = 0;
			byCycle = 1;
			byflag = 0;

		}
	
		BYTE byDataType;	//数据类型 Dlt645_YC_DATATYPE：遥测 Dlt645_YM_DATATYPE:遥脉
		BYTE byDI0;			//数据标识0
		BYTE byDI1;			//数据标识1
		BYTE byDI2;			//数据标识2		
		BYTE byDI3;			//数据标识3
		BYTE byDataNum;		//数据数量
		BYTE byStartIndex;  //起始序号
		BYTE byDataFormat;	//数据格式
		BYTE byDataLen;		//数据长度
		BYTE byFENum;		//0xfe 的个数
		BYTE byCycle;		//循环周期
		BYTE byflag;       //特殊标记
}; /* -----  end of class Dlt645CfgInfo  ----- */


class CProtocol_Dlt645 : public CProtocol
{
    public:
        CProtocol_Dlt645();
        virtual ~CProtocol_Dlt645();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init( BYTE byLineNo  ) ;
		//获取校验和
		virtual BYTE GetCs( const BYTE * pBuf , int len );
		virtual BOOL BroadCast( BYTE * buf , int &len ) ;
		virtual void TimerProc(){ return;  } 
		//判断报文有效性
		virtual BOOL WhetherBufValue(const BYTE *buf, int &len, int &pos );
		//读取配置文件
		virtual BOOL ReadCfgInfo( void );
		//计算固定格式数据
		virtual BOOL CalFormatData( const BYTE *buf, BYTE byDataFormat, BYTE byDataLen, DWORD &dwData );

		virtual BYTE ChangeSendPos( void );

		virtual BYTE ChangeSendPos_YM(void);//跳转遥脉

	public:
		BYTE m_bySendPos;	//发送步骤
		BYTE m_byDataType;	//数据类型
		vector <Dlt645CfgInfo> m_CfgInfo; //配置信息
		char m_szPrintBuf[256];	//打印缓冲区
		BYTE m_bySlaveAddr[6];

		BOOL GetDevData( ) ;
		void print( const char *szBuf, int len=0 );
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;
		DWORD atoh ( char *szBuf );
		DWORD atoh ( char *szBuf , BYTE len, BYTE byFlag);
		//字符串转16进制
		// DWORD atoh ( char *szBuf );

		//16进制转BCD码
		// BYTE HexToBcd ( BYTE c );
		//给m_bySlaveAddr赋值
		BOOL ReadCfgSlaveAddr( char *szLineBuf );
		//读配置值
		BOOL ReadCfgVal( char *szLineBuf );
};


#endif // CPROTOCOL_Dlt645_H

