#include "ModBusEatonACB.h"
#include <math.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusEatonACB
 *      Method:  CModBusEatonACB
 *
 *--------------------------------------------------------------------------------------
 */
#define GPOUT "/dev/esd_gpout"
#define PXR20_MODBUS_TCP 30
#define PXR20_MODBUS_RTU 31
#define PXR25_MODBUS_TCP 32
#define PXR25_MODBUS_RTU 33

#define FUNCCODE_0X02 0X02
#define FUNCCODE_0X03 0X03
#define FUNCCODE_0X04 0X04
#define FUNCCODE_0X10 0X10

#define REG_DEFINE_8193 (8193 - 1)
#define REG_LATEST_EVENT_ID_8194 (8194 - 1)
#define REG_SET_EVENT_ID_8198 (8198 - 1)
#define REG_GET_EVENT_INFO_8204 (8204 - 1)
#define REG_GET_EVENT_DATA_8215 (8215 - 1)
#define REG_GET_ALARM_TYPE_8212 (8212 - 1)

#define REG_SYNC_TIME_2921 (2921 - 1)

#define REG_GET_DEV_TYPE_3004 3003
#define REG_SET_IG_VALUE_3001 (3001 - 1)
#define REG_GET_IG_VALUE_3016 (3016 - 1)

#define REG_TRIP_EVENT 0x80FF
#define REG_ALARM_EVENT 0x81FF

#define REG_TRIP_WAVE_IA 0x000A
#define REG_TRIP_WAVE_IB 0x000B
#define REG_TRIP_WAVE_IC 0x000C
#define REG_TRIP_WAVE_IN 0x000D
#define REG_TRIP_WAVE_IG 0x000E
#define REG_TRIP_WAVE_IG_S 0x000F

#define REG_TRIP_WAVE_VAB 0x0010
#define REG_TRIP_WAVE_VBC 0x0011
#define REG_TRIP_WAVE_VCA 0x0012

#define REG_TRIP_WAVE_COUNT 768		  // 读取768个寄存器数据
#define REG_TRIP_READ_WAVE_COUNT 0X78 // 一次读取0x78个寄存器数据

#define THIRTY_MIN (60 * 15) //

#define DATA_PATH "/mynand/data/"
#define LINK_PATH "/mynand/data/link.status"
#define CONFIG_PATH "/mynand/config/"
#define WAVE_DATA_PATH "/mynand/data/wavedata/"
#define EVENT_DATA_PATH "/mynand/data/eventdata/"
#define MASTER_BUSLINE_PATH "/mynand/config/ModBusMaster/Bus01.ini"

#define MAX_WAVE_DATA_SIZE 15.0 // MB

#define SOFT_YX_POST_START 12 // 软遥信起始位置
#define TRIP_BAOHU_SOFT_YX 50
#define ALARM_BAOHU_SOFT_YX 63

#define PXR20_QUICK_COUNT 4 // pxr20快速报文数量
#define PXR25_QUICK_COUNT 6 // pxr25快速报文数量

#define EVENT_YC_POST_START 59

// 辅助宏：生成N位的掩码
#define MASK(n) (((uint32_t)1 << (n)) - 1)

/*
PXR20  快速报文
4609  12   1  1     //前两个寄存器是遥脉  对应遥脉点 12
1001  16   2  1
非快速报文
4719 4   3    1     //第三个第四个寄存器是遥测 对应遥测点  8
4763 4   1     9  //对应遥测点相反
4851 20  1     11
4915 6     1     21
4923 2    1     24
4959 6     3      9
*/
static RealTimeRegInfo_t pxr20RegInfo[13] = {{4609 - 1, 8, 1, 1}, // 前两个寄存器是遥脉  对应遥脉点 12
											 {1001 - 1, 16, 2, 1},
											 {4617 - 1, 2, 1, 4}, // ig
											 {4619 - 1, 2, 1, 5}, // in

											 // 非快速报文
											 {4661 - 1, 2, 1, 16},
											 {4763 - 1, 4, 1, 20},
											 {4851 - 1, 12, 1, 27},
											 {4915 - 1, 6, 1, 49},
											 {4923 - 1, 2, 1, 52},
											 {4959 - 1, 50, 3, 25},
											 {5009 - 1, 8, 3, 48},
											 {4863 - 1, 4, 1, 33},	// minig maxig
											 {4867 - 1, 4, 1, 35}}; // minin maxin

/*
PXR25  快速报文
4609 12   1  1     //前两个寄存器是遥脉  对应遥脉点 12
4623 6  	1   6
4631 6    1   9
1001 16   2  1
非快速报文
4651 6   1    12
4659 4   1   15
4697 2    1   17
4719 4   3    1     //第三个第四个寄存器是遥测 对应遥测点  19
6259 14    3  2
4763 4   1     20  //对应遥测点相反
4797 4  1     22
4845 50   1   24
4915 6     1     49
4923 14    1    52
4959 6     3      9
*/

static RealTimeRegInfo_t pxr25RegInfo[21] = {{4609 - 1, 8, 1, 1}, // 前两个寄存器是遥脉  对应遥脉点 12
											 {4623 - 1, 6, 1, 6},
											 {4631 - 1, 6, 1, 9},
											 {1001 - 1, 16, 2, 1},
											 {4617 - 1, 2, 1, 4}, // ig
											 {4619 - 1, 2, 1, 5}, // in
											 // 非快速报文
											 {4651 - 1, 6, 1, 12},
											 {4659 - 1, 4, 1, 15},
											 {4697 - 1, 2, 1, 17},
											 {4719 - 1, 4, 3, 1}, // 第三个第四个寄存器是遥测 对应遥测点  19
											 {6259 - 1, 14, 3, 2},
											 {4763 - 1, 4, 1, 20}, // 对应遥测点相反
											 {4797 - 1, 4, 1, 22},
											 {4845 - 1, 18, 1, 24},
											 {4915 - 1, 6, 1, 49},
											 {4923 - 1, 14, 1, 52},
											 {4959 - 1, 50, 3, 25},
											 {5009 - 1, 8, 3, 48},
											 {4863 - 1, 4, 1, 33}, // minig maxig
											 {4867 - 1, 4, 1, 35}, // minig maxig
											 {4871 - 1, 24, 1, 37}};

static int m_initflag = 1; // 创建文件初始化
static bool m_isLink = true;

CModBusEatonACB::CModBusEatonACB()
{
	m_thirtyTimeMin = THIRTY_MIN + 3;

	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_errorCount = 0;
	m_byRecvflag = 0;

	m_fiveTimeSeconds = 0;
	m_tenTimeSeconds = 0;
	m_curEventType = EATONACB_NULL;
	m_nextEventType = EATONACB_NULL;
	m_childEventType = STATUS_NULL;

	m_devType = -1;
	memset(m_waveType, 0, sizeof(m_waveType));
	m_waveMaxType = 0;
	m_waveCurType = 0;
	memset(&m_waveConfig, 0, sizeof(EatonACBWaveConfig_t));

	m_waveConfig.fileVer = 1997;
	m_waveConfig.powerHZ = 50.0;
	m_waveConfig.sampleNum = 1;
	m_waveConfig.sampleFreq = 3200;
	m_waveConfig.samplePoint = 384;
	m_waveConfig.dataType = 3;
	m_waveConfig.timeRate = 1.0;
	m_waveConfig.timeQos = 0;
	// IA/IB/IC/IN/IG/Vab/Vbc/Vca
	char tmp[8][5] = {
		"IA",
		"IB",
		"IC",
		"IN",
		"IG",
		"Vab",
		"Vbc",
		"Vca"};
	for (int i = 0; i < sizeof(m_waveConfig.channelInfo) / sizeof(EatonACBChannelInfo_t); i++)
	{
		char unit = {0};
		if (i < 5)
		{
			unit = 'A';
		}
		else
		{
			unit = 'V';
		}
		memcpy(m_waveConfig.channelInfo[i].channelName, tmp[i], sizeof(tmp[i]));
		strncpy(m_waveConfig.channelInfo[i].channelName, tmp[i], 4);
		m_waveConfig.channelInfo[i].channelName[4] = '\0';

		m_waveConfig.channelInfo[i].channelNo = i + 1;
		m_waveConfig.channelInfo[i].channelUnit = unit;
		m_waveConfig.channelInfo[i].fCoefA = 1;
		m_waveConfig.channelInfo[i].offsetB = 0;
		m_waveConfig.channelInfo[i].skew = 0;
		m_waveConfig.channelInfo[i].fMin = -252000;
		m_waveConfig.channelInfo[i].fMax = 252000;
		m_waveConfig.channelInfo[i].transRatio1 = 0;
		m_waveConfig.channelInfo[i].transRatio2 = 0;
		m_waveConfig.channelInfo[i].pOrS = 0;
	}
	m_realTimeSendCount = 0;
	m_token = 0;
	m_beginTime = time(NULL);
}

CModBusEatonACB::~CModBusEatonACB()
{
} /* -----  end of method CModBusSCJZ9SY::~CModBusSCJZ9SY  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSCJZ9SY
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */

BOOL CModBusEatonACB::WhetherBufValue(BYTE *buf, int &len)
{
	BYTE *pointer = buf;
	WORD wCrc;
	char szBuf[512] = "";
	int pos = 0;
	int funcCode = 0;

	if (m_wModuleType == PXR25_MODBUS_TCP || m_wModuleType == PXR20_MODBUS_TCP)
	{
		pointer += 6;
	}

	while (len >= 4)
	{
		// 判断地址
		if (*pointer != m_wDevAddr)
		{
			sprintf(szBuf, "CModBusEatonACB recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			goto DEFAULT;
		}
		funcCode = *(pointer + 1);

		if ((funcCode & 0x80) == 0x80)
		{
			// 错误码
			sprintf(szBuf, "CModBusEatonACB error funcode [ 0x%2x ] \n", funcCode);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

			if ((m_byRecvflag == REAL_TIME_STATUS_GET_PXR20_VALUE) || (m_byRecvflag == REAL_TIME_STATUS_GET_PXR25_VALUE))
			{

				m_realTimeCurFrame++;
				if (m_realTimeCurFrame >= m_realTimeMaxFrameCount)
				{

					m_childEventType = TRIP_EVENT_STATUS_SET;
					m_toraEventType = EATONACB_ALARM_EVENT;
				}
			}
			if (m_byRecvflag == TRIP_WAVE_STATUS_SET_LASTEST_EVENTID)
			{

				m_childEventType = TRIP_EVENT_STATUS_SET;
				m_toraEventType = EATONACB_ALARM_EVENT;
			}

			return FALSE;
		}
		switch (funcCode)
		{
		case FUNCCODE_0X03:
		case FUNCCODE_0X04:
		case FUNCCODE_0X02:
		{
			// 判断长度
			if (*(pointer + 2) > len - 3)
			{
				sprintf(szBuf, "CModBusEatonACB recv len err recv len=%d\n", *(pointer + 2));
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				goto DEFAULT;
			}
			if (m_wModuleType == PXR25_MODBUS_RTU || m_wModuleType == PXR20_MODBUS_RTU)
			{
				// 判断校验
				wCrc = GetCrc(pointer, (*(pointer + 2) + 3));
				if (*(pointer + (*(pointer + 2) + 3)) != HIBYTE(wCrc) || *(pointer + (*(pointer + 2) + 4)) != LOBYTE(wCrc))
				{
					sprintf(szBuf, "CModBusEatonACB recv crc err recvcrc=%.2x%.2x, localcrc=%.2x%.2x\n",
							*(pointer + (*(pointer + 2) + 3)), *(pointer + (*(pointer + 2) + 4)),
							HIBYTE(wCrc), LOBYTE(wCrc));
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					goto DEFAULT;
				}
			}

			len = *(pointer + 2) + 5;
		}
		break;
		case FUNCCODE_0X10:
		{
			if (m_wModuleType == PXR20_MODBUS_RTU || m_wModuleType == PXR25_MODBUS_RTU)
			{
				// 判断校验
				wCrc = GetCrc(pointer, 6);
				if (*(pointer + 6) != HIBYTE(wCrc) || *(pointer + 7) != LOBYTE(wCrc))
				{
					sprintf(szBuf, "CModBusEatonACB recv crc err recvcrc=%.2x%.2x, localcrc=%.2x%.2x\n",
							*(pointer + 6), *(pointer + 7),
							HIBYTE(wCrc), LOBYTE(wCrc));
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					goto DEFAULT;
				}
			}
			len = 8;
		}
		break;
		default:
			printf("no support funcode [ %2x ] \n", funcCode);
			return FALSE;
			break;
		}

		buf = buf + pos;

		return TRUE;
	DEFAULT:
		pointer++;
		len--;
		pos++;
	}
	return FALSE;
} /* -----  end of method CModBusSCJZ9SY::WhetherBufValue  ----- */

float CModBusEatonACB::HexToFloat32(const uint8_t bytes[4], int order)
{
	int32_t combined = 0;

	switch (order)
	{
	case MODBUS_BYTEORDER_TYPE_ABCD: // 标准Modbus
		combined = (static_cast<int32_t>(bytes[0]) << 24) |
				   (static_cast<int32_t>(bytes[1]) << 16) |
				   (static_cast<int32_t>(bytes[2]) << 8) |
				   (static_cast<int32_t>(bytes[3]));
		break;

	case MODBUS_BYTEORDER_TYPE_BADC: // 字节交换
		combined = (static_cast<int32_t>(bytes[1]) << 24) |
				   (static_cast<int32_t>(bytes[0]) << 16) |
				   (static_cast<int32_t>(bytes[3]) << 8) |
				   (static_cast<int32_t>(bytes[2]));
		break;

	case MODBUS_BYTEORDER_TYPE_CDAB: // 字交换
		combined = (static_cast<int32_t>(bytes[2]) << 24) |
				   (static_cast<int32_t>(bytes[3]) << 16) |
				   (static_cast<int32_t>(bytes[0]) << 8) |
				   (static_cast<int32_t>(bytes[1]));
		break;

	case MODBUS_BYTEORDER_TYPE_DCBA: // 小端序
		combined = (static_cast<int32_t>(bytes[3]) << 24) |
				   (static_cast<int32_t>(bytes[2]) << 16) |
				   (static_cast<int32_t>(bytes[1]) << 8) |
				   (static_cast<int32_t>(bytes[0]));
		break;

	default:
		throw std::invalid_argument("Invalid byte order");
	}

	float result;
	memcpy(&result, &combined, sizeof(float));
	return result;
}

int CModBusEatonACB::HexToInt32(const uint8_t bytes[4], int order)
{
	int32_t combined = 0;

	switch (order)
	{
	case MODBUS_BYTEORDER_TYPE_ABCD: // 标准Modbus
		combined = (static_cast<int32_t>(bytes[0]) << 24) |
				   (static_cast<int32_t>(bytes[1]) << 16) |
				   (static_cast<int32_t>(bytes[2]) << 8) |
				   (static_cast<int32_t>(bytes[3]));
		break;

	case MODBUS_BYTEORDER_TYPE_BADC: // 字节交换
		combined = (static_cast<int32_t>(bytes[1]) << 24) |
				   (static_cast<int32_t>(bytes[0]) << 16) |
				   (static_cast<int32_t>(bytes[3]) << 8) |
				   (static_cast<int32_t>(bytes[2]));
		break;

	case MODBUS_BYTEORDER_TYPE_CDAB: // 字交换
		combined = (static_cast<int32_t>(bytes[2]) << 24) |
				   (static_cast<int32_t>(bytes[3]) << 16) |
				   (static_cast<int32_t>(bytes[0]) << 8) |
				   (static_cast<int32_t>(bytes[1]));
		break;

	case MODBUS_BYTEORDER_TYPE_DCBA: // 小端序
		combined = (static_cast<int32_t>(bytes[3]) << 24) |
				   (static_cast<int32_t>(bytes[2]) << 16) |
				   (static_cast<int32_t>(bytes[1]) << 8) |
				   (static_cast<int32_t>(bytes[0]));
		break;

	default:
		throw std::invalid_argument("Invalid byte order");
	}
	int result;
	memcpy(&result, &combined, sizeof(int));
	return result;
}

std::vector<uint8_t> CModBusEatonACB::ByBitOps(int value, int order)
{
	std::vector<uint8_t> result(4);
	uint32_t uvalue = static_cast<uint32_t>(value);

	switch (order)
	{
	case MODBUS_BYTEORDER_TYPE_ABCD: // 12 34 56 78
		result[0] = static_cast<uint8_t>((uvalue >> 24) & 0xFF);
		result[1] = static_cast<uint8_t>((uvalue >> 16) & 0xFF);
		result[2] = static_cast<uint8_t>((uvalue >> 8) & 0xFF);
		result[3] = static_cast<uint8_t>(uvalue & 0xFF);
		break;

	case MODBUS_BYTEORDER_TYPE_CDAB: // 56 78 12 34
		result[0] = static_cast<uint8_t>((uvalue >> 8) & 0xFF);
		result[1] = static_cast<uint8_t>(uvalue & 0xFF);
		result[2] = static_cast<uint8_t>((uvalue >> 24) & 0xFF);
		result[3] = static_cast<uint8_t>((uvalue >> 16) & 0xFF);
		break;

	case MODBUS_BYTEORDER_TYPE_BADC: // 34 12 78 56
		result[0] = static_cast<uint8_t>((uvalue >> 16) & 0xFF);
		result[1] = static_cast<uint8_t>((uvalue >> 24) & 0xFF);
		result[2] = static_cast<uint8_t>((uvalue >> 0) & 0xFF);
		result[3] = static_cast<uint8_t>((uvalue >> 8) & 0xFF);
		break;

	case MODBUS_BYTEORDER_TYPE_DCBA: // 78 56 34 12
		result[0] = static_cast<uint8_t>(uvalue & 0xFF);
		result[1] = static_cast<uint8_t>((uvalue >> 8) & 0xFF);
		result[2] = static_cast<uint8_t>((uvalue >> 16) & 0xFF);
		result[3] = static_cast<uint8_t>((uvalue >> 24) & 0xFF);
		break;
	}

	return result;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusEatonACB
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusEatonACB::ProcessRecvBuf(BYTE *buf, int len)
{
	int i;
	char szBuf[512];
	if (m_wModuleType == PXR20_MODBUS_TCP || m_wModuleType == PXR25_MODBUS_TCP)
	{
		buf += 6;
		len -= 6;
	}
	switch (m_byRecvflag)
	{

	case SYNC_TIME:
	{

		if (!ProcessRecvSetCommon(buf, len, REG_SYNC_TIME_2921, 8))
		{
			return FALSE;
		}
		m_curEventType = EATONACB_NULL;
		m_childEventType = STATUS_NULL;
		break;
	}
	case STATUS_GET_DEV_TYPE:
	{
		ProcessRecvDevType(buf, len);

		if (m_curEventType == EATONACB_REALTIME)
		{
			if (m_devType == EATONACB_PX25)
			{
				m_childEventType = REAL_TIME_STATUS_GET_PXR25_VALUE;
				if (m_realTimeSendCount < 4)
				{
					m_realTimeMaxFrameCount = PXR25_QUICK_COUNT;
					m_realTimeSendCount++;
				}
				else
				{
					m_realTimeSendCount = 0;
					m_realTimeMaxFrameCount = sizeof(pxr25RegInfo) / sizeof(RealTimeRegInfo_t);
				}
			}
			else
			{

				m_childEventType = REAL_TIME_STATUS_GET_PXR20_VALUE;
				if (m_realTimeSendCount < 4)
				{
					m_realTimeMaxFrameCount = PXR20_QUICK_COUNT;
					m_realTimeSendCount++;
				}
				else
				{
					m_realTimeSendCount = 0;
					m_realTimeMaxFrameCount = sizeof(pxr20RegInfo) / sizeof(RealTimeRegInfo_t);
				}
			}
		}
		else if (m_curEventType == EATONACB_TRIP_EVENT)
		{
			m_childEventType = TRIP_EVENT_STATUS_SET;
			m_toraEventType = EATONACB_TRIP_EVENT;
		}

		break;
	}
	case TRIP_EVENT_STATUS_SET:
	{
		if (!ProcessRecvSetCommon(buf, len, REG_DEFINE_8193, 1))
		{
			return FALSE;
		}
		m_readEventProcess = 1;
		m_childEventType = TRIP_EVENT_STATUS_GET_LASTEST_EVENTID;
		break;
	}
	case TRIP_EVENT_STATUS_GET_LASTEST_EVENTID:
	{
		if (!ProcessRecvGetLastestEventID(buf, len))
		{
			if (m_toraEventType == EATONACB_TRIP_EVENT)
			{
				m_childEventType = TRIP_EVENT_STATUS_SET;
				m_toraEventType = EATONACB_ALARM_EVENT;
			}
			else if (m_toraEventType == EATONACB_ALARM_EVENT)
			{
				m_curEventType = EATONACB_NULL;
				m_childEventType = STATUS_NULL;
			}
			return TRUE;
		}
		m_childEventType = TRIP_EVENT_STATUS_SET_LASTEST_EVENTID;
		break;
	}
	case TRIP_EVENT_STATUS_SET_LASTEST_EVENTID:
	{
		if (!ProcessRecvSetCommon(buf, len, REG_SET_EVENT_ID_8198, 2))
		{
			return FALSE;
		}
		if (m_readEventProcess == 1)
		{
			m_childEventType = TRIP_EVENT_STATUS_GET_LASTEST_EVENTID;
			m_readEventProcess = 2;
		}
		else
		{
			if (m_toraEventType == EATONACB_TRIP_EVENT)
			{
				m_childEventType = TRIP_EVENT_STATUS_GET_LASTEST_EVENTINFO;
			}
			else if (m_toraEventType == EATONACB_ALARM_EVENT)
			{

				m_childEventType = ALARM_EVENT_STATUS_GET_EVENT_TYPE;
			}
		}

		break;
	}
	case ALARM_EVENT_STATUS_GET_EVENT_TYPE:
	{
		if (!ProcessRecvGetAlarmEventType(buf, len))
		{
			return FALSE;
		}
		// m_toraEventType = (EatonACBEventType)buf[0];
		m_childEventType = TRIP_EVENT_STATUS_GET_LASTEST_EVENTINFO;

		break;
	}
	case TRIP_EVENT_STATUS_GET_LASTEST_EVENTINFO:
	{
		if (!ProcessEventData(buf, len))
		{
			return FALSE;
		}
		if (m_toraEventType == EATONACB_TRIP_EVENT)
		{
			m_childEventType = TRIP_WAVE_STATUS_SET_IG_TYPE;
		}
		else
		{
			if (m_curEventId != m_maxEventId)
			{
				m_childEventType = TRIP_EVENT_STATUS_SET;
				m_toraEventType = EATONACB_ALARM_EVENT;
			}
			else
			{
				m_curEventType = EATONACB_NULL;
				m_childEventType = STATUS_NULL;
			}
		}

		break;
	}
	case TRIP_WAVE_STATUS_SET_IG_TYPE:
	{
		if (!ProcessRecvSetCommon(buf, len, REG_SET_IG_VALUE_3001, 1))
		{
			return FALSE;
		}
		m_childEventType = TRIP_WAVE_STATUS_GET_IG_TYPE;
		break;
	}
	case TRIP_WAVE_STATUS_GET_IG_TYPE:
	{
		if (!ProcessRecvGetIGType(buf, len))
		{
			return FALSE;
		}
		m_childEventType = TRIP_WAVE_STATUS_SET;
		break;
	}
	case TRIP_WAVE_STATUS_SET:
	{
		if (!ProcessRecvSetCommon(buf, len, REG_DEFINE_8193, 1))
		{
			return FALSE;
		}
		m_childEventType = TRIP_WAVE_STATUS_GET_LASTEST_EVENTID;
		break;
	}
	case TRIP_WAVE_STATUS_GET_LASTEST_EVENTID:
	{
		if (!ProcessRecvGetLastestEventID(buf, len))
		{
			if (m_toraEventType == EATONACB_TRIP_EVENT)
			{
				m_childEventType = TRIP_EVENT_STATUS_SET;
				m_toraEventType = EATONACB_ALARM_EVENT;
			}
			else
			{
				m_curEventType = EATONACB_NULL;
				m_childEventType = STATUS_NULL;
			}
			return TRUE;
		}
		m_childEventType = TRIP_WAVE_STATUS_SET_LASTEST_EVENTID;
		break;
	}
	case TRIP_WAVE_STATUS_SET_LASTEST_EVENTID:
	{
		if (!ProcessRecvSetCommon(buf, len, REG_SET_EVENT_ID_8198, 2))
		{
			return FALSE;
		}
		m_childEventType = TRIP_WAVE_STATUS_GET_LASTEST_EVENT_TIME;
		break;
	}
	case TRIP_WAVE_STATUS_GET_LASTEST_EVENT_TIME:
	{
		if (!ProcessRecvLastWaveTime(buf, len))
		{
			return FALSE;
		}
		m_childEventType = TRIP_WAVE_STATUS_GET_LASTEST_WAVEINFO;
		break;
	}
	case TRIP_WAVE_STATUS_GET_LASTEST_WAVEINFO:
	{
		if (!ProcessRecvLastWaveData(buf, len))
		{
			return FALSE;
		}
		// m_waveRegNum += REG_TRIP_READ_WAVE_COUNT;
		if (m_waveRegNum < REG_TRIP_WAVE_COUNT)
		{
			break;
		}
		else
		{
			m_waveCurType++;
			if (m_waveCurType < m_waveMaxType)
			{
				m_childEventType = TRIP_WAVE_STATUS_SET;
				m_toraEventType = EATONACB_TRIP_EVENT;
			}
			else
			{
				m_waveCurType = 0;
				MonitorAndClean(WAVE_DATA_PATH, MAX_WAVE_DATA_SIZE);
				std::string devName = m_sDevName;
				std::string fileName = WAVE_DATA_PATH + devName + "_" +
									   std::to_string(m_wDevAddr) + "_" +
									   std::to_string(m_waveConfig.waveID) + "_" +
									   std::to_string(m_waveTime.wYear) + "-" +
									   std::to_string(m_waveTime.wMonth) + "-" +
									   std::to_string(m_waveTime.wDay) + "-" +
									   std::to_string(m_waveTime.wHour) + "-" +
									   std::to_string(m_waveTime.wMinute) + "-" +
									   std::to_string(m_waveTime.wSecond) + "." +
									   std::to_string(m_waveTime.wMilliSec) + ".dat";
				// printf("WriteComtradeDataFloatFile %s\n", fileName.c_str());
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "WriteComtradeDataFloatFile %s\n", fileName.c_str());
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				if (!WriteComtradeDataFloatFile(fileName))
				{
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "WriteComtradeDataFloatFile failed\n");
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					return FALSE;
				}
				m_childEventType = TRIP_EVENT_STATUS_SET;
				m_toraEventType = EATONACB_ALARM_EVENT;
			}
		}

		break;
	}
	case REAL_TIME_STATUS_GET_PXR25_VALUE:
	{
		if (!ProcessRecvPXR25RealTime(buf, len))
		{
			return FALSE;
		}
		if (m_realTimeCurFrame >= m_realTimeMaxFrameCount)
		{
			m_childEventType = TRIP_EVENT_STATUS_SET;
			m_toraEventType = EATONACB_ALARM_EVENT;
		}

		break;
	}
	case REAL_TIME_STATUS_GET_PXR20_VALUE:
	{
		if (!ProcessRecvPXR20RealTime(buf, len))
		{
			return FALSE;
		}
		if (m_realTimeCurFrame >= m_realTimeMaxFrameCount)
		{

			m_childEventType = TRIP_EVENT_STATUS_SET;
			m_toraEventType = EATONACB_ALARM_EVENT;
		}
		break;
	}
	default:
		sprintf(szBuf, "CModBusEatonACB recv no this type\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		break;
	}

	return TRUE;
} /* -----  end of method CModBusSCJZ9SY::ProcessRecvBuf  ----- */

// 处理4609寄存器
BOOL CModBusEatonACB::ProcessRecvRealTime_4609(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	char szBuf[128] = {0};
	int index = 2;				  // 地址 + 功能码
	int byteCount = buf[index++]; // 长度
	// 前两个寄存器是遥脉
	int ympnt = 12; // 遥脉点 pxr25 12  pxr20 5 依次+1
	/*
	if (m_devType == EATONACB_PX25)
	{
		ympnt = 12;
	}
	else if (m_devType == EATONACB_PX20)
	{
		ympnt = 5;
	}*/

	int priValue = buf[index++];
	m_pMethod->SetYmData(m_SerialNo, ympnt - 1, (double)priValue);
	// printf("ymPnt = [%d], value = [%f]\n", ympnt, (double)priValue);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ympnt, priValue);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	ympnt++;
	int secValue = buf[index++];
	m_pMethod->SetYmData(m_SerialNo, ympnt - 1, (double)secValue);
	// printf("ymPnt = [%d], value = [%f]\n", ympnt, (double)secValue);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ympnt, secValue);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	ympnt++;

	int causeCode = MAKEWORD(buf[index + 1], buf[index]);
	m_pMethod->SetYmData(m_SerialNo, ympnt - 1, (double)causeCode);
	// printf("ymPnt = [%d], value = [%f]\n", ympnt, (double)causeCode);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ympnt, causeCode);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	ympnt++;
	index += 2; // 两个字节

	if (secValue == 1)
	{
		secValue = 7;
	}
	else if (secValue == 7)
	{
		secValue = 1;
	}
	else if (secValue == 8)
	{
		secValue = 6;
	}
	m_pMethod->SetYmData(m_SerialNo, ympnt - 1, (double)secValue);
	// printf("ymPnt = [%d], value = [%f]\n", ympnt, (double)secValue);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ympnt, secValue);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

	byteCount -= (index - 3);
	int ycPnt = 0;
	if (m_devType == EATONACB_PX25)
	{
		ycPnt = pxr25RegInfo[m_realTimeCurFrame].point;
	}
	else if (m_devType == EATONACB_PX20)
	{
		ycPnt = pxr20RegInfo[m_realTimeCurFrame].point;
	}
	for (int i = 0; i < byteCount; i += 4)
	{
		float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		m_pMethod->SetYcData(m_SerialNo, ycPnt - 1, value);
		index += 4;
		// printf("ycPnt = [%d], value = [%f]\n", ycPnt, value);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "ycPnt = [%d], value = [%f]\n", ycPnt, value);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		ycPnt++;
	}
	return TRUE;
}
static int unAckCondition = -1;
BOOL CModBusEatonACB::ProcessRecvRealTime_1001(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	char szBuf[128] = {0};
	int index = 2;				  // 地址 + 功能码
	int byteCount = buf[index++]; // 长度
	int yxPnt = 0;
	if (m_devType == EATONACB_PX25)
	{
		yxPnt = pxr25RegInfo[m_realTimeCurFrame].point;
	}
	else if (m_devType == EATONACB_PX20)
	{
		yxPnt = pxr20RegInfo[m_realTimeCurFrame].point;
	}

	int16_t yxValue = MAKEWORD(buf[index], buf[index + 1]);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "yxValue = [%2x]\n", yxValue);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	for (int i = 0; i < (byteCount * 8); i++)
	{
		if (i == 3 || i == 6 || i == 7 || i == 11 || i == 15)
		{
			continue;
		}
		BYTE value = (yxValue >> i) & 1; // 获取从低位开始第i位的值
		// value = rand() % 2;
		m_pMethod->SetYxData(m_SerialNo, yxPnt - 1, value);
		// printf("yxPnt = [%d], value = [%d]\n", yxPnt, value);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "yxPnt = [%d], value = [%d]\n", yxPnt, value);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		if (i == 1)
		{
			if (value == 0 && unAckCondition == 1)
			{
				TIMEDATA yxtime = {0};

				time_t rawtime;
				struct tm *timeinfo;
				// 获取当前时间
				time(&rawtime);
				timeinfo = localtime(&rawtime);
				char buffer[80] = {0};
				strftime(buffer, sizeof(buffer), "%Y-%m-%d %H-%M-%S %A", timeinfo);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "current time : [%s]\n", buffer);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

				yxtime.Year = timeinfo->tm_year;
				yxtime.Month = timeinfo->tm_mon + 1;
				yxtime.Day = timeinfo->tm_mday;
				yxtime.Hour = timeinfo->tm_hour;
				yxtime.Minute = timeinfo->tm_min;
				yxtime.Second = timeinfo->tm_sec;
				yxtime.MiSec = 0;

				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "year:[%d] month:[%d] day:[%d]\n", yxtime.Year, yxtime.Month, yxtime.Day);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

				sprintf(szBuf, "==========================reset SOE start\n");
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				for (int i = 50; i <= 75; i++)
				{
					WORD yx_temp = -1;
					m_pMethod->ReadYxData(m_SerialNo, i - 1, &yx_temp);
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "yx_temp = [%d] \n", yx_temp);
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					if (yx_temp == 1)
					{
						m_pMethod->SetYxDataWithTime(m_SerialNo, i - 1, 0, &yxtime);
						m_pMethod->SetYxData(m_SerialNo, i - 1, 0);
						memset(szBuf, 0, sizeof(szBuf));
						sprintf(szBuf, "yxPnt = [%d], value = [%d] \n", i, 0);
						OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					}
				}
				sprintf(szBuf, "==========================reset SOE end\n");
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			}
			unAckCondition = value;
		}

		yxPnt++;
	}
	return TRUE;
}

BOOL CModBusEatonACB::ProcessRecvRealTime_5009(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	char szBuf[100] = {0};
	int index = 2;				  // 地址+功能码
	int byteCount = buf[index++]; // 字节数
	int ymPnt = 0;

	if (m_devType == EATONACB_PX25)
	{
		ymPnt = pxr25RegInfo[m_realTimeCurFrame].point;
	}
	else if (m_devType == EATONACB_PX20)
	{
		ymPnt = pxr20RegInfo[m_realTimeCurFrame].point;
	}
	int runTime = 0; // 运行时间 单位 小时
	int runTimeSec = 0;
	for (int i = 0; i < byteCount; i += 4, index += 4)
	{
		// double value = MAKELONG(MAKEWORD(buf[index + 1], buf[index]), MAKEWORD(buf[index + 3], buf[index + 2]));
		float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		switch (i)
		{
		case 0:
		{
			runTimeSec += (value * 60); // 分钟
			break;
		}
		case 4:
		{
			runTime += value; // 小时
			runTimeSec += (value * 60 * 60);
			break;
		}
		case 8:
		{
			runTimeSec += (value * 24 * 60 * 60);
			runTime += (value * 24);

			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, (double)runTimeSec);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ymPnt, runTimeSec);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ymPnt++;

			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, (double)runTime);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ymPnt, runTime);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ymPnt++;
			break;
		}
		case 12:
		{
			int health = GetPhyHealth(value);
			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, (double)health);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ymPnt, health);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ymPnt++;
			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, (double)value);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%f]\n", ymPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			break;
		}
		default:
			break;
		}
	}

	return TRUE;
}

BOOL CModBusEatonACB::ProcessRecvRealTime_4959(BYTE *buf, int byteCount, int ymPnt)
{
	if (buf == NULL || byteCount <= 0)
	{
		return FALSE;
	}
	char szBuf[128] = {0};
	int index = 0;
	int years = 0;
	int months = 0;
	int days = 0;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;

	int timePnt = 0;
	for (int i = 0; i < byteCount; i += 4, index += 4)
	{
		float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		if (ymPnt < 37)
		{
			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, value);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%f]\n", ymPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		}
		else if ((ymPnt == 37) || (ymPnt == 44))
		{
			years = value;
		}
		else if (ymPnt == 38 || (ymPnt == 45))
		{
			months = value;
		}
		else if (ymPnt == 39 || (ymPnt == 46))
		{
			days = value;
		}
		else if (ymPnt == 40 || (ymPnt == 47))
		{
			hours = value;
		}
		else if (ymPnt == 41 || (ymPnt == 48))
		{
			minutes = value;
		}
		else if ((ymPnt == 42) || (ymPnt == 49))
		{
			time_t sec = 0;
			seconds = value;
			if (ymPnt == 42)
			{
				timePnt = 37;
			}
			else if (ymPnt == 49)
			{
				timePnt = 38;
			}
			sec = TimeToSeconds(years, months, days, hours, minutes, seconds);
			m_pMethod->SetYmData(m_SerialNo, timePnt - 1, (double)sec);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", timePnt, sec);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		}

		ymPnt++;
	}

	return TRUE;
}

BOOL CModBusEatonACB::ProcessRecvPXR20RealTime(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	char szBuf[128] = {0};
	int index = 2;				  // 地址+功能码
	int byteCount = buf[index++]; // 字节数
	int dataType = pxr20RegInfo[m_realTimeCurFrame].type;
	bool isProcess = false; // 默认未处理
	switch (m_realTimeCurFrame)
	{
	case 0:
	{
		ProcessRecvRealTime_4609(buf, len);
		isProcess = true;
		break;
	}
	case 1:
	{
		ProcessRecvRealTime_1001(buf, len);
		isProcess = true;
		break;
	}
	case 4:
	{
		//{4661 - 1, 4, 3, 1}, // 第三个第四个寄存器是遥测 product
		int ycPnt = pxr20RegInfo[m_realTimeCurFrame].point;
		float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		m_pMethod->SetYcData(m_SerialNo, ycPnt - 1, value);
		index += 4;
		// printf("ycPnt = [%d], value = [%f]\n", ycPnt, value);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "ycPnt = [%d], value = [%f]\n", ycPnt, value);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		isProcess = true;
		break;
	}
	case 9:
	{
		int ymPnt = pxr20RegInfo[m_realTimeCurFrame].point;
		ProcessRecvRealTime_4959(buf + index, byteCount, ymPnt);
		/*
		for (int i = 0; i < byteCount; i += 4, index += 4)
		{
			// double value = MAKELONG(MAKEWORD(buf[index + 1], buf[index]), MAKEWORD(buf[index + 3], buf[index + 2]));
			float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, value);
			// printf("ymPnt = [%d], value = [%f]\n", ymPnt, value);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%f]\n", ymPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ymPnt++;
		}*/
		isProcess = true;
		break;
	}
	case 10:
	{
		ProcessRecvRealTime_5009(buf, len);
		isProcess = true;
		break;
	}
	default:
		break;
	}
	// 采集点类型 1:遥测  2:遥信  3:遥脉
	if (!isProcess)
	{
		switch (dataType)
		{
		case 1:
		{
			int ycPnt = pxr20RegInfo[m_realTimeCurFrame].point;
			for (int i = 0; i < byteCount; i += 4, index += 4)
			{
				float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
				m_pMethod->SetYcData(m_SerialNo, ycPnt - 1, value);
				// printf("ycPnt = [%d], value = [%f]\n", ycPnt, value);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "ycPnt = [%d], value = [%f]\n", ycPnt, value);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				ycPnt++;
			}
			break;
		}
		case 2:
		{
			break;
		}
		case 3:
		{
			int ymPnt = pxr20RegInfo[m_realTimeCurFrame].point;
			for (int i = 0; i < byteCount; i += 4, index += 4)
			{
				// double value = MAKELONG(MAKEWORD(buf[index + 1], buf[index]), MAKEWORD(buf[index + 3], buf[index + 2]));
				int value = HexToInt32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
				m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, (double)value);
				// printf("ymPnt = [%d], value = [%d]\n", ymPnt, value);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ymPnt, value);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				ymPnt++;
			}
			break;
		}
		default:
			break;
		}
	}
	m_realTimeCurFrame++;
	return TRUE;
}
BOOL CModBusEatonACB::ProcessRecvPXR25RealTime(BYTE *buf, int len)
{

	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	char szBuf[100] = {0};
	int index = 2;				  // 地址+功能码
	int byteCount = buf[index++]; // 字节数
	int dataType = pxr25RegInfo[m_realTimeCurFrame].type;
	bool isProcess = false; // 默认未处理
	switch (m_realTimeCurFrame)
	{
	case 0:
	{
		ProcessRecvRealTime_4609(buf, len);
		isProcess = true;
		break;
	}
	case 3:
	{
		ProcessRecvRealTime_1001(buf, len);
		isProcess = true;
		break;
	}
	case 9:
	{
		//{4719 - 1, 4, 3, 1}, // 第三个第四个寄存器是遥测 对应遥测点  19
		int ymPnt = pxr25RegInfo[m_realTimeCurFrame].point;
		float value = HexToInt32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		// m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, value);
		index += 4;
		int ycPnt = 19;
		value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		m_pMethod->SetYcData(m_SerialNo, ycPnt - 1, value);
		index += 4;
		// printf("ycPnt = [%d], value = [%f]\n", ycPnt, value);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "ycPnt = [%d], value = [%f]\n", ycPnt, value);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		isProcess = true;
		break;
	}
	case 16:
	{
		int ymPnt = pxr25RegInfo[m_realTimeCurFrame].point;
		ProcessRecvRealTime_4959(buf + index, byteCount, ymPnt);
		/*
		for (int i = 0; i < byteCount; i += 4, index += 4)
		{
			// double value = MAKELONG(MAKEWORD(buf[index + 1], buf[index]), MAKEWORD(buf[index + 3], buf[index + 2]));
			float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
			m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, value);
			// printf("ymPnt = [%d], value = [%f]\n", ymPnt, value);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ymPnt = [%d], value = [%f]\n", ymPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ymPnt++;
		}*/
		isProcess = true;
		break;
	}
	case 17:
	{
		ProcessRecvRealTime_5009(buf, len);
		isProcess = true;
		break;
	}
	default:
		break;
	}
	// 采集点类型 1:遥测  2:遥信  3:遥脉
	if (!isProcess)
	{
		switch (dataType)
		{
		case 1:
		{
			int ycPnt = pxr25RegInfo[m_realTimeCurFrame].point;
			for (int i = 0; i < byteCount; i += 4, index += 4)
			{
				float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
				m_pMethod->SetYcData(m_SerialNo, ycPnt - 1, value);
				// printf("ycPnt = [%d], value = [%f]\n", ycPnt, value);

				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "ycPnt = [%d], value = [%f]\n", ycPnt, value);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				ycPnt++;
			}
			break;
		}
		case 2:
		{
			break;
		}
		case 3:
		{
			int ymPnt = pxr25RegInfo[m_realTimeCurFrame].point;
			int totVarh = 0; // 总无功
			for (int i = 0; i < byteCount; i += 4, index += 4)
			{
				// double value = MAKELONG(MAKEWORD(buf[index + 1], buf[index]), MAKEWORD(buf[index + 3], buf[index + 2]));
				int value = HexToInt32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
				m_pMethod->SetYmData(m_SerialNo, ymPnt - 1, (double)value);
				// printf("ymPnt = [%d], value = [%d]\n", ymPnt, value);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", ymPnt, value);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				if (ymPnt == 6 || ymPnt == 7)
				{
					totVarh += value;
					if (ymPnt == 7)
					{
						m_pMethod->SetYmData(m_SerialNo, 9 - 1, (double)totVarh);
						memset(szBuf, 0, sizeof(szBuf));
						sprintf(szBuf, "ymPnt = [%d], value = [%d]\n", 9, totVarh);
						OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					}
				}
				ymPnt++;
			}
			break;
		}
		default:
			break;
		}
	}

	m_realTimeCurFrame++;
	return TRUE;
}
BOOL CModBusEatonACB::ProcessRecvGetAlarmEventType(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	int index = 3;
	m_alarmEventType = MAKEWORD(buf[index + 1], buf[index]);
	return TRUE;
}

BOOL CModBusEatonACB::ProcessRecvDevType(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	int index = 0;
	index += 3; // 地址+功能码+长度

	int type = MAKEWORD(buf[index + 1], buf[index]);
	// int type = 5;
	char szBuf[100] = {0};

	if (6 <= type && type <= 9)
	{
		m_devType = EATONACB_PX25;
		m_waveMaxType = 8;
		m_waveCurType = 0;
		m_waveConfig.channelCount = 8;
		m_waveConfig.analogChannelCount = 8;
		m_waveConfig.digitalChannelCount = 0;
		m_waveConfig.devType = EATONACB_PX25;
	}
	else
	{
		m_devType = EATONACB_PX20;
		m_waveMaxType = 5;
		m_waveCurType = 0;
		m_waveConfig.channelCount = 5;
		m_waveConfig.analogChannelCount = 5;
		m_waveConfig.digitalChannelCount = 0;
		m_waveConfig.devType = EATONACB_PX20;
	}
	m_pMethod->SetYmData(m_SerialNo, 0, (double)m_devType + 1);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "dev type = [%d]\n ymPnt = [%d], value = [%d]\n", type, 1, m_devType + 1);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	/*
	int isWrite = 0;
	if (m_devType == EATONACB_PX20)
	{
		if (m_wModuleType != PXR20_MODBUS_TCP)
		{
			m_wModuleType = PXR20_MODBUS_TCP;
			isWrite = 1;
		}
	}
	else if (m_devType == EATONACB_PX25)
	{
		if (m_wModuleType != PXR25_MODBUS_TCP)
		{
			m_wModuleType = PXR25_MODBUS_TCP;
			isWrite = 1;
		}
	}
	if (isWrite)
	{
		std::string fileName = MASTER_BUSLINE_PATH;
		WriteConfig(fileName, "[DEV001]", "module", std::to_string(m_wModuleType));
	}*/
	m_realTimeCurFrame = 0;
	m_realTimeMaxFrameCount = 0;
	return TRUE;
}
BOOL CModBusEatonACB::ProcessRecvLastWaveData(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}

	int index = 0;
	index += 2;

	int byteCount = buf[index++];

	for (int i = 0; i < byteCount; i += 4)
	{
		// 每四个字节表示一个float数据，按照寄存器数量写到对应数组位置，每两个寄存器代表一个数据
		int dataIdx = m_waveRegNum / 2;
		m_waveConfig.waveData[dataIdx].num = m_waveRegNum;
		m_waveConfig.waveData[dataIdx].time = m_waveTimeInterval * (dataIdx);
		m_waveConfig.waveData[dataIdx].value[m_waveCurType] = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
		index += 4;
		m_waveRegNum += 2;
	}

	return TRUE;
}
BOOL CModBusEatonACB::ProcessRecvLastWaveTime(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	int index = 0;
	index += 3; // 地址 功能码 长度
	m_waveTime.wMonth = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wDay = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wYear = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wDayOfWeek = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wHour = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wMinute = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wSecond = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	m_waveTime.wMilliSec = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	index += 2;

	float sec = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
	m_waveTimeInterval = sec * 1000 * 1000;
	char szBuf[128] = {0};
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "m_waveTimeInterval : [%d] us\n", m_waveTimeInterval);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	std::string devName = m_sDevName;
	std::string fileName = WAVE_DATA_PATH +
						   devName + "_" +
						   std::to_string(m_wDevAddr) + "_" +
						   std::to_string(m_waveConfig.waveID) + "_" +
						   std::to_string(m_waveTime.wYear) + "-" +
						   std::to_string(m_waveTime.wMonth) + "-" +
						   std::to_string(m_waveTime.wDay) + "-" +
						   std::to_string(m_waveTime.wHour) + "-" +
						   std::to_string(m_waveTime.wMinute) + "-" +
						   std::to_string(m_waveTime.wSecond) + "." +
						   std::to_string(m_waveTime.wMilliSec) + ".cfg";
	WriteComtradeCfgFile(fileName);

	return TRUE;
}
BOOL CModBusEatonACB::ProcessRecvGetIGType(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}
	char szBuf[128] = {0};
	int index = 0;
	index += 3; // 地址+功能码+长度
	int type = MAKEWORD(buf[index + 1], buf[index]);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ig type = [%d]\n", type);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	memset(m_waveType, 0, sizeof(m_waveType));
	if (type == 0)
	{
		if (m_devType == EATONACB_PX20)
		{
			int tmp[5] = {0x000A, 0x000B, 0x000C, 0x000D, 0x000F};
			memcpy(m_waveType, tmp, sizeof(tmp));
		}
		else if (m_devType == EATONACB_PX25)
		{
			int tmp[8] = {0x000A, 0x000B, 0x000C, 0x000D, 0x000F, 0x0010, 0x0011, 0x0012};
			memcpy(m_waveType, tmp, sizeof(tmp));
		}
	}
	else if (type == 1)
	{
		if (m_devType == EATONACB_PX20)
		{
			int tmp[5] = {0x000A, 0x000B, 0x000C, 0x000D, 0x000E};
			memcpy(m_waveType, tmp, sizeof(tmp));
		}
		else if (m_devType == EATONACB_PX25)
		{
			int tmp[8] = {0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x0010, 0x0011, 0x0012};
			memcpy(m_waveType, tmp, sizeof(tmp));
		}
	}
	return TRUE;
}

BOOL CModBusEatonACB::ProcessEventData(BYTE *buf, int len)
{
	if (buf == NULL || len <= 0)
	{
		return FALSE;
	}

	VARSLIST varList = {0};
	char szBuf[128] = {0};
	int index = 0;
	index += 2;					  // 地址 + 功能码
	int byteCount = buf[index++]; // 字节数
	TIMEDATA time = {0};
	time_t timeSecs = GetTimeData(buf + index, 16, &time);
	time.Year -= 1900;
	index += 16; // 时间
	int isAlarmMinor = 0;
	int causeYxPnt = 0;
	if (m_toraEventType == EATONACB_TRIP_EVENT)
	{
		index += 6; // 三个寄存器
		causeYxPnt = TRIP_BAOHU_SOFT_YX;
	}
	else if (m_toraEventType == EATONACB_ALARM_EVENT)
	{
		causeYxPnt = ALARM_BAOHU_SOFT_YX;
		if (m_alarmEventType == ALARM_MINOR)
		{
			index += 4; // 两个寄存器
			isAlarmMinor = 1;
		}
		else
		{
			index += 6; // 三个寄存器
		}
	}

	int ycPnt = EVENT_YC_POST_START;
	int value = HexToInt32((uint8_t *)m_latestEventID, MODBUS_BYTEORDER_TYPE_CDAB);
	// m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, value, &time);
	int num = 0;
	varList.vars[num].iType = VARIABLE_TYPE_INT;
	varList.vars[num++].value.iVal = value;

	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, value);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

	BYTE primary = buf[index++];
	varList.vars[num].iType = VARIABLE_TYPE_INT;
	varList.vars[num++].value.iVal = primary;
	ycPnt++;
	m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, primary, &time);
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, primary);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	WORD pnt = 0;
	if (primary == 0x01)
	{
		pnt = 1;
	}
	else if (primary == 0x02)
	{
		pnt = 1;
		primary = 0;
	}
	else if (primary == 0x03)
	{
		pnt = SOFT_YX_POST_START;
		primary = 1;
		m_pMethod->SetYxDataWithTime(m_SerialNo, causeYxPnt - 1, primary, &time);
		m_pMethod->SetYxData(m_SerialNo, causeYxPnt - 1, primary);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "yxPnt = [%d], value = [%d] \n", causeYxPnt, primary);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	}
	else if (primary == 0x04)
	{
		pnt = SOFT_YX_POST_START + 1;
		primary = 1;
	}
	else if (primary == 0x0D)
	{
		pnt = SOFT_YX_POST_START + 2;
		primary = 1;
	}
	if (pnt != 0)
	{
		m_pMethod->SetYxDataWithTime(m_SerialNo, pnt - 1, primary, &time);
		m_pMethod->SetYxData(m_SerialNo, pnt - 1, primary);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "yxPnt = [%d], value = [%d] \n", pnt, primary);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	}

	BYTE secondry = buf[index++];
	varList.vars[num].iType = VARIABLE_TYPE_INT;
	varList.vars[num++].value.iVal = secondry;
	ycPnt++;
	m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, secondry, &time);

	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, secondry);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	if (!isAlarmMinor)
	{
		int cause = MAKEWORD(buf[index + 1], buf[index]);
		varList.vars[num].iType = VARIABLE_TYPE_INT;
		varList.vars[num++].value.iVal = cause;

		ycPnt++;
		m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, cause, &time);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "ycPnt = [%d], value = [%2x]\n", ycPnt, cause);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

		if (m_toraEventType == EATONACB_TRIP_EVENT)
		{
			int yxPnt = 0;
			value = GetYxPntValue(&yxPnt, cause);
			if (yxPnt != 0)
			{
				m_pMethod->SetYxDataWithTime(m_SerialNo, yxPnt - 1, value, &time);
				m_pMethod->SetYxData(m_SerialNo, yxPnt - 1, value);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "yxPnt = [%d], value = [%d] \n", yxPnt, value);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

				yxPnt += 13;
				m_pMethod->SetYxDataWithTime(m_SerialNo, yxPnt - 1, value, &time);
				m_pMethod->SetYxData(m_SerialNo, yxPnt - 1, value);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "yxPnt = [%d], value = [%d] \n", yxPnt, value);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			}
		}

		index += 2;

		ycPnt = EVENT_YC_POST_START + 4;
		int ycByte = 6 * 2 * 2; // 6个点，每个点两个寄存器，每个寄存器两个字节  63
		for (int i = 0; i < ycByte; i += 4, index += 4)
		{
			int value = HexToInt32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
			varList.vars[num].iType = VARIABLE_TYPE_UINT;
			varList.vars[num++].value.uiVal = value;
			m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, value, &time);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ycPnt++;
		}
		ycByte = 6 * 2;
		for (int i = 0; i < ycByte; i += 2, index += 2)
		{
			int value = MAKEWORD(buf[index + 1], buf[index]);
			varList.vars[num].iType = VARIABLE_TYPE_INT;
			varList.vars[num++].value.iVal = value;
			m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, value, &time);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ycPnt++;
		}
		ycByte = 6 * 2 * 2; // 6个点，每个点两个寄存器，每个寄存器两个字节
		for (int i = 0; i < ycByte; i += 4, index += 4)
		{
			// float value = HexToFloat32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
			int value = HexToInt32(buf + index, MODBUS_BYTEORDER_TYPE_CDAB);
			varList.vars[num].iType = VARIABLE_TYPE_INT;
			varList.vars[num++].value.iVal = value;
			m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, value, &time);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ycPnt++;
		}
		ycByte = 4 * 2;
		for (int i = 0; i < ycByte; i += 2, index += 2)
		{
			int value = MAKEWORD(buf[index + 1], buf[index]);
			if (ycPnt == 81 || ycPnt == 82)
			{
				varList.vars[num].iType = VARIABLE_TYPE_FLOAT;
				varList.vars[num++].value.fVal = value * 0.1;
				printf("ycPnt = [%d], value = [%f]\n", ycPnt, value * 0.1);
			}
			else if (ycPnt == 83)
			{
				varList.vars[num].iType = VARIABLE_TYPE_FLOAT;
				varList.vars[num++].value.fVal = value * 0.01;
				printf("ycPnt = [%d], value = [%f]\n", ycPnt, value * 0.01);
			}
			else
			{
				varList.vars[num].iType = VARIABLE_TYPE_INT;
				varList.vars[num++].value.iVal = value;
			}

			m_pMethod->SetYcDataWithTime(m_SerialNo, ycPnt - 1, value, &time);
			memset(szBuf, 0, sizeof(szBuf));
			sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, value);
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			ycPnt++;
		}
	}

	varList.vars[num].iType = VARIABLE_TYPE_UINT;
	varList.vars[num++].value.uiVal = timeSecs;
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "ycPnt = [%d], value = [%d]\n", ycPnt, timeSecs);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	varList.num = num;
	if (!isAlarmMinor)
	{
		m_pMethod->SetVarsListData(m_SerialNo, varList);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "set varList data num : [%d]\n", varList.num);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	}

	/*
	for (int i = 0; i < varList.num; i++)
	{
		printf("varList[%d] type = [%d], value = [%d]\n", i, varList.vars[i].iType, varList.vars[i].value.iVal);
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "varList[%d] type = [%d], value = [%d]\n", i, varList.vars[i].iType, varList.vars[i].value.iVal);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	}*/
	return TRUE;
}

/*
PTOC  延时过流保护   0x3d 0x3e  0x50 0x9e 0x95
PIOC   瞬时过流保护  0x03 0x3f 0x42 0x4c
PTOV   过压保护  0x0b
PTUV   欠压保护 0x0c 0x0e
PDOP   逆功率保护  0x41
PTRC    保护脱扣调节 0x4b  0x9d  0x7ff
PTUF    欠频保护 0x0f
PTOF     过频保护  0x10 0x92
过温保护   0x4e
*/

BOOL CModBusEatonACB::ProcessRecvSetCommon(BYTE *buf, int len, int reg, int regNum)
{
	int index = 0;
	index += 2;

	int startRegister = MAKEWORD(buf[index + 1], buf[index]);
	if (startRegister != reg)
	{
		return FALSE;
	}
	index += 2;
	int registerNum = MAKEWORD(buf[index + 1], buf[index]);
	if (registerNum != regNum)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL CModBusEatonACB::ProcessRecvGetLastestEventID(BYTE *buf, int len)
{
	char szBuf[128] = {0};
	char idTmp[4] = {0};
	int index = 0;
	index += 2;
	int byteCount = buf[index];
	index++;

	memcpy(idTmp, buf + index, 4);
	index += 4;
	int earliestID = HexToInt32((uint8_t *)idTmp, MODBUS_BYTEORDER_TYPE_CDAB);

	memcpy(idTmp, buf + index, 4);
	index += 4;
	int lastID = HexToInt32((uint8_t *)idTmp, MODBUS_BYTEORDER_TYPE_CDAB);

	memcpy(idTmp, buf + index, 4);
	index += 4;
	int requestedID = HexToInt32((uint8_t *)idTmp, MODBUS_BYTEORDER_TYPE_CDAB);

	memcpy(idTmp, buf + index, 4);
	index += 4;
	int previousID = HexToInt32((uint8_t *)idTmp, MODBUS_BYTEORDER_TYPE_CDAB);

	memcpy(idTmp, buf + index, 4);
	index += 4;
	int nextID = HexToInt32((uint8_t *)idTmp, MODBUS_BYTEORDER_TYPE_CDAB);

	// printf("earliestID: [0x%2x],lastID: [0x%2x], requestedID: [0x%2x], previousID:[0x%2x], nextID: [0x%2x]\n", earliestID, lastID, requestedID, previousID, nextID);

	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "earID: [0x%2x],lastID: [0x%2x], reqID: [0x%2x], preID:[0x%2x], nextID: [0x%2x]\n", earliestID, lastID, requestedID, previousID, nextID);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	m_maxEventId = lastID;

	std::string value;
	if (m_childEventType == TRIP_EVENT_STATUS_GET_LASTEST_EVENTID)
	{
		if (m_toraEventType == EATONACB_TRIP_EVENT)
		{
			m_readEventProcess = 2;
			m_curEventId = GetFileEventID(m_eventIDFilename, "[last_event_id]", "trip_event_id");
			/*
			if (m_curEventId < 0)
			{
				// 断电上电第一次读取
				// printf("first read trip event id: [%d]\n", m_curEventId);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "first read trip event id: [%d]\n", m_curEventId);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				m_curEventId = lastID;
				m_readEventProcess = 0;
			}
			if (m_readEventProcess != 0)
			{
				if (m_curEventId == lastID)
				{
					// printf("[%s] %s: %s\n", __FUNCTION__, "Trip Event ID", "No change");
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "%s: %s\n", "Trip Event ID", "No change");
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					return FALSE;
				}
				else
				{
					if (m_readEventProcess == 2)
					{
						m_curEventId = nextID;
					}
				}
			}*/

			if (m_curEventId == lastID)
			{
				// printf("[%s] %s: %s\n", __FUNCTION__, "Trip Event ID", "No change");
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "%s: %s\n", "Trip Event ID", "No change");
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				return FALSE;
			}
			else
			{
				m_curEventId = lastID;
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "%s\n", "Trip Event Occur!");
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
			}
			std::vector<uint8_t> tmp = ByBitOps(m_curEventId, MODBUS_BYTEORDER_TYPE_CDAB);
			uint8_t *pTmp = tmp.data();
			memcpy(m_latestEventID, pTmp, 4);
			std::string newHexStr = HexArrayToString(tmp);
			WriteConfig(m_eventIDFilename, "[last_event_id]", "trip_event_id", newHexStr);
		}
		else if (m_toraEventType == EATONACB_ALARM_EVENT)
		{
			m_curEventId = GetFileEventID(m_eventIDFilename, "[last_event_id]", "alarm_event_id");
			if (m_curEventId < 0)
			{
				// 第一次读取
				// printf("first read alarm event id: [%d]\n", m_curEventId);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "first read alarm event id: [%d]\n", m_curEventId);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				m_curEventId = lastID;
				m_readEventProcess = 0;
			}
			if (m_readEventProcess != 0)
			{
				if (m_curEventId == lastID)
				{
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "%s: %s\n", "alarm Event ID", "No change");
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					// printf("[%s] %s: %s\n", __FUNCTION__, "alarm Event ID", "No change");
					return FALSE;
				}
				else
				{
					if (m_readEventProcess == 2)
					{
						m_curEventId = nextID;
						memset(szBuf, 0, sizeof(szBuf));
						sprintf(szBuf, "%s\n", "Alarm Event Occur!");
						OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					}
				}
			}
			std::vector<uint8_t> tmp = ByBitOps(m_curEventId, MODBUS_BYTEORDER_TYPE_CDAB);
			uint8_t *pTmp = tmp.data();
			memcpy(m_latestEventID, pTmp, 4);
			std::string newHexStr = HexArrayToString(tmp);
			WriteConfig(m_eventIDFilename, "[last_event_id]", "alarm_event_id", newHexStr);
		}
	}
	else if (m_childEventType == TRIP_WAVE_STATUS_GET_LASTEST_EVENTID)
	{
		if (m_waveCurType == 0)
		{ /*
			m_curEventId = GetFileEventID(m_eventIDFilename, "[last_event_id]", "wave_event_id");
			if (m_curEventId < 0)
			{
				// 断电上电第一次读取
				// printf("first read trip event id: [%d]\n", m_curEventId);
				memset(szBuf, 0, sizeof(szBuf));
				sprintf(szBuf, "first read wave event id: [%d]\n", m_curEventId);
				OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
				m_curEventId = lastID;
				m_readEventProcess = 0;
			}
			if (m_readEventProcess != 0)
			{
				if (m_curEventId == lastID)
				{
					// printf("[%s] %s: %s\n", __FUNCTION__, "Trip Event ID", "No change");
					memset(szBuf, 0, sizeof(szBuf));
					sprintf(szBuf, "%s: %s\n", "Wave Event ID", "No change");
					OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
					return FALSE;
				}
				else
				{
					if (m_readEventProcess == 2)
					{
						m_curEventId = nextID;
					}
				}
			}*/
			std::vector<uint8_t> tmp = ByBitOps(m_curEventId, MODBUS_BYTEORDER_TYPE_CDAB);
			uint8_t *pTmp = tmp.data();
			memcpy(m_latestEventID, pTmp, 4);
			std::string newHexStr = HexArrayToString(tmp);
			WriteConfig(m_eventIDFilename, "[last_event_id]", "wave_event_id", newHexStr);
		}
	}
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "m_curEventId : [%2x]\n", m_curEventId);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	m_waveConfig.waveID = m_curEventId;
	return TRUE;
}

int CModBusEatonACB::GetPhyHealth(float lifePoint)
{
	int value = 0;
	if (lifePoint >= 0 && lifePoint < 7500)
	{
		value = 1;
	}
	else if (lifePoint >= 7500 && lifePoint < 10000)
	{
		value = 2;
	}
	else
	{
		value = 3;
	}

	return value;
}

void CModBusEatonACB::WriteInt16InLittleEndian(std::ofstream &file, int16_t value)
{
	// 小端序：低位在前
	int8_t bytes[2];
	bytes[0] = value & 0xFF; // 最低位字节
	bytes[1] = (value >> 8) & 0xFF;
	file.write(reinterpret_cast<const char *>(bytes), sizeof(bytes));
	// printf("%d: %x %x\n", value, bytes[0], bytes[1]);
}
void CModBusEatonACB::WriteInt32InLittleEndian(std::ofstream &file, int32_t value)
{
	// 小端序：低位在前
	int8_t bytes[4];
	bytes[0] = value & 0xFF; // 最低位字节
	bytes[1] = (value >> 8) & 0xFF;
	bytes[2] = (value >> 16) & 0xFF;
	bytes[3] = (value >> 24) & 0xFF; // 最高位字节

	file.write(reinterpret_cast<const char *>(bytes), 4);
	// printf("%d: %x %x %x %x\n", value, bytes[0], bytes[1], bytes[2], bytes[3]);
}
void CModBusEatonACB::WriteInt64InLittleEndian(std::ofstream &file, int64_t value)
{
	// 小端序：低位在前
	int8_t bytes[8];
	bytes[0] = value & 0xFF; // 最低位字节
	bytes[1] = (value >> 8) & 0xFF;
	bytes[2] = (value >> 16) & 0xFF;
	bytes[3] = (value >> 24) & 0xFF; // 最高位字节
	bytes[4] = (value >> 32) & 0xFF; // 最高位字节
	bytes[5] = (value >> 40) & 0xFF; // 最高位字节
	bytes[6] = (value >> 48) & 0xFF; // 最高位字节
	bytes[7] = (value >> 56) & 0xFF; // 最高位字节
	file.write(reinterpret_cast<const char *>(bytes), sizeof(value));
	// printf("%ld: %x %x %x %x\n", value, bytes[0], bytes[1], bytes[2], bytes[3]);
}
void CModBusEatonACB::WriteFloat32InLittleEndian(std::ofstream &file, float value)
{
	int32_t int_val;
	memcpy(&int_val, &value, sizeof(float));
	// 小端序：低位在前
	int8_t bytes[4];
	bytes[0] = int_val & 0xFF; // 最低位字节
	bytes[1] = (int_val >> 8) & 0xFF;
	bytes[2] = (int_val >> 16) & 0xFF;
	bytes[3] = (int_val >> 24) & 0xFF; // 最高位字节
	file.write(reinterpret_cast<const char *>(bytes), 4);
	// printf("%f: %x %x %x %x\n", value, bytes[0], bytes[1], bytes[2], bytes[3]);
}

BOOL CModBusEatonACB::WriteComtradeDataFloatFile(const std::string &filename)
{
	std::ofstream dataFile(filename);
	// std::ofstream dataFile(filename, std::ios::binary);
	// std::string filenameAscii = filename + ".txt";
	// std::ofstream asciiFile(filenameAscii); // ASCII文本模式
	if (!dataFile)
	{
		std::cerr << "无法创建二进制文件: " << filename << ".dat" << std::endl;
		return FALSE;
	}

	EatonACBComtrade_t *comtradeData = m_waveConfig.waveData;
	size_t waveDataCount = sizeof(m_waveConfig.waveData) /
						   sizeof(EatonACBComtrade_t);
	for (int i = 0; i < waveDataCount; i++)
	{
		int32_t sampleNum = i + 1;
		// dataFile.write(reinterpret_cast<const char *>(&sampleNum), sizeof(int32_t));
		int32_t timeStamp = comtradeData[i].time;
		// dataFile.write(reinterpret_cast<const char *>(&timeStamp), sizeof(timeStamp));
		// asciiFile << sampleNum << "," << timeStamp;
		// 数据文件类型 0:ascii 1:binary 2:Binary32 3:float32
		switch (m_waveConfig.dataType)
		{
		case 0:
		{
			dataFile << sampleNum << "," << timeStamp;
			break;
		}
		case 1:
		case 2:
		case 3:
		{
			WriteInt32InLittleEndian(dataFile, sampleNum);
			WriteInt32InLittleEndian(dataFile, timeStamp);
			break;
		}
		default:
			break;
		}
		// 3. 8个通道的浮点数值
		for (int j = 0; j < m_waveMaxType; j++)
		{
			// dataFile.write(reinterpret_cast<const char *>(&comtradeData[i].value[j]), sizeof(float));
			// asciiFile << "," << comtradeData[i].value[j];

			switch (m_waveConfig.dataType)
			{
			case 0:
			{
				dataFile << "," << comtradeData[i].value[j];
				break;
			}
			case 1:
			{
				WriteInt16InLittleEndian(dataFile, (int16_t)((comtradeData[i].value[j] - m_waveConfig.channelInfo[j].offsetB) / m_waveConfig.channelInfo[j].fCoefA));
				break;
			}
			case 2:
			{
				// WriteInt32InLittleEndian(dataFile, (int32_t)((comtradeData[i].value[j] - m_waveConfig.channelInfo[j].offsetB) / m_waveConfig.channelInfo[j].fCoefA));
				WriteFloat32InLittleEndian(dataFile, ((comtradeData[i].value[j] - m_waveConfig.channelInfo[j].offsetB) / m_waveConfig.channelInfo[j].fCoefA));
				break;
			}
			case 3:
			{
				WriteFloat32InLittleEndian(dataFile, ((comtradeData[i].value[j] - m_waveConfig.channelInfo[j].offsetB) / m_waveConfig.channelInfo[j].fCoefA));
				break;
			}
			default:
				break;
			}
		}
		if (m_waveConfig.dataType == 0)
		{
			dataFile << "\n";
		}
		// asciiFile << "\n";

		if (!dataFile.good())
		{ // 关键检查点
			std::cerr << "写入失败 at sample " << i << std::endl;
			return FALSE;
		}
	}
	// asciiFile.close();
	dataFile.flush(); // 确保数据写入磁盘
	if (!dataFile.good())
	{
		std::cerr << "最终刷新失败" << std::endl;
		return FALSE;
	}
	dataFile.close();
	return TRUE;
}
BOOL CModBusEatonACB::WriteComtradeCfgFile(const std::string &filename)
{
	std::string comType = ReadConfig(m_comtradeCfgFilename, "[comtrade_cfg]", "data_type");
	// BINARY、FLOAT32、ASCII
	float fA;
	float oB;

	// 数据文件类型 0:ascii 1:binary 2:Binary32 3:float32
	if (comType == "BINARY")
	{
		fA = 10;
		oB = 0;
		m_waveConfig.dataType = 1;
	}
	else if (comType == "ASCII")
	{
		fA = 1;
		oB = 0;
		m_waveConfig.dataType = 0;
	}
	else if (comType == "BINARY32")
	{
		fA = 1;
		oB = 0;
		m_waveConfig.dataType = 2;
	}
	else
	{
		fA = 1;
		oB = 0;
		m_waveConfig.dataType = 3;
	}

	std::ofstream cfg(filename);
	if (!cfg.is_open())
		return false;

	// 第1行：站名，设备ID，标准版本
	cfg << m_sDevName << "," << m_waveConfig.waveID << "," << m_waveConfig.fileVer << "\n";

	// 第2行：总通道数，模拟通道数，数字通道数
	cfg << m_waveConfig.channelCount << ","
		<< m_waveConfig.analogChannelCount << "A" << ","
		<< m_waveConfig.digitalChannelCount << "D" << "\n";

	int analog_ch_count = 0;
	if (m_devType == EATONACB_PX20)
	{
		analog_ch_count = 5;
	}
	else if (m_devType == EATONACB_PX25)
	{
		analog_ch_count = 8;
	}
	// 模拟通道定义（每行9个参数）
	for (int i = 0; i < analog_ch_count; i++)
	{
		m_waveConfig.channelInfo[i].fCoefA = fA;
		m_waveConfig.channelInfo[i].offsetB = oB;
		cfg << (i + 1) << "," // 通道名
			<< m_waveConfig.channelInfo[i].channelName << ","
			<< " , ,"										  // 通道相别标识ph ,被监视的器件ccbm ,可选
			<< m_waveConfig.channelInfo[i].channelUnit << "," // 单位（根据实际调整）
			<< m_waveConfig.channelInfo[i].fCoefA << ","
			<< m_waveConfig.channelInfo[i].offsetB << ","										  // A,B,（BINARY时A=10,B=0）
			<< "0.0,"																			  // 时间偏移
			<< m_waveConfig.channelInfo[i].fMin << "," << m_waveConfig.channelInfo[i].fMax << "," // 最小/最大工程值
			<< "1.0,1.0\n";																		  // 一次/二次变比
	}
	// 额定频率
	cfg << m_waveConfig.powerHZ << "\n";

	// 采样率配置
	cfg << m_waveConfig.sampleNum << "\n"; // 采样率段数
	cfg << m_waveConfig.sampleFreq << "," << m_waveConfig.samplePoint << "\n";

	// 时间戳
	cfg << m_waveTime.wMonth << "/" << m_waveTime.wDay << "/" << m_waveTime.wYear << "," << m_waveTime.wHour << ":" << m_waveTime.wMinute << ":" << m_waveTime.wSecond << "." << m_waveTime.wMilliSec << "\n";
	// 数据格式：BINARY32（关键！）
	cfg << comType + "\n";

	// 时间戳乘数
	cfg << "1.0\n";

	cfg.close();
	return true;
}

// 获取TIMEDATA 数据
time_t CModBusEatonACB::GetTimeData(BYTE *buf, int len, TIMEDATA *time)
{
	if (buf == NULL || len <= 0)
	{
		return -1;
	}
	int index = 0;
	time->Month = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	time->Day = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	time->Year = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	// m_waveTime.wDayOfWeek = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	time->Hour = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	time->Minute = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	time->Second = MAKEWORD(buf[index + 1], buf[index]);
	index += 2;
	time->MiSec = MAKEWORD(buf[index + 1], buf[index]);

	char szBuf[256] = {0};
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "event time: %d-%d-%d %d:%d:%d.%d\n", time->Year, time->Month, time->Day, time->Hour, time->Minute, time->Second, time->MiSec);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

	return TimeToSeconds(time->Year, time->Month, time->Day, time->Hour, time->Minute, time->Second);
}

int CModBusEatonACB::GetYxPntValue(int *yxPnt, int cause)
{
	int value = 0;
	*yxPnt = 0;
	int softYxPnt = 0;
	if (m_toraEventType == EATONACB_TRIP_EVENT)
	{
		softYxPnt = TRIP_BAOHU_SOFT_YX;
	}
	else
	{
		softYxPnt = ALARM_BAOHU_SOFT_YX;
	}
	switch (cause)
	{
	case 0x3d:
	{
		*yxPnt = softYxPnt + 1;
		value = 1;
		break;
	}
	case 0x3e:
	{
		*yxPnt = softYxPnt + 2;
		value = 1;
		break;
	}
	case 0x54:
	case 0x55:
	{
		*yxPnt = softYxPnt + 3;
		value = 1;
		break;
	}
	case 0x41:
	{
		*yxPnt = softYxPnt + 6;
		value = 1;
		break;
	}
	case 0x0f:
	{
		*yxPnt = softYxPnt + 7;
		value = 1;
		break;
	}
	case 0x10:
	{
		*yxPnt = softYxPnt + 8;
		value = 1;
		break;
	}
	case 0x0c:
	{
		*yxPnt = softYxPnt + 9;
		value = 1;
		break;
	}
	case 0x0b:
	{
		*yxPnt = softYxPnt + 10;
		value = 1;
		break;
	}
	case 0x03:
	{
		*yxPnt = softYxPnt + 11;
		value = 1;
		break;
	}
	case 0x4e:
	{
		*yxPnt = softYxPnt + 12;
		value = 1;
		break;
	}
	default:
		break;
	}
	return value;
}

int CModBusEatonACB::GetFileEventID(const std::string &filename, const std::string &mainKey, const std::string &key)
{
	std::string value = ReadConfig(filename, mainKey, key);
	if (value == "-1")
	{
		return -1;
	}
	std::vector<uint8_t> oldEventID = StringToHexArray(value);
	uint8_t *ptr = oldEventID.data();
	return HexToInt32((uint8_t *)ptr, MODBUS_BYTEORDER_TYPE_CDAB);
}

static void printHexArray(const std::vector<uint8_t> &hexArray)
{
	std::cout << "{";
	for (size_t i = 0; i < hexArray.size(); ++i)
	{
		std::cout << "0x" << std::hex << std::uppercase
				  << std::setw(2) << std::setfill('0')
				  << static_cast<int>(hexArray[i]);
		if (i < hexArray.size() - 1)
		{
			std::cout << ", ";
		}
	}
	std::cout << "}" << std::dec << std::endl;
}
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusEatonACB
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文
 *       Input:
 *		Return:  BOOL
 *---------------------------------------------------------------------------------------
 */
BOOL CModBusEatonACB::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	// printf("CModBusEatonACB GetProtocolBuf !!!\n");
	char szBuf[512] = {0};
	/*
	sprintf(szBuf, "%s", "CModBusEatonACB GetProtocolBuf !!!\n");
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
	*/

	m_byRecvflag = 0;
	if (pBusMsg != NULL)
	{
		return FALSE;
	}
	if (m_initflag)
	{

		if (!CreateDir())
		{
			return FALSE;
		}

		if (!CreateFileIfNotExists())
		{
			return FALSE;
		}
		m_initflag = 0;

		/*
		std::vector<uint8_t> tripEid = ReadConfig(m_eventIDFilename, "trip_event_id");
		printHexArray(tripEid);
		std::vector<uint8_t> newValue = {0x45, 0x62, 0x25, 0x00};
		WriteConfig(m_eventIDFilename, "trip_event_id", newValue);

		std::vector<uint8_t> alarmEid = ReadConfig(m_eventIDFilename, "alarm_event_id");
		printHexArray(alarmEid);
		newValue = {0x95, 0x92, 0x35, 0x11};
		WriteConfig(m_eventIDFilename, "alarm_event_id", newValue);

		std::vector<uint8_t> waveEid = ReadConfig(m_eventIDFilename, "wave_event_id");
		printHexArray(waveEid);
		newValue = {0x15, 0x12, 0x75, 0x22};
		WriteConfig(m_eventIDFilename, "wave_event_id", newValue);
		*/
	}

	if (m_errorCount > 3)
	{
		m_errorCount = 0;
		m_curEventType = EATONACB_NULL;
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "error count too many , next task \n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		return FALSE;
	}

	if (m_curEventType == EATONACB_NULL)
	{
		// printf("");
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "no task run \n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		return FALSE;
	}
	memset(szBuf, 0, sizeof(szBuf));
	sprintf(szBuf, "m_childEventType : [%d] m_realTimeSendCount : [%d] \n", m_childEventType, m_realTimeSendCount);
	OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

	switch (m_childEventType)
	{
	case STATUS_NULL:
	{
		return TRUE;
		break;
	}
	case STATUS_GET_DEV_TYPE:
	{
		RegisterDataGet(FUNCCODE_0X03, REG_GET_DEV_TYPE_3004, 0x01);
		m_byRecvflag = STATUS_GET_DEV_TYPE;
		break;
	}
	case TRIP_EVENT_STATUS_SET:
	{
		RegisterDataGet(FUNCCODE_0X10, REG_DEFINE_8193, 0x01);
		memset(m_setValue, 0, sizeof(m_setValue));

		if (m_toraEventType == EATONACB_TRIP_EVENT)
		{
			m_setValue[0] = HIBYTE(REG_TRIP_EVENT);
			m_setValue[1] = LOBYTE(REG_TRIP_EVENT);
			m_setValueLen = 2;
		}
		else
		{
			m_setValue[0] = HIBYTE(REG_ALARM_EVENT);
			m_setValue[1] = LOBYTE(REG_ALARM_EVENT);
			m_setValueLen = 2;
		}
		m_byRecvflag = TRIP_EVENT_STATUS_SET;
		break;
	}
	case TRIP_EVENT_STATUS_GET_LASTEST_EVENTID:
	{
		RegisterDataGet(FUNCCODE_0X03, REG_LATEST_EVENT_ID_8194, 0x0a);
		m_byRecvflag = TRIP_EVENT_STATUS_GET_LASTEST_EVENTID;
		break;
	}
	case TRIP_EVENT_STATUS_SET_LASTEST_EVENTID:
	{
		if (!RegisterDataSetLatestEventID())
		{
			m_curEventType = EATONACB_NULL;
			return FALSE;
		}
		m_byRecvflag = TRIP_EVENT_STATUS_SET_LASTEST_EVENTID;
		break;
	}
	case ALARM_EVENT_STATUS_GET_EVENT_TYPE:
	{
		RegisterDataGet(FUNCCODE_0X04, REG_GET_ALARM_TYPE_8212, 0X01);
		m_byRecvflag = ALARM_EVENT_STATUS_GET_EVENT_TYPE;
		break;
	}
	case TRIP_EVENT_STATUS_GET_LASTEST_EVENTINFO:
	{
		if (m_toraEventType == EATONACB_TRIP_EVENT)
		{
			RegisterDataGet(FUNCCODE_0X04, REG_GET_EVENT_INFO_8204, 0X31);
		}
		else
		{
			if (m_alarmEventType == ALARM_MAJOR)
			{
				RegisterDataGet(FUNCCODE_0X04, REG_GET_EVENT_INFO_8204, 0X31);
			}
			else if (m_alarmEventType == ALARM_MINOR)
			{
				RegisterDataGet(FUNCCODE_0X04, REG_GET_EVENT_INFO_8204, 0X0B);
			}
		}
		m_byRecvflag = TRIP_EVENT_STATUS_GET_LASTEST_EVENTINFO;
		break;
	}
	case TRIP_WAVE_STATUS_SET_IG_TYPE:
	{
		RegisterDataGet(FUNCCODE_0X10, REG_SET_IG_VALUE_3001, 0x01);
		memset(m_setValue, 0, sizeof(m_setValue));
		m_setValue[0] = HIBYTE(0x01ff);
		m_setValue[1] = LOBYTE(0x01ff);
		m_setValueLen = 2;
		m_byRecvflag = TRIP_WAVE_STATUS_SET_IG_TYPE;
		break;
	}
	case TRIP_WAVE_STATUS_GET_IG_TYPE:
	{
		RegisterDataGet(FUNCCODE_0X04, REG_GET_IG_VALUE_3016, 0x01);
		m_byRecvflag = TRIP_WAVE_STATUS_GET_IG_TYPE;
		break;
	}
	case TRIP_WAVE_STATUS_SET: // 型号区分，px20只采电流波形，px25都采，
	{

		RegisterDataGet(FUNCCODE_0X10, REG_DEFINE_8193, 0x01);
		memset(m_setValue, 0, sizeof(m_setValue));
		m_setValue[0] = HIBYTE(m_waveType[m_waveCurType]);
		m_setValue[1] = LOBYTE(m_waveType[m_waveCurType]);
		// m_waveCurType++;
		m_setValueLen = 2;
		m_byRecvflag = TRIP_WAVE_STATUS_SET;
		break;
	}
	case TRIP_WAVE_STATUS_GET_LASTEST_EVENTID:
	{
		RegisterDataGet(FUNCCODE_0X03, REG_LATEST_EVENT_ID_8194, 0x0a);
		m_byRecvflag = TRIP_WAVE_STATUS_GET_LASTEST_EVENTID;
		break;
	}
	case TRIP_WAVE_STATUS_SET_LASTEST_EVENTID:
	{
		if (!RegisterDataSetLatestEventID())
		{
			m_childEventType = TRIP_WAVE_STATUS_SET;
			return FALSE;
		}
		m_byRecvflag = TRIP_WAVE_STATUS_SET_LASTEST_EVENTID;
		break;
	}
	case TRIP_WAVE_STATUS_GET_LASTEST_EVENT_TIME:
	{
		RegisterDataGet(FUNCCODE_0X03, REG_GET_EVENT_INFO_8204, 0x0B);
		m_byRecvflag = TRIP_WAVE_STATUS_GET_LASTEST_EVENT_TIME;
		m_waveRegNum = 0;
		break;
	}
	case TRIP_WAVE_STATUS_GET_LASTEST_WAVEINFO:
	{
		int regNum = 0;
		int startReg = REG_GET_EVENT_DATA_8215 + m_waveRegNum;
		if ((REG_TRIP_WAVE_COUNT - m_waveRegNum) > REG_TRIP_READ_WAVE_COUNT)
		{
			regNum = REG_TRIP_READ_WAVE_COUNT;
		}
		else
		{
			regNum = REG_TRIP_WAVE_COUNT - m_waveRegNum;
		}

		RegisterDataGet(FUNCCODE_0X03, startReg, regNum);

		m_byRecvflag = TRIP_WAVE_STATUS_GET_LASTEST_WAVEINFO;
		break;
	}
	case REAL_TIME_STATUS_GET_PXR25_VALUE:
	{
		int funcCode = FUNCCODE_0X03;
		if (pxr25RegInfo[m_realTimeCurFrame].type == 2)
		{
			funcCode = FUNCCODE_0X02;
		}
		RegisterDataGet(funcCode, pxr25RegInfo[m_realTimeCurFrame].startReg, pxr25RegInfo[m_realTimeCurFrame].regNum);
		m_byRecvflag = REAL_TIME_STATUS_GET_PXR25_VALUE;
		break;
	}
	case REAL_TIME_STATUS_GET_PXR20_VALUE:
	{
		int funcCode = FUNCCODE_0X03;
		if (pxr20RegInfo[m_realTimeCurFrame].type == 2)
		{
			funcCode = FUNCCODE_0X02;
		}
		RegisterDataGet(funcCode, pxr20RegInfo[m_realTimeCurFrame].startReg, pxr20RegInfo[m_realTimeCurFrame].regNum);
		m_byRecvflag = REAL_TIME_STATUS_GET_PXR20_VALUE;
		break;
	}
	case SYNC_TIME:
	{
		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "sync time !\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		time_t rawtime;
		struct tm *timeinfo;
		// 获取当前时间
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char buffer[80] = {0};
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H-%M-%S %A", timeinfo);

		memset(szBuf, 0, sizeof(szBuf));
		sprintf(szBuf, "current time : [%s]\n", buffer);
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

		RegisterDataGet(FUNCCODE_0X10, REG_SYNC_TIME_2921, 0x08);

		int year = timeinfo->tm_year + 1900;
		if (year < 2000)
		{
			year = 2015;
		}
		year -= 2000;
		int tmp[8] = {timeinfo->tm_mon + 1, timeinfo->tm_mday, year, timeinfo->tm_wday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, 0};
		memset(m_setValue, 0, sizeof(m_setValue));
		int index = 0;
		for (int i = 0; i < sizeof(tmp) / sizeof(int); i++)
		{
			m_setValue[index++] = HIBYTE(tmp[i]);
			m_setValue[index++] = LOBYTE(tmp[i]);
		}
		m_setValueLen = index;
		m_byRecvflag = SYNC_TIME;

		break;
	}
	default:
		sprintf(szBuf, "%s", "ModBusSCJZ9SY send buf err !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		return FALSE;
		break;
	}
	len = 0;

	// 判断是否为ModbusTCP还是ModbusRTU
	if (m_wModuleType == PXR20_MODBUS_TCP || m_wModuleType == PXR25_MODBUS_TCP)
	{
		if (m_token > 0xff)
		{
			m_token = 0;
		}
		buf[len++] = 0x00;
		buf[len++] = m_token++; // 事务标识符
		buf[len++] = 0x00;
		buf[len++] = 0x00; // 协议标识符 固定为0x0000（表示Modbus协议）
		buf[len++] = 0x00;
		buf[len++] = 0x00; // 数据长度 默认为0 统计完成后改
	}
	buf[len++] = m_wDevAddr; // 地址
	buf[len++] = m_funcCode; // 命令
	buf[len++] = HIBYTE(m_startRegister);
	buf[len++] = LOBYTE(m_startRegister);
	buf[len++] = 0x00;
	buf[len++] = m_registerNum;

	// sprintf(szBuf, "m_startRegister : [%d]  m_startRegisterHIBYTE: [ %2x ]   m_startRegisterloBYTE: [ %2x ] ", m_startRegister, HIBYTE(m_startRegister), LOBYTE(m_startRegister));
	// OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);

	if (m_funcCode == 0x10)
	{
		buf[len++] = m_setValueLen;
		memcpy(buf + len, m_setValue, m_setValueLen);
		len += m_setValueLen;
	}
	if (m_wModuleType == PXR20_MODBUS_TCP || m_wModuleType == PXR25_MODBUS_TCP)
	{
		buf[5] = len - 6; // 总共长度减6
	}
	else if (m_wModuleType == PXR20_MODBUS_RTU || m_wModuleType == PXR25_MODBUS_RTU)
	{
		WORD wCRC = GetCrc(buf, len);
		buf[len++] = HIBYTE(wCRC);
		buf[len++] = LOBYTE(wCRC);
	}

	// OutBusDebug(m_byLineNo, buf, len, 0);

	m_bySendCount++;
	if (!m_isLink)
	{
		std::string link = ReadConfig(LINK_PATH, "[com_status]", "link");
		if (link == "1")
		{
			WriteConfig(LINK_PATH, "[com_status]", "link", std::to_string(0));

			/*
			std::string strlink = ReadConfig("/mynand/data/unlinkCount.txt", "[count]", "linkCount");
			if (strlink != "")
			{
				int unlinkCount = std::stoi(strlink);
				unlinkCount++;
				WriteConfig("/mynand/data/unlinkCount.txt", "[count]", "linkCount", std::to_string(unlinkCount));
			}
			*/
		}
		else if (link == "")
		{
			CreateComtradeConfig(LINK_PATH);
		}
	}

	m_isLink = false;

	return TRUE;
} /* -----  end of method CModBusSCJZ9SY::GetProtocolBuf  ----- */
// modbus 通用获取数据
void CModBusEatonACB::RegisterDataGet(int funcCode, int startRegister, int registerNum)
{
	m_funcCode = funcCode;
	m_startRegister = startRegister;
	m_registerNum = registerNum;
}

BOOL CModBusEatonACB::RegisterDataSetLatestEventID(void)
{
	m_funcCode = FUNCCODE_0X10;
	m_startRegister = REG_SET_EVENT_ID_8198;
	m_registerNum = 0x02;

	if (!IsEventIDUpdate())
	{
		return FALSE;
	}
	memset(m_setValue, 0, sizeof(m_setValue));
	memcpy(m_setValue, m_latestEventID, sizeof(m_latestEventID) / sizeof(char));
	m_setValueLen = sizeof(m_latestEventID) / sizeof(char);
	return TRUE;
}

// 判断事件id是否更新
BOOL CModBusEatonACB::IsEventIDUpdate()
{
	int i;
	for (i = 0; i < 4; i++)
	{
		// if (m_latestEventID[i] != m_reqEventID[i])

		return TRUE;
	}
	return FALSE;
}
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSCJZ9SY
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */

BOOL CModBusEatonACB::ProcessProtocolBuf(BYTE *buf, int len)
{
	m_isLink = true;
	std::string link = ReadConfig(LINK_PATH, "[com_status]", "link");
	if (link == "0")
	{
		WriteConfig(LINK_PATH, "[com_status]", "link", std::to_string(1));
	}
	else if (link == "")
	{
		CreateComtradeConfig(LINK_PATH);
	}

	if (!WhetherBufValue(buf, len))
	{
		char szBuf[512] = "";
		sprintf(szBuf, "%s", "ModBusEaton recv buf is invalid error  !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		m_byRecvCount++;
		m_errorCount++;
		return FALSE;
	}

	if (!ProcessRecvBuf(buf, len))
	{
		char szBuf[512] = "";
		sprintf(szBuf, "%s", "ModBusEaton recv buf process error !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		m_byRecvCount++;
		m_errorCount++;
		return FALSE;
	}

	m_errorCount = 0;
	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSCJZ9SY
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusEatonACB::Init(BYTE byLineNo)
{
	/*
	if (!CreateDir())
	{
		return FALSE;
	}

	if (!CreateFileIfNotExists())
	{
		return FALSE;
	}
*/
	return TRUE;
} /* -----  end of method CModBusSCJZ9SY::Init  -------*/
BOOL CModBusEatonACB::CreateDir()
{
	if (mkdir(DATA_PATH, 0777) == 0)
	{ // 0777 是权限设置，根据你的需要可以修改
		printf("/mynand/data create success! \n");
	}
	else
	{
		if (errno == EEXIST)
		{
			printf("/mynand/data exist! \n");
		}
		else
		{
			printf("/mynand/data create error! \n");
			return FALSE;
		}
	}

	if (mkdir(WAVE_DATA_PATH, 0777) == 0)
	{ // 0777 是权限设置，根据你的需要可以修改
		printf("/mynand/data/wavedata create success! \n");
	}
	else
	{
		if (errno == EEXIST)
		{
			printf("/mynand/data/wavedata exist! \n");
		}
		else
		{
			printf("/mynand/data/wavedata create error! \n");
			return FALSE;
		}
	}
	if (mkdir(EVENT_DATA_PATH, 0777) == 0)
	{ // 0777 是权限设置，根据你的需要可以修改
		printf("/mynand/data/wavedata create success! \n");
	}
	else
	{
		if (errno == EEXIST)
		{
			printf("/mynand/data/wavedata exist! \n");
		}
		else
		{
			printf("/mynand/data/wavedata create error! \n");
			return FALSE;
		}
	}
	return TRUE;
}

// 检查文件是否存在
BOOL CModBusEatonACB::FileExists(const std::string &filename)
{
	return access(filename.c_str(), F_OK) == 0;
}

BOOL CModBusEatonACB::CreateEventIdConfig(const std::string &filename)
{

	/*
	FILE *file = fopen(filename.c_str(), "w");
	if (!file)
	{
		printf("%s create error!\n", filename.c_str());
		return FALSE;
	}
	fprintf(file, "[last_event_id]\n"
				  "trip_event_id:00000000\n"
				  "alarm_event_id:00000000\n"
				  "wave_event_id:00000000\n");
	fclose(file);
	return TRUE;*/

	std::ofstream file(filename);
	if (!file.is_open())
	{
		printf("%s create error! \n", filename.c_str());
		return FALSE;
	}
	file << "[last_event_id]\n"
			"trip_event_id=-1\n"
			"alarm_event_id=-1\n"
			"wave_event_id=-1\n";
	// 检查写入是否成功
	if (file.fail())
	{
		printf("错误：写入配置文件 '%s' 失败\n", filename.c_str());
		file.close();
		return FALSE;
	}
	file.close();
	return TRUE;
}

BOOL CModBusEatonACB::CreateComtradeConfig(const std::string &filename)
{
	std::ofstream file(filename);
	if (!file.is_open())
	{
		return FALSE;
	}
	file << "[com_status]\n"
			"link=0\n";
	file.close();
	printf("create file [%s]\n", filename.c_str());
	return TRUE;
}

BOOL CModBusEatonACB::CreateFileIfNotExists()
{
	std::string devName = m_sDevName;
	m_eventIDFilename = EVENT_DATA_PATH + devName + "_" + std::to_string(m_wDevAddr) + "_event_id.txt";

	if (FileExists(m_eventIDFilename))
	{
		printf("filename : [%s] exist ! \n", m_eventIDFilename.c_str());
	}
	else
	{
		printf("filename : [%s] not exist ! create \n", m_eventIDFilename.c_str());

		if (!CreateEventIdConfig(m_eventIDFilename))
		{
			printf("create [%s] failed ! \n", m_eventIDFilename.c_str());
			return FALSE;
		}
	}

	m_comtradeCfgFilename = CONFIG_PATH + std::string("comtrade.conf");
	if (!CreateComtradeConfig(LINK_PATH))
	{
		return FALSE;
	}
	/*
	if (FileExists(m_comtradeCfgFilename))
	{
		printf("filename : [%s] exist ! \n", m_comtradeCfgFilename.c_str());
	}
	else
	{
		printf("filename : [%s] not exist ! create \n", m_comtradeCfgFilename.c_str());
		if (!CreateComtradeConfig(m_comtradeCfgFilename))
		{
			return FALSE;
		}
	}*/
	return TRUE;
}

// 将字符串转换为十六进制数组
std::vector<uint8_t> CModBusEatonACB::StringToHexArray(const std::string &hexStr)
{
	std::vector<uint8_t> result;

	// 确保字符串长度是偶数（每个十六进制字节需要2个字符）
	if (hexStr.length() % 2 != 0)
	{
		return result;
	}

	for (size_t i = 0; i < hexStr.length(); i += 2)
	{
		std::string byteStr = hexStr.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
		result.push_back(byte);
	}

	return result;
}

// 将十六进制数组转换为字符串
std::string CModBusEatonACB::HexArrayToString(const std::vector<uint8_t> &hexArray)
{
	std::ostringstream oss;
	oss << std::hex << std::uppercase << std::setfill('0');

	for (size_t i = 0; i < hexArray.size(); ++i)
	{
		oss << std::setw(2) << static_cast<int>(hexArray[i]);
	}

	return oss.str();
}

// 去除字符串两端的空白字符
std::string CModBusEatonACB::Trim(const std::string &str)
{
	size_t first = str.find_first_not_of(" \t\n\r");
	if (std::string::npos == first)
	{
		return "";
	}
	size_t last = str.find_last_not_of(" \t\n\r");
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> CModBusEatonACB::ReadAllLines(const std::string &filename)
{
	std::vector<std::string> lines;
	std::ifstream file(filename);

	if (!file.is_open())
	{
		return {};
	}

	std::string line;
	while (std::getline(file, line))
	{
		lines.push_back(line);
	}

	file.close();
	return lines;
}

std::string CModBusEatonACB::ReadConfig(const std::string &filename, const std::string &mainKey, const std::string &key)
{
	if (!FileExists(filename))
	{
		printf("[%s] %s not exists\n", __FUNCTION__, filename.c_str());
		return {};
	}

	std::vector<std::string> lines = ReadAllLines(filename);
	bool inLastEventSection = false;

	for (const auto &line : lines)
	{
		std::string trimmedLine = Trim(line);

		// 检查是否是节头
		if (trimmedLine == mainKey)
		{
			inLastEventSection = true;
			continue;
		}

		// 如果不在正确的节中，跳过
		if (!inLastEventSection)
		{
			continue;
		}

		// 解析键值对
		size_t colonPos = trimmedLine.find('=');
		if (colonPos != std::string::npos)
		{
			std::string currentKey = Trim(trimmedLine.substr(0, colonPos));
			std::string value = Trim(trimmedLine.substr(colonPos + 1));

			if (currentKey == key)
			{
				return value; // StringToHexArray(value);
			}
		}
	}
	printf("not find key : [%s]\n", key.c_str());
	return {};
}

// 函数③：写入配置值
bool CModBusEatonACB::WriteConfig(const std::string &filename, const std::string &mainKey, const std::string &key, const std::string &newValue)
{
	if (!FileExists(filename))
	{
		// std::cerr << "错误：配置文件不存在！" << std::endl;
		printf("error filename %s not exist\n", filename.c_str());
		return false;
	}

	std::vector<std::string> lines = ReadAllLines(filename);
	bool inLastEventSection = false;
	bool keyFound = false;

	// 查找并修改目标键值
	for (auto &line : lines)
	{
		std::string trimmedLine = Trim(line);

		// 检查是否是节头
		if (trimmedLine == mainKey)
		{
			inLastEventSection = true;
			continue;
		}

		// 如果不在正确的节中，跳过
		if (!inLastEventSection)
		{
			continue;
		}

		// 解析键值对
		size_t colonPos = line.find('=');
		if (colonPos != std::string::npos)
		{
			std::string lineKey = Trim(line.substr(0, colonPos));

			if (lineKey == key)
			{
				// 找到目标键，更新值
				line = key + "=" + newValue;
				keyFound = true;
				break;
			}
		}
	}

	if (!keyFound)
	{
		std::cerr << "错误：未找到键 '" << key << "'" << std::endl;
		return false;
	}

	// 写回文件
	std::ofstream file(filename);
	if (!file.is_open())
	{
		std::cerr << "错误：无法打开文件进行写入！" << std::endl;
		return false;
	}

	for (const auto &line : lines)
	{
		file << line << "\n";
	}

	file.close();
	return true;
}

bool CModBusEatonACB::MonitorAndClean(const std::string &dirPath, double thresholdMB)
{
	// 1. 检查目录
	struct stat st;
	if (stat(dirPath.c_str(), &st) != 0 || !S_ISDIR(st.st_mode))
	{
		std::cerr << "dir not exist: " << dirPath << std::endl;
		return false;
	}

	// 2. 获取文件列表
	std::vector<FileInfo_t> files;
	uint64_t totalSize = 0;

	DIR *dir = opendir(dirPath.c_str());
	if (!dir)
	{
		std::cerr << "not open: " << dirPath << std::endl;
		return false;
	}

	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 ||
			strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}

		std::string fullPath = dirPath + "/" + entry->d_name;

		if (stat(fullPath.c_str(), &st) == 0 && S_ISREG(st.st_mode))
		{
			FileInfo_t info;
			info.name = entry->d_name;
			info.size = st.st_size;
			info.modifyTime = st.st_mtime;
			info.isDirectory = false;

			files.push_back(info);
			totalSize += st.st_size;
		}
	}
	closedir(dir);

	// 3. 检查是否需要清理
	double totalSizeMB = static_cast<double>(totalSize) / (1024 * 1024);
	std::cout << "dirSize: " << totalSizeMB << " MB" << std::endl;

	if (totalSizeMB <= thresholdMB)
	{
		return true;
	}

	// 4. 按时间排序并删除
	std::sort(files.begin(), files.end(),
			  [](const FileInfo_t &a, const FileInfo_t &b)
			  {
				  return a.modifyTime < b.modifyTime;
			  });

	uint64_t deletedSize = 0;
	for (const auto &file : files)
	{
		std::string fullPath = dirPath + "/" + file.name;
		if (remove(fullPath.c_str()) == 0)
		{
			deletedSize += file.size;
			std::cout << "delete: " << file.name << std::endl;

			double remainingMB = static_cast<double>(totalSize - deletedSize) / (1024 * 1024);
			if (remainingMB <= thresholdMB)
			{
				break;
			}
		}
	}

	return true;
}

time_t CModBusEatonACB::TimeToSeconds(int year, int month, int day,
									  int hour, int min, int sec)
{
	struct tm tm;
	memset(&tm, 0, sizeof(struct tm));

	tm.tm_year = year - 1900; // 年份从1900开始
	tm.tm_mon = month - 1;	  // 月份从0开始
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;
	tm.tm_isdst = -1; // 自动判断夏令时

	return mktime(&tm);
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSCJZ9SY
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:   void
 *		Return:  void
 *---------------------------------------------------------------------------------------
 */
void CModBusEatonACB::TimerProc(void)
{
	char szBuf[512] = "";

	time_t current_time = time(NULL);
	int spaceSeconds = std::abs(current_time - m_beginTime);
	m_fiveTimeSeconds += spaceSeconds;
	m_tenTimeSeconds += spaceSeconds;
	m_thirtyTimeMin += spaceSeconds;
	m_resetNetGpioTime += spaceSeconds;
	if (m_thirtyTimeMin >= THIRTY_MIN)
	{
		if ((m_curEventType == EATONACB_NULL) && (m_nextEventType == EATONACB_NULL))
		{
			m_curEventType = EATONACB_SYNC_TIME;
			m_childEventType = SYNC_TIME;
			m_thirtyTimeMin = 0;
		}
		m_resetNetGpioTime = 0;
	}
	if ((m_curEventType == EATONACB_NULL) && (m_nextEventType != EATONACB_NULL))
	{
		m_curEventType = m_nextEventType;
		m_childEventType = STATUS_GET_DEV_TYPE;
		m_nextEventType = EATONACB_NULL;
		// printf("switch task \n");
	}
	if (m_fiveTimeSeconds >= 1) // 5s
	{
		if (m_curEventType == EATONACB_NULL)
		{
			m_curEventType = EATONACB_REALTIME; // 实时数据采集开始
			m_childEventType = STATUS_GET_DEV_TYPE;
			// printf("switch realtime task \n");
			sprintf(szBuf, "%s", "switch realtime task !\n");
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		}
		else if (m_curEventType == EATONACB_TRIP_EVENT)
		{
			m_nextEventType = EATONACB_REALTIME; // 如果正在进行的是trip采集，则下一次采集的是实时数据
												 // printf("trip task running , next task is realtime\n");
		}
		m_fiveTimeSeconds = 0;
	}
	if (m_tenTimeSeconds >= 1) // 10s
	{
		if (m_curEventType == EATONACB_NULL)
		{
			m_curEventType = EATONACB_TRIP_EVENT; // trip事件采集开始
			m_childEventType = STATUS_GET_DEV_TYPE;
			// printf("trip task start \n");
			sprintf(szBuf, "%s", "switch trip task !\n");
			OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		}
		else if (m_curEventType == EATONACB_REALTIME)
		{
			m_nextEventType = EATONACB_TRIP_EVENT; // 如果正在进行的是实时数据采集，则下一次采集的是trip事件采集
												   // printf("realtime task running , next task is trip\n");
		}
		m_tenTimeSeconds = 0;
	}
	m_beginTime = time(NULL);
	if (m_bySendCount > 20 || m_byRecvCount > 20)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if (m_bLinkStatus)
		{
			m_bLinkStatus = FALSE;
		}
	}
} /* -----  end of method CModBusSCJZ9SY::TimerProc  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSCJZ9SY
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusEatonACB::GetDevCommState(void)
{
	if (m_bLinkStatus)
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
} /* -----  end of method CModBusEatonACB::GetDevCommState  ----- */
