#ifndef CPROTOCOL_MODBUS_H
#define CPROTOCOL_MODBUS_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"

class CProtocol_IEC615 : public CProtocol
{/*{{{*/
    public:
        CProtocol_IEC615();
        virtual ~CProtocol_IEC615();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init( BYTE byLineNo  ) ;
		virtual void TimerProc(){ return;  }
	protected:
		BOOL GetDevData( ) ;
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath, char *ip ) ;	//该IP是libiec61850内部连接的ip，也是管理机配置的IP，但是端口号与配置的端口号无关，一致使用102!

		char host_name[32];

};/*}}}*/

#endif // CPROTOCOL_MODBUS_H
