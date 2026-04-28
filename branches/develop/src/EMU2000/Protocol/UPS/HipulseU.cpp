#include "HipulseU.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

#define ALARMSTART 100;

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CUpsHipulse::CUpsHipulse()
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
CUpsHipulse::~CUpsHipulse()
{

}

// --------------------------------------------------------
/// \概要:	获取校验值
///
/// \参数:	pBuf
/// \参数:	len
///
/// \返回:	WORD
// --------------------------------------------------------
WORD CUpsHipulse::GetCrc(BYTE *pBuf, int len)
{
	WORD wCRC = 0;
	for(int i = 1; i < len; i++)
		wCRC += pBuf[i];

	return ~wCRC + 1;
}

// --------------------------------------------------------
/// \概要:	判断报文有效性
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::WhetherBufValue(BYTE *buf, int &len)
{
	if(buf[0] != 0x7E){
		printf((char *)"Start of information is failed!!!\n");
		return FALSE;
	}

	BYTE bAddr = (UPSHexToChar(buf[3]) << 4) | (UPSHexToChar(buf[4]));
	if(bAddr > 254 || bAddr < 1){
		printf((char *)"The device address is failed!!!\n");
		return FALSE;
	}

	BYTE bCID1 = (UPSHexToChar(buf[5]) << 4) | (UPSHexToChar(buf[6]));
	if(bCID1 != 0x2A){
		printf((char *)"Control identification code1 is failed!!!\n");
		return FALSE;
	}

	BYTE bCID2 = (UPSHexToChar(buf[7]) << 4) | (UPSHexToChar(buf[8]));
	if(bCID2 != 0x00){
		printf((char *)"Control identification code2 is failed!!!\n");
		switch(bCID2){
			case 0x01:
				printf((char *)"VER is failed!!!\n");
				break;
			case 0x02:
				printf((char *)"CHKSUM is failed!!!\n");
				break;
			case 0x03:
				printf((char *)"LCHKSUM is failed!!!\n");
				break;
			case 0x04:
				printf((char *)"CID2 is invaild!!!\n");
				break;
			case 0x05:
				printf((char *)"Command format is failed!!!\n");
				break;
			case 0x06:
				printf((char *)"VER is data!!!\n");
				break;
			case 0x10:
				printf((char *)"Invaild permissions!!!\n");
				break;
		}
		return FALSE;
	}


	WORD wCRC =	GetCrc(buf, len - 5);

	if(wCRC != ((((UPSHexToChar(buf[len - 5]) << 4) | (UPSHexToChar(buf[len - 4]))) << 8) | ((UPSHexToChar(buf[len - 3]) << 4) | (UPSHexToChar(buf[len - 2]))))){
		printf((char *)"CheckSum is failed!!!\n");
		printf("----FUNC = %s LINE = %d wCRC = %d----\n", __func__, __LINE__, wCRC);
		return FALSE;
	}

	if(buf[len - 1] != 0x0D){
		printf((char *)"End code is failed!!!\n");
		return FALSE;
	}


	return TRUE;
}

void CUpsHipulse::ChangeSendPos(void)
{
	m_iSendFlag %=	UPSMASTER_HIPULSEU_MAX_POX;
	m_iSendFlag++;
	if(m_iSendFlag >= UPSMASTER_HIPULSEU_MAX_POX)
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
BOOL CUpsHipulse::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
//	printf("----FUNC = %s LINE= %d m_iSendFlag = %d----\n", __func__, __LINE__, m_iSendFlag);
	if(m_iInitFlag != 0)
		ChangeSendPos();

	switch(m_iSendFlag)
	{
		case 0:
			if(m_iInitFlag == 0)
				UPSQueryProtocolVersion(buf, len);//查询协议版本号
			break;
		case 1:
			UPSQueryUserDefinedAnalogData3(buf, len);//模拟量 序号44－60为自定义模拟量量化数据3
			break;
		case 2:
			UPSQueryStatePack(buf, len);//获取开关输入状态
			break;
		case 3:
			UPSQueryAlarmInfo(buf, len); //查询告警状态
			break;
		case 4:
			UPSQueryUserDefinedAnalogData1(buf, len);//模拟量 序号 14-27 为自定义模拟量量化数据1
			break;
		case 5:
			UPSQueryUserDefinedAnalogData2(buf, len);//模拟量 序号28-43为自定义模拟量量化数据2
			break;
		case 6:
			UPSQueryTotalStandardAnalogData(buf, len); //序号0-13 为电总标准模拟量 
			break;
		default:
			break;
	}

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
BOOL CUpsHipulse::ProcessProtocolBuf(BYTE *buf , int len )
{
	if(!WhetherBufValue(buf, len)){
		printf((char *)"CUpsHipulse:ProcessProtocolBuf buf Recv err!!!\n");
		return FALSE;
	}
	switch(m_iSendFlag)
	{
		case 0:
			if(m_iInitFlag == 0)
			{
				UPSParseProtocolVersion(buf, len);//版本号
				m_iInitFlag = 1;
			}
			break;
		case 1:
			UPSParseUserDefinedAnalogData3(buf, len);//自定义模拟量信息化表3
			break;
		case 2:
			UPSParseStatePack(buf, len); //开关输入状态
			break;
		case 3:
			UPSParseAlarmInfo(buf, len); //告警信息报文
			break;
		case 4:
			UPSParseUserDefinedAnalogData1(buf, len);//自定义模拟量信息化表1
			break;
		case 5:
			UPSParseUserDefinedAnalogData2(buf, len);//自定义模拟量信息化表2
			break;
		case  6:
			UPSParseTotalStandardAnalogData(buf,len);//电总标准模拟量
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
BOOL CUpsHipulse::Init(BYTE byLineNo)
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
BYTE CUpsHipulse::UPSCharToHex(BYTE bHex)
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
BYTE CUpsHipulse::UPSHexToChar(BYTE bChar)
{
	if((bChar >=0x30) && (bChar <= 0x39))
		bChar -= 0x30;
	else if((bChar >= 0x41) && (bChar <= 0x46))
		bChar -= 0x37;

	return bChar;
}

// --------------------------------------------------------
/// \概要:	查询开关输入状态
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSQueryStatePack(BYTE *buf , int &len )
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = m_byVersion[0];
	buf[index++] = m_byVersion[1];

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x34;
	buf[index++] = 0x33;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

	len = index;

	return TRUE;
}


// --------------------------------------------------------
/// \概要:	查询协议版本号
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSQueryProtocolVersion(BYTE *buf , int &len )
{
	int index = 0;

	buf[index++] = 0x7E;   //起始标记 1个字节  7EH

	buf[index++] = 0x30;   //通讯协议版本号  可以为任意值
	buf[index++] = 0x30;   

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));  //ADDR
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));  

	buf[index++] = 0x32;  //0x2A  ---CD1
	buf[index++] = 0x41;

	buf[index++] = 0x34;  //0x4F
	buf[index++] = 0x46;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

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
BOOL CUpsHipulse::UPSQueryUserDefinedAnalogData3(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = m_byVersion[0];
	buf[index++] = m_byVersion[1];

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x45;
	buf[index++] = 0x33;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	查询自定义模拟量量化数据(浮点数，厂家扩展模拟量2)
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSQueryUserDefinedAnalogData2(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = m_byVersion[0];
	buf[index++] = m_byVersion[1];

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x45;
	buf[index++] = 0x31;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

	len = index;

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	查询自定义模拟量量化数据(浮点数，厂家扩展模拟量1)
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSQueryUserDefinedAnalogData1(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = m_byVersion[0];
	buf[index++] = m_byVersion[1];

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x45;
	buf[index++] = 0x32;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	电总标准模拟量（浮点数）
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSQueryTotalStandardAnalogData(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = m_byVersion[0];
	buf[index++] = m_byVersion[1];

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x34;
	buf[index++] = 0x31;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

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
BOOL CUpsHipulse::UPSQueryAlarmInfo(BYTE *buf, int &len)
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = m_byVersion[0];
	buf[index++] = m_byVersion[1];

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x34;
	buf[index++] = 0x34;

	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;
	buf[index++] = 0x30;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = UPSCharToHex(HIGBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(HIBYTE(wCRC)));
	buf[index++] = UPSCharToHex(HIGBYTE(LOBYTE(wCRC)));
	buf[index++] = UPSCharToHex(LOWBYTE(LOBYTE(wCRC)));

	buf[index++] = 0x0D;

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
BOOL CUpsHipulse::UPSParseProtocolVersion(BYTE *buf, int len)
{
	memcpy(m_byVersion, buf + 1, 2);
	m_dVersion = (double)UPSHexToChar(buf[1]) + (double)UPSHexToChar(buf[2]) / 10;

	printf("This device protocol version number is %.1f\n", m_dVersion);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析收到报文的4字节遥测
///
/// \参数:	buf
///
/// \返回:	float
// --------------------------------------------------------
#if 0
float CUpsHipulse::UPSParseFloat(BYTE *buf)
{
	float fFloat_val = 0;

	int iValue = 0;
	int iMantissa = 0;
	int iCode = 0;
	int iSign = 0;

	iValue = (((UPSHexToChar(buf[6]) << 4) | UPSHexToChar(buf[7])) << 24) | (((UPSHexToChar(buf[4]) << 4) | UPSHexToChar(buf[5])) << 16) | (((UPSHexToChar(buf[2]) << 4) | UPSHexToChar(buf[3])) << 8) | ((UPSHexToChar(buf[0]) << 4) | UPSHexToChar(buf[1]));
	iMantissa = iValue & 0x007FFFFF;
	iCode = (iValue & 0x7F800000) >> 23;
	iSign = (iValue & 0x80000000) >> 31;
//	printf("----FUNC = %s LINE = %d iValue = %d iMantissa = %d iCode = %d iSign = %d----\n", __func__, __LINE__, iValue, iMantissa, iCode, iSign);

	if(iSign == 1)
		fFloat_val = -((1 + float(iMantissa) / float(UPSFactorial(23))) * (float(UPSFactorial(iCode - 127))));
	else
		fFloat_val = ((1 + float(iMantissa) / float(UPSFactorial(23))) * (float(UPSFactorial(iCode - 127))));

	return fFloat_val;
}
#else
float CUpsHipulse::UPSParseFloat(BYTE *buf)
{
	float fFloat_val = 0;
	memcpy(&fFloat_val, buf, 4);

	return fFloat_val;
}
#endif

// --------------------------------------------------------
/// \概要:	2的阶乘
///
/// \参数:	iNum
///
/// \返回:	int
// --------------------------------------------------------
int CUpsHipulse::UPSFactorial(int iNum)
{
	int iFactorial = 1;
	for(int i = 0; i < iNum; i++)
		iFactorial *= 2;

//	printf("----FUNC = %s LINE = %d iFactorial = %d----\n", __func__, __LINE__, iFactorial);
	return iFactorial;
}

// --------------------------------------------------------
/// \概要:	解析收到报文的1字节遥信
///
/// \参数:	buf
///
/// \返回:	BYTE
// --------------------------------------------------------
BYTE CUpsHipulse::UPSParseByte(BYTE *buf)
{
	BYTE bByte_val = (UPSHexToChar(buf[0]) << 4) | UPSHexToChar(buf[1]);

	return bByte_val;
}

// --------------------------------------------------------
/// \概要:	解析设备回复自定义模拟量化信息报文3
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseUserDefinedAnalogData3(BYTE *buf, int len)
{
	BYTE bYcNum = (UPSHexToChar(buf[15]) << 4 | UPSHexToChar(buf[16]));
//	printf("----FUNC = %s LINE= %d bYcNum = %d----\n", __func__, __LINE__, bYcNum);
	float YcVal = 0;
	for(WORD YcNo = 0; YcNo <=bYcNum; YcNo++)
	{
		YcVal = UPSParseFloat(buf + 17 + YcNo * 8);
	//	printf("----FUNC = %s LINE= %d YcVal = %f----\n", __func__, __LINE__, YcVal);
		m_pMethod->SetYcData(m_SerialNo, 39+YcNo, YcVal);
	}
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析设备回复自定义模拟量化信息报文2
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseUserDefinedAnalogData2(BYTE *buf, int len)
{
	BYTE bYcNum = (UPSHexToChar(buf[15]) << 4 | UPSHexToChar(buf[16]));
	//	printf("----FUNC = %s LINE= %d bYcNum = %d----\n", __func__, __LINE__, bYcNum);
	float YcVal = 0;
	for (WORD YcNo = 0; YcNo <=bYcNum; YcNo++)
	{
		YcVal = UPSParseFloat(buf + 17 + YcNo * 8);
		//	printf("----FUNC = %s LINE= %d YcVal = %f----\n", __func__, __LINE__, YcVal);
		m_pMethod->SetYcData(m_SerialNo, 24+YcNo, YcVal);
	}
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析设备回复自定义模拟量化信息报文1
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseUserDefinedAnalogData1(BYTE *buf, int len)
{
	BYTE bYcNum = (UPSHexToChar(buf[15]) << 4 | UPSHexToChar(buf[16]));
	//	printf("----FUNC = %s LINE= %d bYcNum = %d----\n", __func__, __LINE__, bYcNum);
	float YcVal = 0;
	for (WORD YcNo = 0; YcNo <=bYcNum; YcNo++)
	{
		YcVal = UPSParseFloat(buf + 17 + YcNo * 8);
		//	printf("----FUNC = %s LINE= %d YcVal = %f----\n", __func__, __LINE__, YcVal);
		m_pMethod->SetYcData(m_SerialNo, 11+YcNo, YcVal);
	}
	return TRUE;
}
// --------------------------------------------------------
/// \概要:	解析设备回复电总标准
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseTotalStandardAnalogData(BYTE *buf, int len)
{
	BYTE bYcNum = 11;
	//	printf("----FUNC = %s LINE= %d bYcNum = %d----\n", __func__, __LINE__, bYcNum);
	float YcVal = 0;
	for (WORD YcNo = 0; YcNo <bYcNum; YcNo++)
	{
		YcVal = UPSParseFloat(buf + 15 + YcNo * 8);
		//	printf("----FUNC = %s LINE= %d YcVal = %f----\n", __func__, __LINE__, YcVal);
		m_pMethod->SetYcData(m_SerialNo, YcNo, YcVal);
	}
	////标识电池数量---
	//YcVal =0;
	//m_pMethod->SetYcData(m_SerialNo, 11, YcVal);
 //
	////标示温度数量
	//YcVal = 0;
	//m_pMethod->SetYcData(m_SerialNo, 12, YcVal);

	////用户自定义遥测内容数量
	//YcVal = 0;
	//m_pMethod->SetYcData(m_SerialNo, 13, YcVal);

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
BOOL CUpsHipulse::UPSParseStatePack(BYTE *buf, int len)
{
	BYTE YxVal = 0;
	m_iYxRunStateNum = 0;

	for(WORD YxNo = 0; YxNo < 17; YxNo++)
	{
		YxVal = UPSParseByte(buf + 15 + YxNo * 2);
		switch(YxNo)
		{
			case 0:
				UPSPowerSupplyMode(YxVal);//供电方式的三种状态0-2
				break;
			case 1:
				break;
			case 2:
				UPSPowerSelfCheck(YxVal);//电池自检回复状态 3 4
				break;
			case 3:
				UPSPowerAllFloat(YxVal);//电池均充、浮充  5 6 7
				break;
			case 4:
				UPSPowerStartUpShutDown(YxVal);//开机、关机  8 9
				break;
			case 5:
				UPSPowerSupply(YxVal);//UPS 供电 10 11 12 13
				break;
			case 6:
				UPSPowerGeneratorAccess(YxVal);//发电机接入 14 15
				break;
			case 7:
				UPSPowerInputSwitchState(YxVal);//输入开关状态 16-17
				break;
			case 8:
				UPSPowerRepairBypassSwitchState(YxVal);//维修旁路开关状态 18 19
				break;
			case 9:
				UPSPowerBypassSwitchState(YxVal);//旁路开关状态 20 21
				break;
			case 10:
				UPSPowerOutputSwitchState(YxVal);//输出开关状态 22 23
				break;
			case 11:
				UPSPowerSupplyStateOfParallelSystem(YxVal);//并机系统供电状态 24-25-26-27
				break;
			case 12:
				UPSPowerRotarySwitchState(YxVal);//旋转开关状态28 29 30 31 32 33
				break;
			case 13:
				UPSPowerFilterState(YxVal);//滤波器状态 34 35
				break;
			case 14:
				UPSDormantState(YxVal);//休眠状态 36 37
				break;
			case 15:
				UPSRectifierOperationState(YxVal);//休眠状态 38 39
				break;
			case 16:
				UPSECOState(YxVal);//休眠状态40 41
				break;
			default:
				break;
		}

	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析设备回复告警信息报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseAlarmInfo(BYTE *buf, int len)
{
	BYTE YxVal = 0;
	m_iYxWarnStateNum = 42;

	for(WORD YxNo = 0; YxNo < 67; YxNo++)
	{
		YxVal = UPSParseByte(buf + 15 + YxNo * 2);
		switch(YxNo)
		{
			case 0:
				UPSParseInverter(YxVal);//逆变器同步不同步 42-43
				break;
			case 1:
				UPSParseMainVoltageAbnormal(YxVal);//主路电压异常 44-45
				break;
			case 2:
				UPSParseRectifierLock(YxVal);//整流器封锁46-47
				break;
			case 3:
				UPSParseInverterOutputVoltage(YxVal);//逆变输出电压异常 48-49
				break;
			case 4:
				UPSParseBypassSituation(YxVal); //旁路情況 50 51
				break;
			case 5:
				UPSParseTotalVoltageState(YxVal);//蓄电池总电压状态52 53 54 55 56 57
				break;
			case 6: //
				break;
			case 7:
				break;
			case 8:
				UPSParseMainFirequency(YxVal);//主路频率异常 58-59
				break;
			case 9:
				UPSParseMainFuseBroken(YxVal);//主路熔丝断60 -61
				break;
			case 10:
				UPSParseMainReverseOrder(YxVal);//主路相序反 62-63
				break;
			case 11:
				UPSParseMainPhaseFault(YxVal);// 主路缺相故障64-65
				break;
			case 12:
				UPSParseAuxiliaryPowerSupplyOne(YxVal);//辅助电源1掉电66-67
				break;
			case 13:
				UPSParseAuxiliaryPowerSupplyTow(YxVal);//辅助电源2掉电68-69
				break;
			case 14:
				UPSParseRectifierLimit(YxVal);//整流器限流70-71
				break;
			case 15:
				UPSParseSoftBoot(YxVal);//软启动失败71-73
				break;
			case 16:
				UPSParseRectifierOverTemperature(YxVal);//整流器过温74-75
				break;
			case 17:
				UPSParseInputFilterFail(YxVal);//输入滤波器故障76-77
				break;
			case 18:
				UPSParseFilterOverFlow(YxVal);//滤波器过流78-79
				break;
			case 19:
				UPSParseFilterFail(YxVal); //滤波器接触器故障80 81
				break;
			case 20:
				UPSParseFilterDriveCableFail(YxVal); //整流器驱动电缆故障82 83
				break;
			case 21:
				UPSParseRectifierComFail(YxVal);//整流通讯故障84 85--
				break;
			case 22:
				UPSParseInverterOverTemperature(YxVal);//逆变器过温86-87
				break;
			case 23:
				UPSParseFanFail(YxVal);//风扇故障88-89
				break;
			case 24:
				UPSParseInverterThyristorFail(YxVal);//逆变晶闸管故障90 91
				break;
			case 25:
				UPSParseBypassThyristorFail(YxVal);//旁路晶闸管故障92-93
				break;
			case 26:
				UPSParseUserOperationFail(YxVal);//用户操作错误94-95
				break;
			case 27:
				UPSParseSingleOutputOverload(YxVal);//单机输出过载96-97
				break;
			case 28:
				UPSParseParallelSystemOverload(YxVal);//并机系统过载98 99
				break;
			case 29:
				UPSParseSingleOverloadTimeOut(YxVal); //单机过载超时100-101
				break;
			case 30:
				UPSParseBypassAbNormalShutdown(YxVal);//旁路异常关机102-103
				break;
			case 31:
				UPSParseAcOutputOverpressure(YxVal);//交流输出过压104 105
				break;
			case 32:
				UPSParseInverterOverflow(YxVal);//逆变器过流106 107
				break;
			case 33:
				UPSParseBypassReverse(YxVal);//旁路线序反108 109
				break;
			case 34:
				UPSParseLoadShock(YxVal);//负载冲击转旁路110 111
				break;
			case 35:
				UPSParseBypassSwitchLimit(YxVal);//旁路切换次数限制112 113
				break;
			case 36:
				UPSParseParallelEqualFail(YxVal);//并机均流故障114 115
				break;
			case 37:
				UPSParseBusAbnormalShutdown(YxVal);//母线异常关机116 117
				break;
			case 38:
				UPSParseNeighborBypass(YxVal);//邻机请求转旁路118 119
				break;
			case 39:
				UPSParseParallelPlateFail(YxVal);//并机板故障120 121
				break;
			case 40:
				UPSParseParallelConnectFail(YxVal);//并机线连接故障122 123
				break;
			case 41:
				UPSParseParallelComFail(YxVal);//并机通讯故障124 125
				break;
			case 42:
				UPSParseBypassOverFlowFail(YxVal);//旁路过流故障126 127
				break;
			case 43:
				UPSParseLBSActivation(YxVal);//LBS激活/故障128 129 130
				break;
			case 44:
				UPSParseBypassInductorOverTemperature(YxVal);//旁路电感过温131 132
				break;
			case 45:
				UPSParseStaticSwitchOverTemperature(YxVal);//静态开关过温133 134
				break;
			case 46:
				UPSParseBypassReverseFail(YxVal);//旁路反灌故障135 136
				break;
			case 47:
				UPSParseInverterDriveCableFail(YxVal);//逆变器驱动电缆故障137 138
				break;
			case 48:
				UPSParseInverterComFail(YxVal);//逆变通讯故障139 140
				break;
			case 49:
				UPSParseParallelSystemBatteryFail(YxVal);//并机系统电池预告警故障141 142
				break;
			case 50:
				UPSParseEmergencyShutdown(YxVal);//紧急关机143 144
				break;
			case 51:
				UPSParseAmbientTemperatureHigh(YxVal);//环境温度过高 145 146
				break;
			case 52:
				UPSParseBatteryLife(YxVal);//蓄电池寿命情况 147 148 149
				break;
			case 53:
				UPSParseBatteryTemperatureHigh(YxVal);//电池温度过高150 151
				break;
			case 54:
				UPSParseBatteryGroundFail(YxVal);//电池接地故障152 153
				break;
			case 55:
				UPSParseBatteryFuse(YxVal);//电池熔丝断 154 155
				break;
			case 56:
				UPSParseBCBInput(YxVal);//BCB接入情况156 157 158
				break;
			case 57:
				UPSParseOutputFuse(YxVal); //输出熔丝断（预留）159 160
				break;
			case 58:
				UPSParseBusCapOvervoltage(YxVal);//母线电容过压 161 162
				break;
			case 59:
				UPSParseBusOvervoltage(YxVal);//母线过压163 164
				break;
			case 60:
				UPSParseBusShortCircultFail(YxVal);//母线短路故障（预留）165 166
				break;
			case 61:
				UPSParseInputFlowUnbalance(YxVal);//输入电流不均 167 168
				break;
			case 62:
				UPSParseOutputCapMaintain(YxVal);//输出电容需维护169 170
				break;
			case 63:
				UPSParseFilterCutoffTimes(YxVal);//滤波器投切次数到171 172
				break;
			case 64:
				UPSBatterySRCFailure(YxVal);//电池SRC故障173 174
				break;
			case 65:
				UPSPFCModeException(YxVal);//PFC模式异常 175 176
				break;
			case 66:
				UPSEqualizingTimeout(YxVal);//均充超时 177 178
				break;
			default:
				break;
		}

	}

	return TRUE;
}

void CUpsHipulse::TimerProc()
{
	if(m_bySendCount > 3){
		m_bySendCount = 0;
		if(m_bLinkStatus){
			m_bLinkStatus = FALSE;
			OutBusDebug(m_byLineNo, (BYTE *)"UPS:unlink\n", 30, 2);
		}
	}
}

// --------------------------------------------------------
/// \概要:	供电方式状态的三种情况
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerSupplyMode(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0x01:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0x02:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE9:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	电池自检回复状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerSelfCheck(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	电池均充/浮充状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerAllFloat(BYTE YxVal)
{
	UPSYxRunStateValThree(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:  开机关机
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerStartUpShutDown(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	UPS供电
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerSupply(BYTE YxVal)
{
	UPSYxRunStateValFour(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	发电机接入
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerGeneratorAccess(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	输入开关状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerInputSwitchState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	维修旁路开关状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerRepairBypassSwitchState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路开关状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerBypassSwitchState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	输出开关状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerOutputSwitchState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机系统供电状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerSupplyStateOfParallelSystem(BYTE YxVal)
{
	UPSYxRunStateValFour(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旋转开关状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerRotarySwitchState(BYTE YxVal)
{
	UPSYxRunStateValSix(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	滤波器状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPowerFilterState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	休眠状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSDormantState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	整流器工作状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSRectifierOperationState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	ECO模式状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSECOState(BYTE YxVal)
{
	UPSYxRunStateValTwo(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	遥信返回值只有两种情况（0xE0,0xE1）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxRunStateValTwo(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0xE0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE1:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有三种情况（0xE0,0xE1,0xE2）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxRunStateValThree(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0xE0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE1:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE2:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有四种情况（0xE0,0xE1,0xE2,0xE3）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxRunStateValFour(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0xE0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE1:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE2:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE3:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有六种情况（0xE0,0xE1,0xE2,0xE3,0xE4,0xE5）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxRunStateValSix(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0xE0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE1:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE2:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE3:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE4:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			break;
		case 0xE5:
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxRunStateNum++, 1);
			break;

		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变器告警
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverter(BYTE YxVal)
{
	UPSYxWarnStateValTwoSync(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	主路电压异常
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseMainVoltageAbnormal(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	整流器封锁
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseRectifierLock(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变输出电压异常
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverterOutputVoltage(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路情况
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassSituation(BYTE YxVal)
{
	UPSYxWarnStateValTwoOverrun(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	蓄电池总电压状态
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseTotalVoltageState(BYTE YxVal)
{
	UPSYxWarnStateValSix(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	主路频率异常
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseMainFirequency(BYTE YxVal)
{
	UPSYxWarnStateValTwoOverrun(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	主路熔丝断
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseMainFuseBroken(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	主路相序反
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseMainReverseOrder(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	主路缺相故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseMainPhaseFault(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	辅助电源1掉电
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseAuxiliaryPowerSupplyOne(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	辅助电源2掉电
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseAuxiliaryPowerSupplyTow(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	整流器限流
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseRectifierLimit(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	软启动失败
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseSoftBoot(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	整流器过温
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseRectifierOverTemperature(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	输入滤波器故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInputFilterFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	滤波器过流
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseFilterOverFlow(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	滤波器接触器故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseFilterFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	滤波器驱动电缆故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseFilterDriveCableFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	整流通讯故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseRectifierComFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变器过温
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverterOverTemperature(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	风扇故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseFanFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变晶闸管故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverterThyristorFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路晶闸管故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassThyristorFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	用户操作错误
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseUserOperationFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	单机输出过载
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseSingleOutputOverload(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机系统过载
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseParallelSystemOverload(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	单机过载超时
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseSingleOverloadTimeOut(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路异常关机
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassAbNormalShutdown(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	交流输出过压
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseAcOutputOverpressure(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变器过流
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverterOverflow(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路相序反
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassReverse(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	负载冲击转旁路
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseLoadShock(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路切换次数限制
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassSwitchLimit(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机均流故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseParallelEqualFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	母线异常关机
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBusAbnormalShutdown(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	邻机请求转旁路
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseNeighborBypass(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机板故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseParallelPlateFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机线连接故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseParallelConnectFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机通讯故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseParallelComFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路过流故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassOverFlowFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	LBS激活/故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseLBSActivation(BYTE YxVal)
{
	UPSYxWarnStateValThreeUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路电感过温
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassInductorOverTemperature(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	静态开关过温
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseStaticSwitchOverTemperature(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	旁路反灌故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBypassReverseFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变器驱动电缆故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverterDriveCableFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	逆变通讯故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInverterComFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	并机系统电池预告警故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseParallelSystemBatteryFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	紧急关机
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseEmergencyShutdown(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	环境温度过高
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseAmbientTemperatureHigh(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	蓄电池寿命情况
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBatteryLife(BYTE YxVal)
{
	UPSYxWarnStateValThreeUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	电池温度过高
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBatteryTemperatureHigh(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	电池接地故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBatteryGroundFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	电池熔丝断
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBatteryFuse(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	BCB接入情况
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBCBInput(BYTE YxVal)
{
	UPSYxWarnStateValThreeUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	输出熔丝断
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseOutputFuse(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	母线电容过压
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBusCapOvervoltage(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	母线过压
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBusOvervoltage(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	母线短路故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseBusShortCircultFail(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	输入电流不均
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseInputFlowUnbalance(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	输出电容需维护
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseOutputCapMaintain(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	滤波器投切次数到
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSParseFilterCutoffTimes(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	电池SCR故障
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSBatterySRCFailure(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	PFC模式异常
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSPFCModeException(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	均充超时
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSEqualizingTimeout(BYTE YxVal)
{
	UPSYxWarnStateValTwoUnusual(YxVal);

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	遥信返回值只有两种情况（0x00,0x03）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxWarnStateValTwoSync(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0x00:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0x03:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有两种情况（0x00,0xF0）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxWarnStateValTwoUnusual(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0x00:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xF0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有两种情况（0x00,0xE0）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxWarnStateValTwoOverrun(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0x00:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xE0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有三种情况（0x00,0xF0）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxWarnStateValThreeUnusual(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0x00:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xF0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xF1:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥信返回值只有六种情况（0x00,0x01,0x02,0xF0,0xE1,0xE2）
///
/// \参数:	YxVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::UPSYxWarnStateValSix(BYTE YxVal)
{
	switch(YxVal)
	{
		case 0x00:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0x01:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0x02:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xF0:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xE1:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			break;
		case 0xE2:
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 0);
			m_pMethod->SetYxData(m_SerialNo, m_iYxWarnStateNum++, 1);
			break;
		default:
			break;
	}

	return TRUE;
}


// --------------------------------------------------------
/// \概要:	获取设备通讯状态
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsHipulse::GetDevCommState (void )
{
	if(m_bLinkStatus)
		return COM_DEV_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}

