#include "ModBusZhenZhouWenkongyi.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


CModBusZhenZhouWenkongyi::CModBusZhenZhouWenkongyi()
{
	m_bySendPos = 0;
	m_byRecvflag = 0;
	m_byRecvCount = 0;
	m_bLinkStatus = 0;
	m_bySendCount = 0;
}


CModBusZhenZhouWenkongyi::~CModBusZhenZhouWenkongyi()
{
}

BOOL CModBusZhenZhouWenkongyi::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg /*= NULL*/)
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
	case 0:	//yc：
		wStartReguster = 0;
		byRegisterNum = 4;
		m_byRecvflag = 1;
		break;
	case 1://yx：
		wStartReguster = 0;
		byRegisterNum = 1;
		m_byRecvflag = 2;
		break;
	default:
		{
			sprintf(szBuf, "%s", "CModBusZhenZhouWenkongyi send buf err !!!\n");
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		}
	}

	len = 0;
	buf[len++] = m_wDevAddr;   //地址
	buf[len++] = 0x03;   //命令
	buf[len++] = HIBYTE(wStartReguster);
	buf[len++] = LOBYTE(wStartReguster);
	buf[len++] = 0x00;
	buf[len++] = byRegisterNum;

	WORD wCRC = GetCrc(buf, len);
	buf[len++] = HIBYTE(wCRC);
	buf[len++] = LOBYTE(wCRC);

	m_bySendCount++;
	m_bySendPos++;
	if (m_bySendPos >1)
		m_bySendPos = 0;

	return TRUE;
}

BOOL CModBusZhenZhouWenkongyi::ProcessProtocolBuf(BYTE * buf, int len)
{
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "CModBusZhenZhouWenkongyi recv buf err !!!\n");
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

BOOL CModBusZhenZhouWenkongyi::ProcessRecvBuf(BYTE *buf, int len)
{
	int i;
	char szBuf[256];

	switch (m_byRecvflag)
	{
	case 1:
		ProcessRecvBuf_yc(buf, len);
		break;
	case 2:
		ProcessRecvBuf_yx(buf, len);
		break;
	default:
		sprintf(szBuf, "CModBusZhenZhouWenkongyi recv no this type\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		break;

	}

	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY::ProcessRecvBuf  ----- */


BOOL CModBusZhenZhouWenkongyi::ProcessRecvBuf_yc(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[ 0 ] ;
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byIndex = 5;
	BYTE byPnt = 0;
	for (int i = 0; i < byCount; i += 2)
	{
		float fVal = GetTemprature(pBuf[byIndex], pBuf[byIndex + 1]);

		byIndex += 2;
		m_pMethod->SetYcData(m_SerialNo, byPnt, fVal);
		byPnt++;
	}

	return TRUE;
}

float CModBusZhenZhouWenkongyi::GetTemprature( BYTE hdata , BYTE ldata )
{
	float fVal = 0;
	fVal = (hdata - 40)  + ( float )( ldata / 10.0 );
	return fVal;
}

BOOL CModBusZhenZhouWenkongyi::ProcessRecvBuf_yx(BYTE *buf, int len)
{
	BYTE * pBuf = buf;
	BYTE byAddr = pBuf[0];
	BYTE byFunc = pBuf[1];
	BYTE byCount = pBuf[2];
	BYTE byHidata = pBuf[3];
	BYTE byLodata = pBuf[4];
	BYTE byPnt = 0;
	for ( int i = 0 ; i < 6 ; i++ )
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
* Description:  查看接收报文有效性
*       Input:  缓冲区 长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusZhenZhouWenkongyi::WhetherBufValue(BYTE *buf, int &len)
{
	BYTE *pointer = buf;
	WORD wCrc;
	char szBuf[256];
	int pos = 0;
	while (len >= 4)
	{
		//判断地址
		if (*pointer != m_wDevAddr)
		{
			sprintf(szBuf, "CModBusSCJZ9SY recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//判断功能码
		if (*(pointer + 1) != 0x03)
		{
			sprintf(szBuf, "CModBusSCJZ9SY recv funcode err recv fuccode=%d\n", *(pointer + 1));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//判断长度
		if (*(pointer + 2) > len - 3)
		{
			sprintf(szBuf, "CModBusSCJZ9SY recv len err recv len=%d\n", *(pointer + 2));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//判断校验
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

BOOL CModBusZhenZhouWenkongyi::GetDevCommState()
{
	return false;
}
