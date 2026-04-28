/// \文件:	NanziPDS.h
/// \概要:	NanziPDS协议头文件
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-09-11

#ifndef _NANZIPDS_H
#define _NANZIPDS_H

#include "CProtocol_NanziPDS.h"
#include <linux/can.h>
#include <time.h>

#if 0
enum
{
	YcType,
	YxType,
	SoeType,
};
#endif

class CNanziPDS : public CProtocol_NanziPDS
{
	public:
		CNanziPDS();
		virtual ~CNanziPDS();

		BYTE m_bSourceAddress;
		BYTE m_bDestinationAddress;
		BYTE m_bPriority;
		BYTE m_bPreSetSystemClockSucFlag;
		BOOL m_byLinkStatus;
		BYTE m_bySendCount;
		int m_iOnTime;
		time_t m_tLastTime;
		struct can_frame m_structFrame;
		TIMEDATA m_tWarningMessageTimeFlag;

		virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);
		virtual BOOL Init(BYTE byLineNo);
		virtual void TimerProc();

		BOOL WhetherBufValue(BYTE *buf, int len);
		BOOL PresetSystemClock(BYTE *buf, int &len);
		BOOL ClockSync(BYTE *buf, int &len);
		struct tm* GetLocalTime(time_t *timeFlag);
		int MakeFrame(BYTE *bPreBuf, BYTE *buf, int len);

		BOOL ThreePhaseCurrentYc(BYTE *buf, WORD wYcSort, WORD wSerialNo);
		BOOL PowerFactorYc(BYTE *buf, WORD wYcSort, WORD wSerialNo);
		BOOL ThreePhaseVoltageYc(BYTE *buf, WORD wYcSort, WORD wSerialNo);
		BOOL ThreePahseBetweenVoltageYc(BYTE *buf, WORD wYcSort, WORD wSerialNo);
		BOOL MainVariableTemperatureYc(BYTE *buf, WORD wYcSort, WORD wSerialNo);
		BOOL StraightFlowYc(BYTE *buf, WORD wYcSort, WORD wSerialNo);
		BOOL NormalStateQuantityMessageYx(BYTE *buf, WORD wSerialNo);
		BOOL SoeMessageDeal(BYTE *buf, WORD wSerialNo);
		BOOL ProtectiveActionInformation(BYTE *buf, WORD wSerialNo);
		virtual BOOL GetDevCommState();

	//	template <class Type>
	//	BOOL SetVal(BYTE bBype, Type Val, WORD wPnt);
};


#endif

