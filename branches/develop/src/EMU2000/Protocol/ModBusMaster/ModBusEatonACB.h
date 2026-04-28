

#ifndef MODBUSEATON_H
#define MODBUSEATON_H

#include "CProtocol_ModBusMaster.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <iostream>

typedef enum _MODBUSMASTER_EATONACB_TYPE
{
	EATONACB_NULL = 0,
	EATONACB_TRIP_EVENT,  // trip事件
	EATONACB_TRIP_WAVE,	  // trip波形
	EATONACB_ALARM_EVENT, // alarm事件
	EATONACB_ALARM_WAVE,  // alarm波形
	EATONACB_REALTIME,	  // 实时数据
	EATONACB_STATUS_WORD, // 状态字
	EATONACB_SYNC_TIME,
	EATONACB_MAX_TYPE,
} EatonACBEventType;

// 伊顿ACB设备类型
typedef enum _EATONACB_DEV_TYPE
{
	EATONACB_PX20 = 0,
	EATONACB_PX25,
} EatonACBDevType;

// alarm事件的两种类型 Major Alarm 和Minor Alarm
typedef enum
{
	ALARM_MAJOR = 5,
	ALARM_MINOR,
} AlarmEventType;

typedef enum
{
	STATUS_NULL = 0,
	STATUS_GET_DEV_TYPE,					 // 读取3004寄存器，获取ACB的型号. 6-9为PXR25，其余值均为PXR20
	TRIP_EVENT_STATUS_SET,					 // 读取trip事件数据① 向8193寄存器写入0x80FF
	TRIP_EVENT_STATUS_GET_LASTEST_EVENTID,	 // 读取trip事件数据② 读取8196-8197寄存器，获得最新的Trip事件ID，ID更新则表示有新的Trip事件产生
	TRIP_EVENT_STATUS_SET_LASTEST_EVENTID,	 // 读取trip事件数据③ 把最新的事件ID写入8198-8199寄存器
	TRIP_EVENT_STATUS_GET_LASTEST_EVENTINFO, // 读取trip事件数据④ 读出8204-8252寄存器的数据，获得Trip事件信息

	// 获取ACB的型号 对于PXR20，只需要读取IA/IB/IC/IN/IG电流波形即可，对于PXR25，才需要读取电压波形
	TRIP_WAVE_STATUS_SET_IG_TYPE,			 // 向3001寄存器写入0x01FF
	TRIP_WAVE_STATUS_GET_IG_TYPE,			 // 获取IG的源电流波形还是剩余电流波形
	TRIP_WAVE_STATUS_SET,					 // 读取trip波形数据① 向8193寄存器写入0x000A，查询Trip事件的A相波形。
	TRIP_WAVE_STATUS_GET_LASTEST_EVENTID,	 // 读取trip波形数据② 读取8196-8197寄存器，获得最新的带波形的Trip事件ID
	TRIP_WAVE_STATUS_SET_LASTEST_EVENTID,	 // 读取trip波形数据③ 把最新的事件ID写入8198-8199寄存器
	TRIP_WAVE_STATUS_GET_LASTEST_EVENT_TIME, // 读取trip波形数据④ 读取8204-8211寄存器，获取波形的时间信息。
	TRIP_WAVE_STATUS_GET_LASTEST_WAVEINFO,	 // 读取trip波形数据⑥ 从8215开始，对于PXR20/25，连续读取768个寄存器，获取A相波形数据。

	ALARM_EVENT_STATUS_SET,					  // 读取alarm事件数据① 向8193寄存器写入0x81FF
	ALARM_EVENT_STATUS_GET_LASTEST_EVENTID,	  // 读取alarm事件数据② 读取8196-8197寄存器，获取最新的Alarm事件ID，ID更新则表示有新的Alarm事件产生
	ALARM_EVENT_STATUS_SET_LASTEST_EVENTID,	  // 读取alarm事件数据③ 将事件ID写入8198-8199寄存器，请求查询该Alarm事件
	ALARM_EVENT_STATUS_GET_EVENT_TYPE,		  // 读取alarm事件数据④ 读取8212寄存器，获取Alarm事件类型。5 = Major Alarm，6 = Minor Alarm。
	ALARM_EVENT_STATUS_GET_LASTEST_EVENTINFO, // 读取alarm事件数据⑤ 对于Major Alarm事件，读取8204-8252寄存器，获取事件信息；对于Minor Alarm事件，读取8204-8214寄存器，获取事件信息。

	REAL_TIME_STATUS_GET_PXR25_VALUE, // 获取PXR25实时数据
	REAL_TIME_STATUS_GET_PXR20_VALUE, // 获取PXR20实时数据

	STATUS_WORD_GET_VALUE, // 获取状态字

	SYNC_TIME, // 同步时间
} EatonACBEventStatus;

// modbus字节序类型 分为四种 ABCD  BADC CDAB  DCBA
typedef enum
{
	MODBUS_BYTEORDER_TYPE_NULL = 0,
	MODBUS_BYTEORDER_TYPE_ABCD,
	MODBUS_BYTEORDER_TYPE_BADC,
	MODBUS_BYTEORDER_TYPE_CDAB,
	MODBUS_BYTEORDER_TYPE_DCBA
} ModbusByteOrderType;

// 具体类型数值结构体，包含通道编号，通道名称，通道相，单位，fCoefA，fCoefB，fTime,fMin~,fMax~
typedef struct _tagEatonACBChannelInfo
{
	int channelNo;		   // 通道索引号An  1-8
	char channelName[10];  // 通道标识ch_id   IA/IB/IC/IN/IG/Vab/Vbc/Vca
	char channelPhase[10]; // 通道相别标识ph  A/B/C/N/G
	char ccbm[10];		   // 被监视的器件ccbm 无要求
	char channelUnit;	   // 通道单位  A/V
	float fCoefA;		   // 增益系数a 1
	float offsetB;		   // 偏移量b 0
	float skew;			   // 通道时滞
	float fMin;			   // 最小值	-252000
	float fMax;			   // 最大值	252000
	float transRatio1;	   // 一次侧变比 无要求
	float transRatio2;	   // 二次侧变比 无要求
	char pOrS;			   // P/S 无要求
} EatonACBChannelInfo_t;

typedef struct
{
	int num;		// 序号
	int time;		// 采样时间
	float value[8]; // 值
} EatonACBComtrade_t;

// 波形配置文件结构体
typedef struct
{
	unsigned char devName[8]; // 厂站名
	int waveID;				  // 波形ID
	int fileVer;			  // 文件版本
	int channelCount;		  // 通道总数
	int analogChannelCount;	  // 模拟通道总数
	int digitalChannelCount;  // 状态通道数量
	float powerHZ;			  // 电网频率 50
	int sampleNum;			  // 采样率个数 1
	float sampleFreq;		  // 采样率samp 3200
	int samplePoint;		  // 采样率个数nrates 384
	int dataType;			  // 数据文件类型，默认float32    0:ascii 1:binary 2:Binary32 3:float32
	REALTIME waveTime;		  // 事件触发时间
	float timeRate;			  // 时标倍率因子 1
	int timeQos;			  // 时标品质  无要求
	int devType;
	// EatonACBChannelInfo_t channelPX20Info[5]; // px20 通道信息
	EatonACBChannelInfo_t channelInfo[8];
	EatonACBComtrade_t waveData[384];
} EatonACBWaveConfig_t;

// 实时数据 结构体 起始寄存器和寄存器数量
typedef struct
{
	int startReg;
	int regNum;
	int type;  // 采集点类型 1:遥测  2:遥信  3:遥脉
	int point; // 对应excel采集点

} RealTimeRegInfo_t;

typedef struct
{
	std::string name;
	uint64_t size;
	time_t modifyTime;
	bool isDirectory;
} FileInfo_t;

class CModBusEatonACB : public CProtocol_ModBusMaster
{
public:
	/* ====================  LIFECYCLE     ======================================= */
	CModBusEatonACB();	/* constructor      */
	~CModBusEatonACB(); /* destructor       */

	virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
	virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);
	virtual BOOL Init(BYTE byLineNo);

	virtual void TimerProc();
	// 获得装置通讯状态
	virtual BOOL GetDevCommState();

protected:
	/* ====================  DATA MEMBERS  ======================================= */

private:
	// 写comtrade数据文件
	BOOL WriteComtradeDataFloatFile(const std::string &filename);
	// 写comtrade配置文件
	BOOL WriteComtradeCfgFile(const std::string &filename);
	// 判断报文有效性
	BOOL WhetherBufValue(BYTE *buf, int &len);

	// 设置最新事件id
	BOOL RegisterDataSetLatestEventID(void);

	// modbus通用函数
	void RegisterDataGet(int funcCode, int startRegister, int registerNum);

	// 处理设置寄存器返回报文  04 功能码
	BOOL ProcessRecvSetCommon(BYTE *buf, int len, int reg, int regNum);
	// 获取最新事件ID返回报文
	BOOL ProcessRecvGetLastestEventID(BYTE *buf, int len);
	// 从报文中获取时间数据
	time_t GetTimeData(BYTE *buf, int len, TIMEDATA *time);

	// 处理1001寄存器数据 yx
	BOOL ProcessRecvRealTime_1001(BYTE *buf, int len);
	// 处理4609寄存器数据 yc
	BOOL ProcessRecvRealTime_4609(BYTE *buf, int len);
	// 处理5009寄存器数据
	BOOL ProcessRecvRealTime_5009(BYTE *buf, int len);
	// 处理4959寄存器数据
	BOOL ProcessRecvRealTime_4959(BYTE *buf, int byteCount, int ymPnt);

	// 处理PXR20实时数据返回报文
	BOOL ProcessRecvPXR20RealTime(BYTE *buf, int len);
	// 处理PXR25实时数据返回报文
	BOOL ProcessRecvPXR25RealTime(BYTE *buf, int len);
	// 处理事件数据返回报文 trip事件信息，alarm事件信息，trip波形信息
	BOOL ProcessEventData(BYTE *buf, int len);
	// 获取IG类型
	BOOL ProcessRecvGetIGType(BYTE *buf, int len);
	// 获取波形时间
	BOOL ProcessRecvLastWaveTime(BYTE *buf, int len);
	// 获取波形信息
	BOOL ProcessRecvLastWaveData(BYTE *buf, int len);
	// 获取设备类型
	BOOL ProcessRecvDevType(BYTE *buf, int len);
	// 获取Alarm事件类型
	BOOL ProcessRecvGetAlarmEventType(BYTE *buf, int len);

	// 处理报文
	BOOL ProcessRecvBuf(BYTE *buf, int len);

	BOOL IsEventIDUpdate();
	// int 转数组
	std::vector<uint8_t> ByBitOps(int value, int order);
	// 四字节转换为float
	float HexToFloat32(const uint8_t bytes[4], int order);
	// 四字节转换为int
	int HexToInt32(const uint8_t bytes[4], int order);
	// 小端序写入
	void WriteInt16InLittleEndian(std::ofstream &file, int16_t value);
	void WriteInt32InLittleEndian(std::ofstream &file, int32_t value);
	void WriteFloat32InLittleEndian(std::ofstream &file, float value);
	void WriteInt64InLittleEndian(std::ofstream &file, int64_t value);
	// 创建文件夹
	BOOL CreateDir();
	// 文件是否存在
	BOOL FileExists(const std::string &filename);
	// 创建事件ID配置文件
	BOOL CreateEventIdConfig(const std::string &filename);
	// 创建Comtrade配置文件
	BOOL CreateComtradeConfig(const std::string &filename);
	// 判断文件是否存在，不存在则创建文件
	BOOL CreateFileIfNotExists();
	// 将字符串转换为十六进制数组
	std::vector<uint8_t> StringToHexArray(const std::string &hexStr);
	// 将十六进制数组转换为字符串
	std::string HexArrayToString(const std::vector<uint8_t> &hexArray);
	// 去除字符串两端的空白字符
	std::string Trim(const std::string &str);
	// 读取文件所有行
	std::vector<std::string> ReadAllLines(const std::string &filename);
	// 读取Event配置值
	std::string ReadConfig(const std::string &filename, const std::string &mainkey, const std::string &key);
	// 写入配置值
	bool WriteConfig(const std::string &filename, const std::string &mainKey, const std::string &key, const std::string &newValue);
	// 判断波形文件夹是否大于15MB
	bool MonitorAndClean(const std::string &dirPath, double thresholdMB);
	// 获取设备健康度 0 1 2  0<= Lifepoint <7500 ok ;7500 <= lifePoint < 10000  Waning ;Lifepoint >= 10000  Alarm
	int GetPhyHealth(float lifePoint);
	// 获取软遥信值 报警
	int GetYxPntValue(int *yxPnt, int cause);
	// 获取文件中事件ID
	int GetFileEventID(const std::string &filename, const std::string &mainKey, const std::string &key);
	// 时间字符串转换为秒数  %s "2024-01-01 12:00:00"
	time_t TimeToSeconds(int year, int month, int day, int hour, int min, int sec);

private:
	/* ====================  DATA MEMBERS  ======================================= */
	int m_token; // tcp token

	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;
	BYTE m_errorCount; // 错误次数，错误两次后，取消当前任务，进行下一个
	int m_byRecvflag;
	BYTE buf_fz[4];			// 存 放 辅 助 信 息
	time_t m_beginTime;		// 计时开始时间
	int m_fiveTimeSeconds;	// 计时秒数每5秒重置一次
	int m_tenTimeSeconds;	// 计时秒数每10秒重置一次
	int m_thirtyTimeMin;	// 计时秒数每30分钟重置一次 用来做同步时钟
	int m_resetNetGpioTime; // 重启网卡
	int m_devType;

	EatonACBEventType m_curEventType; // 正在采集类型
	// 下一个采集类型 根据时间分为两种，一种5s 一种10s，当检测到当前的采集类型与要采集的类型相同，不做处理；如果不同，则赋值给nextEventType
	EatonACBEventType m_nextEventType;

	int m_childEventType; // 流程子状态
	int m_recvchildEventType;
	int m_funcCode; // 功能码

	int m_toraEventType;  // 当前采集trip事件还是alarm事件
	int m_alarmEventType; // alarm事件分为  Major Alarm 和Minor Alarm

	// std::vector<uint8_t> m_latestEventID;

	char m_latestEventID[4]; // 要读取的事件id

	u_int16_t m_startRegister;
	u_int16_t m_registerNum;

	char m_setValue[32];
	int m_setValueLen;

	int m_waveType[10];				   // 波形类型
	int m_waveMaxType;				   // 波形类型数量，px20是5个 px25是8个
	int m_waveCurType;				   // 当前波形类型
	REALTIME m_waveTime;			   // 波形时间信息
	int m_waveTimeInterval;			   // 波形点到点的时间间隔
	EatonACBWaveConfig_t m_waveConfig; // 波形配置信息 comtrade 包含数据信息

	int m_waveRegNum;			 // 读取到波形的寄存器数量
	int m_realTimeSendCount;	 // 实时数据发送次数 当超过四次置0，实时数据全部上一次
	int m_realTimeMaxFrameCount; // 实时数据最大帧数量数量
	int m_realTimeCurFrame;		 // 实时数据当前帧    根据设备类型确定  非快速数据报文20s采一次  快速报文5s采一次

	std::string m_eventIDFilename;	   // eventid文件名每个设备都有一个
	std::string m_comtradeCfgFilename; // comtrade配置文件名

	std::string m_comtradeDatType; // comtrade数据类型

	int m_maxEventId; // 最大trip事件id  单次读上来的要写到文件
	int m_curEventId;

	/*①读取最新事件id，从文件获取id，判断是否为最新
	②不是最新，则将文件中id写到请求寄存器
	③再读最新事件id，将nextid作为当前id写到请求寄存器，读事件，读波形
	④程序第一次启动，不经过③，直接将最老的id写到请求寄存器，读事件，读波形
	⑤alarm相同流程*/
	int m_readEventProcess; // 0是程序第一次启动，1是第一次读取最新id 2是设置事件id后第二次读取最新id 读取id步骤
};
#endif
