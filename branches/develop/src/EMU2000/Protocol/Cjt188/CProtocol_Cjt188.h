
#ifndef CPROTOCOL_CJT188_H
#define CPROTOCOL_CJT188_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

#define CJT188PREFIXFILENAME				"/mynand/config/Cjt188/template/"  /* Cjt188固定名字前缀 */

#define	CJT188PRINT			/* 打印到终端 */
//#define	CJT188DEBUG			/* 打印到总线 */

#define	CJT188_READDATA_DATATYPE		1				/* 读当前数据数据类型 */
#define	CJT188_TIME_DATATYPE			2				/* 对时数据类型 */


#define	CJT188_MAX_BUF_LEN			256			/* 最大缓存区长度 */
#define	CJT188_MAX_RESEND_COUNT		3			/* 最大重发次数*/
#define	CJT188_MAX_RECV_ERR_COUNT	3			/* 最大接收错误次数*/

extern "C" void GetCurrentTime( REALTIME *pRealTime );

/*
 * =====================================================================================
 *        Class:  CJT188CfgInfo
 *  Description:  配置信息
 * =====================================================================================
 */
class Cjt188CfgInfo
{
	public:
		Cjt188CfgInfo ()                             /* constructor */
		{
			byDataType = 0;
			byDI0 = 0;
			byDI1 = 0;
			byCycle = 1;
		}
	
		BYTE byDataType;	//数据类型 Cjt188_YC_DATATYPE：遥测 Cjt188_YM_DATATYPE:遥脉
		BYTE byDI0;			//数据标识0
		BYTE byDI1;			//数据标识1
		BYTE byCycle;		//循环周期
}; /* -----  end of class Cjt188CfgInfo  ----- */


class CProtocol_Cjt188 : public CProtocol
{
    public:
        CProtocol_Cjt188();
        virtual ~CProtocol_Cjt188();

		/* 虚函数 */
		//获取协议报文
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		//处理协议报文
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		//初始化总线
		virtual BOOL Init( BYTE byLineNo  ) ;
		//广播报文
		virtual BOOL BroadCast( BYTE * buf , int &len ) ;
		//时间处理
		virtual void TimerProc(){ return;  } 

	public:
		BYTE m_bySendPos;	//发送步骤
		BYTE m_byDataType;	//数据类型
		vector <Cjt188CfgInfo> m_CfgInfo; //配置信息
		char m_szPrintBuf[256];	//打印缓冲区
		BYTE m_bySlaveAddr[7];
		BYTE  m_byMeterType;
		BYTE m_byFENum;
		BYTE m_bySer;
	
	protected://虚函数
		//获取校验和
		virtual BYTE GetCs( const BYTE * pBuf , int len );
		//判断报文有效性
		virtual BOOL WhetherBufValue(const BYTE *buf, int &len, int &pos );
		//读取配置文件
		virtual BOOL ReadCfgInfo( void );
		//移动发送位置
		virtual BYTE ChangeSendPos( void );

	protected:
		//打印调试信息
		void print( const char *szBuf, int len=0 );

	protected://协议通用函数
		//获取配置文件里的装置数据
		BOOL GetDevData( ) ;
		//处理配置文件数据
		BOOL ProcessFileData( CProfile &profile );
		//创建模块
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;
		// 字符串转16进制
		DWORD atoh ( char *szBuf, BYTE len, BYTE byFlag );
		//16进制转BCD码 
		BYTE HexToBcd( BYTE c );
		//BCD码转16进制
		BYTE BcdToHex( BYTE c );

};


#endif // CPROTOCOL_Cjt188_H

