/// \文件:	UpsKeShiDa.cpp
/// \概要:	科士达 UPS 协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-07-02

#include "UpsDelta.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

#define ALARMSTART 100;

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CUpsDelta::CUpsDelta()
{
	m_iSendFlag = -1;
	m_iInitFlag = 0;
	m_bySendCount = 0;
	m_bLinkStatus = FALSE;
	m_iYxRunStateNum = 0;
	m_iYxWarnStateNum = 36;
	memset(m_byVersion, 0, sizeof(m_byVersion));
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CUpsDelta::~CUpsDelta()
{

}

WORD CUpsDelta::GetCrc(BYTE *pBuf, int len)
{
	WORD wCRC = 0;
	for(int i = 1; i < len; i++)
		wCRC += pBuf[i];

	return ~wCRC + 1;
}


BOOL CUpsDelta::WhetherBufValue(BYTE *buf, int &len)
{
	return TRUE;
}

void CUpsDelta::ChangeSendPos(void)
{
	m_iSendFlag %=	UPSMASTER_KESHIDA_MAX_POX;
	m_iSendFlag++;
	if(m_iSendFlag >= UPSMASTER_KESHIDA_MAX_POX)
		m_iSendFlag = 0;
}

// --------------------------------------------------------
/// \概要:	获得协议报文
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	switch(m_iSendFlag)
	{
		case 0:
			UPSQueryProtocolyc(buf, len);
			break;
		case 1:
			UPSQueryBatteryVoltage(buf, len);
			break;
		case 2:
			UPSQueryStatePack(buf, len);
			break;
		default:
			break;
	}

	ChangeSendPos();
	m_bySendCount++;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析协议报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::ProcessProtocolBuf(BYTE *buf , int len )
{
	if (!WhetherBufValue(buf, len)) {
		printf((char *)"CUpsKeShiDa:ProcessProtocolBuf buf Recv err!!!\n");
		return FALSE;
	}


//	printf("----FUNC = %s LINE= %d m_iSendFlag = %d----\n", __func__, __LINE__, m_iSendFlag);
	switch(m_iSendFlag)
	{
		case 0:
				UPSParseProtocolyc(buf, len);
			break;
		case 1:
			UPSParseBatteryVoltage(buf, len);
			break;
		case 2:
			UPSParseAlarmInfo(buf, len);
			break;
		default:
			break;
	}

	m_bySendCount = 0;
	m_bLinkStatus = TRUE;
	return TRUE ;
}

// --------------------------------------------------------
/// \概要:	初始化
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::Init(BYTE byLineNo)
{

	return TRUE ;
}

// --------------------------------------------------------
/// \概要:	ASCII 转 16进制
///
/// \参数:	bHex
///
/// \返回:	BYTE
// --------------------------------------------------------
BYTE CUpsDelta::UPSCharToHex(BYTE bHex)
{
	if((bHex >= 0) && (bHex < 10))
		bHex += 0x30;
	else if((bHex >= 10) && (bHex <= 15))
		bHex += 0x37;

	return bHex;
}

// --------------------------------------------------------
/// \概要:	16进制转ASCII
///
/// \参数:	bChar
///
/// \返回:	BYTE
// --------------------------------------------------------
BYTE CUpsDelta::UPSHexToChar(BYTE bChar)
{
	if((bChar >=0x30) && (bChar <= 0x39))
		bChar -= 0x30;
	else if((bChar >= 0x41) && (bChar <= 0x46))
		bChar -= 0x37;

	return bChar;
}
// --------------------------------------------------------
/// \概要:	查询协议版本号
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::UPSQueryProtocolyc(BYTE *buf , int &len )
{
	int index = 0;

	buf[index++] = '~';

	buf[index++] = 0x30;
	buf[index++] = 0x30;

	buf[index++] = 'P';

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x33;

	buf[index++] = 'R';
	buf[index++] = 'A';
	buf[index++] = 'T';
	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	查询自定义模拟量量化数据(浮点数，厂家扩展模拟量3)
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::UPSQueryBatteryVoltage(BYTE *buf, int &len)
{
	int index = 0;
	buf[index++] = '~';

	buf[index++] = 0x30;
	buf[index++] = 0x30;

	buf[index++] = 'P';

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x33;

	buf[index++] = 'S';
	buf[index++] = 'T';
	buf[index++] = 'B';
	
	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	查询告警状态（标准帧）
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::UPSQueryAlarmInfo(BYTE *buf, int &len)
{
	int index = 0;
	buf[index++] = '~';

	buf[index++] = 0x30;
	buf[index++] = 0x30;

	buf[index++] = 'P';

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x33;

	buf[index++] = 'S';
	buf[index++] = 'T';
	buf[index++] = 'A';

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析设备回复版本号报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::UPSParseProtocolyc(BYTE *buf, int len)
{
	char tt[200] = { '\0' };
	char cTemp;
	for (int i = 0; i < len; i++)
	{
		sprintf(&cTemp, "%c", buf[i]);
		tt[i] = cTemp;
	}
	printf(tt);
	printf("\n");
}


// --------------------------------------------------------
/// \概要:	解析设备回复自定义模拟量化信息报文3
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::UPSParseBatteryVoltage(BYTE *buf, int len)
{
	char tt[200] = { '\0' };
	char cTemp;
	for (int i = 0; i < len; i++)
	{
		sprintf( &cTemp, "%c", buf[i]);
		tt[i] = cTemp;
	}
	printf(tt);
	printf("\n");
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析设备回复开关输入状态报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsDelta::UPSParseAlarmInfo(BYTE *buf, int len)
{
	char tt[200] = { '\0' };
	char cTemp;
	for (int i = 0; i < len; i++)
	{
		sprintf(&cTemp, "%c", buf[i]);
		tt[i] = cTemp;
	}
	printf(tt);
	printf("\n");

	return TRUE;
}

void CUpsDelta::TimerProc()
{
	if(m_bySendCount > 3){
		m_bySendCount = 0;
		if(m_bLinkStatus){
			m_bLinkStatus = FALSE;
		}
	}
}

BOOL CUpsDelta::GetDevCommState (void )
{
	if(m_bLinkStatus)
		return COM_DEV_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}

