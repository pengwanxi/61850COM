#include "ModBusLGT8000E9S4 .h"
#include  <math.h>

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


CModBusGT8000E9S4::CModBusGT8000E9S4()
{
	m_bySendPos = 0;
	m_byRecvflag = 0;
	m_byRecvCount = 0;
	m_bLinkStatus = 0;
	m_bySendCount = 0;
	m_specialflag = 0;
	m_log.setLogKey("GT8000EAS4");
}


CModBusGT8000E9S4::~CModBusGT8000E9S4()
{
}

BOOL CModBusGT8000E9S4::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg /*= NULL*/)
{
	char szBuf[256] = "";
	BYTE byRegisterNum = 0;
	WORD wStartReguster = 0;
	m_byRecvflag = 0;
	//if (m_specialflag == 0)
	//{
	//	//��ȡС�����ǼĴ���
	//	wStartReguster = 11;
	//	byRegisterNum = 2;
	//	m_byRecvflag = 0;
	//}
	//else{
		switch (m_bySendPos)
		{
			case 0:	//A���ѹ��B���ѹ��C���ѹ��AB�ߵ�ѹ��	BC�ߵ�ѹ��CA�ߵ�ѹ��A�������B�������C�����
				wStartReguster = 0x00;
				byRegisterNum = 18;
				m_byRecvflag = 1;
				break;
			case 1://�����й�����
				wStartReguster = 24;
				byRegisterNum = 2;
				m_byRecvflag = 2;
				break;
			case 2://�����޹�����
				wStartReguster = 32;
				byRegisterNum = 2;
				m_byRecvflag = 3;
				break;
			case 3://�������ڹ���
				wStartReguster = 40;
				byRegisterNum = 2;
				m_byRecvflag = 4;
				break;
			case 4://�ܹ������� Ƶ��
				wStartReguster = 48;
				byRegisterNum = 4;
				m_byRecvflag = 5;
				break;
			case 5://�����й��������й��������޹��������޹�
				wStartReguster = 52;
				byRegisterNum = 8;
				m_byRecvflag = 6;
				break;
			default:
			{
				 sprintf(szBuf, "%s", "CModBusGT8000E9S4 send buf err !!!\n");
				 OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			}
		//}
	}
	len = 0;
	buf[len++] = m_wDevAddr;   //��ַ
	buf[len++] = 0x03;   //����
	buf[len++] = HIBYTE(wStartReguster);
	buf[len++] = LOBYTE(wStartReguster);
	buf[len++] = 0x00;
	buf[len++] = byRegisterNum;

	WORD wCRC = GetCrc(buf, len);
	buf[len++] = HIBYTE(wCRC);
	buf[len++] = LOBYTE(wCRC);

	m_bySendCount++;
	m_bySendPos++;
	if (m_bySendPos >6)
		m_bySendPos = 0;
	return TRUE;
}

BOOL CModBusGT8000E9S4::ProcessProtocolBuf(BYTE * buf, int len)
{
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "CModBusGT8000E9S4 recv buf err !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

		m_byRecvCount++;
		return FALSE;
	}

	ProcessRecvBuf(buf, len);

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}

BOOL CModBusGT8000E9S4::ProcessRecvBuf(BYTE *buf, int len)
{
	int i;
	char szBuf[256];
	//if (m_byRecvflag == 0)
	//{
	//	//���㹦�ʷ���λ������С����λ�á�����С����λ�á���ѹС����λ�� buf[3],buf[4],buf[5],buf[6]
	//	m_specialflag = 1;

	//	if ((buf[3] & 0x01) == 0x00)m_Reactive_Sign = 1;   //�޹�����λ
	//	else if (buf[3] & 0x01 == 0x01) m_Reactive_Sign = -1;

	//	if ((buf[3] & 0x08) == 0x00) m_Active_Sign = 1;//�й�����λ
	//	else if ((buf[3] & 0x08) == 0x01) m_Active_Sign = -1;

	//	m_Dpq = buf[4];//����С����λ��

	//	m_Dct = buf[5];//����С����λ��

	//	m_Dpt = buf[6];//��ѹС����λ��

	//}
	//else 
	if (m_byRecvflag == 1)//A���ѹ\B���ѹ\C���ѹ\AB�ߵ�ѹ\BC�ߵ�ѹ\	CA�ߵ�ѹ\A�����\B�����\C�����
	{
		ProcessRecvBuf_0_8_yc(buf, len);
	}
	else if (m_byRecvflag == 2)//�����й�����
	{
		ProcessRecvBuf_9_yc(buf, len);
	}
	else if (m_byRecvflag == 3)//�����޹�����
	{
		ProcessRecvBuf_10_yc(buf, len);
	}
	else if (m_byRecvflag == 4)
	{
		ProcessRecvBuf_11_yc(buf, len);
	}
	else if (m_byRecvflag == 5)
	{
		ProcessRecvBuf_12_13_yc(buf, len);
	}
	else if (m_byRecvflag == 6)
	{
		ProcessRecvBuf_ym(buf, len);
	}
	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY::ProcessRecvBuf  ----- */

BOOL CModBusGT8000E9S4::ProcessRecvBuf_ym(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	BYTE temp_buf[4] = { 0 };
	for (int i = 0; i < 4; i++)
	{
	
		float fVal = 0.0;

		temp_buf[0] = pBuf[byIndex + 1];
		temp_buf[1] = pBuf[byIndex];
		temp_buf[2] = pBuf[byIndex + 3];
		temp_buf[3] = pBuf[byIndex + 2];

		memcpy(&fVal, temp_buf, 4);
	
		byIndex += 4;

	
		m_pMethod->SetYmData(m_SerialNo, i, (QWORD)fVal);
		byIndex += 4;
	}
}
BOOL CModBusGT8000E9S4::ProcessRecvBuf_0_8_yc(BYTE *buf, int len)//���0-8
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	BYTE temp_buf[4] = {0};
	for (int i = 0; i < byCount / 4; i++)
	{

		float fVal = 0.0;

		temp_buf[0] = pBuf[byIndex + 1];
		temp_buf[1] = pBuf[byIndex];
		temp_buf[2] = pBuf[byIndex + 3];
		temp_buf[3] = pBuf[byIndex + 2];

		memcpy(&fVal, temp_buf, 4);
	
		m_pMethod->SetYcData(m_SerialNo, i, fVal);
		printf("yc%d = %f\n", i, fVal);
		byIndex += 4;

	}
	return TRUE;
}


BOOL CModBusGT8000E9S4::ProcessRecvBuf_9_yc(BYTE *buf, int len)//���9  �����й�����
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	BYTE temp_buf[4] = { 0 };
	for (int i = 0; i < byCount / 4; i++)
	{
		float fVal = 0.0;

		temp_buf[0] = pBuf[byIndex + 1];
		temp_buf[1] = pBuf[byIndex];
		temp_buf[2] = pBuf[byIndex + 3];
		temp_buf[3] = pBuf[byIndex + 2];

		memcpy(&fVal, temp_buf, 4);
		
		byIndex += 4;
		m_pMethod->SetYcData(m_SerialNo, 9 + i, fVal);
	}
	return TRUE;
}
BOOL CModBusGT8000E9S4::ProcessRecvBuf_10_yc(BYTE *buf, int len)//���10 �����޹�����
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	BYTE temp_buf[4] = { 0 };
	for (int i = 0; i < byCount / 4; i++)
	{
		float fVal = 0.0;

		temp_buf[0] = pBuf[byIndex + 1];
		temp_buf[1] = pBuf[byIndex];
		temp_buf[2] = pBuf[byIndex + 3];
		temp_buf[3] = pBuf[byIndex + 2];

		memcpy(&fVal, temp_buf, 4);

		byIndex += 4;
		m_pMethod->SetYcData(m_SerialNo, 10 + i, fVal);
	}
	return TRUE;
}
BOOL CModBusGT8000E9S4::ProcessRecvBuf_11_yc(BYTE *buf, int len)//���11 �������ڹ�����
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	BYTE temp_buf[4] = { 0 };
	for (int i = 0; i < byCount / 4; i++)
	{
		float fVal = 0.0;

		temp_buf[0] = pBuf[byIndex + 1];
		temp_buf[1] = pBuf[byIndex];
		temp_buf[2] = pBuf[byIndex + 3];
		temp_buf[3] = pBuf[byIndex + 2];

		memcpy(&fVal, temp_buf, 4);
		byIndex += 4;
		m_pMethod->SetYcData(m_SerialNo, 11 + i, fVal);
	}
	return TRUE;
}
BOOL CModBusGT8000E9S4::ProcessRecvBuf_12_13_yc(BYTE *buf, int len)//���11 �ܹ�������\Ƶ��
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	BYTE temp_buf[4] = { 0 };
	for (int i = 0; i < byCount / 4; i++)
	{
		float fVal = 0.0;

		temp_buf[0] = pBuf[byIndex + 1];
		temp_buf[1] = pBuf[byIndex];
		temp_buf[2] = pBuf[byIndex + 3];
		temp_buf[3] = pBuf[byIndex + 2];

		memcpy(&fVal, temp_buf, 4);
		byIndex += 4;
		m_pMethod->SetYcData(m_SerialNo, 12 + i, fVal);
	}
	return TRUE;
}

BOOL CModBusGT8000E9S4::ProcessRecvBuf_yx(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byHidata = pBuf[3];
	BYTE byLodata = pBuf[4];
	BYTE byPnt = 0;
	for (int i = 0; i < 2; i++)
	{
		BYTE val = (byLodata >> i) & 0x01;
		m_pMethod->SetYxData(m_SerialNo, byPnt, val);
		byPnt++;
	}

	return TRUE;
}


/*
*--------------------------------------------------------------------------------------
*      Method:  WhetherBufValue
* Description:  �鿴���ձ�����Ч��
*       Input:  ������ ����
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusGT8000E9S4::WhetherBufValue(BYTE *buf, int &len)
{
	BYTE *pointer = buf;
	WORD wCrc;
	char szBuf[256];
	int pos = 0;
	while (len >= 4)
	{
		//�жϵ�ַ
		if (*pointer != m_wDevAddr)
		{
			sprintf(szBuf, "CModBusGT8000E9S4 recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�жϹ�����
		if (*(pointer + 1) != 0x03)
		{
			sprintf(szBuf, "CModBusGT8000E9S4 recv funcode err recv fuccode=%d\n", *(pointer + 1));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�жϳ���
		if (*(pointer + 2) > len - 3)
		{
			sprintf(szBuf, "CModBusSCJZ9SY recv len err recv len=%d\n", *(pointer + 2));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�ж�У��
		wCrc = GetCrc(pointer, (*(pointer + 2) + 3));
		if (*(pointer + (*(pointer + 2) + 3)) != HIBYTE(wCrc)
			|| *(pointer + (*(pointer + 2) + 4)) != LOBYTE(wCrc))
		{
			sprintf(szBuf, "CModBusSCJZ9SY recv crc err recvcrc=%.2x%.2x, localcrc=%.2x%.2x\n",
				*(pointer + (*(pointer + 2) + 3)), *(pointer + (*(pointer + 2) + 4)),
				HIBYTE(wCrc), LOBYTE(wCrc));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		buf = buf + pos;
		len = *(pointer + 2) + 5;
		return TRUE;
	DEFAULT:
		pointer++;
		len--;
		pos++;
	}
	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY::WhetherBufValue  ----- */

BOOL CModBusGT8000E9S4::GetDevCommState()
{
	if (m_bLinkStatus)
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}

	//return false;
}

void  CModBusGT8000E9S4::TimerProc(void)
{
	if (m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if (m_bLinkStatus)
		{
			m_bLinkStatus = FALSE;
		}
	}
}		/* -----  end of method ModBusLiuLiangJi::TimerProc  ----- */