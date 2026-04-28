#ifndef CPROTOCOL_XIN_AO_SLAVE_MODULE_H
#define CPROTOCOL_XIN_AO_SLAVE_MODULE_H
#include "Protocol_Xin_ao_Slave.h"
#include <map> 
using namespace  std ;

typedef struct  tagDT
{
	BYTE bySerialNo;
	BYTE dataType; // 1 : yc , 2 : yx , 3 : ym 
	WORD byPnt;
	BYTE byDtIndex;
}XINAO_DT;

class CProtocol_Xin_ao_Slave_Module : public CProtocol_Xin_ao_Slave
{
    public:
        CProtocol_Xin_ao_Slave_Module();
        virtual ~CProtocol_Xin_ao_Slave_Module();

        virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		BOOL IscanSend();
		BOOL GetTransmitData(BYTE * pbuf, int &len, PBUSMSG pBusMsg);
		int GetDataFromMap(BYTE * pbuf, int index);
		BOOL GetConfirmBuf(BYTE * pbuf, int &len, PBUSMSG pBusMsg);
		BOOL GetCertificateBuf(BYTE * pbuf, int &len, PBUSMSG pBusMsg);
		WORD GetLength(int len);
		virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);
		BOOL processAFN(BYTE * pbuf, int len);
		BOOL processAFN_05(BYTE * pbuf, int len);
		BYTE GetBCD(BYTE byData);
		BYTE ToBCD(BYTE bVal);
		BOOL processAFN_06(BYTE * pbuf, int len);
		virtual BOOL Init(BYTE byLineNo);
		BOOL readServerCfg( );
		BOOL readDtData();
		BOOL AddDataToMap(char * pData, XINAO_DT * pdt);
		BOOL GetProfileData(char * pSec, char * pkey, char * poutPut, int outputsize);
		//获取校验和
		BYTE GetCs( const BYTE * pBuf , int len );
		virtual void TimerProc() ;
		
		char m_szGatewayID[ 10 ]; 
		char m_szToken[20];
		char m_szReginID[20];
		BYTE m_byRegin[3];
		BYTE m_byToken[16];
		char  m_szProtocol_DA[ 10 ];
		map<BYTE , XINAO_DT> m_protocol_DT;
		BYTE m_MSA;

		enum{ ecertificate , econfirm , etransmit,estandby};
		enum{ eyc = 1, eyx , eym };
		enum { eserialno ,edt ,epnt,edata};
		BYTE m_bySendSeq;
		BOOL m_bConfirm;
		time_t m_startTime;
		time_t m_endTime;
};

#endif // CPROTOCOL_XIN_AO_SLAVE_MODULE_H
