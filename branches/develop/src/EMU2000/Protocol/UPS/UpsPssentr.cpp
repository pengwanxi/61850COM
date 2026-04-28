#include "UpsPssentr.h" 

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
typedef struct __YCMAP {
	int  pos;		//遥测值位置
	BYTE valFlag;	//存储数据格式，0为byte，1为WORD
}YCMAP;

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CUpsSentry::CUpsSentry()
{
	SendFlag = 0;
	m_bySendCount = 0;
	m_bLinkStatus = FALSE;
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CUpsSentry::~CUpsSentry()
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
BOOL CUpsSentry::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{

	//pack packets
	int iDent = 0; //IDENT设置值，根据设置的站址确定，只能取值0 - 7, m_addr取值为>1

	if (m_wDevAddr > 0)
	{
		iDent = (m_wDevAddr - 1) % 8;
	}
	buf[0] = 192 + iDent;

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
BOOL CUpsSentry::ProcessProtocolBuf(BYTE *buf, int len)
{
	float fYcvalue = 0.0;
	int i;
	if (len < 4 || len >= 512) //若缓冲区乱码或溢出 
	{
		
		return FALSE;
	}

	//检查校验值
	if (GetChecksum(buf, 100) != MAKEWORD(buf[101], buf[102]) && GetChecksum(buf, 101) != MAKEWORD(buf[101], buf[102]))
		return FALSE;

	if (buf[1] == 103)
	{
		//遥测
		YCMAP info[] = {
			{ 2, 1 }, { 4, 1 }, { 6, 1 }, { 8, 0 }, { 9, 1 }, { 11, 0 }, { 18, 0 }, { 35, 0 }, { 36, 0 }, { 37, 0 },
			{ 38, 0 }, { 39, 0 }, { 40, 0 }, { 41, 1 }, { 43, 1 }, { 45, 1 }, { 47, 0 }, { 48, 0 }, { 49, 0 }, { 50, 0 },
			{ 51, 1 }, { 53, 1 }, { 55, 1 }, { 57, 1 }, { 59, 0 }, { 60, 0 }, { 61, 0 }, { 62, 0 }, { 63, 0 }, { 64, 0 },
			{ 65, 0 }, { 66, 0 }, { 67, 0 }, { 68, 1 }, { 70, 0 }, { 71, 1 }, { 73, 0 }, { 74, 1 }, { 76, 0 }, { 77, 0 },
			{ 78, 0 }, { 79, 0 }, { 80, 0 }, { 81, 0 }, { 82, 0 }, { 82, 0 }, { 83, 1 }, { 85, 1 }, { 87, 1 }, { 89, 1 }
		};
		BOOL flag82 = FALSE;
		for ( i = 0; i<sizeof(info) / sizeof(YCMAP); i++)
		{
			if (info[i].valFlag == 0)
			{
				BYTE bVal = buf[info[i].pos];
				if (info[i].pos == 82)
				{
					if (!flag82)
					{
						if (buf[82] & (1 << 0))
							bVal = 1;
						else
							bVal = 0;
						flag82 = TRUE;
					}
					else
					{
						if (bVal & (1 << 7))
							bVal = 50;
						else
							bVal = 60;
						flag82 = FALSE;
					}
				}
				m_pMethod->SetYcData(m_SerialNo, i, (float)bVal);
			}
			else if (info[i].valFlag == 1)
			{
				WORD wVal = 0;
				if (buf[info[i].pos] == 43 || buf[info[i].pos] == 45)			//此两处为高位在前，低位在后
					wVal = MAKEWORD(buf[info[i].pos] + 1, buf[info[i].pos]);
				else
					wVal = MAKEWORD(buf[info[i].pos],buf[info[i].pos + 1]);
				m_pMethod->SetYcData(m_SerialNo, i, (float)wVal);
			}
		}

		//遥信数据
		for (i = 0; i<128; i++)
		{
			BYTE  bStatus =0;
			if (buf[19 + i / 8] & (1 << (i % 8)))
				bStatus = 1;
			
			m_pMethod->SetYxData(m_SerialNo, i, bStatus);
		}

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
BOOL CUpsSentry::Init(BYTE byLineNo)
{
	return TRUE;
}

void CUpsSentry::TimerProc()
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

BOOL CUpsSentry::GetDevCommState (void)
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
 WORD CUpsSentry::GetChecksum(unsigned char *Buf, int len)
{
	WORD sum = 0;
	for (int i = 0; i<len; i++)
	{
		sum += Buf[i];
	}
	return sum;
}

