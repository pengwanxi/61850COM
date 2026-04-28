/******************************************************************************
 *  rdbObj.h : header file
 *  Copyright (C): 2010 by houpeng
 ******************************************************************************/
#ifndef _RTDBOBJ_H__
#define _RTDBOBJ_H__

#include "shm.h"
#include "semObj.h"
#include "../share/rdbDef.h"
#include <iostream>
using namespace std;


#define	SHMDBKEY  20193568
/******************************************************************************/
class CRTDBObj
{
	public:
		CRTDBObj();
		virtual ~CRTDBObj();

		/* Attributes */
	public:
		BOOL  m_bInitFlag;
		WORD  m_wRunState;

		DWORD m_dwAllSize;
		DWORD m_dwExtSize;
		WORD m_wStnSum;
		int  m_nAnalogSum;
		int  m_nDigitalSum;
		int  m_nRelaySum;
		int  m_nPulseSum;
		int  m_nDZSum;

		CSemObj   m_semWrite;
		CShm      m_MemoryObj;
		SHM_SPACE *m_pRTDBSpace;

	private:
		BOOL StnInit(WORD wStn, void* pParam);
		void PntInit(WORD wStn, LPCSTR lpszFile);
		/*lel*/
		BOOL StnBusAddrInit(WORD wStn, void *pParam);
		/*end*/

		int WriteAIEInfo(WORD wStn, WORD wPnt, int dwVal, float fVal);
		int WriteSOEInfo1(WORD wStn, WORD wPnt, WORD wVal, WORD wAttr=0);
		int WriteSOEInfo2(WORD wStn, WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond, WORD wAttr=1);
		int WriteSOEInfo3(WORD wStn, WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond, WORD wAttr = 1);
		int countDecimalPlaces(float value) ;
		// int AnalogProc16(unsigned char *pBuf, int nLen);
		// int AIEItemProc16(unsigned char *pBuf, int nLen);
		// int AnalogProc32(unsigned char *pBuf, int nLen);
		// int AIEItemProc32(unsigned char *pBuf, int nLen);
		// int AnalogRealProc1(unsigned char *pBuf, int nLen);
		// int AIEItemRealProc1(unsigned char *pBuf, int nLen);
		// int AnalogRealProc2(unsigned char *pBuf, int nLen);
		// int AIEItemRealProc2(unsigned char *pBuf, int nLen);
		// int DigitalProc(unsigned char *pBuf, int nLen);
		// int DIEItemProc(unsigned char *pBuf, int nLen);
		// int DIEDataProc(unsigned char *pBuf, int nLen);
		// int TimeDIEProc1(unsigned char *pBuf, int nLen);
		// int TimeDIEProc2(unsigned char *pBuf, int nLen);
		// int TimeDIEProc3(unsigned char *pBuf, int nLen);
		// int SOEProc1(unsigned char *pBuf, int nLen);
		// int SOEProc2(unsigned char *pBuf, int nLen);
		// int PulseProc32(unsigned char *pBuf, int nLen);

		/* Operations */
	public:
		void ReadConfig(LPCSTR lpszFile);
		void SetSpaceSize(int nExtLen);
		int OpenRTDBObj(char *szPrompt=NULL);
		int OpenRTDBObj_Cgi(char *szPrompt=NULL);
		long CreateRTDBObj(char *szPrompt=NULL);
		void RTDBInit(void);
		void FreeRTDBObj(void);
		void TimerProc(WORD wTick);

		void  SetStnAttrib(WORD wStn, char* szParam);
		const STNPARAM* GetStnObj(WORD wStn);
		const SOEITEM* GetTheSOE(int iPos);
		const AIEITEM* GetTheAIE(int iPos);
		// int WriteData(unsigned char *pBuf, int nLen);
		int ReadData(unsigned char *pBuf, int nLen);
		/* Analog */
		void SetAnalogParam(ANALOGITEM *pObj, char* szParam);
		ANALOGITEM* GetAnalogObj(WORD wStn, WORD wPnt);
		int GetAnologName(WORD wStn, WORD wPnt, char *szName);
		int WriteAIVal(WORD wStn, WORD wPnt, int32 dwVal, void *ts=NULL);
		int SetAIVal(WORD wStn, WORD wPnt, float fVal, void *ts=NULL);
		float GetAIRipeVal(WORD wStn, WORD wPnt, int32 dwVal);
		/* Digital */
		void SetDigitalParam(DIGITALITEM *pObj, char* szParam);
		DIGITALITEM* GetDigitalObj(WORD wStn, WORD wPnt);
		int GetDigitalName(WORD wStn, WORD wPnt, char *szName);
		int GetDigitalVal(WORD wStn, WORD wPnt, WORD* pwVal);
		int WriteDIVal(WORD wStn, WORD wPnt, WORD wVal, void *ts=NULL);
		int SetDIVal(WORD wStn, WORD wPnt, WORD wVal);
		/* Pulse */
		void SetPulseParam(PULSEITEM *pObj, char* szParam);
		PULSEITEM* GetPulseObj(WORD wStn, WORD wPnt);
		DZITEM* GetDzObj(WORD wStn, WORD wPnt);
		int GetPulseName(WORD wStn, WORD wPnt, char *szName);
		int GetPulseVal(WORD wStn, WORD wPnt, QWORD* pdwVal);
		int WritePulseVal(WORD wStn, WORD wPnt, QWORD dwVal, void *ts=NULL);
		/* Relay */
		void SetRelayParam(RELAYITEM *pObj, char* szParam);
		RELAYITEM* GetRelayObj(WORD wStn, WORD wPnt);
		int GetRelayName(WORD wStn, WORD wPnt, char *szName);
		int GetRelayVal(WORD wStn, WORD wPnt, WORD* pwVal);
		int SetDOVal(WORD wStn, WORD wPnt, WORD wVal);

		/* */
		void WriteVal( WORD wSeriNo ,BYTE byType , void * pData ) ;


};

/******************************************************************************/
#endif /* #ifndef _RTDBOBJ_H__ */
