/// \文件:	UpsBaiSe.cpp
/// \概要:	百色公司定制 UPS协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-07-02

#include "UpsBaiSe.h"

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CUpsBaiSe::CUpsBaiSe()
{
	SendFlag = 0;
	m_bySendCount = 0;
	m_bLinkStatus = FALSE;
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CUpsBaiSe::~CUpsBaiSe()
{

}

// --------------------------------------------------------
/// \概要:	获得报文
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsBaiSe::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{/*
	if(pBusMsg != NULL)
	{
	if(pBusMsg->byMsgType == YK_PROTO)
	{
	GetYKBuffer(buf, len, pBusMsg);
	return TRUE;
	}
	}*/
	switch(SendFlag)
	{
		case 0:
			UPSBAISEQueryStatePack(buf, len);
			SendFlag = 1;
			break;
		case 1:
			UPSBAISEQueryCellStatePack(buf, len);
			SendFlag = 2;
			break;
		case 2:
			UPSBAISEQueryG2StatePack(buf, len);
			SendFlag = 3;
			break;
		case 3:
			UPSBAISEQueryG3StatePack(buf, len);
			SendFlag = 0;
			break;
		default:
			SendFlag = 0;
			break;
	}
	m_bySendCount++;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsBaiSe::ProcessProtocolBuf(BYTE *buf, int len)
{
	switch(SendFlag)
	{
		case 0:
			UPSBAISEQueryG3StateDeal(buf, len);
			break;
		case 1:
			UPSBAISEQueryStateDeal(buf, len);
			break;
		case 2:
			UPSBAISEQueryCellStateDeal(buf, len);
			break;
		case 3:
			UPSBAISEQueryG2StateDeal(buf, len);
			break;
		default:
			SendFlag = 0;
			break;
	}
	m_bySendCount = 0;
	m_bLinkStatus = TRUE;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	初始化
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsBaiSe::Init(BYTE byLineNo)
{
	return TRUE;
}


// --------------------------------------------------------
/// \概要:	遥控信号处理
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsBaiSe::GetYKBuffer(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	YK_DATA *yk_data;
	yk_data = (YK_DATA *)pBusMsg->pData;
//	SrcwSerialNo = pBusMsg->SrcInfo.wSerialNo;

	if(pBusMsg->dwDataType == YK_SEL || pBusMsg->dwDataType == YK_CANCEL)
	{
		if(yk_data->wPnt < 8)
		{
			if(pBusMsg->dwDataType == YK_SEL)
			{
				m_pMethod->SetYkSelRtn (this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
			}
			else
			{
				m_pMethod->SetYkCancelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
			}
			return TRUE;
		}
		yk_data->byVal = YK_ERROR;
		m_pMethod->SetYkSelRtn (this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
		return TRUE;
	}
	else if(pBusMsg->dwDataType == YK_EXCT)
	{
	//	printf("YK EXCT\n");
		if(yk_data->wPnt < 8)
		{
			if(yk_data->byVal == 1)
			{
				switch(yk_data->wPnt)
				{
					case 0:
						UPSBAISETestTenSecondPack(buf, len);
						break;
					case 1:
						UPSBAISETestundervoltagePack(buf, len);
						break;
					case 2:
						UPSBAISETestSomeMinutesPack(buf, len);
						break;
					case 3:
						UPSBAISEControlBuzzerPack(buf, len);
						break;
					case 4:
						UPSBAISEPowerOffPack(buf, len);
						break;
					case 5:
						UPSBAISEPowerOffPowerOnPack(buf, len);
						break;
					case 6:
						UPSBAISECancelPowerOffPack(buf, len);
						break;
					case 7:
						UPSBAISECancelTestPack(buf, len);
						break;
					default:
						yk_data->byVal = YK_ERROR;
						m_pMethod->SetYkSelRtn (this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
						return TRUE;
						break;
				}
			}
			m_pMethod->SetYkExeRtn (this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
			return TRUE;
		}
		yk_data->byVal = YK_ERROR;
		m_pMethod->SetYkSelRtn (this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
		return TRUE;
	}

	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryStatePack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'Q';
	buf[index++] = '1';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryCellStatePack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'G';
	buf[index++] = '1';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryG2StatePack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'G';
	buf[index++] = '2';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryG3StatePack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'G';
	buf[index++] = '3';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISETestTenSecondPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'T';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISETestundervoltagePack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'T';
	buf[index++] = 'L';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISETestSomeMinutesPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'T';
	buf[index++] = '0';
	buf[index++] = '1';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISEControlBuzzerPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'Q';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISEPowerOffPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'S';
	buf[index++] = '0';
	buf[index++] = '1';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISEPowerOffPowerOnPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'S';
	buf[index++] = '0';
	buf[index++] = '1';
	buf[index++] = 'R';
	buf[index++] = '0';
	buf[index++] = '0';
	buf[index++] = '1';
	buf[index++] = '1';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISECancelPowerOffPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'C';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}
BOOL CUpsBaiSe::UPSBAISECancelTestPack(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 'C';
	buf[index++] = 'T';
	buf[index++] = 13; // ' <CR>'
	len = index;
	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryStateDeal(BYTE *buf, int len)
{
	int i = 0;
	BYTE ErrorFlag = UPSBAISEMSGFALSE;
	while(len >= 47)
	{
		for(i=0; i<47; i++)
		{
			if(i == 0)
			{
				if(buf[i] != '(')
					break;
			}
			else if(i == 4 || i == 10 || i == 16
					|| i == 25 || i == 35)
			{
				if(buf[i] != '.')
					break;
			}
			else if(i == 6 || i == 12 || i == 18
					|| i == 22 || i == 27 || i == 32 || i == 37)
			{
				if(buf[i] != ' ')
					break;
			}
			else if(i == 46)
			{
				if(buf[i] != 13)
					break;
				else
				{
					ErrorFlag = UPSBAISEMSGTRUE;
					break;
				}
			}
			else if(i < 37 && i != 29 && i != 30)
			{
				if(!(buf[i] >= '0' && buf[i] <= '9'))
					break;
			}
			else if(i > 37)
			{
				if(buf[i] != '0' && buf[i] != '1')
					break;
			}
		}

		if(ErrorFlag == UPSBAISEMSGTRUE)
			break;
		else
		{
			buf = buf + 1;
			len--;
		}

	}
	if(ErrorFlag == UPSBAISEMSGFALSE)
	{
		printf("Q1  error\n");
		return FALSE;
	}
	/*
	   printf("Q1 %lf %f %f %f %f %f %f %d %d %d %d %d %d %d %d\n",atof((const char *)buf+1),atof((const char *)buf+7),atof((const char *)buf+13),
	   atof((const char *)buf+19),atof((const char *)buf+23),atof((const char *)buf+28),atof((const char *)buf+33),buf[38]-48,buf[39]-48,
	   buf[40]-48,buf[41]-48,buf[42]-48,buf[43]-48,buf[44]-48,buf[45]-48);*/
	float YcVal[7];
	BYTE YcNo = 0;
	BYTE YxNo = 0;
	memset(YcVal, 0, 7);
	YcVal[0] = atof((const char *)buf+1);
	YcVal[1] = atof((const char *)buf+7);
	YcVal[2] = atof((const char *)buf+13);
	YcVal[3] = atof((const char *)buf+19);
	YcVal[4] = atof((const char *)buf+23);
	YcVal[5] = atof((const char *)buf+28);
	YcVal[6] = atof((const char *)buf+33);

	for(YcNo=0; YcNo<7; YcNo++)
	{
		m_pMethod->SetYcData(m_SerialNo, YcNo, YcVal[YcNo]);
		/*char buf[100];
		  sprintf(buf,"YC m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n",m_byLineNo, m_wDevAddr, YcNo, YcVal[YcNo]);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);*/
	}
	for(YxNo=0; YxNo<8; YxNo++)
	{
		BYTE YxVal = buf[38+YxNo]-48;
		m_pMethod->SetYxData(m_SerialNo, YxNo, YxVal);
		/*char buf[100];
		  sprintf(buf,"YX m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n",m_byLineNo, m_wDevAddr, YxNo, YxVal);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);*/
	}

	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryCellStateDeal(BYTE *buf, int len)
{
	int i = 0;
	BYTE ErrorFlag = UPSBAISEMSGFALSE;
	while(len >= 41)
	{
		for(i=0; i<41; i++)
		{
			if(i == 0)
			{
				if(buf[i] != '!')
					break;
			}
			else if(i == 17 || i == 23
					|| i == 28 || i == 33 || i == 38)
			{
				if(buf[i] != '.')
					break;
			}
			else if(i == 4 || i == 8 || i == 13
					|| i == 19 || i == 25 || i == 30 || i == 35)
			{
				if(buf[i] != ' ')
					break;
			}
			else if(i == 20)
			{
				if(buf[i] != '+' && buf[i] != '-')
					break;
			}
			else if(i == 40)
			{
				if(buf[i] != 13)
					break;
				else
				{
					ErrorFlag = UPSBAISEMSGTRUE;
					break;
				}
			}
			else if(i == 5)
			{
				if(buf[i] != '0' && buf[i] != '1')
					break;
			}
			else if(i < 40)
			{
				if(!(buf[i] >= '0' && buf[i] <= '9'))
					break;
			}
		}
		if(ErrorFlag == UPSBAISEMSGTRUE)
			break;
		else
		{
			buf = buf + 1;
			len--;
		}
	}
	if(ErrorFlag == UPSBAISEMSGFALSE)
	{
		printf("G1 error\n");
		return FALSE;
	}
	/*
	   printf("G1 %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f \n",atof((const char *)buf+1),atof((const char *)buf+5),
	   atof((const char *)buf+9),atof((const char *)buf+14),atof((const char *)buf+20),
	   atof((const char *)buf+26),atof((const char *)buf+31),atof((const char *)buf+36));*/
	float YcVal[8];
	BYTE YcNo = 0;
	memset(YcVal, 0, 8);
	YcVal[0] = atof((const char *)buf+1);
	YcVal[1] = atof((const char *)buf+5);
	YcVal[2] = atof((const char *)buf+9);
	YcVal[3] = atof((const char *)buf+14);
	YcVal[4] = atof((const char *)buf+20);
	YcVal[5] = atof((const char *)buf+26);
	YcVal[6] = atof((const char *)buf+31);
	YcVal[7] = atof((const char *)buf+36);

	for(YcNo=0; YcNo<8; YcNo++)
	{
		m_pMethod->SetYcData(m_SerialNo, YcNo+7, YcVal[YcNo]);
		/*char buf[100];
		  sprintf(buf,"YC m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n",m_byLineNo, m_wDevAddr, YcNo+7, YcVal[YcNo]);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);*/
	}
	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryG2StateDeal(BYTE *buf, int len)
{
	int i = 0;
	BYTE ErrorFlag = UPSBAISEMSGFALSE;
	while(len >= 28)
	{
		for(i=0; i<28; i++)
		{
			if(i == 0)
			{
				if(buf[i] != '!')
					break;
			}
			else if(i == 9 || i == 18)
			{
				if(buf[i] != ' ')
					break;
			}
			else if(i == 27)
			{
				if(buf[i] != 13)
					break;
				else
				{
					ErrorFlag = UPSBAISEMSGTRUE;
					break;
				}
			}
			else if(i < 28)
			{
				if(buf[i] != '0' && buf[i] != '1')
					break;
			}
		}
		if(ErrorFlag == UPSBAISEMSGTRUE)
			break;
		else
		{
			buf = buf + 1;
			len--;
		}

	}
	if(ErrorFlag == UPSBAISEMSGFALSE)
	{
		printf("G2 error\n");
		return FALSE;
	}
	/*
	   printf("G2 %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d %d%d%d%d%d%d%d%d \n",buf[1]-48,buf[2]-48,buf[3]-48,
	   buf[4]-48,buf[5]-48,buf[6]-48,buf[7]-48,buf[8]-48,buf[10]-48,buf[11]-48,buf[12]-48,
	   buf[13]-48,buf[14]-48,buf[15]-48,buf[16]-48,buf[17]-48,buf[19]-48,buf[20]-48,
	   buf[21]-48,buf[22]-48,buf[23]-48,buf[24]-48,buf[25]-48,buf[26]-48);*/
	BYTE YxNo = 0;
	for(YxNo=0; YxNo<8; YxNo++)
	{
		BYTE YxVal = buf[1+YxNo]-48;
		m_pMethod->SetYxData(m_SerialNo, YxNo+8, YxVal);
		/*char buf[100];
		  sprintf(buf,"YX m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n",m_byLineNo, m_wDevAddr, YxNo+8, YxVal);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);*/
	}
	for(YxNo=8; YxNo<16; YxNo++)
	{
		BYTE YxVal = buf[2+YxNo]-48;
		m_pMethod->SetYxData(m_SerialNo, YxNo+8, YxVal);
		/*char buf[100];
		  sprintf(buf,"YX m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n",m_byLineNo, m_wDevAddr, YxNo+8, YxVal);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);*/
	}
	for(YxNo=16; YxNo<24; YxNo++)
	{
		BYTE YxVal = buf[3+YxNo]-48;
		m_pMethod->SetYxData(m_SerialNo, YxNo+8, YxVal);
		/*char buf[100];
		  sprintf(buf,"YX m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n",m_byLineNo, m_wDevAddr, YxNo+8, YxVal);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);*/
	}

	return TRUE;
}

BOOL CUpsBaiSe::UPSBAISEQueryG3StateDeal(BYTE *buf, int len)
{
	int i = 0;
	BYTE ErrorFlag = UPSBAISEMSGFALSE;
	while(len >= 73)
	{
		for(i=0; i<73; i++)
		{
			if(i == 0)
			{
				if(buf[i] != '!')
					break;
			}
			else if(i == 4 || i == 10 || i == 16 || i == 22 || i == 28 || i == 34
					|| i == 40 || i == 46 || i == 52 || i == 58 || i == 64 || i == 70)
			{
				if(buf[i] != '.')
					break;
			}
			else if(i == 18 || i == 36 || i == 54)
			{
				if(buf[i] != ' ')
					break;
			}
			else if(i == 6 || i == 12 || i == 24 || i == 30 || i == 42
					|| i == 48 || i == 60 || i == 66)
			{
				if(buf[i] != '/')
					break;
			}
			else if(i == 72)
			{
				if(buf[i] != 13)
					break;
				else
				{
					ErrorFlag = UPSBAISEMSGTRUE;
					break;
				}
			}
			else if(i < 73)
			{
				if(!(buf[i] >= '0' && buf[i] <= '9'))
					break;
			}
		}
		if(ErrorFlag == UPSBAISEMSGTRUE)
			break;
		else
		{
			buf = buf + 1;
			len--;
		}

	}
	if(ErrorFlag == UPSBAISEMSGFALSE)
	{
		printf("G3 error\n");
		return FALSE;
	}
	float YcVal[12];
	BYTE YcNo = 0;
	memset(YcVal, 0, 8);
	YcVal[0] = atof((const char *)buf+1);
	YcVal[1] = atof((const char *)buf+7);
	YcVal[2] = atof((const char *)buf+13);
	YcVal[3] = atof((const char *)buf+19);
	YcVal[4] = atof((const char *)buf+25);
	YcVal[5] = atof((const char *)buf+31);
	YcVal[6] = atof((const char *)buf+37);
	YcVal[7] = atof((const char *)buf+43);
	YcVal[8] = atof((const char *)buf+49);
	YcVal[9] = atof((const char *)buf+55);
	YcVal[10] = atof((const char *)buf+61);
	YcVal[11] = atof((const char *)buf+67);

	for(YcNo=0; YcNo<12; YcNo++)
	{
		m_pMethod->SetYcData(m_SerialNo, YcNo+15, YcVal[YcNo]);
		/*char buf[100];
		  sprintf(buf,"YC m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n",m_byLineNo, m_wDevAddr, YcNo+15, YcVal[YcNo]);
		  OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
		  */
	}
	return TRUE;
}
void CUpsBaiSe::TimerProc()
{
	if(m_bySendCount > 3)
	{
		m_bySendCount = 0;
		if(m_bLinkStatus)
		{
			m_bLinkStatus = FALSE;
			OutBusDebug(m_byLineNo, (BYTE *)"UPS:unlink\n", 30, 2);
		}
	}
}

BOOL CUpsBaiSe::GetDevCommState (void)
{
	if (m_bLinkStatus)
	{
		return COM_DEV_NORMAL;
	}

	else
	{
		return COM_DEV_ABNORMAL;
	}
}

