#ifndef CPROTOCOL_DDB_H
#define CPROTOCOL_DDB_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
#include "../../librtdb/rdbObj.h"
#include "../../share/rdbDef.h"
#include <time.h>
#include <sys/time.h>


class CProtocol_DDB : public CProtocol
{
    public:
        CProtocol_DDB();
        virtual ~CProtocol_DDB();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init( BYTE byLineNo  ) ;
		virtual void TimerProc(){ return;  } 

	protected:
		BOOL GetDevData( ) ;
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule ,  WORD iAddr , char * sName , char * stplatePath ) ;
		BOOL BroadCast( BYTE * buf , int &len );
};


#endif // CPROTOCOL_DDB_H
