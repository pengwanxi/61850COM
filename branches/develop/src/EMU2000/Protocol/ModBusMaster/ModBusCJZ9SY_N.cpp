/*
* =====================================================================================
*
*       Filename:  CModBusSCJZ9SY_N.cpp
*
*        Version:  1.0
*        Created:  2019��07��12�� 13ʱ15��10��
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  mengqp (),
*   Organization:
*
*		  history:
* =====================================================================================
*/


#include "ModBusCJZ9SY_N.h"
#include  <math.h>

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_N
*      Method:  CModBusSCJZ9SY_N
* 
*--------------------------------------------------------------------------------------
*/

CModBusSCJZ9SY_N::CModBusSCJZ9SY_N()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_bySendPos = 0;
	m_byRecvflag = 0;
	buf_fz[4] = {0};
}  /* -----  end of method CModBusSCJZ9SY::CModBusSCJZ9SY  (constructor)  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY
*      Method:  ~CModBusSCJZ9SY
* Description:  destructor
*--------------------------------------------------------------------------------------
*/
CModBusSCJZ9SY_N::~CModBusSCJZ9SY_N()
{
}  /* -----  end of method CModBusSCJZ9SY::~CModBusSCJZ9SY  (destructor)  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY
*      Method:  WhetherBufValue
* Description:  �鿴���ձ�����Ч��
*       Input:  ������ ����
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_N::WhetherBufValue(BYTE *buf, int &len)
{
	BYTE *pointer = buf;
	WORD wCrc;
	char szBuf[256];
	int pos = 0;
	while(len >= 4)
	{
		//�жϵ�ַ
		if (*pointer != m_wDevAddr)
		{
			sprintf(szBuf, "CModBusSCJZ9SY_N recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�жϹ�����
		if (*(pointer + 1) != 0x03)
		{
			sprintf(szBuf, "CModBusSCJZ9SY_N recv funcode err recv fuccode=%d\n", *(pointer + 1));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�жϳ���
		if (*(pointer + 2) > len - 3)
		{
			sprintf(szBuf, "CModBusSCJZ9SY_N recv len err recv len=%d\n", *(pointer + 2));
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}

		//�ж�У��
		wCrc = GetCrc(pointer, (*(pointer + 2) + 3));
		if (*(pointer + (*(pointer + 2) + 3)) != HIBYTE(wCrc)
			|| *(pointer + (*(pointer + 2) + 4)) != LOBYTE(wCrc))
		{
			sprintf(szBuf, "CModBusSCJZ9SY_N recv crc err recvcrc=%.2x%.2x, localcrc=%.2x%.2x\n",
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


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_N
*      Method:  ProcessRecvBuf
* Description:  ��������  ��������
*       Input:  ����������
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_N::ProcessRecvBuf(BYTE *buf, int len)
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
	case 3:
		ProcessRecvBuf_Three(buf, len);
		break;
	case 4:
		ProcessRecvBuf_Four(buf, len);
		break;
	case 5:
		ProcessRecvBuf_Five(buf, len);
		break;
	default:
		sprintf(szBuf, "CModBusSCJZ9SY_N recv no this type\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		break;

	}

	return FALSE;
}		/* -----  end of method CModBusSCJZ9SY::ProcessRecvBuf  ----- */

//yc��0 - 9 ���ʷ���λ������С����λ�á�����С����λ�á���ѹС����λ��
BOOL CModBusSCJZ9SY_N::ProcessRecvBuf_One(BYTE *buf, int len)
{

	int i, j;
	WORD wYcVal;
	float fYcVal;
	float YM[8];	
	for (j = 23; j <= 26; j++)
	{
		buf_fz[j - 23] = buf[j];
	}

	for (i = 0; i <= 9; i++) //�� �� �� Ϣ
	{
		if (i <= 2)//�� ѹ
		{
			//fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) * 0.0001 * pow(10, buf_fz[1]);
			fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]);//�޸�Ϊ��ѹֱ��
			m_pMethod->SetYcData(m_SerialNo, i, fYcVal);
		}
		if (i >= 3 && i <= 5)//�� ��
		{
			//fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) *0.0001 * pow(10, buf_fz[0]);
			fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]);//�޸�Ϊ����ֱ��
			m_pMethod->SetYcData(m_SerialNo,i, fYcVal);
		}
		if (i==6) //���й�  
		{
			if((buf_fz[0] & 0x10) == 0)//SIGN ��0~7 λ�ֱ��ʾPa��Pb��Pc��Ps��Qa��Qb��Qc��Qs �ķ��ţ�0��Ϊ���� 1��Ϊ����
			{
				fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) *0.0001 * pow(10, buf_fz[1]);
				m_pMethod->SetYcData(m_SerialNo, i, fYcVal);
			}
			if((buf_fz[0] & 0x10) == 0x10)//SIGN ��0~7 λ�ֱ��ʾPa��Pb��Pc��Ps��Qa��Qb��Qc��Qs �ķ��ţ�0��Ϊ���� 1��Ϊ����
			{
				fYcVal = -1 * MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) *0.0001 * pow(10, buf_fz[1]);
				m_pMethod->SetYcData(m_SerialNo, i, fYcVal);
			}
		}
		if (i==7)//���޹�0x02
		{
			if((buf_fz[0] & 0x01) == 0)//SIGN ��0~7 λ�ֱ��ʾPa��Pb��Pc��Ps��Qa��Qb��Qc��Qs �ķ��ţ�0��Ϊ���� 1��Ϊ����
			{
				fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) *0.0001 * pow(10, buf_fz[1]);
				m_pMethod->SetYcData(m_SerialNo, i, fYcVal);
			}
			if((buf_fz[0] & 0x01) == 0x01)//SIGN ��0~7 λ�ֱ��ʾPa��Pb��Pc��Ps��Qa��Qb��Qc��Qs �ķ��ţ�0��Ϊ���� 1��Ϊ����
			{
				fYcVal = -1 * MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) *0.0001 * pow(10, buf_fz[1]);
				m_pMethod->SetYcData(m_SerialNo, i, fYcVal);
			}

		}
		if (i==8)//�ܹ�������
		{
			fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]) *0.001;
			m_pMethod->SetYcData(m_SerialNo, i , fYcVal);

		}
		if (i==9)//Ƶ��
		{
			fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2])*0.01;
			m_pMethod->SetYcData(m_SerialNo, i , fYcVal);

		}		

	}
	return TRUE;

}
//yc:28-30
BOOL CModBusSCJZ9SY_N::ProcessRecvBuf_Two(BYTE *buf, int len)
{
	int i, j;
	WORD wYcVal;
	float fYcVal;
	for (i = 0; i < 3; i++)
	{
		
		//fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2])*0.0001 * pow(10, buf_fz[1]);
		fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]);
		m_pMethod->SetYcData(m_SerialNo, i + 10, fYcVal);
	}
	return TRUE;

}
//yc:55-60
BOOL CModBusSCJZ9SY_N::ProcessRecvBuf_Three(BYTE *buf, int len)
{
	int i, j;
	WORD wYcVal;
	float fYcVal;
	for (i = 0; i < 6; i++)
	{
		fYcVal = MAKEWORD(buf[4 + i * 2], buf[3 + i * 2]);
		m_pMethod->SetYcData(m_SerialNo, i + 13, fYcVal);
	}
	return TRUE;

}
//ym��20-27
BOOL CModBusSCJZ9SY_N::ProcessRecvBuf_Four(BYTE *buf, int len)
{
	int i, j;
	/*float YM[4];
	printf("ym:");
	for (i = 0; i < buf[2] + 5; i++)
		printf("%x ",buf[i]);

	for (i = 0; i < 4; i++)
	{
		memcpy(YM + i, buf +3 + 4 * i, 4);
		m_pMethod->SetYmData(m_SerialNo, i, YM[i]);
	}
	printf("ym:over  m_bySendPos:%d\n", m_bySendPos);*/

	float ym=0.0;
	char *Modbus_HoldReg[4];				 //���屣�ּĴ���ָ������
	for (i = 0; i < 4; i++)
	{
		//��һ����ָ���ʼ��
		Modbus_HoldReg[2] = ((char*)(&ym)) + 3;	 //�͵�ַָ���λ
		Modbus_HoldReg[3] = ((char*)(&ym)) + 2;
		Modbus_HoldReg[0] = ((char*)(&ym)) + 1;
		Modbus_HoldReg[1] = ((char*)(&ym)) + 0;	 //�ߵ�ַָ���λ

		//�ڶ���������ַָ�����ڴ浥Ԫ��ֵ����ӦModbusЭ���е����ݽ�����
		*Modbus_HoldReg[0] = buf[3+4*i];
		*Modbus_HoldReg[1] = buf[4+4*i];
		*Modbus_HoldReg[2] = buf[5+4*i];
		*Modbus_HoldReg[3] = buf[6+4*i];

		m_pMethod->SetYmData(m_SerialNo, i,  (QWORD)ym);
	}
	
	return TRUE;

}
// yx��35
BOOL CModBusSCJZ9SY_N::ProcessRecvBuf_Five(BYTE *buf, int len)
{
	BYTE byYxByte;
	BYTE byYxValue;
	for (int i = 0; i<7; i++)
	{
		BYTE byYxValue = 0;
		BYTE byYxBit = 0;
		byYxBit = (buf[3] >>  i) & 0x01;
		if (byYxBit == 0x01)
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}
		m_pMethod->SetYxData(m_SerialNo, i, byYxValue);	
	}
	return TRUE;

}

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_N
*      Method:  GetProtocolBuf
* Description:  ��ȡ���ͱ���
*       Input:  
*		Return:  BOOL
*---------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_N::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
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
	   case 0:	//yc��0-9 ���ʷ���λ������С����λ�á�����С����λ�á���ѹС����λ��  12���Ĵ���
			wStartReguster = 0;
			byRegisterNum = 0x0C;
			m_byRecvflag = 1;
			break;
	   case 1://yc��28-30
			 wStartReguster = 28;
			 byRegisterNum = 0x03;
			 m_byRecvflag = 2;
			break;
	   case 2://yc��55-60
			wStartReguster = 55;
			byRegisterNum = 0x06;
			m_byRecvflag = 3;
			break;
	   case 3://ym��20-27
		   wStartReguster = 20;
		   byRegisterNum = 0x08;
		   m_byRecvflag = 4;
		   break;
	   case 4:// yx��35 
		   wStartReguster = 35;
		   byRegisterNum = 0x01;
		   m_byRecvflag = 5;
		   break;
	   default:
		   sprintf(szBuf, "%s", "ModBusSCJZ9SY send buf err !!!\n");
		   OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		   break;
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

	return TRUE;
}		/* -----  end of method CModBusSCJZ9SY::GetProtocolBuf  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_N
*      Method:  GetSendPos
* Description:  ��ȡ����λ��
*       Input:  void
*		Return:  BYTE ����λ��

*--------------------------------------------------------------------------------------
*/
BYTE CModBusSCJZ9SY_N::GetSendPos(void)
{
	return (m_bySendPos % MODBUSMASTER_CJZ9SY_MAX_POS);
}		/* -----  end of method CModBusSCJZ9SY::GetSendPos  ----- */


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY_N
*      Method:  ChangeSendPos
* Description:  �ı䷢��λ��
*       Input:	 void
*		Return:  void
*--------------------------------------------------------------------------------------
*/
void CModBusSCJZ9SY_N::ChangeSendPos(void)
{
	m_bySendPos = m_bySendPos++ % MODBUSMASTER_CJZ9SY_MAX_POS;

	if (m_bySendPos >= (MODBUSMASTER_CJZ9SY_MAX_POS))
	{
		m_bySendPos = 0;
	}

}		/* -----  end of method CModBusSCJZ9SY::ChangeSendPos  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY
*      Method:  ProcessProtocolBuf
* Description:  �������ձ���
*       Input:  ����������
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/

BOOL CModBusSCJZ9SY_N::ProcessProtocolBuf(BYTE *buf, int len) 
{
	
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "ModBusSCJZ9SY_N recv buf err !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

		m_byRecvCount++;
		return FALSE;
	}
	
	ProcessRecvBuf(buf, len);

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----end of method CModBusSCJZ9SY::ProcessProtocolBuf ------- */

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY
*      Method:  Init
* Description:  ��ʼ��Э��
*       Input:  ���ߺ�
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_N::Init(BYTE byLineNo)
{
	return TRUE;
}		/* -----  end of method CModBusSCJZ9SY::Init  -------*/

/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY
*      Method:  TimerProc
* Description:  ʱ�Ӵ���
*       Input:   void
*		Return:  void
*---------------------------------------------------------------------------------------
*/
void CModBusSCJZ9SY_N::TimerProc(void)
{
	if (m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if (m_bLinkStatus)
		{
			m_bLinkStatus = FALSE;

			OutBusDebug(m_byLineNo, (BYTE *)"CModBusSCJZ9SY_N:unlink\n", 30, 2);
		}
	}
}		/* -----  end of method CModBusSCJZ9SY::TimerProc  ----- */


/*
*--------------------------------------------------------------------------------------
*       Class:  CModBusSCJZ9SY
*      Method:  GetDevCommState
* Description:  ��ȡװ��״̬
*       Input:  void
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL CModBusSCJZ9SY_N::GetDevCommState(void)
{
	if (m_bLinkStatus)
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModBusSCJZ9SY_N::GetDevCommState  ----- */

