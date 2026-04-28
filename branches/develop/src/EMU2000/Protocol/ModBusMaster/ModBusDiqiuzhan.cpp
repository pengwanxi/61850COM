#include "ModBusDiqiuzhan.h"

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


CModBusDiqiuzhan::CModBusDiqiuzhan()
{
	m_bySendPos = 0;
	m_byRecvflag = 0;
	m_byRecvCount = 0;
	m_bLinkStatus = 0;
	m_bySendCount = 0;
	m_log.setLogKey("PMW2000");
}


CModBusDiqiuzhan::~CModBusDiqiuzhan()
{
}

BOOL CModBusDiqiuzhan::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg /*= NULL*/)
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
	case 0:	//ia ib ic in
		wStartReguster =0x0308;
		byRegisterNum = 4;
		m_byRecvflag = 1;
		break;
	case 1://uab 
		wStartReguster = 0x0304;
		byRegisterNum = 1;
		m_byRecvflag = 2;
		break;
	case 2://ubc
		wStartReguster = 0x0306;
		byRegisterNum = 1;
		m_byRecvflag = 3;
		break;
	case 3://uca
		wStartReguster = 0x0305;
		byRegisterNum = 1;
		m_byRecvflag = 4;
		break;
	case 4://ua ub uc
		wStartReguster = 0x0300;
		byRegisterNum = 3;
		m_byRecvflag = 5;
		break;
	case 5://p
		wStartReguster = 0x0313;
		byRegisterNum = 1;
		m_byRecvflag = 6;
		break;
	case 6://q
		wStartReguster = 0x0317;
		byRegisterNum = 1;
		m_byRecvflag = 7;
		break;
	case 7://S
		wStartReguster = 0x030F;
		byRegisterNum = 1;
		m_byRecvflag = 8;
		break;
	case 8://pf F
		wStartReguster = 0x031B;
		byRegisterNum = 2;
		m_byRecvflag = 9;
		break;
	case 9://���й���� 
		wStartReguster = 0x0200;
		byRegisterNum = 2;
		m_byRecvflag = 10;
		break;
	case 10://���޹����
		wStartReguster = 0x0206;
		byRegisterNum = 2;
		m_byRecvflag = 11;
		break;
	case 11:
		wStartReguster = 0x0322;
		byRegisterNum = 1;
		m_byRecvflag = 12;
		break;
	default:
		{
			sprintf(szBuf, "%s", "CModBusDiqiuzhan send buf err !!!\n");
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
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
	if (m_bySendPos >11)
		m_bySendPos = 0;

	return TRUE;
}

BOOL CModBusDiqiuzhan::ProcessProtocolBuf(BYTE * buf, int len)
{
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "CModBusDiqiuzhan recv buf err !!!\n");
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

BOOL CModBusDiqiuzhan::ProcessRecvBuf(BYTE *buf, int len)
{
	int i;
	char szBuf[256];

	if (m_byRecvflag >= 1 && m_byRecvflag <= 9)
	{
		ProcessRecvBuf_yc(buf, len);
	}
	else if (m_byRecvflag >= 10 && m_byRecvflag <= 11)
	{
		ProcessRecvBuf_ym(buf, len);
	}
	else if (m_byRecvflag == 12)
	{
		ProcessRecvBuf_yx(buf, len);
	}

	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY::ProcessRecvBuf  ----- */

BOOL CModBusDiqiuzhan::ProcessRecvBuf_ym(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	if (m_byRecvflag == 10)
		byPnt = 0;
	else if (m_byRecvflag == 11)
		byPnt = 1;

	
	double fVal_H = MAKEWORD(pBuf[byIndex + 1], pBuf[byIndex])*1.0;
	double fVal_L = MAKEWORD(pBuf[byIndex + 3], pBuf[byIndex + 2]) * 1.0;

	double val = fVal_H * 10000 + fVal_L ;
	char szBuf[1024] = { 0 };
	sprintf(szBuf , "buslineNo = %d addr = %d pnt = %d %.4f\n",m_byLineNo , m_wDevAddr ,  byPnt , val);
	if (m_byLineNo == 0 /*&& m_wDevAddr == 6*/)
		m_log.writeLog(szBuf);

	m_pMethod->SetYmData(m_SerialNo, byPnt, (QWORD)val);

}

BOOL CModBusDiqiuzhan::ProcessRecvBuf_yc(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[ 0 ] ;
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 3;
	BYTE byPnt = 0;
	if (m_byRecvflag == 1)
		byPnt = 0;
	else if (m_byRecvflag == 2)
		byPnt = 4;
	else if (m_byRecvflag == 3)
		byPnt = 5;
	else if (m_byRecvflag == 4)
		byPnt = 6;
	else if (m_byRecvflag == 5)
		byPnt = 7;
	else if (m_byRecvflag == 6)
		byPnt = 10;
	else if (m_byRecvflag == 7)
		byPnt = 11; 
	else if (m_byRecvflag == 8)
		byPnt = 12; 
	else if (m_byRecvflag == 9)
		byPnt = 13; 


	for (int i = 0; i < byCount; i += 2)
	{
		float fVal = MAKEWORD( pBuf[byIndex + 1], pBuf[byIndex]);

		byIndex += 2;
		m_pMethod->SetYcData(m_SerialNo, byPnt, fVal);
		byPnt++;
	}

	return TRUE;
}

BOOL CModBusDiqiuzhan::ProcessRecvBuf_yx(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byHidata = pBuf[3];
	BYTE byLodata = pBuf[4];
	BYTE byPnt = 0;
	for ( int i = 0 ; i < 2 ; i++ )
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
BOOL CModBusDiqiuzhan::WhetherBufValue(BYTE *buf, int &len)
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
			sprintf(szBuf, "CModBusDiqiuzhan recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�жϹ�����
		if (*(pointer + 1) != 0x03)
		{
			sprintf(szBuf, "CModBusDiqiuzhan recv funcode err recv fuccode=%d\n", *(pointer + 1));
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

BOOL CModBusDiqiuzhan::GetDevCommState()
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

void  CModBusDiqiuzhan::TimerProc(void)
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