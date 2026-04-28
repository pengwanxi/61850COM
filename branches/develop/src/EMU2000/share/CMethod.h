/*
 * =====================================================================================
 *
 *       Filename:  CMethod.h
 *
 *    Description:   ͨѶ�������ڴ湲���ռ�֮���ṩ��������
 *
 *        Version:  1.0
 *        Created:  2014��07��22�� 08ʱ27��27��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *        Company:  esdtek
 *
 * =====================================================================================
 */

#ifndef CMETHOD_H_INC
#define CMETHOD_H_INC

#include <stdio.h>
#include <stdlib.h>

#include "rdbDef.h"
#include "typedef.h"
#include "gDataType.h"

class CProtocol;
class CRTDBObj;
class CBusManger;
class CBasePort;
struct tagBusManager;
typedef tagBusManager *PBUSMANAGER;

/*
 * =====================================================================================
 *        Class:  CMethod
 *  Description:  ���� �ṩ����
 * =====================================================================================
 */
class CMethod
{ /*{{{*/
public:
	/* ====================  LIFECYCLE     ======================================= */
	CMethod()
	{
		m_pRdbObj = NULL;
		m_pRtuObj = NULL;
		m_pBusManager = NULL;
		m_pPort = NULL;
		return;
	} /* constructor      */
	virtual ~CMethod() { return; } /* destructor       */

	/* ====================  ACCESSORS     ======================================= */

	/* ====================  MUTATORS      ======================================= */
	/* ====================  public  method      ======================================= */
	// ͨ��wSerialNo ��ȡ�����ߺź���Ӧ��ַ
	virtual BOOL GetBusLineAndAddr(WORD wSerialNum, BYTE &byBusNo, WORD &wDevAddr, char *pDevName = NULL) { return FALSE; }

	// ͨ�����ߺź�װ�õ�ַ�Ż�ȡ ������
	virtual int GetSerialNo(BYTE byBusNo, WORD wDevAddr) { return -1; }

	/*==================================ң�⴦��=========================================*/
	virtual void YcUpdate(WORD SerialNo, YC_DATA YcData[], UINT YcNum) { return; }
	// ���ô���һ��ң��
	virtual void SetYcData(WORD wSerialNo, WORD wPnt, float fVal) { return; }
	// ���ô���һ����ʱ���ң��
	virtual void SetYcDataWithTime(WORD wSerialNo, WORD wPnt, float fVal, TIMEDATA *pTime) { return; }
	// ��ȡ���е�ң��ֵ
	virtual void ReadAllYcData(float *pData) { return; }
	// ��ȡ�����ڴ��е�һ��ң��Դ��ֵ
	virtual DWORD ReadYcData(WORD wSerialNO, WORD wPnt) { return 0; }
	/*==================================ң�Ŵ���=========================================*/
	virtual void YxUpdate(WORD SerialNo, YX_DATA YxData[], UINT YxNum) { return; }
	// ���ô���һ��ң�ţ�0 1��
	virtual void SetYxData(WORD wSerialNo, WORD wPnt, BYTE byVal) { return; }
	// ���ô���һ��ң��ֵ��ֵ����Ϊ0 1 2 3 ...��
	virtual void SetYxVariousData(WORD wSerialNo, WORD wPnt, WORD byVal) { return; }
	// ���ô���һ����ʱ���ң��
	virtual void SetYxDataWithTime(WORD wSerialNo, WORD wPnt, BYTE byVal, TIMEDATA *pTime) { return; }
	// ��ȡ���е�ң��ֵ
	virtual void ReadAllYxData(BYTE *pData) { return; }
	// ��ȡ�����ڴ��е�һ��ң��Դ��ֵ
	virtual int ReadYxData(WORD wSerialNO, WORD wPnt, WORD *pwVal) { return -1; }
	/*==================================ң������=========================================*/
	virtual void YmUpDate(WORD SerialNo, YM_DATA YmData[], UINT YmNum) { return; }
	// ���ô���һ��ң��
	virtual void SetYmData(WORD wSerialNO, WORD wPnt, double fVal) { return; }

	virtual void SetYmData(WORD wSerialNO, WORD wPnt, QWORD dVal) { return; }

	virtual void SetYmData_double(WORD wSerialNO, WORD wPnt, double dVal) { return; }

	virtual BOOL GetYmData(WORD wSerialNo, WORD wPnt, QWORD &dwVal) { return FALSE; }
	// ��ȡ���е��ֵ
	virtual void ReadAllYmData(QWORD *pdwData) { return; }
	// ��ȡ�����ڴ��е�һ��ң��Դ��ֵ
	virtual int ReadYmData(WORD wSerialNO, WORD wPnt, QWORD *pdwVal) { return -1; }
	/*==================================ң�ش���=========================================*/
    virtual void SetVarsListData(WORD wSerialNo, VARSLIST varslist)
    {return;
    }
    // ң��ѡ�񷵻�
	virtual void SetYkSelRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal) { return; }
	// ң��ִ�з���
	virtual void SetYkExeRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal) { return; }
	// ң��ȡ������
	virtual void SetYkCancelRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal) { return; }

	// ң��ѡ��
	virtual void SetYkSel(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal) {};
	// ң��ִ��
	virtual void SetYkExe(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal) {};
	// ң��ȡ��
	virtual void SetYkCancel(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal) {};
	/*==================================��ֵ����=========================================*/
	// �ٻ���ֵ��
	virtual void SetDzZoneCall(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// �ٻ���ֵ������
	virtual void SetDzZoneCallRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ���л�Ԥ��
	virtual void SetDzZoneSwitchPreset(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ���л�Ԥ�÷���
	virtual void SetDzZoneSwitchPresetRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ���л�ִ��
	virtual void SetDzZoneSwitchExct(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ���л�ִ�з���
	virtual void SetDzZoneSwitchExctRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ���л�ȡ��
	virtual void SetDzZoneSwitchCancel(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ���л�ȡ������
	virtual void SetDzZoneSwitchCancelRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// ��ֵ����
	virtual void SetDzZoneError(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo) {};
	// �ٻ���ֵ
	virtual void SetDzCall(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// �ٻ���ֵͨ����ʼ���
	virtual void SetDzCall_By_StartOrder(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDZStartOrder, DZ_DATA DzData[], int iDzDataNum) {};
	// �ٻ���ֵ����
	virtual void SetDzCallRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵдԤ��
	virtual void SetDzWritePreset(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵдԤ�÷���
	virtual void SetDzWritePresetRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵдִ��
	virtual void SetDzWriteExct(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵдִ�з���
	virtual void SetDzWriteExctRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵдȡ��;
	virtual void SetDzWriteCancel(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵдȡ������
	virtual void SetDzWriteCancelRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};
	// ��ֵ��
	virtual void SetDzError(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum) {};

	// ͸������
	virtual void Unvarnished(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, char *pCmd, int len, int iFlag) {}

	virtual void UnvarnishedRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, char *pCmd, int len, int iFlag) {}

	/* ====================  OPERATORS     ======================================= */
	virtual void CloseSocket(BYTE byBusLine) { return; }
	// �򿪱������׽���
	virtual void OpenSocket(BYTE byBusLine) { ; }
	virtual BOOL IsPortValid() { return FALSE; } // �ж϶˿��Ƿ����
	virtual BOOL GetDevCommState(BYTE byLineNo, WORD wDevNo) { return FALSE; }
	virtual BOOL GetDevCommState(WORD wSerialNo) { return FALSE; }
	virtual BOOL GetCommState(BYTE byLineNo) { return FALSE; }
	virtual BYTE GetToTalBusNum() { return 0; }
	virtual BYTE GetDevNum(BYTE byBusNo) { return 0; }
	// ͨ�����ߺź�ģ��ŵõ�װ�õ�ַ
	virtual WORD GetAddrByLineNoAndModuleNo(BYTE byLineNo, WORD wModuleNo) { return 0; }
	/*lel*/
	// ͨ�����ߺź�ģ��ŵõ�װ�ÿ����ַ(����)
	virtual char *GetDevNameByLineNoAndModuleNo(BYTE byLineNo, WORD wModuleNo) { return 0; }
	/*end*/
	virtual BYTE GetBusLineProtocolType(BYTE byLineNo) { return 0; }
	virtual BOOL IsSoeTime(UINT uiMilSec, UINT uiSec, UINT uiMin, UINT uiHour, UINT uiDay, UINT uiMonth, UINT uiYear) const { return FALSE; }

	// ��ȡ���вɼ����ߵ�װ��������������ת��������������װ�ã�
	virtual WORD GetGatherDevCount() { return 0; }

	// �������вɼ����ߵ�װ������
	virtual BOOL SetGatherDevCount(WORD wCount) { return FALSE; }

	// ��ȡ�����ɼ����ߵ�װ������
	virtual BYTE GetSingleGatherDevCount(BYTE byBusNo, BYTE byDevIndex = 0, WORD *pAddr = NULL) { return 0; }
	// ��ȡ����
	virtual PBUSMANAGER GetBus(BYTE byIndex) { return NULL; }

public:
	/* ====================  DATA MEMBERS  ======================================= */
	CRTDBObj *m_pRdbObj;
	CProtocol *m_pRtuObj;
	CBusManger *m_pBusManager;
	CBasePort *m_pPort;
}; /*}}}*/

#endif /* ----- #ifndef CMETHOD_H_INC  ----- */
