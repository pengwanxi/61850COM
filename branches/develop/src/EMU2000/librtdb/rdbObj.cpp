/**********************************************************************
  rdbObj.cpp : implementation file
  Copyright (C): 2013 by houpeng
 ***********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <signal.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

#include <time.h>
#include <sys/time.h>
#include <cmath>

#include "rdbObj.h"
#include "../share/gDataType.h"
#include "../share/global.h"

#define ONE_EXTEND_PAGE 4096	   // 4K
#define MAX_EXTEND_SIZE 0x01000000 // 16M
#define RTDBSER_SOE_VAL 1		   // ֻ����SOE��ֵ
#define RTDBSER_YX_VAL 0		   // ֻ����YX��ֵ

extern "C" char *GetWorkPath();
extern "C" void OutPromptText(char *lpszText);
extern "C" void LogPromptText(const char *fmt, ...);
extern "C" void Get_System_Time(long *ts, unsigned short *ms);
extern void Get_ACSI_Timestamp(ACSI_TIMESTAMP *ts, void *p);
/*****************************************************************************/
typedef struct tagSTNDEF
{
	WORD wNum;
	char szName[16]; // װ������
	WORD wAICount;	 // ң��
	WORD wDICount;	 // ң��
	WORD wDOCount;	 // ң��
	WORD wPICount;	 // ң��
	WORD wDZCount;	 // �������
} STNDEF;

/*lel*/
typedef struct tagSTNBusAddr
{
	BYTE byBusNo;  // ����
	WORD wDevAddr; // ��ַ

} STNBUSADDR;

STNBUSADDR g_StnBusAddr[MAX_STN_SUM];
/*end*/

int g_nExtendSize = 0;
STNDEF g_StnDef[MAX_STN_SUM];

/*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

	void SetDefaultConfig()
	{
		int i;
		STNDEF *pObj;
		for (i = 0; i < MAX_STN_SUM; i++)
		{
			pObj = &g_StnDef[i];
			pObj->wNum = (WORD)i;
			sprintf(pObj->szName, "Unit%02d", i);
			pObj->wAICount = 32;
			pObj->wDICount = 32;
			pObj->wDOCount = 4;
			pObj->wPICount = 4;
			pObj->wDZCount = 1;
		}
		/*lel*/
		STNBUSADDR *pStnBusAddr;
		for (i = 0; i < MAX_STN_SUM; i++)
		{
			pStnBusAddr = &g_StnBusAddr[i];
			pStnBusAddr->byBusNo = 0;
			pStnBusAddr->wDevAddr = 0;
		}
		/*end*/
	}

#define CONFIG_STATION_SUM 0x1001
#define CONFIG_RDBASE_SIZE 0x1002
#define CONFIG_EXTEND_SIZE 0x1003

#define CONFIG_STN_PARAM 0x2001
#define CONFIG_AI_PARAM 0x2101
#define CONFIG_DI_PARAM 0x2102
#define CONFIG_PI_PARAM 0x2103
#define CONFIG_DO_PARAM 0x2104
#define CONFIG_AO_PARAM 0x2105

	int ParseConfigItem(char *strItem, WORD *pwNum)
	{
		char strType[32];
		int i, nLen;

		if (strstr(strItem, "station_sum"))
			return CONFIG_STATION_SUM;
		if (strstr(strItem, "rdbase_size"))
			return CONFIG_RDBASE_SIZE;
		if (strstr(strItem, "extend_size"))
			return CONFIG_EXTEND_SIZE;
		i = 0;
		nLen = strlen(strItem);
		while (!isdigit(strItem[i]) && i < (int)sizeof(strType))
		{
			strType[i] = toupper(strItem[i]);
			if (++i >= nLen)
				break;
		}
		strType[i] = '\0';
		if (i >= nLen)
			*pwNum = 0;
		else
			*pwNum = (WORD)atoi(&strItem[i]);
		if (strcmp(strType, "STN") == 0)
			return CONFIG_STN_PARAM;
		if (strcmp(strType, "AI") == 0)
			return CONFIG_AI_PARAM;
		if (strcmp(strType, "DI") == 0)
			return CONFIG_DI_PARAM;
		if (strcmp(strType, "PI") == 0)
			return CONFIG_PI_PARAM;
		if (strcmp(strType, "DO") == 0)
			return CONFIG_DO_PARAM;
		return -1;
	}

	void GetCurrentTime(REALTIME *pRealTime)
	{
		time_t lSecond;
		struct tm currTime;
		struct timeval tv;
		struct timezone tz;

		gettimeofday(&tv, &tz);
		lSecond = (time_t)(tv.tv_sec);
		localtime_r(&lSecond, &currTime);
		pRealTime->wMilliSec = tv.tv_usec / 1000;
		pRealTime->wSecond = currTime.tm_sec;
		pRealTime->wMinute = currTime.tm_min;
		pRealTime->wHour = currTime.tm_hour;
		pRealTime->wDay = currTime.tm_mday;
		pRealTime->wMonth = 1 + currTime.tm_mon;
		pRealTime->wYear = 1900 + currTime.tm_year;
	}

	bool IsLeapYear(int year)
	{
		if ((year % 4) != 0)
			return false;
		if ((year % 100) != 0)
			return true;
		if ((year % 400) != 0)
			return false;
		return true;
	}

#define ONE_DAY_TIME 86400
	time_t MakeSecond(unsigned short wYear, unsigned short wMonth, unsigned short wDay,
					  unsigned short wHour, unsigned short wMinute, unsigned short wSecond)
	{
		int i;
		time_t sum = 0;

		if (wMonth < 1 || wMonth > 12 ||
			wDay < 1 || wDay > 31 ||
			wHour > 23 || wMinute > 59)
			return 0;
		if (wMonth == 2)
		{
			if (IsLeapYear(wYear))
			{
				if (wDay > 29)
					return 0;
			}
			else
			{
				if (wDay > 28)
					return 0;
			}
		}
		for (i = 1970; i < wYear; i++)
		{
			if (!IsLeapYear(i))
				sum += 365 * ONE_DAY_TIME;
			else
				sum += 366 * ONE_DAY_TIME;
		}
		for (i = 1; i < wMonth; i++)
		{
			if (i == 1 || i == 3 || i == 5 || i == 7 || i == 8 || i == 10 || i == 12)
				sum += 31 * ONE_DAY_TIME;
			else if (i == 4 || i == 6 || i == 9 || i == 11)
				sum += 30 * ONE_DAY_TIME;
			else
			{
				if (IsLeapYear(wYear))
					sum += 29 * ONE_DAY_TIME;
				else
					sum += 28 * ONE_DAY_TIME;
			}
		}
		sum += (wDay - 1) * ONE_DAY_TIME;
		sum += wHour * 3600;
		sum += wMinute * 60;
		sum += wSecond;
		return sum;
	}

	int SetCurrentTime(REALTIME *pRealTime)
	{
#if 1
		if (pRealTime == NULL)
			return -1;
		struct tm t_tm;
		struct timeval tv;
		time_t t_time;
		t_tm.tm_year = pRealTime->wYear - 1900;
		t_tm.tm_mon = pRealTime->wMonth - 1;
		t_tm.tm_mday = pRealTime->wDay;
		t_tm.tm_hour = pRealTime->wHour;
		t_tm.tm_min = pRealTime->wMinute;
		t_tm.tm_sec = pRealTime->wSecond;

		t_time = mktime(&t_tm);
		tv.tv_sec = t_time;
		tv.tv_usec = pRealTime->wMilliSec * 1000;

		//	printf("!!!!!!!!!!!!!!!!!!!!!!!\n");
		//	printf("----FUNC = %s LINE = %d sec = %ld ----\n", __func__, __LINE__, tv.tv_sec);
		return settimeofday(&tv, NULL);
#else
	if (pRealTime == NULL)
		return -1;
	struct timeval tv;
	tv.tv_sec = MakeSecond(pRealTime->wYear, pRealTime->wMonth, pRealTime->wDay,
						   pRealTime->wHour, pRealTime->wMinute, pRealTime->wSecond);
	tv.tv_usec = pRealTime->wMilliSec * 1000;
	printf("@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf("----FUNC = %s LINE = %d sec = %ld ----\n", __func__, __LINE__, tv.tv_sec);
	return settimeofday(&tv, NULL);
#endif
	}

#ifdef __cplusplus
}
#endif
/******************************************************************************
 * CRTDBObj
 */
CRTDBObj::CRTDBObj()
{
	m_bInitFlag = FALSE;
	m_wRunState = 0;

	m_dwAllSize = sizeof(SHM_SPACE);
	m_dwExtSize = 0;

	m_wStnSum = 1;
	m_nAnalogSum = 0;
	m_nDigitalSum = 0;
	m_nPulseSum = 0;
	m_nRelaySum = 0;
	m_nDZSum = 0;

	m_pRTDBSpace = NULL;
}

CRTDBObj::~CRTDBObj()
{
	FreeRTDBObj();
}

void CRTDBObj::SetSpaceSize(int nExtLen)
{
	if (nExtLen < 0)
		m_dwExtSize = 0;
	else if (nExtLen == 0)
		m_dwExtSize = (DWORD)g_nExtendSize;
	else if (nExtLen > 0 && nExtLen <= MAX_EXTEND_SIZE)
	{
		m_dwExtSize = (DWORD)nExtLen;
	}
	m_dwAllSize = sizeof(SHM_SPACE) + m_dwExtSize;
}

long CRTDBObj::CreateRTDBObj(char *szPrompt)
{
	int nSize;
	nSize = (int)m_dwAllSize;
    printf("21111\n");
	if (m_MemoryObj.Create(SHMDBKEY, nSize, 0600) < 0)
		return -1;
    printf("21112\n");
	long nPos = (long)m_MemoryObj.AttachShm();
    printf("21113\n");
	if (nPos == -1)
		return -2;
    printf("21114\n");
	m_semWrite.Create(SHMDBKEY + 1);
	m_pRTDBSpace = (SHM_SPACE *)nPos;
    printf("21115\n");
	if (szPrompt)
		sprintf(szPrompt, "Memory(%d) at 0x%x", nSize, (unsigned long)m_pRTDBSpace);
    printf("21116\n");
	m_MemoryObj.InitShmVal();
    printf("21117 %p\n",m_pRTDBSpace);
	m_pRTDBSpace->dwAllSize = (DWORD)nSize;
	m_pRTDBSpace->dwExtSize = m_dwExtSize;
	m_pRTDBSpace->dwQuality = 0;
	m_pRTDBSpace->dwEdition = (DWORD)SHM_STRUCT_VER;
    printf("21118\n");

	return nPos;
}

int CRTDBObj::OpenRTDBObj(char *szPrompt)
{
	int nSize = sizeof(SHM_SPACE);
	if (m_MemoryObj.Open(SHMDBKEY, nSize, 0600) < 0)
		return -1;
	long nPos = (long)m_MemoryObj.AttachShm();
	if (nPos == -1)
		return -2;
	m_semWrite.Create(SHMDBKEY + 1);
	m_pRTDBSpace = (SHM_SPACE *)nPos;
	if (szPrompt)
		sprintf(szPrompt, "Memory(%d) at 0x%x", nSize, (unsigned long)m_pRTDBSpace);
	m_dwAllSize = m_pRTDBSpace->dwAllSize;
	m_dwExtSize = m_pRTDBSpace->dwExtSize;
	m_wStnSum = m_pRTDBSpace->sysInfo.wStnSum;
	m_nAnalogSum = m_pRTDBSpace->sysInfo.nAnalogSum;
	m_nDigitalSum = m_pRTDBSpace->sysInfo.nDigitalSum;
	m_nPulseSum = m_pRTDBSpace->sysInfo.nPulseSum;
	m_nRelaySum = m_pRTDBSpace->sysInfo.nRelaySum;
	m_nDZSum = m_pRTDBSpace->sysInfo.nAdjustSum;
	m_bInitFlag = (BOOL)m_pRTDBSpace->sysInfo.wState;

	// ���¶���ָ��ָ��  2015��03��27�� 13ʱ29��46��
	for (int i = 0; i < m_wStnSum; i++)
	{
		STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[i];
		pStnObj->pAnalogTab = &m_pRTDBSpace->RTDBase.AnalogTable[pStnObj->dwAnalogPos];
		pStnObj->pDigitalTab = &m_pRTDBSpace->RTDBase.DigitalTable[pStnObj->dwDigitalPos];
		pStnObj->pRelayTab = &m_pRTDBSpace->RTDBase.RelayTable[pStnObj->dwRelayPos];
		pStnObj->pPulseTab = &m_pRTDBSpace->RTDBase.PulseTable[pStnObj->dwPulsePos];
		pStnObj->pDzTab = &m_pRTDBSpace->RTDBase.DzTable[pStnObj->dwDZPos];
	}

	return nPos;
}

int CRTDBObj::OpenRTDBObj_Cgi(char *szPrompt)
{
	// int i;
	int nSize = sizeof(SHM_SPACE);
	if (m_MemoryObj.Open(SHMDBKEY, nSize, 0600) < 0)
		return -1;
	int nPos = (int)m_MemoryObj.AttachShm();
	if (nPos == -1)
	{
		printf("%s %s %d nPos = %d\n", __FILE__, __FUNCTION__, __LINE__, nPos);
		return -2;
	}

	m_semWrite.Create(SHMDBKEY + 1);
	m_pRTDBSpace = (SHM_SPACE *)nPos;
	if (szPrompt)
		sprintf(szPrompt, "Memory(%d) at 0x%x", nSize, (unsigned long)m_pRTDBSpace);
	m_dwAllSize = m_pRTDBSpace->dwAllSize;
	m_dwExtSize = m_pRTDBSpace->dwExtSize;
	m_wStnSum = m_pRTDBSpace->sysInfo.wStnSum;
	m_nAnalogSum = m_pRTDBSpace->sysInfo.nAnalogSum;
	m_nDigitalSum = m_pRTDBSpace->sysInfo.nDigitalSum;
	m_nPulseSum = m_pRTDBSpace->sysInfo.nPulseSum;
	m_nRelaySum = m_pRTDBSpace->sysInfo.nRelaySum;
	m_nDZSum = m_pRTDBSpace->sysInfo.nAdjustSum;
	m_bInitFlag = (BOOL)m_pRTDBSpace->sysInfo.wState;
	return nPos;
}

void CRTDBObj::RTDBInit(void)
{
	WORD i;
	char strFile[96];

	// ��ʼ�����ݿ����
	for (i = 0; i < m_wStnSum; i++)
	{
		// ��g_StnDef�е����ݷŵ��������ݿ���
		StnInit((WORD)i, &g_StnDef[i]);
		/*lel*/
		/*��ȡ���ߺź͵�ַ��*/
		StnBusAddrInit((WORD)i, &g_StnBusAddr[i]);
		/*end*/

		sprintf(strFile, "%s/Station/stn%02d.conf", GetWorkPath(), i + 1);

		// �ٴ�װ���ļ��и��������ݿ������ݸ�ֵ
		PntInit(i, strFile);
	}

	memset(m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus, 1, sizeof(m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus));
	memset(m_pRTDBSpace->RTDBase.StnComStatus.byDevComStatus, 1, sizeof(m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus));
	for (i = 0; i < 500; i++)
	{
		memset(m_pRTDBSpace->RTDBase.StnComStatus.byBusTypeAndProtocolName[i], 0, 200);
	}
	m_pRTDBSpace->sysInfo.wStnSum = m_wStnSum;
	m_pRTDBSpace->sysInfo.nAnalogSum = m_nAnalogSum;
	m_pRTDBSpace->sysInfo.nDigitalSum = m_nDigitalSum;
	m_pRTDBSpace->sysInfo.nRelaySum = m_nRelaySum;
	m_pRTDBSpace->sysInfo.nPulseSum = m_nPulseSum;
	m_pRTDBSpace->sysInfo.nAdjustSum = m_nDZSum;
	m_pRTDBSpace->sysInfo.nSOEWritePos = 0;
	m_pRTDBSpace->sysInfo.nAIEWritePos = 0;
	m_pRTDBSpace->sysInfo.wState = 1;
	m_pRTDBSpace->sysInfo.nIsDuty = 1;
	m_bInitFlag = TRUE;
}

void CRTDBObj::ReadConfig(LPCSTR lpszFile)
{
	int k;
	FILE *hFile;
	char strLine[192];
	char *pItem, *pParam;
	WORD wNum;

	SetDefaultConfig();
	// ��������Ϣ
	hFile = fopen(lpszFile, "r");
	if (hFile != NULL)
	{
		LogPromptText("Open file %s ok.\n", lpszFile);
		while (fgets(strLine, sizeof(strLine), hFile))
		{
			// ltrim(strLine);
			if (strLine[0] == ';' || strLine[0] == '#')
				continue;
			// �����ı���
			pItem = strtok(strLine, "=");
			if (pItem == NULL)
				continue;
			pParam = strtok(NULL, "=");
			if (pParam == NULL)
				continue;
			// ����������
			int nType = ParseConfigItem(pItem, &wNum);
			switch (nType)
			{
			case CONFIG_STATION_SUM:
				m_wStnSum = (WORD)atoi(pParam);
				if (m_wStnSum > MAX_STN_SUM)
					m_wStnSum = MAX_STN_SUM;
				break;
			case CONFIG_RDBASE_SIZE:
				k = atoi(pParam);
				break;
			case CONFIG_EXTEND_SIZE:
				k = atoi(pParam);
				if (k > 0)
					g_nExtendSize = ONE_EXTEND_PAGE * k;
				break;
			case CONFIG_STN_PARAM:
				SetStnAttrib(wNum - 1, pParam);
				break;
			}
		}
		/*lel*/
		int j = 0;
		STNBUSADDR *pObjBusAddr = NULL;
		for (int i = 0; i < m_wStnSum; i++)
		{
			pObjBusAddr = &g_StnBusAddr[i];
			if (((i + 1) <= m_wStnSum) && (pObjBusAddr->byBusNo != (pObjBusAddr - 1)->byBusNo))
				j = 0;

			pObjBusAddr->wDevAddr = ++j;
		}
		/*end*/

		fclose(hFile);
	}
}

void CRTDBObj::SetStnAttrib(WORD wStn, char *szParam)
{
	int i = 0;
	if (wStn >= m_wStnSum)
		return;
	STNDEF *pObj = &g_StnDef[wStn];
	/*lel*/
	STNBUSADDR *pObjBusAddr = &g_StnBusAddr[wStn];
	/*end*/

	if (strlen(szParam) <= 0)
		return;
	char *p = strtok(szParam, ",");
	while (p)
	{
		switch (i)
		{
		case 0: // ��������
			sprintf(pObj->szName, "%s", p);
			break;
		case 1: // ң������
			pObj->wAICount = (WORD)atoi(p);
			break;
		case 2: // ң������
			pObj->wDICount = (WORD)atoi(p);
			break;
		case 3: // ң������
			pObj->wDOCount = (WORD)atof(p);
			break;
		case 4: // ��������
			pObj->wPICount = (WORD)atof(p);
			break;
		case 5: // ��ֵ����
			pObj->wDZCount = (WORD)atof(p);
			break;
		/*lel*/
		case 6: // ������Ϣ
			pObjBusAddr->byBusNo = (BYTE)atoi(p);
			break;
			/*end*/
		}
		p = strtok(NULL, ",");
		i++;
	}
}

/*lel*/
BOOL CRTDBObj::StnBusAddrInit(WORD wStn, void *pParam)
{
	STNBUS_ADDR *pStnBusAddr;
	STNBUSADDR *pStnDef = (STNBUSADDR *)pParam;
	if (wStn >= MAX_STN_SUM)
		return FALSE;
	pStnBusAddr = &m_pRTDBSpace->RTDBase.StnBusAddr[wStn];
	pStnBusAddr->byBusNo = pStnDef->byBusNo;
	pStnBusAddr->wDevAddr = pStnDef->wDevAddr;
	return TRUE;
}
/*end*/

BOOL CRTDBObj::StnInit(WORD wStn, void *pParam)
{
	int i, nCount, iIndex;
	STNPARAM *pStnObj;
	ANALOGITEM *pAiItem;
	DIGITALITEM *pDiItem;
	PULSEITEM *pPiItem;
	RELAYITEM *pDoItem;
	DZITEM *pDzItem;

	STNDEF *pStnDef = (STNDEF *)pParam;
	if (wStn >= MAX_STN_SUM)
		return FALSE;
	pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];

	pStnObj->wStnNum = wStn;
	sprintf(pStnObj->szStnName, pStnDef->szName);
	// ң������ʼ��
	if ((m_nAnalogSum + pStnDef->wAICount) > MAX_ANALOG_SUM)
	{
		// �������ʣ����
		nCount = max(0, MAX_ANALOG_SUM - m_nAnalogSum);
	}
	else
	{
		nCount = pStnDef->wAICount;
	}
	pStnObj->wAnalogSum = nCount;
	pStnObj->dwAnalogPos = m_nAnalogSum;
	pStnObj->pAnalogTab = &m_pRTDBSpace->RTDBase.AnalogTable[m_nAnalogSum];
	pAiItem = pStnObj->pAnalogTab;
	iIndex = m_nAnalogSum;
	for (i = 0; i < nCount; i++)
	{
		pAiItem->wPntID = (WORD)i;
		pAiItem->byType = 0;
		pAiItem->byUnit = 0;
		sprintf(pAiItem->szName, "%02dAI%03d", wStn + 1, i + 1);
		pAiItem->wPntCtrl = 1;
		pAiItem->wThreshold = 0;
		pAiItem->fRatio = 1.0;
		pAiItem->fOffset = 0.0f;
		pAiItem->dwRawVal = (DWORD)0;
		pAiItem->fRealVal = (float)0.0f;
		pAiItem->iTransNum = iIndex;
		pAiItem++;
		iIndex++;
	}
	m_nAnalogSum += pStnObj->wAnalogSum;
	// ң������ʼ��
	if ((m_nDigitalSum + pStnDef->wDICount) <= MAX_DIGITAL_SUM)
		nCount = pStnDef->wDICount;
	else
		nCount = max(0, MAX_DIGITAL_SUM - m_nDigitalSum);

	pStnObj->wDigitalSum = nCount;
	pStnObj->dwDigitalPos = m_nDigitalSum;
	pStnObj->pDigitalTab = &m_pRTDBSpace->RTDBase.DigitalTable[m_nDigitalSum];
	pDiItem = pStnObj->pDigitalTab;
	iIndex = m_nDigitalSum;
	for (i = 0; i < nCount; i++)
	{
		pDiItem->wPntID = (WORD)i;
		pDiItem->byType = 0;
		pDiItem->byAttr = 0;
		sprintf(pDiItem->szName, "%02dDI%03d", wStn + 1, i + 1);
		pDiItem->wPntCtrl = 1;
		pDiItem->wEvtCode = -1;
		pDiItem->wReserve = 0;
		pDiItem->wStatus = DISTATUS_VALID;
		pDiItem->iTransNum = iIndex;
		pDiItem++;
		iIndex++;
	}
	m_nDigitalSum += pStnObj->wDigitalSum;
	// ң�ض����ʼ��
	if ((m_nRelaySum + pStnDef->wDOCount) <= MAX_RELAY_SUM)
		nCount = pStnDef->wDOCount;
	else
		nCount = max(0, MAX_RELAY_SUM - m_nRelaySum);

	pStnObj->wRelaySum = nCount;
	pStnObj->dwRelayPos = m_nRelaySum;
	pStnObj->pRelayTab = &m_pRTDBSpace->RTDBase.RelayTable[m_nRelaySum];
	pDoItem = pStnObj->pRelayTab;
	iIndex = m_nRelaySum;
	for (i = 0; i < nCount; i++)
	{
		pDoItem->wPntID = (WORD)i;
		pDoItem->byType = 0;
		pDoItem->byAttr = 0;
		sprintf(pDoItem->szName, "%02dDO%03d", wStn + 1, i + 1);
		pDoItem->wPntCtrl = 1;
		pDoItem->wStatus = 0;
		pDoItem->iTransNum = iIndex;
		pDoItem++;
		iIndex++;
	}
	m_nRelaySum += pStnObj->wRelaySum;

	// ���ܶ����ʼ��
	if ((m_nPulseSum + pStnDef->wPICount) <= MAX_PULSE_SUM)
		nCount = pStnDef->wPICount;
	else
		nCount = max(0, MAX_PULSE_SUM - m_nPulseSum);
	pStnObj->wPulseSum = nCount;
	pStnObj->dwPulsePos = m_nPulseSum;
	pStnObj->pPulseTab = &m_pRTDBSpace->RTDBase.PulseTable[m_nPulseSum];
	pPiItem = pStnObj->pPulseTab;
	iIndex = m_nPulseSum;
	for (i = 0; i < nCount; i++)
	{
		pPiItem->wPntID = (WORD)i;
		pPiItem->byType = 0;
		pPiItem->byAttr = 0;
		sprintf(pPiItem->szName, "%02dPI%03d", wStn + 1, i + 1);
		pPiItem->wPntCtrl = 1;
		pPiItem->wReserve = 0;
		pPiItem->fRatio = 1.0;
		pPiItem->dwRawVal = 0;
		pPiItem->iTransNum = iIndex;
		pPiItem++;
		iIndex++;
	}
	m_nPulseSum += pStnObj->wPulseSum;

	// �������ʼ��
	m_nDZSum += pStnObj->wDZSum;
	if ((m_nDZSum + pStnDef->wDZCount) <= MAX_DZ_SUM)
		nCount = pStnDef->wDZCount;
	else
		nCount = max(0, MAX_DZ_SUM - m_nDZSum);
	pStnObj->wDZSum = nCount;
	pStnObj->dwDZPos = m_nDZSum;
	pStnObj->pDzTab = &m_pRTDBSpace->RTDBase.DzTable[m_nDZSum];
	pDzItem = pStnObj->pDzTab;
	iIndex = m_nDZSum;
	for (i = 0; i < nCount; i++)
	{
		pDzItem->wPntID = (WORD)i;
		pDzItem->byType = 0;
		pDzItem->byAttr = 0;
		sprintf(pDzItem->szName, "%02dDZ%03d", wStn + 1, i + 1);
		pDzItem->fRatio = 1.0;
		pDzItem->dwRawVal = 0;
		pDzItem->iTransNum = iIndex;
		pDzItem->fOffset = 0;
		pDzItem->fRealVal = 0;
		pDzItem++;
		iIndex++;
	}
	m_nDZSum += pStnObj->wDZSum;

	pStnObj->wStatus = 1;
	return TRUE;
}

void CRTDBObj::PntInit(WORD wStn, LPCSTR lpszFile)
{
	FILE *hFile;
	char strLine[256];
	char *pItem, *pParam;
	WORD wNum;

	if (wStn >= m_wStnSum)
		return;
	STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];
	// �����������Ϣ
	hFile = fopen(lpszFile, "r");
	if (hFile != NULL)
	{
		LogPromptText("Open file %s ok.\n", lpszFile);
		while (fgets(strLine, sizeof(strLine), hFile))
		{
			// ltrim(strLine);
			if (strLine[0] == ';' || strLine[0] == '#')
				continue;
			// �����ı���
			pItem = strtok(strLine, "=");
			if (pItem == NULL)
				continue;
			pParam = strtok(NULL, "=");
			if (pParam == NULL)
				continue;
			// ����������
			int nType = ParseConfigItem(pItem, &wNum);
			switch (nType)
			{
				// ң�⴦��
			case CONFIG_AI_PARAM:
				if ((wNum - 1) < pStnObj->wAnalogSum)
				{
					ANALOGITEM *pAIObj = GetAnalogObj(wStn, wNum - 1);
					if (pAIObj)
						SetAnalogParam(pAIObj, pParam);
				}
				break;
				// ң�Ŵ���
			case CONFIG_DI_PARAM:
				if ((wNum - 1) < pStnObj->wDigitalSum)
				{
					DIGITALITEM *pDIObj = GetDigitalObj(wStn, wNum - 1);

					if (pDIObj)
						SetDigitalParam(pDIObj, pParam);
				}
				break;
				// ң������
			case CONFIG_PI_PARAM:
				if ((wNum - 1) < pStnObj->wPulseSum)
				{
					PULSEITEM *pPIObj = GetPulseObj(wStn, wNum - 1);
					if (pPIObj)
						SetPulseParam(pPIObj, pParam);
				}
				break;
				// ң�ش���
			case CONFIG_DO_PARAM:
				if ((wNum - 1) < pStnObj->wRelaySum)
				{
					RELAYITEM *pDoObj = GetRelayObj(wStn, wNum - 1);
					if (pDoObj)
						SetRelayParam(pDoObj, pParam);
				}
				break;
			}
		}
		fclose(hFile);
	}
}

void CRTDBObj::FreeRTDBObj(void)
{
	m_bInitFlag = FALSE;
	m_wRunState = 0;
	m_wStnSum = 0;
	m_nAnalogSum = 0;
	m_nDigitalSum = 0;
	m_nPulseSum = 0;
	m_nRelaySum = 0;
	m_nDZSum = 0;

	//	m_pRTDBSpace->dwQuality = 0xffffffff;
	m_MemoryObj.DetachShm();
	m_MemoryObj.RemoveShm();
	m_pRTDBSpace = NULL;
}

void CRTDBObj::TimerProc(WORD wTick)
{
	if (m_pRTDBSpace)
	{
	}
	//	printf( "CRTDBObj::TimerProc(%d) \n", wTick );
}

const STNPARAM *CRTDBObj::GetStnObj(WORD wStn)
{
	if (!m_pRTDBSpace)
		return NULL;
	if (wStn >= MAX_STN_SUM)
		return NULL;
	return &m_pRTDBSpace->RTDBase.StnUnit[wStn];
}

const SOEITEM *CRTDBObj::GetTheSOE(int iPos)
{
	if (!m_pRTDBSpace)
		return NULL;
	if (iPos < 0 || iPos >= SOE_QUEUE_SUM)
		return NULL;
	return &m_pRTDBSpace->soeArray[iPos];
}

const AIEITEM *CRTDBObj::GetTheAIE(int iPos)
{
	if (!m_pRTDBSpace)
		return NULL;
	if (iPos < 0 || iPos >= AIE_QUEUE_SUM)
		return NULL;
	return &m_pRTDBSpace->aieArray[iPos];
}

int CRTDBObj::WriteAIEInfo(WORD wStn, WORD wPnt, int dwVal, float fVal)
{ /*{{{*/
	int nPos = m_pRTDBSpace->sysInfo.nAIEWritePos;
	AIEITEM *pItem = &m_pRTDBSpace->aieArray[nPos]; // aieArray:Ӧ������ײ�����!
	pItem->byType = 0;
	pItem->wStnID = wStn;
	pItem->wPntNum = wPnt;
	pItem->dwValue = dwVal;
	pItem->fValue = fVal;
	nPos = (nPos + 1) % AIE_QUEUE_SUM;
	m_pRTDBSpace->sysInfo.nAIEWritePos = nPos;
	return 0;
} /*}}}*/

int CRTDBObj::WriteSOEInfo1(WORD wStn, WORD wPnt, WORD wVal, WORD wAttr)
{
	int nPos = m_pRTDBSpace->sysInfo.nSOEWritePos;
	SOEITEM *pItem = &m_pRTDBSpace->soeArray[nPos];

	Get_System_Time(&pItem->lTime, &pItem->wMiSecond);
	pItem->byState = (wVal == 0) ? 0 : 1;
	pItem->wStnID = wStn;
	pItem->wPntNum = wPnt;
	pItem->wAttrib = wAttr;

	nPos = (nPos + 1) % SOE_QUEUE_SUM;
	m_pRTDBSpace->sysInfo.nSOEWritePos = nPos;
	return 0;
}

int CRTDBObj::WriteSOEInfo2(WORD wStn, WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond, WORD wAttr)
{
	int nPos = m_pRTDBSpace->sysInfo.nSOEWritePos;
	SOEITEM *pItem = &m_pRTDBSpace->soeArray[nPos];

	pItem->lTime = lTime;
	pItem->wMiSecond = wMiSecond;
	pItem->byState = (wVal == 0) ? 0 : 1;
	pItem->wStnID = wStn;
	pItem->wPntNum = wPnt;
	pItem->wAttrib = wAttr;

	// printf( "WritePos = %d\n" , nPos ) ;
	nPos = (nPos + 1) % SOE_QUEUE_SUM;
	m_pRTDBSpace->sysInfo.nSOEWritePos = nPos;

	return 0;
}

int CRTDBObj::WriteSOEInfo3(WORD wStn, WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond, WORD wAttr)
{
	int nPos = m_pRTDBSpace->sysInfo.nSOEWritePos;
	SOEITEM *pItem = &m_pRTDBSpace->soeArray[nPos];

	pItem->lTime = lTime;
	pItem->wMiSecond = wMiSecond;
	pItem->byState = wVal;
	pItem->wStnID = wStn;
	pItem->wPntNum = wPnt;
	pItem->wAttrib = wAttr;

	// printf( "WritePos = %d\n" , nPos ) ;
	nPos = (nPos + 1) % SOE_QUEUE_SUM;
	m_pRTDBSpace->sysInfo.nSOEWritePos = nPos;

	return 0;
}

/*int CRTDBObj::WriteData(unsigned char *pBuf, int nLen)
  {
  int nError = 0;
  BYTE byType  = pBuf[6];
  BYTE byStnID = pBuf[7];
  WORD wStnID = byStnID ;
  if(wStnID>=MAX_STN_SUM) return -3;
  m_semWrite.semTake();
  switch(byType)
  {
  case 0x61: //16λAI��������
  AnalogProc16(pBuf, nLen);
  break;
  case 0x62: //16λAI�仯����
  AIEItemProc16(pBuf, nLen);
  break;
  case 0x63: //32λAI��������
  AnalogProc32(pBuf, nLen);
  break;
  case 0x64: //32λAI�仯����
  AIEItemProc32(pBuf, nLen);
  break;
  case 0x65: //1�ֽ�DI��������
  DigitalProc(pBuf, nLen);
  break;
  case 0x66: //1�ֽ�DI�仯����
  DIEItemProc(pBuf, nLen);
  break;
  case 0x67: //16λ��DI�仯����
  DIEDataProc(pBuf, nLen);
  break;
  case 0x68: //��ʱ��DI����1(���ʱ��)
  TimeDIEProc1(pBuf, nLen);
  break;
  case 0x69: //SOE��ʽ1(���ʱ��)
  SOEProc1(pBuf, nLen);
  break;
  case 0x6a: //��ʱ��DI����2(����ʱ��)
  TimeDIEProc2(pBuf, nLen);
  break;
  case 0x6b: //SOE��ʽ2(����ʱ��)
  SOEProc2(pBuf, nLen);
  break;
  case 0x6c: //PI�仯����
  PulseProc32(pBuf, nLen);
  break;
  case 0x6d: //��ʱ��DI����3(ACSIʱ��)
  TimeDIEProc3(pBuf, nLen);
  break;
  case 0x6e: //4�ֽڸ�����AI��������
  AnalogRealProc1(pBuf, nLen);
  break;
  case 0x6f: //4�ֽڸ�����AI�仯����
  AIEItemRealProc1(pBuf, nLen);
  break;
  case 0x71: //4�ֽڸ�����AI��������(8�ֽ�ʱ��ͷ)
  AnalogRealProc2(pBuf, nLen);
  break;
  case 0x72: //4�ֽڸ�����AI�仯����(8�ֽ�ʱ��ͷ)
  AIEItemRealProc2(pBuf, nLen);
  break;
  }
  m_semWrite.semGive();
  return nError;
  }

  int CRTDBObj::AnalogProc16(unsigned char *pBuf, int nLen)
  {
  int   i, nSize;
  BYTE  byStn;
  WORD  wPnt;
short wVal;

nSize = nLen-10;
byStn = pBuf[7];
wPnt = *((WORD*)&pBuf[8]);
for( i=0; i<nSize; i+=2, wPnt++ )
{
	wVal = *((WORD*)&pBuf[i+10]);
	WriteAIVal(byStn, wPnt, wVal);
}
return 0;
}

int CRTDBObj::AIEItemProc16(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt;
	short wVal;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=4 )
	{
		wPnt = *((WORD*)&pBuf[i+8]);
		wVal = *((WORD*)&pBuf[i+10]);
		WriteAIVal(byStn, wPnt, wVal);
	}
	return 0;
}

int CRTDBObj::AnalogProc32(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt;
	int32 dwVal;

	nSize = nLen-10;
	byStn = pBuf[7];
	wPnt = *((WORD*)&pBuf[8]);
	for( i=0; i<nSize; i+=4, wPnt++ )
	{
		dwVal = *((DWORD*)&pBuf[i+10]);
		WriteAIVal(byStn, wPnt, dwVal);
	}
	return 0;
}

int CRTDBObj::AIEItemProc32(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt;
	int32 dwVal;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=6 )
	{
		wPnt  = *((WORD*)&pBuf[i+8]);
		dwVal = *((DWORD*)&pBuf[i+10]);
		WriteAIVal(byStn, wPnt, dwVal);
	}
	return 0;
}

int CRTDBObj::AnalogRealProc1(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt;
	float fVal;

	byStn = pBuf[7];
	nSize = nLen-10;
	wPnt = *((WORD*)&pBuf[8]);
	for( i=0; i<nSize; i+=4, wPnt++ )
	{
		fVal = *((float*)&pBuf[i+10]);
		SetAIVal(byStn, wPnt, fVal);
	}
	return 0;
}

int CRTDBObj::AIEItemRealProc1(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt;
	float fVal;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=6 )
	{
		wPnt = *((WORD*)&pBuf[i+8]);
		fVal = *((float*)&pBuf[i+10]);
		SetAIVal(byStn, wPnt, fVal);
	}
	return 0;
}

int CRTDBObj::AnalogRealProc2(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn, bySec;
	WORD  wPnt, wMS, wYear;
	float fVal;
	ACSI_TIMESTAMP t, *p=NULL;

	if( pBuf[8] == 1 )
	{
		wMS = *((WORD*)&pBuf[9]);
		bySec = (BYTE)(wMS/1000);
		wMS   = wMS % 1000;
		wYear = MAKEWORD(pBuf[15], (pBuf[14]&0xf0)>>4);
		t.SecondSinceEpoch = MakeSecond(wYear, pBuf[14]&0x0f, pBuf[13], pBuf[12], pBuf[11], bySec);
		t.FractionOfSecond = wMS*0x00ffffff /1000;
		t.TimeQuality = 0x0a;
		p = &t;
	}
	else if( pBuf[8] == 2 )
	{
		wMS = *((WORD*)&pBuf[9]);
		t.SecondSinceEpoch = *((LONG*)&pBuf[11]);
		t.FractionOfSecond = wMS*0x00ffffff /1000;
		t.TimeQuality = 0x0a;
		p = &t;
	}
	byStn = pBuf[7];
	nSize = nLen-10;
	wPnt = *((WORD*)&pBuf[8+8]);
	for( i=8; i<nSize; i+=4, wPnt++ )
	{
		fVal = *((float*)&pBuf[i+10]);
		SetAIVal(byStn, wPnt, fVal, p);
	}
	return 0;
}

int CRTDBObj::AIEItemRealProc2(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn, bySec;
	WORD  wPnt, wMS, wYear;
	float fVal;
	ACSI_TIMESTAMP t, *p=NULL;

	if( pBuf[8] == 1 )
	{
		wMS = *((WORD*)&pBuf[9]);
		bySec = (BYTE)(wMS/1000);
		wMS   = wMS % 1000;
		wYear = MAKEWORD(pBuf[15], (pBuf[14]&0xf0)>>4);
		t.SecondSinceEpoch = MakeSecond(wYear, pBuf[14]&0x0f, pBuf[13], pBuf[12], pBuf[11], bySec);
		t.FractionOfSecond = wMS*0x00ffffff /1000;
		t.TimeQuality = 0x0a;
		p = &t;
	}
	else if( pBuf[8] == 2 )
	{
		wMS = *((WORD*)&pBuf[9]);
		t.SecondSinceEpoch = *((LONG*)&pBuf[11]);
		t.FractionOfSecond = wMS*0x00ffffff /1000;
		t.TimeQuality = 0x0a;
		p = &t;
	}
	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=8; i<nSize; i+=6 )
	{
		wPnt = *((WORD*)&pBuf[i+8]);
		fVal = *((float*)&pBuf[i+10]);
		SetAIVal(byStn, wPnt, fVal, p);
	}
	return 0;
}

int CRTDBObj::DigitalProc(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt, wVal;

	nSize = nLen-10;
	byStn = pBuf[7];
	wPnt = *((WORD*)&pBuf[8]);
	for( i=0; i<nSize; i++, wPnt++ )
	{
		wVal = (WORD)(pBuf[i+10]);
		if( WriteDIVal(byStn, wPnt, wVal) > 0 )
			WriteSOEInfo1(byStn, wPnt, wVal);
	}
	return 0;
}

int CRTDBObj::DIEItemProc(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt, wVal;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=3 )
	{
		wPnt = *((WORD*)&pBuf[i+8]);
		wVal = (WORD)(pBuf[i+10]);
		if( WriteDIVal(byStn, wPnt, wVal) > 0 )
			WriteSOEInfo1(byStn, wPnt, wVal);
	}
	return 0;
}

int CRTDBObj::DIEDataProc(unsigned char *pBuf, int nLen)
{
	int  i, j, nSize;
	BYTE byStn;
	WORD wPnt, wVal;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=4 )
	{
		wPnt = *((WORD*)&pBuf[i+8]);
		wVal = *((WORD*)&pBuf[i+10]);
		for( j=0; j<16; j++, wPnt++ )
		{
			if( WriteDIVal(byStn, wPnt, (wVal & 0x0001)) > 0 )
				WriteSOEInfo1(byStn, wPnt, (wVal & 0x0001));
			wVal = wVal >> 1;
		}
	}
	return 0;
}

int CRTDBObj::TimeDIEProc1(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt, wVal, wMiSecond;
	LONG  lTime;
	ACSI_TIMESTAMP ts, *p=NULL;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=9 )
	{
		wPnt = *((WORD*)&pBuf[i+14]);
		wVal = (WORD)(pBuf[i+16]);
		wMiSecond = *((WORD*)&pBuf[i+8]);
		lTime     = *((LONG*)&pBuf[i+10]);
		ts.SecondSinceEpoch = lTime;
		ts.FractionOfSecond = wMiSecond*0x00ffffff/1000;
		ts.TimeQuality = 0x0a;
		p = &ts;
		//ң�ű�λ����
		if( WriteDIVal(byStn, wPnt, wVal, p) > 0 )
		{
			//SOE��¼����
			WriteSOEInfo2(byStn, wPnt, wVal, lTime, wMiSecond, 2);
		}
	}
	return 0;
}

int CRTDBObj::TimeDIEProc2(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt, wVal, wMiSecond, wYear, wMonth;
	LONG  lTime;
	REALTIME t;

	GetCurrentTime( &t );
	wYear  = (WORD)t.wYear;
	wMonth = (WORD)t.wMonth;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=9 )
	{
		wPnt = *((WORD*)&pBuf[i+14]);
		wVal = (WORD)(pBuf[i+16]);
		//ң�ű�λ����
		if( WriteDIVal(byStn, wPnt, wVal) > 0 )
		{
			//SOE��¼����
			if(pBuf[i+13] > 31) continue;
			if(pBuf[i+12] > 23) continue;
			if(pBuf[i+11] > 59) continue;
			if(pBuf[i+10] > 59) continue;
			lTime = MakeSecond(wYear, wMonth, pBuf[i+13],
					pBuf[i+12], pBuf[i+11], pBuf[i+10]);
			wMiSecond = *((WORD*)&pBuf[i+8]);
			WriteSOEInfo2(byStn, wPnt, wVal, lTime, wMiSecond, 2);
		}
	}
	return 0;
}

int CRTDBObj::TimeDIEProc3(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt, wVal, wMiSecond;
	LONG  lTime;
	ACSI_TIMESTAMP ts;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=11 )
	{
		ts   = *((ACSI_TIMESTAMP*)&pBuf[i+8]);
		wPnt = *((WORD*)&pBuf[i+16]);
		wVal = (WORD)(pBuf[i+18]);
		//ң�ű�λ����
		if( WriteDIVal(byStn, wPnt, wVal, &ts) > 0 )
		{
			//SOE��¼����
			lTime     = ts.SecondSinceEpoch;
			wMiSecond = ts.FractionOfSecond*1000/0x00ffffff;
			WriteSOEInfo2(byStn, wPnt, wVal, lTime, wMiSecond, 2);
		}
	}
	return 0;
}

int CRTDBObj::SOEProc1(unsigned char *pBuf, int nLen)
{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt, wVal, wMiSecond;
	LONG  lTime;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=9 )
	{
		wPnt = *((WORD*)&pBuf[i+14]);
		wVal = (WORD)(pBuf[i+16]);
		wMiSecond = *((WORD*)&pBuf[i+8]);
		lTime     = *((LONG*)&pBuf[i+10]);
		WriteSOEInfo2(byStn, wPnt, wVal, lTime, wMiSecond);
	}
	return 0;
}

int CRTDBObj::SOEProc2(unsigned char *pBuf, int nLen)
{*/
/*   BYTE  byStn;
	 WORD  wPnt, wVal, wMiSecond;
	 LONG  lTime;
	 BYTE byIndex = 2 ;
	 RTDBDATA *pRdata = NULL ;
	 if( pBuf == NULL )
	 return -1 ;

	 struct tm tmstruct ;
	 pRdata = (RTDBDATA *)pBuf ;
	 byStn = pRdata->byDevNum ;

	 wMiSecond = MAKEWORD( pRdata->byDataBuf[ 0 ] , pRdata->byDataBuf[ 1 ] ) ;
	 tmstruct.tm_sec = pRdata->byDataBuf[ byIndex++ ] ;
	 tmstruct.tm_min = pRdata->byDataBuf[ byIndex++ ] ;
	 tmstruct.tm_hour = pRdata->byDataBuf[ byIndex++ ] ;
	 tmstruct.tm_mday = pRdata->byDataBuf[ byIndex++ ] ;
	 tmstruct.tm_mon = pRdata->byDataBuf[ byIndex++ ] ;
	 tmstruct.tm_year = pRdata->byDataBuf[ byIndex++ ] ;

	 BYTE byH = 0 , byL = 0 ;
	 byL = pRdata->byDataBuf[ byIndex + 1 ] ;
	 byH = pRdata->byDataBuf[ byIndex ] ;
	 wPnt = MAKEWORD( byL , byH ) ;
	 byIndex += 2 ;
	 wVal = pRdata->byDataBuf[ byIndex ] ;

	 lTime = mktime( &tmstruct ) ;
	 WriteSOEInfo2(byStn, wPnt, wVal, lTime, wMiSecond);
	 */
/*	return 0;
	}

	int CRTDBObj::PulseProc32(unsigned char *pBuf, int nLen)
	{
	int   i, nSize;
	BYTE  byStn;
	WORD  wPnt;
	DWORD dwVal;

	nSize = nLen-8;
	byStn = pBuf[7];
	for( i=0; i<nSize; i+=6 )
	{
	wPnt  = *((WORD*)&pBuf[i+8]);
	dwVal = *((DWORD*)&pBuf[i+10]);
	WritePulseVal(byStn, wPnt, dwVal);
	}
	return 0;
	}

//������
int CRTDBObj::ReadData(unsigned char *pBuf, int nLen)
{
int nError = 0;
return nError;
}*/

/*==============================ģ����=======================================*/
/*=����,����,��λ,ϵ��,���,������*/
void CRTDBObj::SetAnalogParam(ANALOGITEM *pObj, char *szParam)
{
	int i = 0;
	int nLen = strlen(szParam);
	if (nLen <= 0)
		return;
	char *p = strtok(szParam, ",");
	while (p)
	{
		switch (i)
		{
		case 0: // ����
			sprintf(pObj->szName, "%s", p);
			break;
		case 1: // ����
			pObj->byType = (BYTE)atoi(p);
			break;
		case 2: // ��λ
			pObj->byUnit = (BYTE)atoi(p);
			break;
		case 3: // ϵ��
			pObj->fRatio = (double)atof(p);
			break;
		case 4: // ƫ��
			pObj->fOffset = (float)atof(p);
			break;
		case 5: // �������
			pObj->wPntCtrl = (WORD)atoi(p);
			break;
		case 6: // �仯��ֵ
			pObj->wThreshold = (WORD)atoi(p);
			break;
		}
		p = strtok(NULL, ",");
		i++;
	}
}

ANALOGITEM *CRTDBObj::GetAnalogObj(WORD wStn, WORD wPnt)
{
	if (!m_pRTDBSpace)
		return NULL;
	if (wStn >= MAX_STN_SUM)
		return NULL;
	STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];
	if (wPnt >= pStnObj->wAnalogSum)
		return NULL;
	return &pStnObj->pAnalogTab[wPnt];
}

int CRTDBObj::GetAnologName(WORD wStn, WORD wPnt, char *szName)
{ /*{{{*/
	ANALOGITEM *pObj = GetAnalogObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	sprintf(szName, pObj->szName);
	return 0;
} /*}}}*/

int CRTDBObj::WriteAIVal(WORD wStn, WORD wPnt, int32 dwVal, void *ts)
{ /*{{{*/
	ANALOGITEM *pObj = GetAnalogObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;

    if ( pObj->update == 0) {
        pObj->update = 1;
        Get_ACSI_Timestamp(&pObj->ACSITime, ts);
        pObj->fRealVal = dwVal * pObj->fRatio + pObj->fOffset;
    }
	if (pObj->dwRawVal != dwVal)
	{
		pObj->fRealVal = dwVal * pObj->fRatio + pObj->fOffset;
        if (abs(pObj->dwRawVal - dwVal) >= pObj->wThreshold) {
            Get_ACSI_Timestamp(&pObj->ACSITime, ts);
            WriteAIEInfo((BYTE)wStn, wPnt, dwVal, pObj->fRealVal);
        }
		pObj->dwRawVal = dwVal;
	}

	return 0;
} /*}}}*/

int CRTDBObj::SetAIVal(WORD wStn, WORD wPnt, float fVal, void *ts)
{ /*{{{*/
	if ((wStn) == 55 && (wPnt) >= 200)
	{
		FILE *fp = fopen("/mnt/szve.txt", "at+");
		fprintf(fp, "%f  ", fVal);
		fclose(fp);
	}
	ANALOGITEM *pObj = GetAnalogObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
    // printf("%s %p %d\n", "SetAi", ts, wPnt);
    if ( pObj->update == 0) {
        pObj->update = 1;
        Get_ACSI_Timestamp(&pObj->ACSITime, ts);
    }
    // printf("%s %d\n", "SetAi", pObj->ACSITime.SecondSinceEpoch);
	if (pObj->fRealVal != fVal)
	{
		if (pObj->fRatio != 0)
		{
			int32 dwVal = (int32)((fVal - pObj->fOffset) / pObj->fRatio);
            // printf("SetAIVal: wStn=%d, wPnt=%d, fVal=%f, dwVal=%d wThreshold=%d\n", wStn, wPnt, fVal, dwVal, pObj->wThreshold);
            // printf("SetAIVal: wStn=%d, wPnt=%d, fVal=%f, dwVal=%d %d wThreshold=%d\n", wStn, wPnt, fVal,pObj->dwRawVal,dwVal, pObj->wThreshold);
			if (abs(pObj->dwRawVal - dwVal) >= pObj->wThreshold)
			{
                Get_ACSI_Timestamp(&pObj->ACSITime, ts);
				if ((wStn) == 55 && (wPnt) >= 200)
				{
					FILE *fp1 = fopen("/mnt/szve.txt", "at+");
					fprintf(fp1, "%f  ", fVal);
					fclose(fp1);
				}
				WriteAIEInfo(wStn, wPnt, dwVal, fVal);
				pObj->fRealVal = fVal;
			}
			pObj->dwRawVal = dwVal;
		}
	}
	return 0;
} /*}}}*/

float CRTDBObj::GetAIRipeVal(WORD wStn, WORD wPnt, int32 dwVal)
{ /*{{{*/
	float fVal = 0;
	ANALOGITEM *pObj = GetAnalogObj(wStn, wPnt);
	if (pObj)
		fVal = dwVal * pObj->fRatio + pObj->fOffset;
	return fVal;
} /*}}}*/

/*==============================ң����=======================================*/
/*=����,����,����,������*/
void CRTDBObj::SetDigitalParam(DIGITALITEM *pObj, char *szParam)
{ /*{{{*/
	int i = 0;
	int nLen = strlen(szParam);
	if (nLen <= 0)
		return;
	char *p = strtok(szParam, ",");
	while (p)
	{
		switch (i)
		{
		case 0: // ����
			sprintf(pObj->szName, "%s", p);
			break;
		case 1: // ����
			pObj->byType = (BYTE)atoi(p);
			break;
		case 2: // ����
			pObj->byAttr = (BYTE)atoi(p);
			break;
		case 3: // �������
			pObj->wPntCtrl = (WORD)atoi(p);
			break;
		case 4: // �����
			pObj->wEvtCode = (short)atoi(p);
			break;
		}
		p = strtok(NULL, ",");
		i++;
	}
} /*}}}*/

DIGITALITEM *CRTDBObj::GetDigitalObj(WORD wStn, WORD wPnt)
{ /*{{{*/
	if (!m_pRTDBSpace)
		return NULL;
	if (wStn >= MAX_STN_SUM)
		return NULL;
	STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];
	if (wPnt >= pStnObj->wDigitalSum)
		return NULL;
	return &pStnObj->pDigitalTab[wPnt];
} /*}}}*/

int CRTDBObj::GetDigitalName(WORD wStn, WORD wPnt, char *szName)
{ /*{{{*/
	DIGITALITEM *pObj = GetDigitalObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	sprintf(szName, pObj->szName);
	return 0;
} /*}}}*/

int CRTDBObj::GetDigitalVal(WORD wStn, WORD wPnt, WORD *pwVal)
{ /*{{{*/
	DIGITALITEM *pObj = GetDigitalObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	*pwVal = pObj->wStatus & DISTATUS_VALUE;
	return 0;
} /*}}}*/

int CRTDBObj::WriteDIVal(WORD wStn, WORD wPnt, WORD wVal, void *ts)
{ /*{{{*/
	// printf(" wStn=%d,  wPnt=%d, wVal=%d\n", wStn, wPnt,wVal);
	DIGITALITEM *pObj = GetDigitalObj(wStn, wPnt);
	if (pObj == NULL)
		return 0;
	// if( (pObj->wPntCtrl & DICTRL_ENABLE) == 0 ) return 0;
	if (pObj->byType == 7)
	{
		return 0;
	} // ��0��1ң�żӵ������� ��˼���ǲ���������Ĵ�����

	WORD wBack = pObj->wStatus;
	if (pObj->byType == 5)
	{
		if (pObj->byAttr == 1)
			wBack = ((wVal >> 12) & 0x000f) * 1000 + ((wVal >> 8) & 0x000f) * 100 + ((wVal >> 4) & 0x000f) * 10 + (wVal & 0x000f);
		else
			wBack = wVal;
	}
	else
	{
		wBack = pObj->wStatus & ~DISTATUS_VALUE;
		if ((pObj->wPntCtrl & DICTRL_DOUBLEBIT) == 0) // ��λ��Ϣ
		{
			if (wVal != 0)
				wBack |= DIVALUE_ON;
			if ((pObj->wPntCtrl & DICTRL_OPPOSITE) != 0)
				wBack = wBack ^ DIVALUE_ON;
		}
		else // ˫λ��Ϣ
		{
			wBack |= (wVal & DISTATUS_VALUE);
			if ((pObj->wPntCtrl & DICTRL_OPPOSITE) != 0)
				wBack = wBack ^ DISTATUS_VALUE;
		}
		if ((pObj->wStatus & DISTATUS_VALID) == 0)
		{

			pObj->wStatus = wBack;
			Get_ACSI_Timestamp(&pObj->ACSITime, ts);
			pObj->wStatus |= DISTATUS_VALID;

			return 0;
		}
	}
    if ( pObj->update == 0) {
        pObj->update = 1;
        Get_ACSI_Timestamp(&pObj->ACSITime, ts);
        printf("SetDIVal: %d %d %d %d\n", wStn, wPnt, wVal,pObj->ACSITime.SecondSinceEpoch);
    }
	if (pObj->wStatus != wBack)
	{
        Get_ACSI_Timestamp(&pObj->ACSITime, ts);
		pObj->wStatus = wBack;
		return 1;
	}
	return 0;
} /*}}}*/

int CRTDBObj::SetDIVal(WORD wStn, WORD wPnt, WORD wVal)
{ /*{{{*/
	//	printf("YX:wSerialNo:%d wPnt:%d YX:%d\n", wStn, wPnt, wVal);
	DIGITALITEM *pObj = GetDigitalObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
    printf("SetDIVal: %d %d %d %d\n", wStn, wPnt, wVal,pObj->ACSITime.SecondSinceEpoch);
    if (pObj->wStatus != wVal) {
        Get_ACSI_Timestamp(&pObj->ACSITime, NULL);
    }
	pObj->wStatus = wVal;
    if ( pObj->update == 0) {
        pObj->update = 1;
        Get_ACSI_Timestamp(&pObj->ACSITime, NULL);
    }
	return 0;
} /*}}}*/

//==============================������=======================================
//=����,����,����,������
void CRTDBObj::SetPulseParam(PULSEITEM *pObj, char *szParam)
{ /*{{{*/
	int i = 0;
	int nLen = strlen(szParam);
	if (nLen <= 0)
		return;
	char *p = strtok(szParam, ",");
	while (p)
	{
		switch (i)
		{
		case 0: // ����
			sprintf(pObj->szName, "%s", p);
			break;
		case 1: // ����
			pObj->byType = (BYTE)atoi(p);
			break;
		case 2: // ����
			pObj->byAttr = (BYTE)atoi(p);
			break;
		case 3: // ϵ��
			pObj->fRatio =(double)atof(p);
			break;
		case 4: // ������
			pObj->wPntCtrl = (WORD)atoi(p);
			break;
		}
		p = strtok(NULL, ",");
		i++;
	}
} /*}}}*/

PULSEITEM *CRTDBObj::GetPulseObj(WORD wStn, WORD wPnt)
{ /*{{{*/
	if (!m_pRTDBSpace)
		return NULL;
	if (wStn >= MAX_STN_SUM)
		return NULL;
	STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];
	if (wPnt >= pStnObj->wPulseSum)
		return NULL;
	return &pStnObj->pPulseTab[wPnt];
} /*}}}*/

DZITEM *CRTDBObj::GetDzObj(WORD wStn, WORD wPnt)
{ /*{{{*/
	if (!m_pRTDBSpace)
		return NULL;
	if (wStn >= MAX_STN_SUM)
		return NULL;
	STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];
	if (wPnt >= pStnObj->wDZSum)
		return NULL;
	return &pStnObj->pDzTab[wPnt];
}

int CRTDBObj::GetPulseName(WORD wStn, WORD wPnt, char *szName)
{ /*{{{*/
	PULSEITEM *pObj = GetPulseObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	sprintf(szName, pObj->szName);
	return 0;
} /*}}}*/

int CRTDBObj::GetPulseVal(WORD wStn, WORD wPnt, QWORD *pdwVal)
{ /*{{{*/
	PULSEITEM *pObj = GetPulseObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	*pdwVal = pObj->dwRawVal; //+ by cyz!
	return 0;
} /*}}}*/

int CRTDBObj::WritePulseVal(WORD wStn, WORD wPnt, QWORD dwVal, void *ts)
{ /*{{{*/
	PULSEITEM *pObj = GetPulseObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
    if (pObj->dwRawVal != dwVal) {
        Get_ACSI_Timestamp(&pObj->ACSITime, ts);
    }
	pObj->dwRawVal = dwVal;
    if ( pObj->update == 0) {
        pObj->update = 1;
        Get_ACSI_Timestamp(&pObj->ACSITime, ts);
    }

	return 0;
} /*}}}*/

//==============================ң����=======================================
//=����,����,����,������
void CRTDBObj::SetRelayParam(RELAYITEM *pObj, char *szParam)
{ /*{{{*/
	int i = 0;
	int nLen = strlen(szParam);
	if (nLen <= 0)
		return;
	char *p = strtok(szParam, ",");
	while (p)
	{
		switch (i)
		{
		case 0: // ����
			sprintf(pObj->szName, "%s", p);
			break;
		case 1: // ����
			pObj->byType = (BYTE)atoi(p);
			break;
		case 2: // ����
			pObj->byAttr = (BYTE)atoi(p);
			break;
		case 3: // �������
			pObj->wPntCtrl = (WORD)atoi(p);
			break;
		}
		p = strtok(NULL, ",");
		i++;
	}
} /*}}}*/

RELAYITEM *CRTDBObj::GetRelayObj(WORD wStn, WORD wPnt)
{ /*{{{*/
	if (!m_pRTDBSpace)
		return NULL;
	if (wStn >= MAX_STN_SUM)
		return NULL;
	STNPARAM *pStnObj = &m_pRTDBSpace->RTDBase.StnUnit[wStn];
	if (wPnt >= pStnObj->wRelaySum)
		return NULL;
	return &pStnObj->pRelayTab[wPnt];
} /*}}}*/

int CRTDBObj::GetRelayName(WORD wStn, WORD wPnt, char *szName)
{ /*{{{*/
	RELAYITEM *pObj = GetRelayObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	sprintf(szName, pObj->szName);
	return 0;
} /*}}}*/

int CRTDBObj::GetRelayVal(WORD wStn, WORD wPnt, WORD *pwVal)
{ /*{{{*/
	RELAYITEM *pObj = GetRelayObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	*pwVal = pObj->wStatus;
	return 0;
} /*}}}*/

int CRTDBObj::SetDOVal(WORD wStn, WORD wPnt, WORD wVal)
{ /*{{{*/
	RELAYITEM *pObj = GetRelayObj(wStn, wPnt);
	if (pObj == NULL)
		return -1;
	pObj->wStatus = wVal;
	return 0;
} /*}}}*/

void CRTDBObj::WriteVal(WORD wSeriNo, BYTE byType, void *pData)
{ /*{{{*/

	if ((wSeriNo >= MAX_STN_SUM) || (pData == NULL))
		return;

	m_semWrite.semTake();
	switch (byType)
	{
	case YC_TYPE:
	{
		PYC_DATA pYcData = (PYC_DATA)pData;
		if (pYcData->fYcValue > 2000000000)
		{
			m_semWrite.semGive();
			return;
		}

		if (0 == pYcData->byYcType)
		{
			SetAIVal(pYcData->wSerialNo, pYcData->wPnt, pYcData->fYcValue);
		}
		else if (2 == pYcData->byYcType)
		{
			WORD wMS = (pYcData->Second * 1000 + pYcData->MilSecond);
			wMS = wMS % 1000;
			ACSI_TIMESTAMP t;

			t.SecondSinceEpoch = MakeSecond(pYcData->Year, pYcData->Month, pYcData->Day, pYcData->Hour, pYcData->Minute, pYcData->Second);
			t.FractionOfSecond = wMS * 0x00ffffff / 1000;
			t.TimeQuality = 0x0a;

			SetAIVal(pYcData->wSerialNo, pYcData->wPnt, pYcData->fYcValue, &t);
		}
		// DWORD dwVal = 0 ;
		// memcpy( &dwVal , &pYcData->fYcValue , sizeof( DWORD ) ) ;
		// WriteAIVal( pYcData->wSerialNo , pYcData->wPnt , (DWORD)pYcData->fYcValue ) ;
	}
	break;
	case YX_TYPE:
	{
		PYX_DATA pYxData = (PYX_DATA)pData;
		if (pYxData->byYxType == 2) // ң�Ŵ�ʱ��!
		{
			LONG lTime;
			// WORD wMiSecond = 0 ;					//of mine!
			struct tm tmstruct;
			TIMEDATA srcTime;
			srcTime.Year = pYxData->Year;
			srcTime.Month = pYxData->Month;
			srcTime.Day = pYxData->Day;
			srcTime.Hour = pYxData->Hour;
			srcTime.Minute = pYxData->Minute;
			srcTime.Second = pYxData->Second;
			// wMiSecond = pYxData->MilSecond ;		//of mine!
			SetStructTm(srcTime, tmstruct, lTime);

			// lTime = mktime( &tmstruct ) ;
			// printf ( "pYx->month=%d pYx->year=%d lTime=%ld\n", pYxData->Month, pYxData->Year,lTime );

			// printf( "SOE:wSerialNo:%d wPnt:%d YX:%d\n" , pYxData->wSerialNo , pYxData->wPnt,pYxData->YxValue ) ;
			WriteSOEInfo2(pYxData->wSerialNo, pYxData->wPnt, pYxData->YxValue, lTime, pYxData->MilSecond, RTDBSER_SOE_VAL);
		}
		else if (pYxData->byYxType == 0) // ��ͨң��!
		{
			// printf( "YX:wSerialNo:%d wPnt:%d YX:%d\n" , pYxData->wSerialNo , pYxData->wPnt,pYxData->YxValue ) ;
			DIGITALITEM *pObj = GetDigitalObj(pYxData->wSerialNo, pYxData->wPnt);
			if (pObj == NULL)
			{
				m_semWrite.semGive();
				return;
			}

			if ((pObj->wStatus & 0x0001) != pYxData->YxValue)
			{
				// printf( "----YX:wSerialNo:%d wPnt:%d YX:%d\n" , pYxData->wSerialNo , pYxData->wPnt,pYxData->YxValue ) ;
				if (WriteDIVal(pYxData->wSerialNo, pYxData->wPnt, pYxData->YxValue) > 0)
				{
					// printf( "\n YX: SOE\n " );
					WriteSOEInfo2(pYxData->wSerialNo, pYxData->wPnt, pYxData->YxValue, 0, 0, RTDBSER_YX_VAL);
				}
			}
            else {
                if (pObj->update == 0) {
                    pObj->update = 1;
                    Get_ACSI_Timestamp(&pObj->ACSITime, NULL);
                }
            }
		}
		else if (pYxData->byYxType == 5)
		{

			DIGITALITEM *pObj = GetDigitalObj(pYxData->wSerialNo, pYxData->wPnt);
			if (pObj == NULL)
			{
				m_semWrite.semGive();
				return;
			}
			// �����ر�ֵһ��
			pObj->byType = 7;

			if (!SetDIVal(pYxData->wSerialNo, pYxData->wPnt, ((WORD)pYxData->YxValue)))
			{
				WriteSOEInfo3(pYxData->wSerialNo, pYxData->wPnt, pYxData->YxValue, 0, 0, RTDBSER_YX_VAL);
			}
		}
	}
	break;
	case YM_TYPE:
	{
		PYM_DATA pYmData = (PYM_DATA)pData;
		PULSEITEM *PulInfo = GetPulseObj(pYmData->wSerialNo, pYmData->wPnt);
		if (PulInfo == NULL)
		{
			m_semWrite.semGive();
			return;
		}
		QWORD result =0;
		if( PulInfo->fRatio!=0)
		{
			result = static_cast<QWORD>(std::round(pYmData->YmValue * PulInfo->fRatio));
		}else{
			result = pYmData->YmValue;
		}
		//printf("val=%llu  ratio=%f result: %llu \n", (pYmData->YmValue), PulInfo->fRatio, result);
		WritePulseVal(pYmData->wSerialNo, pYmData->wPnt, result);
		/*
		if (result >= 0.0 && result <= 4294967295.0)
		{
			DWORD Dw_YmValue = static_cast<DWORD>(result);
			WritePulseVal(pYmData->wSerialNo, pYmData->wPnt, Dw_YmValue);
		}
		else
		{
			std::cerr << "Error: Overflow detected!" << std::endl;
		}
		*/
		break;
	}
	case YM_TYPE_DOUBLE://测试使用
	{
		PYM_DATA_DOUBLE pYmData = (PYM_DATA_DOUBLE)pData;
		PULSEITEM *PulInfo = GetPulseObj(pYmData->wSerialNo, pYmData->wPnt);
		if (PulInfo == NULL)
		{
			m_semWrite.semGive();
			return;
		}
		printf("ration:%f\n", PulInfo->fRatio);


	    double tempratio=(double)(PulInfo->fRatio);
		double result = (pYmData->YmValue) *tempratio;
		printf("val=%f  ratio=%f result: %f \n", (pYmData->YmValue), PulInfo->fRatio, result);
		if (result >= 0.0 && result <= 4294967295.0)
		{
			DWORD Dw_YmValue = static_cast<DWORD>(result);
			WritePulseVal(pYmData->wSerialNo, pYmData->wPnt, Dw_YmValue);
		}
		else
		{
			std::cerr << "Error: Overflow detected!" << std::endl;
		}

		break;
	}

	case VARSLIST_TYPE:
	{
        int nPos = m_pRTDBSpace->sysInfo.nVARLISTWritePos;
        VARSLIST*pItem = &m_pRTDBSpace->varslist[nPos];
        memcpy(pItem, pData, sizeof(VARSLIST));
        // printf( "WritePos = %d num=%d\n" , nPos, pItem->num ); ;
        nPos = (nPos + 1) % VARSLIST_MAX_NUM;
        m_pRTDBSpace->sysInfo.nVARLISTWritePos= nPos;

	}
	}
	m_semWrite.semGive();

	return;
} /*}}}*/
int CRTDBObj::countDecimalPlaces(float value)
{
	// 将 float 转换为字符串，保留足够多的小数位
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(10) << value; //设置足够高的精度
	std::string str = oss.str();

	// 查找小数点的位置
	size_t decimalPos = str.find('.');
	if (decimalPos == std::string::npos)
	{
		return 0; // 没有小数点，说明是整数
	}

	// 计算小数点后的字符数量
	return str.length() - decimalPos - 1;
}
/*****************************************************************************/
