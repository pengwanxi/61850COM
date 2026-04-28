#ifndef CPROTOCOL_MODBUS_H
#define CPROTOCOL_MODBUS_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"

class CProtocol_ModBus : public CProtocol
{
    public:
        CProtocol_ModBus();
        virtual ~CProtocol_ModBus();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init( BYTE byLineNo  ) ;
		virtual WORD GetCrc( BYTE * pBuf , int len );
		virtual BOOL BroadCast( BYTE * buf , int &len ) ;
		virtual void TimerProc(){ return;  } 
	protected:
		BOOL GetDevData( ) ;
	protected:
		BOOL ProcessFileData( CProfile &profile );

		BOOL CreateModule(int iModule, int iSerialNo, WORD iAddr, char * sName, char * stplatePath, char *szSysid);
};

#endif // CPROTOCOL_MODBUS_H