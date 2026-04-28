/// \文件:	NanziPDS.cpp
/// \概要:	NanziPDS 协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-09-11



#include "NanziPDS.h"
#include "../../share/global.h"

#define DEV_MASTER_STATION_ADDR_MIN			0x01
#define DEV_MASTER_STATION_ADDR_MAX			0x0F
#define PREBLOCK_ADDR_MIN					0x10
#define PREBLOCK_ADDR_MAX					0x3F
#define ONTIMEFLAG							300
#define ANALOG_TELEMETRY_MESSAGE_DATA_NUM	3
#define ANALOG_SWITCHES_NUM					32
#define COMSTATUS_ONLINE					1
#define COMSTATUS_FAULT						0

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CNanziPDS::CNanziPDS()
{
	m_bSourceAddress = 0;
	m_bDestinationAddress = 0;
	m_bPriority = 0;
	m_bPreSetSystemClockSucFlag = 0;
	m_iOnTime = 0;
	m_tLastTime = 0;
	m_byLinkStatus = COMSTATUS_ONLINE;
	m_bySendCount = 0;
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CNanziPDS::~CNanziPDS()
{

}

// --------------------------------------------------------
/// \概要:	获得发送报文
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	if((m_bSourceAddress != 0) && (m_iOnTime > ONTIMEFLAG))
	{
		PresetSystemClock(buf, len);
		m_bPreSetSystemClockSucFlag = 1;
		time(&m_tLastTime);
		m_iOnTime = 0;
#if 0
		for(int i = 0; i < len; i++)
			printf("%02x ", buf[i]);
		printf("\n");
#endif
		return TRUE;
	}

	if(m_bPreSetSystemClockSucFlag == 1)
	{
		ClockSync(buf, len);
		m_bPreSetSystemClockSucFlag = 0;
#if 0
		for(int i = 0; i < len; i++)
			printf("%02x ", buf[i]);
		printf("\n");
#endif
	}

	m_bySendCount++;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	解析获得的报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::ProcessProtocolBuf(BYTE *buf, int len)
{
	if(!WhetherBufValue(buf, len))
	{
		printf("----FUNC = %s LINE = %d ProcessProtocolBuf Buf Recv Err!!!\n----", __func__, __LINE__);
		return FALSE;
	}

	WORD wYcSort = 0;
	WORD wSerialNo = (WORD)m_pMethod->GetSerialNo(m_byLineNo, m_bSourceAddress);
//	printf("----FUNC = %s LINE = %d wSerialNo = %d m_bSourceAddress = %d m_bDestinationAddress = %d m_byLineNo = %d----\n", __func__, __LINE__, wSerialNo, m_bSourceAddress, m_bDestinationAddress, m_byLineNo);
	if(wSerialNo == -1)
		return FALSE;
	switch(buf[8])
	{
		case 0x01:
			wYcSort = 0;
			ThreePhaseCurrentYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x02:
			wYcSort = 1;
			PowerFactorYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x03:
			wYcSort = 2;
			ThreePhaseVoltageYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x04:
			wYcSort = 3;
			ThreePahseBetweenVoltageYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x05:
			wYcSort = 4;
			MainVariableTemperatureYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x06:
			wYcSort = 5;
			StraightFlowYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x07:
			wYcSort = 6;
			ThreePhaseCurrentYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x08:
			wYcSort = 7;
			PowerFactorYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x09:
			wYcSort = 8;
			ThreePhaseVoltageYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x0A:
			wYcSort = 9;
			ThreePahseBetweenVoltageYc(buf + 10, wYcSort, wSerialNo);
			break;

		case 0x10:
			NormalStateQuantityMessageYx(buf + 10, wSerialNo);
			break;

		case 0x13:
			SoeMessageDeal(buf + 10, wSerialNo);
			break;

#if 0
		case 0xA7:
			ProtectiveActionInformation(buf + 8);
			break;
#endif
	}
	m_bySendCount = 0;
	m_byLinkStatus = COMSTATUS_ONLINE;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	初始化
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::Init(BYTE byLineNo)
{

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	时间处理
// --------------------------------------------------------
void CNanziPDS::TimerProc()
{
	time_t onTime;
	time(&onTime);
	m_iOnTime = (int)(onTime - m_tLastTime);

	if(m_bySendCount > 10)
	{
		m_bySendCount = 0;
		if(m_byLinkStatus)
		{
			m_byLinkStatus = COMSTATUS_FAULT;
			printf("NanziPDS %d unlink\n", m_bSourceAddress);
		}
	}
}

// --------------------------------------------------------
/// \概要:	发送广播预置时钟
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::PresetSystemClock(BYTE *buf, int &len)
{
	int index = 0;
	BYTE bPreBuf[8];

	time_t timeFlag;
	struct tm *localTime;
	localTime = GetLocalTime(&timeFlag);

	bPreBuf[index++] = 0x3D;
	bPreBuf[index++] = 0x07;
	bPreBuf[index++] = localTime->tm_sec;
	bPreBuf[index++] = localTime->tm_min;
	bPreBuf[index++] = localTime->tm_hour;
	bPreBuf[index++] = localTime->tm_mday;
	bPreBuf[index++] = localTime->tm_mon + 1;
	bPreBuf[index++] = localTime->tm_year - 100;

	len = MakeFrame(bPreBuf, buf, index);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	时钟同步信号
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::ClockSync(BYTE *buf, int &len)
{
	int index = 0;
	BYTE bPreBuf[8];

	bPreBuf[index++] = 0x3E;
	bPreBuf[index++] = 0x01;

	len = MakeFrame(bPreBuf, buf, index);
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	获取本地时钟
///
/// \参数:	timeFlag
///
/// \返回:	struct tm*
// --------------------------------------------------------
struct tm* CNanziPDS::GetLocalTime(time_t *timeFlag)
{
	struct tm *localTime;
	time(timeFlag);
	localTime = localtime(timeFlag);

	return localTime;
}

int CNanziPDS::MakeFrame(BYTE *bPreBuf, BYTE *buf, int len)
{
	struct can_frame frame;
	memset(&frame, 0, sizeof(frame));
	BYTE bCanId[4] = {"\0"};
	bCanId[0] = m_bDestinationAddress << 5;
	bCanId[1] = ((m_bDestinationAddress >> 3) & 0x1F) | ((m_bSourceAddress << 5) & 0xA0);
	bCanId[2] = m_bSourceAddress >> 3;
	bCanId[3] = 0x80;

	memcpy(&frame.can_id, bCanId, sizeof(bCanId));
	frame.can_dlc = len;
	memcpy(frame.data, bPreBuf, len);

	memcpy(buf, &frame, sizeof(frame));

	return sizeof(frame);
}

// --------------------------------------------------------
/// \概要:	判断接收的报文是否有效
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::WhetherBufValue(BYTE *buf, int len)
{
	if(sizeof(m_structFrame) != len)
	{
		printf("----FUNC = %s LINE = %d Received Message Length Err!!----\n", __func__, __LINE__);
		return FALSE;
	}

	BYTE can_id[4] = {'\0'};
	for(int i = 0; i < (int)sizeof(can_id); i++)
		can_id[i] = buf[i];

	m_bSourceAddress = (((can_id[1] << 8) | can_id[0]) >> 5);
	m_bDestinationAddress = (((can_id[2] << 8) | can_id[1]) >> 5) & 0x00FF;
	m_bPriority = can_id[2] & 0x10;

	if((m_bDestinationAddress < DEV_MASTER_STATION_ADDR_MIN) || (m_bDestinationAddress > DEV_MASTER_STATION_ADDR_MAX))
	{
		printf("----FUNC = %s LINE = %d Device Host Addr Out Of Range!!----\n", __func__, __LINE__);
		m_bDestinationAddress = 0;
		return FALSE;
	}

	if((m_bSourceAddress < PREBLOCK_ADDR_MIN) || (m_bSourceAddress > PREBLOCK_ADDR_MAX))
	{
		printf("----FUNC = %s LINE = %d Preblock Addr Out Of Range!!----\n", __func__, __LINE__);
		m_bSourceAddress = 0;
		return FALSE;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	三相电流遥测信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::ThreePhaseCurrentYc(BYTE *buf, WORD wYcSort, WORD wSerialNo)
{
	float YcVal = 0;

	for(WORD YcNo = 0; YcNo < ANALOG_TELEMETRY_MESSAGE_DATA_NUM; YcNo++)
	{
		YcVal = buf[YcNo * 2] | (buf[YcNo * 2 + 1] << 8);
	//	printf("----FUNC = %s LINE = %d wSerialNo = %d YcVal = %f----\n", __func__, __LINE__, wSerialNo, YcVal);
		m_pMethod->SetYcData(wSerialNo, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM, YcVal);
	//	SetVal<float>(YcType, YcVal, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	有功、无功、功率因数遥测信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::PowerFactorYc(BYTE *buf, WORD wYcSort, WORD wSerialNo)
{
	float YcVal = 0;
	WORD sTemp = 0;

	for(WORD YcNo = 0; YcNo < ANALOG_TELEMETRY_MESSAGE_DATA_NUM; YcNo++)
	{
		sTemp = buf[YcNo * 2] | (buf[YcNo * 2 + 1] << 8);
		if (sTemp & 0x1000)
		{
			sTemp = sTemp & 0x0FFF;
			YcVal = -sTemp;
		}
		else
			YcVal = sTemp;
		
		//printf("----PowerFactorYc: FUNC = %s LINE = %d wSerialNo = %d YcVal = %f----\n", __func__, __LINE__, wSerialNo, YcVal);
		m_pMethod->SetYcData(wSerialNo, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM, YcVal);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	三相相电压遥测信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::ThreePhaseVoltageYc(BYTE *buf, WORD wYcSort, WORD wSerialNo)
{
	float YcVal = 0;

	for(WORD YcNo = 0; YcNo < ANALOG_TELEMETRY_MESSAGE_DATA_NUM; YcNo++)
	{
		YcVal = buf[YcNo * 2] | (buf[YcNo * 2 + 1] << 8);
	//	printf("----FUNC = %s LINE = %d wSerialNo = %d YcVal = %f----\n", __func__, __LINE__, wSerialNo, YcVal);
		m_pMethod->SetYcData(wSerialNo, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM, YcVal);
	//	SetVal<float>(YcType, YcVal, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	三相相间电压遥测信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::ThreePahseBetweenVoltageYc(BYTE *buf, WORD wYcSort, WORD wSerialNo)
{
	float YcVal = 0;

	for(WORD YcNo = 0; YcNo < ANALOG_TELEMETRY_MESSAGE_DATA_NUM; YcNo++)
	{
		YcVal = buf[YcNo * 2] | (buf[YcNo * 2 + 1] << 8);
	//	printf("----FUNC = %s LINE = %d wSerialNo = %d YcVal = %f----\n", __func__, __LINE__, wSerialNo, YcVal);
		m_pMethod->SetYcData(wSerialNo, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM, YcVal);
	//	SetVal<float>(YcType, YcVal, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	主变温度遥测信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::MainVariableTemperatureYc(BYTE *buf, WORD wYcSort, WORD wSerialNo)
{
	float YcVal = 0;

	for(WORD YcNo = 0; YcNo < ANALOG_TELEMETRY_MESSAGE_DATA_NUM; YcNo++)
	{
		YcVal = buf[YcNo * 2] | (buf[YcNo * 2 + 1] << 8);
	//	printf("----FUNC = %s LINE = %d wSerialNo = %d YcVal = %f----\n", __func__, __LINE__, wSerialNo, YcVal);
		m_pMethod->SetYcData(wSerialNo, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM, YcVal);
	//	SetVal<float>(YcType, YcVal, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	直流量遥测信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::StraightFlowYc(BYTE *buf, WORD wYcSort, WORD wSerialNo)
{
	float YcVal = 0;

	for(WORD YcNo = 0; YcNo < ANALOG_TELEMETRY_MESSAGE_DATA_NUM; YcNo++)
	{
		YcVal = buf[YcNo * 2] | (buf[YcNo * 2 + 1] << 8);
	//	printf("----FUNC = %s LINE = %d wSerialNo = %d YcVal = %f----\n", __func__, __LINE__, wSerialNo, YcVal);
		m_pMethod->SetYcData(wSerialNo, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM, YcVal);
	//	SetVal<float>(YcType, YcVal, YcNo + wYcSort * ANALOG_TELEMETRY_MESSAGE_DATA_NUM);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	普通状态量报文遥信
///
/// \参数:	buf
/// \参数:	wYxSort
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::NormalStateQuantityMessageYx(BYTE *buf, WORD wSerialNo)
{
	int iYxval = 0;
	switch(buf[0])
	{
		case 0x01:
			iYxval = buf[1] | (buf[2] << 8) | (buf[3] << 16) | (buf[4] << 24);
			for(WORD YxNo = 0; YxNo < ANALOG_SWITCHES_NUM; YxNo++)
			{
				if(((iYxval >> YxNo) & 0x01) == 0x01)
					m_pMethod->SetYxData(wSerialNo, YxNo, 1);
				//	SetVal<int>(YxType, 1, YxNo);
				else if(((iYxval >> YxNo) & 0x01) == 0x00)
					m_pMethod->SetYxData(wSerialNo, YxNo, 0);
				//	SetVal<int>(YxType, 0, YxNo);
			}
			break;

		case 0x02:
			iYxval = buf[1] | (buf[2] << 8) | (buf[3] << 16) | (buf[4] << 24);
			for(WORD YxNo = 0; YxNo < ANALOG_SWITCHES_NUM; YxNo++)
			{
				if(((iYxval >> YxNo) & 0x01) == 0x01)
					m_pMethod->SetYxData(wSerialNo, YxNo + ANALOG_SWITCHES_NUM, 1);
				else if(((iYxval >> YxNo) & 0x01) == 0x00)
					m_pMethod->SetYxData(wSerialNo, YxNo + ANALOG_SWITCHES_NUM, 0);
			}
			break;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	SOE报文信息处理
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::SoeMessageDeal(BYTE *buf, WORD wSerialNo)
{
	BYTE bSoeVal = 0;
	TIMEDATA SoeData;
	switch(buf[4] & 0x80)
	{
		case 0x80:
			bSoeVal = 1;
			break;

		case 0x00:
			bSoeVal = 0;
			break;
	}

	time_t timeFlag;
	struct tm *localTime;
	localTime = GetLocalTime(&timeFlag);

	SoeData.MiSec = buf[2] | (buf[3] << 8);
	SoeData.Second = localTime->tm_sec;
	SoeData.Minute = buf[1];
	SoeData.Hour = buf[0];
	SoeData.Day = localTime->tm_mday;
	SoeData.Month = localTime->tm_mon + 1;
	SoeData.Year = localTime->tm_year - 100;

	m_pMethod->(wSerialNo, buf[4] & 0x7F, bSoeVal, &SoeData);

	return TRUE;
}

#if 0
// --------------------------------------------------------
/// \概要:	保护动作信息
///
/// \参数:	buf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPD::ProtectiveActionInformation(BYTE *buf)
{
	if(buf[1] == 0x00)
	{
		time_t timeFlag;
		struct tm *localTime;
		localTime = GetLocalTime(&timeFlag);

		m_tWarningMessageTimeFlag.MiSec = buf[3] | (buf[4] << 8);
		m_tWarningMessageTimeFlag.Second = localTime->tm_sec;
		m_tWarningMessageTimeFlag.Minute = buf[5];
		m_tWarningMessageTimeFlag.Hour = buf[6];
		m_tWarningMessageTimeFlag.Day = localTime->tm_mday;
		m_tWarningMessageTimeFlag.Month = localTime->tm_mon + 1;
		m_tWarningMessageTimeFlag.Year = localTime->tm_year - 100;
	}
	else
		m_pMethod->(m_SerialNo, buf[4], 1, &m_tWarningMessageTimeFlag);

	return TRUE;
}
#endif

#if 0
template <class Type>
BOOL CNanziPDS::SetVal(BYTE bBype, Type Val, WORD wPnt)
{

	WORD wSerialNo = (WORD)m_pMethod->GetSerialNo(m_byLineNo, m_bSourceAddress);
	switch(bBype)
	{
		case YcType:
			m_pMethod->SetYcData(wSerialNo, wPnt, Val);
			break;

		case YxType:
			m_pMethod->SetYxData(wSerialNo, wPnt, Val);
			break;

	}
}
#endif

// --------------------------------------------------------
/// \概要:	获得设备装置状态
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CNanziPDS::GetDevCommState()
{
	if(m_byLinkStatus == COMSTATUS_ONLINE)
		return COM_DEV_NORMAL;
	else if(m_byLinkStatus == COMSTATUS_FAULT)
		return COM_DEV_ABNORMAL;

	return COM_DEV_NORMAL;
}
