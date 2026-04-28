/// \文件:	UpsKeShiDa.cpp
/// \概要:	科士达 UPS 协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-07-02

#include "UpsKeShiDa.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

#define ALARMSTART 100;

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CUpsKeShiDa::CUpsKeShiDa()
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
CUpsKeShiDa::~CUpsKeShiDa()
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
WORD CUpsKeShiDa::GetCrc(BYTE *pBuf, int len)
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
BOOL CUpsKeShiDa::WhetherBufValue(BYTE *buf, int &len)
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

void CUpsKeShiDa::ChangeSendPos(void)
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
BOOL CUpsKeShiDa::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
//	printf("----FUNC = %s LINE= %d m_iSendFlag = %d----\n", __func__, __LINE__, m_iSendFlag);
	if(m_iInitFlag != 0)
		ChangeSendPos();

	switch(m_iSendFlag)
	{
		case 0:
			if(m_iInitFlag == 0)
				UPSQueryProtocolVersion(buf, len);
			break;
		case 1:
			UPSQueryUserDefinedAnalogData(buf, len);
			break;
		case 2:
			UPSQueryStatePack(buf, len);
			break;
		case 3:
			UPSQueryAlarmInfo(buf, len);
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
BOOL CUpsKeShiDa::ProcessProtocolBuf(BYTE *buf , int len )
{
	if(!WhetherBufValue(buf, len)){
		printf((char *)"CUpsKeShiDa:ProcessProtocolBuf buf Recv err!!!\n");
		return FALSE;
	}


//	printf("----FUNC = %s LINE= %d m_iSendFlag = %d----\n", __func__, __LINE__, m_iSendFlag);
	switch(m_iSendFlag)
	{
		case 0:
			if(m_iInitFlag == 0)
			{
				UPSParseProtocolVersion(buf, len);
				m_iInitFlag = 1;
			}
			break;
		case 1:
			UPSParseUserDefinedAnalogData(buf, len);
			break;
		case 2:
			UPSParseStatePack(buf, len);
			break;
		case 3:
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
BOOL CUpsKeShiDa::Init(BYTE byLineNo)
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
BYTE CUpsKeShiDa::UPSCharToHex(BYTE bHex)
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
BYTE CUpsKeShiDa::UPSHexToChar(BYTE bChar)
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
BOOL CUpsKeShiDa::UPSQueryStatePack(BYTE *buf , int &len )
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
BOOL CUpsKeShiDa::UPSQueryProtocolVersion(BYTE *buf , int &len )
{
	int index = 0;

	buf[index++] = 0x7E;

	buf[index++] = 0x30;
	buf[index++] = 0x30;

	buf[index++] = UPSCharToHex(HIGBYTE(m_wDevAddr));
	buf[index++] = UPSCharToHex(LOWBYTE(m_wDevAddr));

	buf[index++] = 0x32;
	buf[index++] = 0x41;

	buf[index++] = 0x34;
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
BOOL CUpsKeShiDa::UPSQueryUserDefinedAnalogData(BYTE *buf, int &len)
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
/// \概要:	查询告警状态（标准帧）
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CUpsKeShiDa::UPSQueryAlarmInfo(BYTE *buf, int &len)
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
BOOL CUpsKeShiDa::UPSParseProtocolVersion(BYTE *buf, int len)
{
	memcpy(m_byVersion, buf + 1, 2);
	m_dVersion = (double)UPSHexToChar(buf[1]) + (double)UPSHexToChar(buf[2]) / 10;

//	printf("This device protocol version number is %.1f\n", m_dVersion);

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
float CUpsKeShiDa::UPSParseFloat(BYTE *buf)
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
float CUpsKeShiDa::UPSParseFloat(BYTE *buf)
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
int CUpsKeShiDa::UPSFactorial(int iNum)
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
BYTE CUpsKeShiDa::UPSParseByte(BYTE *buf)
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
BOOL CUpsKeShiDa::UPSParseUserDefinedAnalogData(BYTE *buf, int len)
{
	BYTE bYcNum = (UPSHexToChar(buf[15]) << 4 | UPSHexToChar(buf[16]));
//	printf("----FUNC = %s LINE= %d bYcNum = %d----\n", __func__, __LINE__, bYcNum);
	float YcVal = 0;
	for(WORD YcNo = 0; YcNo < bYcNum; YcNo++)
	{
		YcVal = UPSParseFloat(buf + 17 + YcNo * 8);
	//	printf("----FUNC = %s LINE= %d YcVal = %f----\n", __func__, __LINE__, YcVal);
		m_pMethod->SetYcData(m_SerialNo, YcNo, YcVal);
	}

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
BOOL CUpsKeShiDa::UPSParseStatePack(BYTE *buf, int len)
{
	BYTE YxVal = 0;
	m_iYxRunStateNum = 0;

	for(WORD YxNo = 0; YxNo < 14; YxNo++)
	{
		YxVal = UPSParseByte(buf + 15 + YxNo * 2);
		switch(YxNo)
		{
			case 0:
				UPSPowerSupplyMode(YxVal);
				break;
			case 1:
				break;
			case 2:
				UPSPowerSelfCheck(YxVal);
				break;
			case 3:
				UPSPowerAllFloat(YxVal);
				break;
			case 4:
				UPSPowerStartUpShutDown(YxVal);
				break;
			case 5:
				UPSPowerSupply(YxVal);
				break;
			case 6:
				UPSPowerGeneratorAccess(YxVal);
				break;
			case 7:
				UPSPowerInputSwitchState(YxVal);
				break;
			case 8:
				UPSPowerRepairBypassSwitchState(YxVal);
				break;
			case 9:
				UPSPowerBypassSwitchState(YxVal);
				break;
			case 10:
				UPSPowerOutputSwitchState(YxVal);
				break;
			case 11:
				UPSPowerSupplyStateOfParallelSystem(YxVal);
				break;
			case 12:
				UPSPowerRotarySwitchState(YxVal);
				break;
			case 13:
				UPSPowerFilterState(YxVal);
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
BOOL CUpsKeShiDa::UPSParseAlarmInfo(BYTE *buf, int len)
{
	BYTE YxVal = 0;
	m_iYxWarnStateNum = 36;

	for(WORD YxNo = 0; YxNo < 64; YxNo++)
	{
		YxVal = UPSParseByte(buf + 15 + YxNo * 2);
		switch(YxNo)
		{
			case 0:
				UPSParseInverter(YxVal);
				break;
			case 1:
				UPSParseMainVoltageAbnormal(YxVal);
				break;
			case 2:
				UPSParseRectifierLock(YxVal);
				break;
			case 3:
				UPSParseInverterOutputVoltage(YxVal);
				break;
			case 4:
				UPSParseBypassSituation(YxVal);
				break;
			case 5:
				UPSParseTotalVoltageState(YxVal);
				break;
			case 6:
				break;
			case 7:
				break;
			case 8:
				UPSParseMainFirequency(YxVal);
				break;
			case 9:
				UPSParseMainFuseBroken(YxVal);
				break;
			case 10:
				UPSParseMainReverseOrder(YxVal);
				break;
			case 11:
				UPSParseMainPhaseFault(YxVal);
				break;
			case 12:
				UPSParseAuxiliaryPowerSupplyOne(YxVal);
				break;
			case 13:
				UPSParseAuxiliaryPowerSupplyTow(YxVal);
				break;
			case 14:
				UPSParseRectifierLimit(YxVal);
				break;
			case 15:
				UPSParseSoftBoot(YxVal);
				break;
			case 16:
				UPSParseRectifierOverTemperature(YxVal);
				break;
			case 17:
				UPSParseInputFilterFail(YxVal);
				break;
			case 18:
				UPSParseFilterOverFlow(YxVal);
				break;
			case 19:
				UPSParseFilterFail(YxVal);
				break;
			case 20:
				UPSParseFilterDriveCableFail(YxVal);
				break;
			case 21:
				UPSParseRectifierComFail(YxVal);
				break;
			case 22:
				UPSParseInverterOverTemperature(YxVal);
				break;
			case 23:
				UPSParseFanFail(YxVal);
				break;
			case 24:
				UPSParseInverterThyristorFail(YxVal);
				break;
			case 25:
				UPSParseBypassThyristorFail(YxVal);
				break;
			case 26:
				UPSParseUserOperationFail(YxVal);
				break;
			case 27:
				UPSParseSingleOutputOverload(YxVal);
				break;
			case 28:
				UPSParseParallelSystemOverload(YxVal);
				break;
			case 29:
				UPSParseSingleOverloadTimeOut(YxVal);
				break;
			case 30:
				UPSParseBypassAbNormalShutdown(YxVal);
				break;
			case 31:
				UPSParseAcOutputOverpressure(YxVal);
				break;
			case 32:
				UPSParseInverterOverflow(YxVal);
				break;
			case 33:
				UPSParseBypassReverse(YxVal);
				break;
			case 34:
				UPSParseLoadShock(YxVal);
				break;
			case 35:
				UPSParseBypassSwitchLimit(YxVal);
				break;
			case 36:
				UPSParseParallelEqualFail(YxVal);
				break;
			case 37:
				UPSParseBusAbnormalShutdown(YxVal);
				break;
			case 38:
				UPSParseNeighborBypass(YxVal);
				break;
			case 39:
				UPSParseParallelPlateFail(YxVal);
				break;
			case 40:
				UPSParseParallelConnectFail(YxVal);
				break;
			case 41:
				UPSParseParallelComFail(YxVal);
				break;
			case 42:
				UPSParseBypassOverFlowFail(YxVal);
				break;
			case 43:
				UPSParseLBSActivation(YxVal);
				break;
			case 44:
				UPSParseBypassInductorOverTemperature(YxVal);
				break;
			case 45:
				UPSParseStaticSwitchOverTemperature(YxVal);
				break;
			case 46:
				UPSParseBypassReverseFail(YxVal);
				break;
			case 47:
				UPSParseInverterDriveCableFail(YxVal);
				break;
			case 48:
				UPSParseInverterComFail(YxVal);
				break;
			case 49:
				UPSParseParallelSystemBatteryFail(YxVal);
				break;
			case 50:
				UPSParseEmergencyShutdown(YxVal);
				break;
			case 51:
				UPSParseAmbientTemperatureHigh(YxVal);
				break;
			case 52:
				UPSParseBatteryLife(YxVal);
				break;
			case 53:
				UPSParseBatteryTemperatureHigh(YxVal);
				break;
			case 54:
				UPSParseBatteryGroundFail(YxVal);
				break;
			case 55:
				UPSParseBatteryFuse(YxVal);
				break;
			case 56:
				UPSParseBCBInput(YxVal);
				break;
			case 57:
				UPSParseOutputFuse(YxVal);
				break;
			case 58:
				UPSParseBusCapOvervoltage(YxVal);
				break;
			case 59:
				UPSParseBusOvervoltage(YxVal);
				break;
			case 60:
				UPSParseBusShortCircultFail(YxVal);
				break;
			case 61:
				UPSParseInputFlowUnbalance(YxVal);
				break;
			case 62:
				UPSParseOutputCapMaintain(YxVal);
				break;
			case 63:
				UPSParseFilterCutoffTimes(YxVal);
				break;
			default:
				break;
		}

	}

	return TRUE;
}

void CUpsKeShiDa::TimerProc()
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
BOOL CUpsKeShiDa::UPSPowerSupplyMode(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerSelfCheck(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerAllFloat(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerStartUpShutDown(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerSupply(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerGeneratorAccess(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerInputSwitchState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerRepairBypassSwitchState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerBypassSwitchState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerOutputSwitchState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerSupplyStateOfParallelSystem(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerRotarySwitchState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSPowerFilterState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxRunStateValTwo(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxRunStateValThree(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxRunStateValFour(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxRunStateValSix(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverter(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseMainVoltageAbnormal(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseRectifierLock(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverterOutputVoltage(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassSituation(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseTotalVoltageState(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseMainFirequency(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseMainFuseBroken(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseMainReverseOrder(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseMainPhaseFault(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseAuxiliaryPowerSupplyOne(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseAuxiliaryPowerSupplyTow(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseRectifierLimit(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseSoftBoot(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseRectifierOverTemperature(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInputFilterFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseFilterOverFlow(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseFilterFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseFilterDriveCableFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseRectifierComFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverterOverTemperature(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseFanFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverterThyristorFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassThyristorFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseUserOperationFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseSingleOutputOverload(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseParallelSystemOverload(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseSingleOverloadTimeOut(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassAbNormalShutdown(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseAcOutputOverpressure(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverterOverflow(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassReverse(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseLoadShock(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassSwitchLimit(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseParallelEqualFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBusAbnormalShutdown(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseNeighborBypass(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseParallelPlateFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseParallelConnectFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseParallelComFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassOverFlowFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseLBSActivation(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassInductorOverTemperature(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseStaticSwitchOverTemperature(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBypassReverseFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverterDriveCableFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInverterComFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseParallelSystemBatteryFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseEmergencyShutdown(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseAmbientTemperatureHigh(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBatteryLife(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBatteryTemperatureHigh(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBatteryGroundFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBatteryFuse(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBCBInput(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseOutputFuse(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBusCapOvervoltage(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBusOvervoltage(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseBusShortCircultFail(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseInputFlowUnbalance(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseOutputCapMaintain(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSParseFilterCutoffTimes(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxWarnStateValTwoSync(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxWarnStateValTwoUnusual(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxWarnStateValTwoOverrun(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxWarnStateValThreeUnusual(BYTE YxVal)
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
BOOL CUpsKeShiDa::UPSYxWarnStateValSix(BYTE YxVal)
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
BOOL CUpsKeShiDa::GetDevCommState (void )
{
	if(m_bLinkStatus)
		return COM_DEV_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}

