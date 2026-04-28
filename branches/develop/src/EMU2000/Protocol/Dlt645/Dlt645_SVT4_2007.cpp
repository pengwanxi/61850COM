/*
 * =====================================================================================
 *
 *       Filename:  Dlt645_SVT4_2007.cpp
 *
 *    Description:  dlt645 2007�汾Э��
 *
 *        Version:  1.0
 *        Created:  2014��11��10�� 14ʱ12��10��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *   Organization:
 *
 *		  history:
 * =====================================================================================
 */
#include <stdio.h>
#include "Dlt645_SVT4_2007.h"

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  CDlt645_SVT4
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_SVT4::CDlt645_SVT4()
{ /*{{{*/
	m_dayflag = -1;
	m_firstsendflag = 0;
	num = 0;
	InitProtocolStatus();
} /* -----  end of method CDlt645_SVT4::CDlt645_SVT4  (constructor)  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  ~CDlt645_SVT4
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_SVT4::~CDlt645_SVT4()
{
} /* -----  end of method CDlt645_SVT4::~CDlt645_SVT4  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  ProcessYcData
 * Description:  ң�⴦��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::ProcessYcData(const BYTE *buf, int len)
{ /*{{{*/
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	DWORD dwYcVal = 0;
	BYTE byflag = 0;
	const BYTE *pointer;
	if (len < 16)
		return FALSE;

	if (buf[8] != 0x91)
		return FALSE;

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	byflag = m_CfgInfo[m_bySendPos].byflag;

	pointer = buf + 14;
	while (byDataNum > 0)
	{
		float fYcVal;
		CalFormatData(pointer, byDataFormat, byDataLen, dwYcVal);
		fYcVal = (float)dwYcVal;
		m_pMethod->SetYcData(m_SerialNo, wPnt, fYcVal);

		pointer += byDataLen;
		wPnt++;
		byDataNum--;

		sprintf(m_szPrintBuf, "yc pnt:%d value:%f", wPnt, fYcVal);
		printf(" %s  \n", m_szPrintBuf);
	}
	return TRUE;
} /* -----  end of method CDlt645_SVT4::ProcessYcData  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  ProcessYmData
 * Description:  ң������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::ProcessYmData(const BYTE *buf, int len)
{ /*{{{*/
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	DWORD dwYmVal = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;
	if (len < 16)
	{
		print((char *)"len < 16");
		return FALSE;
	}

	if (buf[8] != 0x91)
	{
		sprintf(m_szPrintBuf, "buf[8]=%.2x", buf[8]);
		print(m_szPrintBuf);
		return FALSE;
	}

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;

	pointer = buf + 14;
	while (byDataNum > 0)
	{
		CalFormatData(pointer, byDataFormat, byDataLen, dwYmVal);
		m_pMethod->SetYmData(m_SerialNo, wPnt, static_cast<QWORD>(dwYmVal));

		printf("--ym--m_SerialNo=%d, wPnt=%d, dwYmVal=%d\n", m_SerialNo, wPnt, dwYmVal);
		pointer += byDataLen;
		wPnt++;
		byDataNum--;

		sprintf(m_szPrintBuf, "ym pnt:%d value:%lu", wPnt, dwYmVal);
		print(m_szPrintBuf);
	}

	return TRUE;
} /* -----  end of method CDlt645_SVT4::ProcessYmData  ----- */ /*}}}*/

BOOL CDlt645_SVT4::ProcessYxData(const BYTE *buf, int len)
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	pointer = buf + 14;

	BYTE byYxByte;
	BYTE byYxValue[8] = {0};
	printf("\n---------------------\n");
	for (int i = 0; i < 7; i++)
	{

		BYTE byYxBit = 0;
		byYxBit = ((*pointer - 0x33) >> i) & 0x01;
		if (byYxBit == 0x01)
		{
			byYxValue[i] = 1;
		}
		else
		{
			byYxValue[i] = 0;
		}
		printf("%d  ", byYxValue[i]);
	}
	printf("\n---------------------\n");

	for (int j = 0; j < byDataNum; j++)
	{
		if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 0 && byYxValue[4] == 0) // 00000
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0) // 00100
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 1 && byYxValue[1] == 0 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0) // 00101
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 0 && byYxValue[1] == 1 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0) // 00110
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 1 && byYxValue[1] == 1 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0) // 00111
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0) // 01000
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 1 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0) // 01001
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 0 && byYxValue[1] == 1 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0) // 01010
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 1 && byYxValue[1] == 1 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0) // 01011
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 1 && byYxValue[1] == 1 && byYxValue[2] == 1 && byYxValue[3] == 1 && byYxValue[4] == 0) // 01111
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 0 && byYxValue[1] == 1 && byYxValue[2] == 0 && byYxValue[3] == 0 && byYxValue[4] == 1) // 10010
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 0 && byYxValue[4] == 1) // 10000
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[5] == 0 && byYxValue[6] == 0) // 00
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else if (byYxValue[5] == 1 && byYxValue[6] == 0) // 00
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);

		else
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);

		wPnt++;
	}

	return FALSE;
}
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  ProcessBuf
 * Description:  �������ձ���
 *       Input:	 ���ջ���������
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::ProcessBuf(const BYTE *buf, int len)
{ /*{{{*/

	switch (m_byDataType)
	{
	case DLT645_YC_DATATYPE:
		print("ң������");
		ProcessYcData(buf, len);
		break;

	case DLT645_YM_DATATYPE:
		print("ң������");
		ProcessYmData(buf, len);
		break;

	case DLT645_YX_DATATYPE:
		print("ң������\n");
		ProcessYxData(buf, len);
		break;

	default:
		sprintf(m_szPrintBuf, "δ�ҵ�����������%d", m_byDataType);
		print(m_szPrintBuf);
		return FALSE;
		break;
	} /* -----  end switch  ----- */
	return TRUE;
} /* -----  end of method CDlt645_SVT4::ProcessBuf  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  IsTimeToSync
 * Description:  �Ƿ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::IsTimeToSync(void)
{ /*{{{*/
	if (m_bLinkStatus && m_bLinkTimeSyn)
	{
		m_bLinkTimeSyn = FALSE;
		return TRUE;
	}

	REALTIME curTime;
	GetCurrentTime(&curTime);

	if (12 == curTime.wHour)
	{
		if (1 > curTime.wMinute && 10 > curTime.wSecond)
		{
			if (m_bTimeSynFlag)
				return FALSE;
			else
				return TRUE;
		}
		else
		{
			m_bTimeSynFlag = FALSE;
		}
	}

	return FALSE;
} /* -----  end of method CDlt645_SVT4::IsTimeToSync  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  RequestReadData
 * Description:  ��������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::RequestReadData(BYTE *buf, int &len)
{ /*{{{*/
	len = 0;
	for (int i = 0; i < m_CfgInfo[m_bySendPos].byFENum; i++)
	{
		buf[len++] = 0xfe;
	}
	buf[len++] = 0x68;
	// ��ַλ
	for (int i = 0; i < 6; i++)
	{
		buf[len++] = m_bySlaveAddr[i];
	}
	buf[len++] = 0x68;
	buf[len++] = 0x11; // ������
	buf[len++] = 0x04; // ���ݳ���
	// 2007Ϊ4����ʶ��
	buf[len++] = m_CfgInfo[m_bySendPos].byDI0 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI1 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI2 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI3 + 0x33;
	buf[len++] = GetCs(buf + m_CfgInfo[m_bySendPos].byFENum, 14); // by cyz!
	buf[len++] = 0x16;

	return TRUE;
} /* -----  end of method CDlt645_SVT4::RequestReadData  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  TimeSync
 * Description:  ��ʱ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::TimeSync(BYTE *buf, int &len)
{ /*{{{*/
	REALTIME curTime;
	len = 0;
	for (int i = 0; i < m_CfgInfo[0].byFENum; i++)
	/*for ( int i=0; i<m_CfgInfo[m_bySendPos].byFENum; i++)*/
	{
		buf[len++] = 0xfe;
	}
	buf[len++] = 0x68;
	// ��ַλ
	for (int i = 0; i < 6; i++)
	{
		buf[len++] = 0x99;
	}
	buf[len++] = 0x68;
	buf[len++] = 0x08; // ������
	buf[len++] = 0x06; // ���ݳ���

	GetCurrentTime(&curTime);
	////2007Ϊ4����ʶ��
	// buf[len++] = (BYTE)(curTime.wSecond + 0x33);
	// buf[len++] = (BYTE)curTime.wMinute + 0x33;
	// buf[len++] = (BYTE)curTime.wHour + 0x33;
	// buf[len++] = (BYTE)curTime.wDay + 0x33;
	// buf[len++] = (BYTE)curTime.wMonth + 0x33;
	// buf[len++] = (BYTE)(curTime.wYear-2000)+ 0x33;

	buf[len++] = DEC_TO_BCD(curTime.wSecond);
	buf[len++] = DEC_TO_BCD(curTime.wMinute);
	buf[len++] = DEC_TO_BCD(curTime.wHour);
	buf[len++] = DEC_TO_BCD(curTime.wDay);
	buf[len++] = DEC_TO_BCD(curTime.wMonth);
	buf[len++] = DEC_TO_BCD((curTime.wYear - 2000));

	buf[len++] = GetCs(buf, 16);
	buf[len++] = 0x16;

	return TRUE;
} /* -----  end of method CDlt645_SVT4::TimeSync  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  GetSendBuf
 * Description:	 ��ȡ���ͱ��ĺͳ���
 *       Input:	 ���ͻ����� ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::GetSendBuf(BYTE *buf, int &len)
{ /*{{{*/
	switch (m_byDataType)
	{
	case DLT645_YC_DATATYPE:

	case DLT645_YM_DATATYPE:

	case DLT645_YX_DATATYPE:
		print("��������");
		RequestReadData(buf, len);
		break;

	case DLT645_TIME_DATATYPE:
		print("��ʱ");
		TimeSync(buf, len);
		break;

	default:
		sprintf(m_szPrintBuf, "Dlt645_2007 ��%d�����������������ô���", m_bySendPos);
		print(m_szPrintBuf);
		return FALSE;
		break;
	} /* -----  end switch  ----- */
	return TRUE;
} /* -----  end of method CDlt645_SVT4::GetSendBuf  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  InitProtocolStatus
 * Description:  ��ʼ��Э��״̬����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::InitProtocolStatus(void)
{							/*{{{*/
	m_bLinkStatus = FALSE;	// ����״̬Ϊ��
	m_bySendPos = 0;		// ����λ����0
	m_byDataType = 0;		// ��������Ϊ��
	m_byRecvErrorCount = 0; // ���մ������0
	m_bIsReSend = FALSE;	// �ط���ʶλ0
	m_bIsSending = FALSE;	// ���ͺ���1 ���պ�ֵ0
	m_bIsNeedResend = TRUE; // �Ƿ���Ҫ�ط�
	m_bTimeSynFlag = FALSE; // ��ʱ��ʶ
	m_bLinkTimeSyn = TRUE;	// װ����ͨ���ʱһ��
	// �ط�����������
	m_byReSendLen = 0;
	memset(m_byReSendBuf, 0, DLT645_MAX_BUF_LEN);

	return TRUE;
} /* -----  end of method CDlt645_SVT4::InitProtocolStatus  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  TimerProc
 * Description:  ʱ�䴦������ ��Ҫ����һЩ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDlt645_SVT4::TimerProc(void)
{ /*{{{*/

	// ���մ����������
	if (m_byRecvErrorCount > DLT645_MAX_RECV_ERR_COUNT)
	{
		sprintf(m_szPrintBuf, "recv err count %d > %d InitProtocolStatus", m_byRecvErrorCount, DLT645_MAX_RECV_ERR_COUNT);
		print(m_szPrintBuf);
		InitProtocolStatus();
	}
} /* -----  end of method CDlt645_SVT4::TimerProc  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  ProcessProtocolBuf
 * Description:	 �����յ������ݻ���
 *       Input:  ���յ������ݻ��� ���泤��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::ProcessProtocolBuf(BYTE *buf, int len)
{ /*{{{*/
	print("ProcessProtocolBuf");
	printf("--------SVT4--recv---------\n");
	for (int i = 0; i < len; i++)
	{
		printf("%02x ", buf[i]);
	}
	int pos = 0;
	BOOL bRtn = TRUE;
	if (!WhetherBufValue(buf, len, pos))
	{
		// ���Ĵ�����
		print("Dlt6456 WhetherBufValue buf Recv err!!!\n");
		m_byRecvErrorCount++;
		m_bIsReSend = TRUE;
		return FALSE;
	}
	bRtn = ProcessBuf(buf + pos, len);
	if (!bRtn)
	{
		print("�������ķ��������δ����");
	}
	// ����״̬
	m_byRecvErrorCount = 0;
	m_bLinkStatus = TRUE;
	m_bIsReSend = FALSE;
	m_byResendCount = 0;
	m_bIsSending = FALSE;

	// ������ȷ����
	return TRUE;
} /* -----  end of method CDlt645_SVT4::ProcessProtocolBuf  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�����ݻ���
 *       Input:  ������ ���������ݳ��� ������Ϣ
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{ /*{{{*/
	BOOL bRtn = TRUE;
	// ��һ�� ��ȡϵͳʱ��
	REALTIME curTime;
	GetCurrentTime(&curTime);
	if (m_dayflag != curTime.wDay && curTime.wSecond != 0) // ÿ���ʱһ�� ���ұܿ�������
	{
		printf("---------SVT4-----Curtime One  Time  Everyday-------devname=%s-----------------\n", m_sDevName);
		m_byDataType = 0;	// ����Ҫ����  1 yx 2yc 4ym
		TimeSync(buf, len); // ��ʱ
		m_dayflag = curTime.wDay;
	}
	else if (curTime.wMinute >= 1 && curTime.wMinute <= 13) // ��һ��ѯ����  ��1-13�� �����ڴ�λ��0 ��ʼ��ѯ�ɼ�
	{

		ChangeSendPos();
		if (m_firstsendflag == 0)
		{
			m_bySendPos = 0;
		}
		printf("GetSendBuf [1-13]  pos=%d----\n", m_bySendPos);
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		bRtn = GetSendBuf(buf, len);
		m_firstsendflag = 1;
	}
	else if (curTime.wMinute >= 16 && curTime.wMinute <= 28) // �ڶ���ѯ���� ��16-28�� �����ڴ�λ��0 ��ʼ��ѯ�ɼ�
	{

		ChangeSendPos();
		if (m_firstsendflag == 0)
		{
			m_bySendPos = 0;
		}
		printf("GetSendBuf [16-28]  pos=%d----\n", m_bySendPos);
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		bRtn = GetSendBuf(buf, len);

		m_firstsendflag = 1;
	}
	else if (curTime.wMinute >= 31 && curTime.wMinute <= 43) // ������ѯ���� ��31-43�� �����ڴ�λ��0 ��ʼ��ѯ�ɼ�
	{

		ChangeSendPos();
		if (m_firstsendflag == 0)
		{
			m_bySendPos = 0;
		}
		printf("GetSendBuf [31-43]  pos=%d----\n", m_bySendPos);
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		bRtn = GetSendBuf(buf, len);

		m_firstsendflag = 1;
	}
	else if (curTime.wMinute >= 46 && curTime.wMinute <= 58) // ������ѯ���� ��46-58�� �����ڴ�λ��0 ��ʼ��ѯ�ɼ�
	{

		ChangeSendPos();
		if (m_firstsendflag == 0)
		{
			m_bySendPos = 0;
		}
		printf("GetSendBuf [46-58]  pos=%d----\n", m_bySendPos);
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		bRtn = GetSendBuf(buf, len);
		m_firstsendflag = 1;
	}
	else
	{
		m_firstsendflag = 0;
		return FALSE;
	}
	printf("----SVT4--sendbuf-------------pos=%d----------\n", m_bySendPos);
	for (int j = 0; j < len; j++)
	{
		printf("%02x ", buf[j]);
	}
	printf("\n");

	return bRtn;
	/*return TRUE;*/
} /* -----  end of method CDlt645_SVT4::GetProtocolBuf  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_Cjt188
 *      Method:  GetDevNameToAddr
 * Description:  ͨ��װ�õ����ֶ�ȡͨѶ��ַ
 *       Input:  void
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::GetDevNameToAddr(void)
{ /*{{{*/
	int len = strlen(m_sDevName);
	if (len < 12)
	{
		return FALSE;
	}
	m_bySlaveAddr[0] = atoh(m_sDevName + len - 2, 2, 1);
	m_bySlaveAddr[1] = atoh(m_sDevName + len - 4, 2, 1);
	m_bySlaveAddr[2] = atoh(m_sDevName + len - 6, 2, 1);
	m_bySlaveAddr[3] = atoh(m_sDevName + len - 8, 2, 1);
	m_bySlaveAddr[4] = atoh(m_sDevName + len - 10, 2, 1);
	m_bySlaveAddr[5] = atoh(m_sDevName + len - 12, 2, 1);

	return TRUE;

} /* -----  end of method CProtocol_Cjt188::GetDevNameToAddr  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  Init
 * Description:	 ��ʼ��Э������
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::Init(BYTE byLineNo)
{ /*{{{*/
	if (byLineNo > 22)
		return FALSE;

	if (!ReadCfgInfo())
	{
		print("CDlt645_SVT4:ReadCfgInfo Err!!!\n");
		return FALSE;
	}

	if (!InitProtocolStatus())
	{
		print("CDlt645_SVT4:InitProtocolStatus Err\n");
		return FALSE;
	}
	print("Dlt645 Init OK");
	return TRUE;
} /* -----  end of method CDlt645_SVT4::Init  ----- */ /*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_SVT4
 *      Method:  GetDevCommState
 * Description:	 ����װ������״̬
 *       Input:
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_SVT4::GetDevCommState(void)
{ /*{{{*/
	if (m_bLinkStatus)
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
} /* -----  end of method CDlt645_SVT4::GetDevCommState  ----- */ /*}}}*/
