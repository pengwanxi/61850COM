
#ifndef CPROTOCOL_3762_H
#define CPROTOCOL_3762_H

#include "Dlt645_2007.h"

using namespace std;
extern "C" void GetCurrentTime( REALTIME *pRealTime );


class CProtocol_3762 : public CDlt645_2007
{
    public:
        CProtocol_3762();
        virtual ~CProtocol_3762();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		BYTE m_sendmsg_sequence;
};


#endif // CPROTOCOL_Dlt645_H

