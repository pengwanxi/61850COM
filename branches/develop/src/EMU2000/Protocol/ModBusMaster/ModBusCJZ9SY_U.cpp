/*
* =====================================================================================
*
*       Filename:  CModBusSCJZ9SY_U.cpp
*
*        Version:  1.0
*        Created:  2019年07月12日 13时15分10秒
*       Revision:  none
*       Compiler:  gcc
*
*         Author: xrb
*   Organization:
                  只需要3个相电压，3个线电压
*
*		  history:
* =====================================================================================
*/


#include "ModBusCJZ9SY_U.h"
#include  <math.h>

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  CModBusSCJZ9SY_U
* 
*--------------------------------------------------------------------------------------
*/
CModBusSCJZ9SY_U::CModBusSCJZ9SY_U()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_bySendPos = 0;
	m_byRecvflag = 0;
	buf_fz[4] = {0};
}  /* -----  end of method CModBusSCJZ9SY_U::CModBusSCJZ9SY_U  (constructor)  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  ~CModBusSCJZ9SY_U
* Description:  destructor
*--------------------------------------------------------------------------------------
*/
CModBusSCJZ9SY_U::~CModBusSCJZ9SY_U()
{
}  /* -----  end of method CModBusSCJZ9SY_U::~CModBusSCJZ9SY_U  (destructor)  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  WhetherBufValue
* Description:  查看接收报文有效性
*       Input:  缓冲区 长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_U::WhetherBufValue(BYTE *buf, int &len)
{
	BYTE *pointer = buf;
	WORD wCrc;
	char szBuf[256];
	int pos = 0;
	while(len >= 4)
	{
		//判断地址
		if (*pointer != m_wDevAddr)
		{
			sprintf(szBuf, "CModBusSCJZ9SY_U recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//判断功能码
		if (*(pointer + 1) != 0x03)
		{
			sprintf(szBuf, "CModBusSCJZ9SY_U recv funcode err recv fuccode=%d\n", *(pointer + 1));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//判断长度
		if (*(pointer + 2) > len - 3)
		{
			sprintf(szBuf, "CModBusSCJZ9SY_U recv len err recv len=%d\n", *(pointer + 2));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//判断校验
		wCrc = GetCrc(pointer, (*(pointer + 2) + 3));
		if (*(pointer + (*(pointer + 2) + 3)) != HIBYTE(wCrc)
			|| *(pointer + (*(pointer + 2) + 4)) != LOBYTE(wCrc))
		{
			sprintf(szBuf, "CModBusSCJZ9SY_U recv crc err recvcrc=%.2x%.2x, localcrc=%.2x%.2x\n",
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
}		/* -----  end of method CModBusSCJZ9SY_U::WhetherBufValue  ----- */


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  ProcessRecvBuf
* Description:  处理报文  更新数据
*       Input:  缓冲区长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_U::ProcessRecvBuf(BYTE *buf, int len)
{
	int i;
	char szBuf[256];
	GetSendPos( );
	switch (m_byRecvflag)
	{
	case 1:
		ProcessRecvBuf_One(buf, len);
		break;
	case 2:
		ProcessRecvBuf_Two(buf, len);
		break;
	default:
		sprintf(szBuf, "CModBusSCJZ9SY_U recv no this type\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		break;

	}

	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY_U::ProcessRecvBuf  ----- */

//yc：0 - 9 功率符号位、功率小数点位置、电流小数点位置、电压小数点位置
BOOL CModBusSCJZ9SY_U::ProcessRecvBuf_One(BYTE *buf, int len)
{

	int i, j;
	//WORD wYcVal;
	float fYcVal;
	//float YM[8];	
	for (j = 23; j <= 26; j++)
	{
		buf_fz[j - 23] = buf[j];
	}

	for (i = 0; i <= 2; i++) //A 相电压、B相电压、C相电压
	{
		
			fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]);
			m_pMethod->SetYcData(m_SerialNo, i, fYcVal);
		

	}
	return TRUE;

}
//yc:28-30
BOOL CModBusSCJZ9SY_U::ProcessRecvBuf_Two(BYTE *buf, int len)
{
	int i, j;
	WORD wYcVal;
	float fYcVal;
	for (i = 0; i < 3; i++)
	{
		
		fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]);
		m_pMethod->SetYcData(m_SerialNo, i + 3, fYcVal);
	}
	return TRUE;

}


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  GetProtocolBuf
* Description:  获取发送报文
*       Input:  
*		Return:  BOOL
*---------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_U::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	char szBuf[256] = "";
	BYTE byRegisterNum = 0;
	WORD wStartReguster = 0;
	m_byRecvflag = 0;
	
	if (pBusMsg != NULL)
	{
		return FALSE;
	}

	ChangeSendPos();

	switch (m_bySendPos)
	{
	   case 0:	//yc：0-9 功率符号位、功率小数点位置、电流小数点位置、电压小数点位置  5个寄存器
			wStartReguster = 0;
			byRegisterNum = 0x0C;
			m_byRecvflag = 1;
			break;
	   case 1://yc：28-30
			 wStartReguster = 28;
			 byRegisterNum = 0x03;
			 m_byRecvflag = 2;
			break;
	   default:
		   sprintf(szBuf, "%s", "ModBusSCJZ9SY send buf err !!!\n");
		   OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		   break;
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

	return TRUE;
}		/* -----  end of method CModBusSCJZ9SY_U::GetProtocolBuf  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  GetSendPos
* Description:  获取发送位置
*       Input:  void
*		Return:  BYTE 发送位置

*--------------------------------------------------------------------------------------
*/
BYTE CModBusSCJZ9SY_U::GetSendPos(void)
{
	return (m_bySendPos % MODBUSMASTER_CJZ9SY_MAX_POS);
}		/* -----  end of method CModBusSCJZ9SY_U::GetSendPos  ----- */


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  ChangeSendPos
* Description:  改变发送位置
*       Input:	 void
*		Return:  void
*--------------------------------------------------------------------------------------
*/
void CModBusSCJZ9SY_U::ChangeSendPos(void)
{
	m_bySendPos = m_bySendPos++ % MODBUSMASTER_CJZ9SY_MAX_POS;

	if (m_bySendPos >= (MODBUSMASTER_CJZ9SY_MAX_POS))
	{
		m_bySendPos = 0;
	}

}		/* -----  end of method CModBusSCJZ9SY_U::ChangeSendPos  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  ProcessProtocolBuf
* Description:  处理接收报文
*       Input:  缓冲区长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/

BOOL CModBusSCJZ9SY_U::ProcessProtocolBuf(BYTE *buf, int len) 
{
	
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "ModBusSCJZ9SY recv buf err !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

		m_byRecvCount++;
		return FALSE;
	}
	
	ProcessRecvBuf(buf, len);

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----end of method CModBusSCJZ9SY_U::ProcessProtocolBuf ------- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  Init
* Description:  初始化协议
*       Input:  总线号
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_U::Init(BYTE byLineNo)
{
	return TRUE;
}		/* -----  end of method CModBusSCJZ9SY_U::Init  -------*/

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  TimerProc
* Description:  时钟处理
*       Input:   void
*		Return:  void
*---------------------------------------------------------------------------------------
*/
void CModBusSCJZ9SY_U::TimerProc(void)
{
	if (m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if (m_bLinkStatus)
		{
			m_bLinkStatus = FALSE;

			OutBusDebug(m_byLineNo, (BYTE *)"CModBusSCJZ9SY_U:unlink\n", 30, 2);
		}
	}
}		/* -----  end of method CModBusSCJZ9SY_U::TimerProc  ----- */


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_U
*      Method:  GetDevCommState
* Description:  获取装置状态
*       Input:  void
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_U::GetDevCommState(void)
{
	if (m_bLinkStatus)
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModBusSCJZ9SY_U::GetDevCommState  ----- */

