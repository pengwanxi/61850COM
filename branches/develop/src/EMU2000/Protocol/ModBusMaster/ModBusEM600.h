#pragma once
#include "CProtocol_ModBusMaster.h"
class CModBusEM600 : public CProtocol_ModBusMaster
{
public:
	CModBusEM600();
	~CModBusEM600();
	virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
	virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);
	BOOL ProcessRecvBuf(BYTE *buf, int len);
	BOOL ProcessRecvBuf_yc(BYTE *buf, int len);
	float GetTemprature(BYTE hdata, BYTE ldata);
	BOOL ProcessRecvBuf_yx(BYTE *buf, int len);
	BOOL WhetherBufValue(BYTE *buf, int &len);
	virtual BOOL Init(BYTE byLineNo) { return TRUE; }
	//获得装置通讯状态
	virtual BOOL GetDevCommState();
	BYTE m_bySendPos;
	BYTE m_byRecvflag;
	BYTE m_byRecvCount;
	BYTE m_bLinkStatus;
	BYTE m_bySendCount;
};

