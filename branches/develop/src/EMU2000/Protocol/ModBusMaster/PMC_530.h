/*
 *	Item	 : 兰大二院PMC_530A通讯特殊组件.
 * 	Filename : PMC_530.cpp
 *	Finished : 2016-11-10
 *	Author	 : zhaochunyun
 *	Funftion : 监测遥测、遥脉、遥信实时数据。遥控命令下发
 *	Explaintaion : 遥测、遥脉、遥信符合标准ModBus协议
 *	遥控属于特殊协议，四个寄存器控制分别对应四条命令。分别是合预置、合执行、分预置、分执行。本文档采用单点遥控的方式实现。
 *
 */
#ifndef PMC_530_H
#define PMC_530_H

#include "CProtocol_ModBusMaster.h"
typedef unsigned int UINT;
typedef unsigned short int USHORT;

class PMC_530 : public CProtocol_ModBusMaster{
	public:
		PMC_530();
		virtual ~PMC_530();

		BYTE m_byPortStatus;
		int m_wErrorTimer;
		int loopflag;
		const UINT TEST;
		const USHORT TEST0;
		const int ERROR_CONST;
		const int COMSTATUS_ONLINE;
		const int COMSTATUS_FAULT;
		DWORD yktype;
		YK_DATA *yk_data;
		BYTE byBusNo;
		WORD wDevNo;
		WORD wPnt;
		BYTE byVal;

		virtual BOOL GetProtocolBuf(BYTE*, int &, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf(BYTE *, int);
		virtual BOOL Init(BYTE);

		virtual void TimerProc();
		virtual BOOL GetDevCommState();
		virtual BOOL ResolvYkFrame(BYTE *, int &);
		virtual void ResolvYcFrame(BYTE *);
		virtual void ResolvYxFrame(BYTE *);
		virtual void ResolvYmFrame(BYTE *);
		virtual BOOL GetYKBuffer(BYTE *, int &, PBUSMSG);
};

#endif
