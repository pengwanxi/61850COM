#include "ModBusEM600.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


CModBusEM600::CModBusEM600()
{
	m_bySendPos = 0;
	m_byRecvflag = 0;
	m_byRecvCount = 0;
	m_bLinkStatus = 0;
	m_bySendCount = 0;
}


CModBusEM600::~CModBusEM600()
{
}

BOOL CModBusEM600::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg /*= NULL*/)
{
	char szBuf[256] = "";
	BYTE byRegisterNum = 0;
	WORD wStartReguster = 0;
	m_byRecvflag = 0;

	if (pBusMsg != NULL)
	{
		return FALSE;
	}

	switch (m_bySendPos)
	{
	case 0:	//���ٶ�ȡ
		wStartReguster = 0x07D0;
		byRegisterNum = 24;
		m_byRecvflag = 1;
		break;
	default:
		{
			sprintf(szBuf, "%s", "CModBusEM600 send buf err !!!\n");
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			return FALSE;
		}
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
	if (m_bySendPos >=1)
		m_bySendPos = 0;

	return TRUE;
}

BOOL CModBusEM600::ProcessProtocolBuf(BYTE * buf, int len)
{
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "CModBusEM600 recv buf err !!!\n");
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

BOOL CModBusEM600::ProcessRecvBuf(BYTE *buf, int len)
{

	switch (m_byRecvflag)
	{
	case 1:
		ProcessRecvBuf_yc(buf, len);
		break;
	default:
		break;

	}

	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY::ProcessRecvBuf  ----- */


BOOL CModBusEM600::ProcessRecvBuf_yc(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[ 0 ] ;
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
//����ң��
	BYTE yxData = buf[byIndex];
	for ( int m = 0 ; m < 4 ; m++ )
	{
		BYTE val = (yxData >> m) & 0x01;
		m_pMethod->SetYxData(m_SerialNo, byPnt, val);
		byPnt++;
	}
//����ң��

	byIndex = 3 + 4;//����ң��
	byPnt = 0;
	for (int i = 0; i < byCount - 4 ; i += 2)
	{
		WORD fVal = 0;
		if (byPnt == 14 || byPnt == 12 || byPnt == 11)
		{
			short sVal = 0;
			fVal = MAKEWORD(buf[byIndex + 1], buf[byIndex]);
			memcpy(&sVal, &fVal, 2);
			m_pMethod->SetYcData(m_SerialNo, byPnt, sVal);
		}
		else
		{
			fVal = MAKEWORD(buf[byIndex + 1], buf[byIndex]);
			m_pMethod->SetYcData(m_SerialNo, byPnt, fVal);
		}

		byIndex += 2;
		byPnt++;
	}
//��������
	byIndex = 3 + 34;
	byPnt = 0;
	for (int i = 0; i < 4; i++ )
	{
		WORD HVal = 0;
		WORD LVal = 0;
		HVal = MAKEWORD(buf[byIndex + 1], buf[byIndex]);
		LVal = MAKEWORD(buf[byIndex + 3], buf[byIndex+ 2]);
		DWORD dwVal = MAKELONG(LVal, HVal);
		QWORD dVal = static_cast<QWORD>(dwVal); 
		m_pMethod->SetYmData(m_SerialNo, byPnt,  dVal);

		byIndex += 4;
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
BOOL CModBusEM600::WhetherBufValue(BYTE *buf, int &len)
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
			sprintf(szBuf, "CModBusSCJZ9SY recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�жϹ�����
		if (*(pointer + 1) != 0x03)
		{
			sprintf(szBuf, "CModBusSCJZ9SY recv funcode err recv fuccode=%d\n", *(pointer + 1));
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

BOOL CModBusEM600::GetDevCommState()
{
	return false;
}
