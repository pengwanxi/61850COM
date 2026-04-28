
#ifndef CPROTOCOL_LFP_H
#define CPROTOCOL_LFP_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

#define LFPPREFIXFILENAME				"/myapp/config/LFP/template/"  /* LFP固定名字前缀 */

// #define	LFPPRINT			/* 打印到终端 */
#define	LFPDEBUG			/* 打印到总线 */

#define	LFP_READDATA_DATATYPE		1				/* 读当前数据数据类型 */
#define	LFP_TIME_DATATYPE			2				/* 对时数据类型 */


#define	LFP_MAX_BUF_LEN			256			/* 最大缓存区长度 */
#define	LFP_MAX_RESEND_COUNT		3			/* 最大重发次数*/
#define	LFP_MAX_RECV_ERR_COUNT	3			/* 最大接收错误次数*/

extern "C" void GetCurrentTime( REALTIME *pRealTime );

/*
 * =====================================================================================
 *        Class:  CLFPCfgInfo
 *  Description:  配置信息
 * =====================================================================================
 */
class CProtocol_LFP : public CProtocol
{
    public:
        CProtocol_LFP();
        virtual ~CProtocol_LFP();

	protected://协议通用函数
		//获取配置文件里的装置数据
		BOOL GetDevData( ) ;
		//处理配置文件数据
		BOOL ProcessFileData( CProfile &profile );
		//创建模块
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;
	public:
		/* 虚函数 */
		//初始化总线
		virtual BOOL Init( BYTE byLineNo  ) ;
		//获取校验和
		virtual WORD GetCrc( BYTE * pBuf , int len );

	protected:
		//打印调试信息
		void print( const char *szBuf, int len=0 );
};


#endif // CPROTOCOL_LFP_H

