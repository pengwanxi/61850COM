#include "CProtocol_ModBusMaster.h"
#include "../../share/Clog.h"
class CModBusGT8000E9S4 : public CProtocol_ModBusMaster
{
public:
	CModBusGT8000E9S4();
	~CModBusGT8000E9S4();
	virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
	virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);
	BOOL ProcessRecvBuf(BYTE *buf, int len);
	BOOL ProcessRecvBuf_ym(BYTE *buf, int len);
	BOOL ProcessRecvBuf_0_8_yc(BYTE *buf, int len);
	BOOL ProcessRecvBuf_9_yc(BYTE *buf, int len);
	BOOL ProcessRecvBuf_10_yc(BYTE *buf, int len);
	BOOL ProcessRecvBuf_11_yc(BYTE *buf, int len);
	BOOL ProcessRecvBuf_12_13_yc(BYTE *buf, int len);
	
	float GetTemprature(BYTE hdata, BYTE ldata);
	BOOL ProcessRecvBuf_yx(BYTE *buf, int len);
	BOOL WhetherBufValue(BYTE *buf, int &len);
	virtual BOOL Init(BYTE byLineNo) { return TRUE; }
	virtual void TimerProc();
	//获得装置通讯状态
	virtual BOOL GetDevCommState();
	BYTE m_bySendPos;
	BYTE m_byRecvflag;
	BYTE m_byRecvCount;
	BYTE m_bLinkStatus;
	BYTE m_bySendCount;
	Clog m_log;
	BYTE m_specialflag;//特殊寄存器如小数点位置 读取标记
    short  m_Reactive_Sign;//无功符号位
    short m_Active_Sign;//有功符号位
	short m_Apparent_Sign;//视在功率符号位
	BYTE m_Dpq;//功率小数点位置
	BYTE m_Dct;//电流小数点位置
	BYTE m_Dpt;//电压小数点位置


};

