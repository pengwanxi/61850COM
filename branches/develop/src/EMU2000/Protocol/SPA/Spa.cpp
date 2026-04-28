/// \文件:	SPA.cpp
/// \概要:	ABB Spa 协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2017-09-25


#include "Spa.h"
#include "../../share/global.h"

extern "C" void GetCurrentTime(REALTIME *pRealTime);
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


// int iGLinePos;									//模板某一行
// int iGLineNum;									//模板总行数
// int iGLinePosLast;								//模板行数上一行
// int iGTimeFlag;
time_t tGLastTime;
BYTE bGSrcBusNo;
BYTE bGVal;
BYTE bGType;
WORD wGSrcDevAddr;
WORD wGPnt;

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CSPA::CSPA()
{
	InitProtocolStatus();
	iGLinePos = 0;
	iGLinePosLast = 0;
	iGTimeFlag = 0;
	iGLineNum = 0;
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CSPA::~CSPA()
{
	m_SPA_CfgInfo.clear();
}

// --------------------------------------------------------
/// \概要:	打印
///
/// \参数:	buf
/// \参数:	len
// --------------------------------------------------------
void CSPA::print(char *buf, int len)
{
	OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf), 2);
}

// --------------------------------------------------------
/// \概要:	初始化协议基本状态
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::InitProtocolStatus()
{
	m_bLinkStatus      = FALSE;    // 链接状态为断
	m_SendStatus       = QUERY_YX; // 设为复位通信单元
	m_dwLinkTimeOut    = 0;        // 链接超时为0
	m_dwYkTimeOut      = 0;        // 遥控超时为0
	m_byYkErrorCount   = 0;        // 遥控错误计数0
	m_byRecvErrorCount = 0;        // 接收错误计数0
	m_bIsReSend        = FALSE;    // 重发标识位0
	m_byResendCount    = 0;        // 重发次数清零
	m_bIsSending       = FALSE;    // 发送后置1 接收后置0
	m_bIsNeedResend    = TRUE;     // 是否需要重发
	m_bIsYking         = FALSE;    // 是否遥控状态
	m_bIsYmCall        = FALSE;    // 是否召换YM

	m_wReSendLen    = 0;
	m_byYkSendLen   = 0;
	m_byRemoteBusNo = 0;
	m_byRemoteAddr  = 0;

	memset(m_byReSendBuf, 0, SPA_MAX_BUF_LEN);
	memset(m_byYkSendBuf, 0, sizeof(m_byYkSendBuf));
	memset(DebugBuf, 0, sizeof(DebugBuf));

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	获得模板信息
///
/// \参数:	byDataType
/// \参数:	wPnt
/// \参数:	tCfgInfo
/// \参数:	byFunType
/// \参数:	byInfoIndex
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::GetModuleInfo(BYTE bDataType, WORD wPnt, CfgInfo &tCfgInfo)
{
	int i = 0;
	for(i = 0; i < (int)m_SPA_CfgInfo.size(); i++){
		if(bDataType == m_SPA_CfgInfo[i].bDataType){
			if((wPnt >= m_SPA_CfgInfo[i].wStartNo) && (wPnt < (m_SPA_CfgInfo[i].wStartNo + m_SPA_CfgInfo[i].bDataNums))){
				memcpy(&tCfgInfo, &m_SPA_CfgInfo[i], sizeof(CfgInfo));
				iGLinePos = i;
				return TRUE;
			}
		}
	}

	if(i >= (int)m_SPA_CfgInfo.size()){
		sprintf(DebugBuf, "YK wPnt = %d not found", wPnt);
		printf("%s\n", DebugBuf);

		return FALSE;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理遥信报文
///
/// \参数:	buf
/// \参数:	len
//
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::SPA_YX_Process(BYTE *buf, int len)
{
	char cYxData[20];
	int i = 0, j = 0, iDataVal = 0, iPos = 0, iPnt = 0;
	BYTE bYxVal;

	if(buf == NULL || len < 6)
		return FALSE;

	if(buf[0] != ':')
		return FALSE;

	memset(cYxData, 0, sizeof(cYxData));
	for(i = 0; i < len; i++){
		if(buf[i + 1] >= '0' && buf[i + 1] <= '9'){
			cYxData[iPos++] = buf[i + 1];
			continue;
		}

		if(buf[i + 1] == '/' || buf[i + 1] == ':'){
			/*读完一个数*/
			iDataVal = atoi(cYxData);
			//printf("%s %d iDataVal = %d i = %d\n", __func__, __LINE__, iDataVal, i);
			if(m_SPA_CfgInfo[iGLinePos].wCoverCode != 0){
			//	int num = 0;
				for(j = 0; j < 16; j++){
					if(m_SPA_CfgInfo[iGLinePos].wCoverCode & (0x8000 >> j)){
						if(iDataVal & (0x8000 >> j))
							bYxVal = 1;
						else
							bYxVal = 0;
						m_pMethod->SetYxData(m_SerialNo, i + m_SPA_CfgInfo[iGLinePos].wStartNo, bYxVal);
					}
				}
			}
			else{
				if(iDataVal != 0)
					bYxVal = 1;
				else
					bYxVal = 0;
				m_pMethod->SetYxData(m_SerialNo, iPnt + m_SPA_CfgInfo[iGLinePos].wStartNo, bYxVal);
				iPnt++;
			}
			iPos = 0;
			memset(cYxData, 0, sizeof(cYxData));

			if(buf[i + 1] == ':')
				break;
		}
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理遥测报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::SPA_YC_Process(BYTE *buf, int len)
{
	char cYcData[20];
	int i = 0, iPos = 0, iPnt = 0;
	float fYcVal;
	BOOL bSign = TRUE;

	if(buf == NULL || len < 6)
		return FALSE;

	if(buf[0] != ':')
		return FALSE;

	memset(cYcData, 0, sizeof(cYcData));

	for(i = 0; i < len; i++){
		if(bSign)
			if(buf[i + 1] == '-' || buf[i + 1] == '+'){
				cYcData[iPos++] = buf[i + 1];
				continue;
			}

		if((buf[i + 1] >= '0' && buf[i + 1] <= '9') || buf[i + 1] == '.'){
			cYcData[iPos++] = buf[i + 1];
			continue;
		}

		bSign = FALSE;

		if(buf[i + 1] == '/' || buf[i + 1] == ':'){
			/*读完一个数*/
			fYcVal = atof(cYcData);
			//printf("%s %d fYcVal = %f i = %d, iPnt = %d , serialNo = %d\n", __func__, __LINE__, fYcVal, i, iPnt,m_SerialNo );
			m_pMethod->SetYcData(m_SerialNo, iPnt + m_SPA_CfgInfo[iGLinePos].wStartNo, fYcVal);
			iPnt++;

			iPos = 0;
			bSign = TRUE;
			memset(cYcData, 0, sizeof(cYcData));

			if(buf[i + 1] == ':')
				break;
		}
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理遥脉报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::SPA_YM_Process(BYTE *buf, int len)
{
	char cYmData[40];
	int i = 0, iPos = 0, iPnt = 0;
	long fYmVal;
	BOOL bSign = TRUE;

	if(buf == NULL || len < 6)
		return FALSE;

	if(buf[0] != ':')
		return FALSE;

	memset(cYmData, 0, sizeof(cYmData));

	for(i = 0; i < len; i++){
		if(bSign)
			if(buf[i + 1] == '-' || buf[i + 1] == '+'){
				cYmData[iPos++] = buf[i + 1];
				continue;
			}

		if(buf[i + 1] >= '0' && buf[i + 1] <= '9'){
			cYmData[iPos++] = buf[i + 1];
			continue;
		}

		bSign = FALSE;

		if(buf[i + 1] == '/' || buf[i + 1] == ':'){
			/*读完一个数*/
			fYmVal = atoi(cYmData);
			m_pMethod->SetYmData(m_SerialNo, iPnt + m_SPA_CfgInfo[iGLinePos].wStartNo, (QWORD)fYmVal);
			iPnt++;

			iPos = 0;
			bSign = TRUE;
			memset(cYmData, 0, sizeof(cYmData));

			if(buf[i + 1] == ':')
				break;
		}
	}

	return TRUE;
}

BOOL CSPA::SPA_YK_Process(BYTE *buf, int len)
{
	int byVal = 2;

	if(bGVal == 0)
		byVal = 1;
	else if(bGVal == 1)
		byVal = 0;

	//printf("AAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
	m_pMethod->SetYkExeRtn(this, bGSrcBusNo, wGSrcDevAddr, wGPnt, byVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	判断是否是闰年
///
/// \参数:	uYear
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::CParSetting_IsLeapYear(UINT uYear)
{
	if((uYear % 100) == 0){
		if((uYear % 400) == 0)
			return TRUE;
	}
	else if((uYear % 4) == 0)
		return TRUE;

	return FALSE;
}

// --------------------------------------------------------
/// \概要:	获得当前系统时间
///
/// \参数:	NewSoeTime
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::GetNewTime(TIMEDATA *NewSoeTime)
{
	time_t lSecond;
	struct tm currTime;
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	lSecond = (time_t)(tv.tv_sec);
	localtime_r(&lSecond, &currTime);
	NewSoeTime->MiSec  = tv.tv_usec / 1000;
	NewSoeTime->Second = currTime.tm_sec;
	NewSoeTime->Minute = currTime.tm_min;
	NewSoeTime->Hour   = currTime.tm_hour;
	NewSoeTime->Day    = currTime.tm_mday;
	NewSoeTime->Month  = currTime.tm_mon + 1;
	NewSoeTime->Year   = currTime.tm_year + 1990;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	得到完整SOE时间
///
/// \参数:	tSpaSoeTime
/// \参数:	wMilliSec
/// \参数:	bSec
/// \参数:	bMin
/// \参数:	bHour
/// \参数:	bDay
/// \参数:	bMonth
/// \参数:	wYear
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::Protocol_GetSoeTime(TIMEDATA *tSpaSoeTime, WORD wMilliSec, BYTE bSec, BYTE bMin, BYTE bHour, BYTE bDay, BYTE bMonth, WORD wYear)
{
	TIMEDATA NewSoeTime;

	GetNewTime(&NewSoeTime);

	/*毫秒*/
	if(wMilliSec != 0xffff)
		NewSoeTime.MiSec = wMilliSec;

	/*秒*/
	if(bSec != 0xff){
		if(NewSoeTime.Second < bSec)
			NewSoeTime.Minute--;
		NewSoeTime.Second = bSec;
	}

	/*分*/
	if(NewSoeTime.Minute < 0){
		NewSoeTime.Hour--;
		NewSoeTime.Minute += 60;
	}
	if(bMin != 0xff){
		if(NewSoeTime.Minute < bMin)
			NewSoeTime.Hour--;
		NewSoeTime.Minute = bMin;
	}

	/*时*/
	if(NewSoeTime.Hour < 0){
		NewSoeTime.Day--;
		NewSoeTime.Hour += 24;
	}
	if(bMin != 0xff){
		if(NewSoeTime.Hour < bHour)
			NewSoeTime.Day--;
		NewSoeTime.Hour = bHour;
	}

	/*年、月、日*/
	if(bDay != 0xFF){
		if(wYear < 100)
			wYear += 2000;
		NewSoeTime.Day   = bDay;
		NewSoeTime.Month = bMonth;
		NewSoeTime.Year  = wYear;
	}
	else{
		if(NewSoeTime.Day < 1){

			NewSoeTime.Month--;
			if(NewSoeTime.Month < 1){

				NewSoeTime.Year--;
				NewSoeTime.Month += 12;
			}

			/*判断月份*/
			switch(NewSoeTime.Month){
				case 1:
				case 3:
				case 5:
				case 7:
				case 8:
				case 10:
				case 12:
					NewSoeTime.Day = 31;
					break;

				case 4:
				case 6:
				case 9:
				case 11:
					NewSoeTime.Day = 30;
					break;

				case 2:
					if(CParSetting_IsLeapYear(NewSoeTime.Year))
						NewSoeTime.Day = 29;
					else
						NewSoeTime.Day = 28;
					break;
			}
		}
	}

	memcpy(tSpaSoeTime, &NewSoeTime, sizeof(TIMEDATA));

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理SOE报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::SPA_SOE_Process(BYTE *buf, int len)
{
	char cSoeTimeData[20], cSoeEventData[20];
	int i = 0, iPos = 0;
	float fTime;
	BYTE bEvent = 0, bSec = 0, bYxVal;
	BOOL bSign = TRUE;
	WORD wMilliSec = 0;
	TIMEDATA tSpaSoeTime;

	if(buf == NULL || len < 6)
		return FALSE;

	if(buf[0] != ':')
		return FALSE;

	memset(cSoeTimeData, 0, sizeof(cSoeTimeData));
	memset(cSoeEventData, 0, sizeof(cSoeEventData));

	for(i = 0; i < len; i++){
		if(buf[i + 1] == '/' || buf[i + 1] == ':'){
			bEvent = atoi(cSoeEventData);

			if(bEvent == 50 || bEvent == 51)
				m_SendStatus = QUERY_CLEAR;

			bSec = (BYTE)(fTime * 1000) / 1000;
			wMilliSec = (WORD)(fTime * 1000) % 1000;

			WORD wYxNo = 0;
			BOOL isCode0 = IsHaveEventCode(m_SPA_CfgInfo[iGLinePos].mapEventCode0, bEvent);
			BOOL isCode1 = IsHaveEventCode(m_SPA_CfgInfo[iGLinePos].mapEventCode1, bEvent);
			if (isCode0)
			{
				wYxNo = m_SPA_CfgInfo[iGLinePos].mapEventCode0[bEvent];
				bYxVal = 0;
				printf("addr = %d wYxNo = %d , bYxVal = %d\n" , m_wDevAddr , wYxNo , bYxVal );
			}
			else if (isCode1)
			{
				wYxNo = m_SPA_CfgInfo[iGLinePos].mapEventCode1[bEvent];
				bYxVal = 1;
				printf("addr = %d wYxNo = %d , bYxVal = %d\n" , m_wDevAddr , wYxNo , bYxVal );
			}

			Protocol_GetSoeTime(&tSpaSoeTime, wMilliSec, bSec, 0xff, 0xff, 0xff, 0xff, 0xffff);
			//m_pMethod->(m_SerialNo, wYxNo, bYxVal, &tSpaSoeTime);
            m_pMethod->SetYxData(m_SerialNo,wYxNo,bYxVal ) ;



			bSign = TRUE;
			iPos = 0 ;
		}
		else{
			if((buf[i + 1] >= '0' && buf[i + 1] <= '9') || buf[i + 1] == '.'){
				if(bSign)
					cSoeTimeData[iPos++] = buf[i + 1];
				else
					cSoeEventData[iPos++] = buf[i + 1];
			}
			else if(buf[i + 1] == ' ' && iPos > 0){
				/*读到空格，且正处理时间域*/
				fTime = atof(cSoeTimeData);
				iPos = 0, bSign = FALSE;
			}
		}
	}

	return TRUE;
}

BOOL  CSPA::IsHaveEventCode(map<WORD, WORD>& tEventCode,WORD bEventCode )
{
	int size = tEventCode.size();
	if (size == 0)
		return FALSE;

	if (tEventCode.end() == tEventCode.find(bEventCode))
		return FALSE;
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ProcessMessage(BYTE *buf, int len)
{
	int i = 0;
	BYTE *bData = NULL;

	for(i = 0; i < len; i++)
		if((buf[i + 2] < '0') || (buf[i + 2] > '9'))
			break;

	bData = buf + i + 2;

	//printf("%s %d *bData = %c, bDataType = %d\n", __func__, __LINE__, *bData, m_SPA_CfgInfo[iGLinePos].bDataType);
	switch(*bData){
		case 'A':			/*确认报文*/
			switch(bGType){
				case SPA_DATACLASS_YK:
					bGType = 0;
					return SPA_YK_Process(bData + 1, len - i - 3);
					break;

				default:
					return FALSE;
			}
			break;

		case 'N':			/*非确认报文*/
			break;

		case 'D':			/*数据报文*/
			switch(m_SPA_CfgInfo[iGLinePos].bDataType){
				case SPA_DATACLASS_YX:
					return SPA_YX_Process(bData + 1, len - i - 3);
					break;

				case SPA_DATACLASS_YC:
					return SPA_YC_Process(bData + 1, len - i - 3);
					break;

				case SPA_DATACLASS_YM:
					return SPA_YM_Process(bData + 1, len - i - 3);
					break;

				case SPA_DATACLASS_SOE:
					return SPA_SOE_Process(bData + 1, len - i - 3);
					break;

				default:
					return FALSE;
			}
			break;

		default:
			return FALSE;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	召唤遥信
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::CallYxData(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dR%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	召唤遥测
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::CallYcData(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dR%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	召唤遥脉
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::CallYmData(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dR%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	读定值
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ReadDzData(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dR%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	设置使能
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::WriteEnable(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dW%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	写定值
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::WriteDzData(BYTE *buf, int &len)
{

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	设置禁用
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::WriteDisEnable(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dW%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	重设
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ResetData(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dW%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	读最新SOE事件
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ReadSoeData(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dR%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	读子站状态
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ReadStatus(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dR%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	清空状态
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ClearStatus(BYTE *buf, int &len)
{
	int index = 0;

	index = sprintf((char *)buf, ">%dW%s:", m_wDevAddr, m_SPA_CfgInfo[iGLinePos].sZcode);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	对时
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::OnTime(BYTE *buf, int &len)
{
	printf("%s\n", (char *)"对时");
	int index    = 0;

	time_t pNowTime;
	struct tm pLocalTime;
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	pNowTime = (time_t)(tv.tv_sec);
	localtime_r(&pNowTime, &pLocalTime);

	char cTime[50];
	int iLen = 0;

	memset(cTime, 0, sizeof(cTime));

	sprintf(cTime, "%02d-%02d-%02d %02d.%02d;%02d.%03d", (int)(pLocalTime.tm_year - 100), (int)(pLocalTime.tm_mon + 1), (int)pLocalTime.tm_mday, (int)pLocalTime.tm_hour, (int)pLocalTime.tm_min, (int)pLocalTime.tm_sec, (int)tv.tv_usec);

	iLen = strlen(cTime);
	char cBuf[iLen + 1];

	memset(cBuf, 0, sizeof(cBuf));

	strncpy(cBuf, cTime, iLen);

	index = sprintf((char *)buf, ">900WT:%s:", cBuf);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;			//CRC H
	buf[index++] = iCrc & 0xff;					//CRC L
	buf[index++] = 0x0d;						//CR

	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	获得发送报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::GetSendBuf(BYTE *buf, int &len)
{
	BOOL bRtn = TRUE;

	m_SendStatus = m_SPA_CfgInfo[iGLinePos].bDataType;
	switch(m_SendStatus){
		case QUERY_YX:
//			printf("%s\n", (char *)"召唤遥信");
			CallYxData(buf, len);
			break;

		case QUERY_YC:
//			printf("%s\n", (char *)"召唤遥测");
			CallYcData(buf, len);
			break;

		case QUERY_YM:
//			printf("%s\n", (char *)"召唤遥脉");
			CallYmData(buf, len);
			break;

		case QUERY_DZ_R:
//			printf("%s\n", (char *)"读定值");
			ReadDzData(buf, len);
			break;

		case QUERY_ENABLE_W:
//			printf("%s\n", (char *)"设置使能");
			WriteEnable(buf, len);
			break;

		case QUERY_DZ_W:
//			printf("%s\n", (char *)"写定值");
			WriteDzData(buf, len);
			break;

		case QUERY_DISABLE_W:
//			printf("%s\n", (char *)"设置禁用");
			WriteDisEnable(buf, len);
			break;

		case QUERY_BHRESET:
//			printf("%s\n", (char *)"重设");
			ResetData(buf, len);
			break;

		case QUERY_SOE:
//			printf("%s\n", (char *)"读最新SOE事件");
			ReadSoeData(buf, len);
			break;

		case QUERY_STATUS_R:
//			printf("%s\n", (char *)"读子站状态");
			ReadStatus(buf, len);
			break;

		case QUERY_CLEAR:
//			printf("%s\n", (char *)"清空状态");
			ClearStatus(buf, len);
			break;

		default:
			sprintf(DebugBuf,  "SPA:GetProtocolBuf can't find m_SendStatus = %d\n", m_SendStatus);
			printf("%s\n", DebugBuf);
			break;
	}

	return bRtn;
}

// --------------------------------------------------------
/// \概要:	遥控选择
///
/// \参数:	pYkData
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::YkSel(PBUSMSG pBusMsg, YK_DATA *pYkData, BYTE *buf, int &len)
{
	CfgInfo YkSelCfgInfo;

	if(!GetModuleInfo(SPA_DATACLASS_YK, pYkData->wPnt, YkSelCfgInfo))
		return FALSE;

	m_pMethod->SetYkSelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, pYkData->wPnt, pYkData->byVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥控执行
///
/// \参数:	pYkData
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::YkExct(PBUSMSG pBusMsg, YK_DATA *pYkData, BYTE *buf, int &len)
{
	int index = 0;

	CfgInfo YkExctCfgInfo;

	bGSrcBusNo = pBusMsg->SrcInfo.byBusNo;
	wGSrcDevAddr = pBusMsg->SrcInfo.wDevNo;
	wGPnt = pYkData->wPnt;
	bGVal = pYkData->byVal;

	if(!GetModuleInfo(SPA_DATACLASS_YK, pYkData->wPnt, YkExctCfgInfo))
		return FALSE;

	bGType = YkExctCfgInfo.bDataType;

	if(pYkData->byVal == 0)
		index = sprintf((char *)buf, ">%dW%s:%d:", m_wDevAddr, YkExctCfgInfo.sZcode, SPA_YK_OPEN);
	else
		index = sprintf((char *)buf, ">%dW%s:%d:", m_wDevAddr, YkExctCfgInfo.sZcode, SPA_YK_CLOSE);

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) & 0xff;				//CRC H
	buf[index++] = iCrc & 0xff;						//CRC L
	buf[index++] = 0x0d;							//CR

	printf("#############################\n");
	len = index;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥控取消
///
/// \参数:	pYkData
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::YkCancel(PBUSMSG pBusMsg, YK_DATA *pYkData, BYTE *buf, int &len)
{
	CfgInfo YkCancelCfgInfo;

	if(!GetModuleInfo(SPA_DATACLASS_YK, pYkData->wPnt, YkCancelCfgInfo))
		return FALSE;

	m_pMethod->SetYkCancelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, pYkData->wPnt, pYkData->byVal);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理总线消息
///
/// \参数:	pBusMsg
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ProcessBusMsg(PBUSMSG pBusMsg, BYTE *buf, int &len)
{
	BOOL bRtn = TRUE;
	switch (pBusMsg->byMsgType){
		case YK_PROTO:			//遥控消息
		{
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
			m_byRemoteBusNo = pBusMsg->SrcInfo.byBusNo;
			m_byRemoteAddr = pBusMsg->SrcInfo.wDevNo;
			YK_DATA *pYkData = (YK_DATA *)pBusMsg->pData;
			switch(pBusMsg->dwDataType){
				case YK_SEL:
					printf("%s\n", (char *)"遥控选择");
					bRtn = YkSel(pBusMsg, pYkData, buf, len);
					m_byYkErrorCount = 1;
					break;

				case YK_EXCT:
					printf("%s\n", (char *)"遥控执行");
					bRtn = YkExct(pBusMsg, pYkData, buf, len);
					m_byYkErrorCount = 1;
					break;

				case YK_CANCEL:
					printf("%s\n", (char *)"遥控取消");
					bRtn = YkCancel(pBusMsg, pYkData, buf, len);
					m_byYkErrorCount = 0;
					break;

				default:
					break;
			}

			m_byYkSendLen = len;
			memcpy(m_byYkSendBuf, buf, m_byYkSendLen);
			m_dwYkTimeOut = 0;
			break;
		}
		default:
		{
			sprintf(DebugBuf, "SPA:ProcessBusMsg can't find msgtype = %d\n", pBusMsg->byMsgType);
			printf("%s\n", DebugBuf);
			return FALSE;
		//	break;
		}
	}				/* -----  end switch  ----- */

	return bRtn;
}

BOOL CSPA::DefaultCfgInfo()
{
	CfgInfo tCfgInfo;

	tCfgInfo.bDataType = 1;
	tCfgInfo.bDataNums = 8;
	tCfgInfo.wStartNo = 0;
	tCfgInfo.wCoverCode = 0;
	strcpy(tCfgInfo.sZcode, (char *)"O1/8");
	tCfgInfo.bEventCode1 = 0;
	tCfgInfo.bEventCode0 = 0;
	tCfgInfo.bLoop = 1;

	m_SPA_CfgInfo.push_back(tCfgInfo);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	读取配置信息
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ReadCfgInfo()
{
	FILE *fp = NULL;
	char *p  = NULL;
	BYTE i   = 0;
	BYTE bNum = 0;
	char szLineBuf[256];
	char szFileName[256];
	int iNum;

	memset(szLineBuf, 0, sizeof(szLineBuf));
	memset(szFileName, 0, sizeof(szFileName));
	sprintf(szFileName, "%s%s", ABBSPAPREFIXFILENAME, m_sTemplatePath);
	fp = fopen(szFileName, "r");
	if(fp == NULL){
		sprintf(DebugBuf, "CSPA:ReadCfgInfo fopen %s err!!!\n", szFileName);
		printf("%s\n", DebugBuf);
		return FALSE;
	}
	else{
		sprintf(DebugBuf, "CSPA:ReadCfgInfo fopen %s Ok!!!\n", szFileName);
		printf("%s\n", DebugBuf);
	}

	while(fgets(szLineBuf, sizeof(szLineBuf), fp) != NULL){
		i = 0;
		rtrim(szLineBuf);
		if(szLineBuf[0] == '#' || szLineBuf[0] == ';' || (szLineBuf[0]-'0') < 0 || (szLineBuf[0] - '0') > 9)
			continue;

		CfgInfo tCfgInfo;
		p = strtok(szLineBuf, ",");
		if(p == NULL)
			continue;
		else
			tCfgInfo.bDataType = atoi(p);

		while((p = strtok(NULL, ","))){
			++i, iNum = atoi(p);
			if(iNum > 255 || iNum < 0){
				sprintf(DebugBuf, "CSPA:ReadCfgInfo file: %s line:%d byte:%d is err!!! \n", m_sTemplatePath, (int)m_SPA_CfgInfo.size(), i);
				printf("%s\n", DebugBuf);
				continue;
			}
			switch (i){
				case 1:
					tCfgInfo.bDataNums = atoi(p);
					break;

				case 2:
					tCfgInfo.wStartNo = atoi(p);
					break;

				case 3:
					tCfgInfo.wCoverCode = atoi(p);
					break;

				case 4:
					sprintf(tCfgInfo.sZcode, "%s", p);
					break;

				case 5:
				{
					tCfgInfo.bEventCode1 = atoi(p);
					addEventCodeToMap(tCfgInfo, p , 1);
				}
					break;

				case 6:
				{
					tCfgInfo.bEventCode0 = atoi(p);
					addEventCodeToMap(tCfgInfo, p, 0);
				}
					break;

				case 7:
					tCfgInfo.bLoop = atoi(p);
					break;

				default:
					break;
			}
		}
		bNum++;
		m_SPA_CfgInfo.push_back(tCfgInfo);
	}

	iGLineNum = bNum;
	fclose(fp);

	return TRUE;
}

void CSPA::addEventCodeToMap(CfgInfo &tCfgInfo, char * p ,BYTE byCodeNo )
{
	if (p == NULL)
		return;
	char buf[1024] = { 0 };
	memcpy(buf, p, 1024);

	int i = 0,n = 0 ;
	char m = buf[i];
	char data[10] = { 0 };
	int index = 0;
	while ( m != 0 )
	{
		if (m >= '0' && m <= '9')
			data[n++] = m;
		else if (m == ' ')
		{
			int key = atoi(data);
			// 0 代表事件代码0 ，1代表事件代码1
			if (byCodeNo == 0)
				tCfgInfo.mapEventCode0[key] = index;
			else if (byCodeNo == 1)
				tCfgInfo.mapEventCode1[key] = index;

			memset(data, 0, sizeof(data));
			n = 0;
			index++;
		}
		m = buf[++i];
	}
}



// --------------------------------------------------------
/// \概要:	设置链路状态
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::GetDevCommState()
{
	if(m_bLinkStatus)
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	时间处理函数，主要处理一些超时、总召唤等与时间有关的
// --------------------------------------------------------
void CSPA::TimerProc()
{
	return;
	int Interval = 250;

	time_t tNowTime;
	time(&tNowTime);
	iGTimeFlag = (int)(tNowTime - tGLastTime);
	/*通讯超时时间*/
	m_dwLinkTimeOut += Interval;
	if(m_dwLinkTimeOut >= SPA_LINK_TIMEOUT){
		if(m_bLinkStatus == TRUE)
			InitProtocolStatus();
	}

	/*遥控超时 再处理*/
	if(m_byYkErrorCount > 0){
		m_dwYkTimeOut += Interval;
		if(m_dwYkTimeOut >= SPA_YK_TIMEOUT){
			m_dwYkTimeOut = 0;
			m_bIsYking    = TRUE;
			m_byYkErrorCount++;
			if(m_byYkErrorCount > 3)
				m_byYkErrorCount = 0;
		}
	}

	/*接收错误次数*/
	if(m_byRecvErrorCount > SPA_MAX_ERROR_COUNT){
		m_byResendCount = 0;
		InitProtocolStatus();
	}

	/*重发计数*/
	if(m_byResendCount >= SPA_MAX_RESEND_COUNT){
		m_byResendCount = 0;
		InitProtocolStatus();
	}
}

// --------------------------------------------------------
/// \概要:	处理收到的数据缓存
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::ProcessProtocolBuf(BYTE *buf, int len)
{
	int iPos   = 0;
	BOOL bRtn = TRUE;
	int i;
	for ( i = 0; i < len; i++)
	{
		//if (buf[i] == '<')
			if (buf[i-1] == 0x0a&&buf[i]=='<')
	
			break;
	}
	len=len-i+1;	
  	BYTE dealbuf[len];
	for(int j=0;j<len;j++)
	{
		dealbuf[j]=buf[j+i-1];
		
	}
	if(!WhetherBufValue(dealbuf, len, iPos)){
		printf("%s\n", (char *)"CSPA:ProcessProtocolBuf buf Recv err!!!");
		m_byRecvErrorCount++;
		m_bIsReSend = TRUE;
		return FALSE;
	}
// 	char pritbuf[len + 1];
// 	memset(pritbuf, 0, sizeof(pritbuf));
// 	strncpy(pritbuf, (char *)buf, len);
	//printf("%s\n", pritbuf);

	bRtn = ProcessMessage(&dealbuf[iPos], len);

	/*此处只判断是否处理 不能因为子站传的正确报文而没有处理导致通讯异常*/
	if( !bRtn )
		printf("%s\n", (char *)"处理报文发生错误或未处理");

	iGLinePos = (iGLinePos + 1) % iGLineNum;

	m_byRecvErrorCount = 0;
	m_bLinkStatus = TRUE;
	m_dwLinkTimeOut = 0;
	m_bIsReSend = FALSE;
	m_byResendCount = 0;
	m_bIsSending = FALSE;

	return bRtn;
}

// --------------------------------------------------------
/// \概要:	获取协议数据缓存
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
    //printf( "%s %s %d %d\n" , __FILE__ , __FUNCTION__ , __LINE__ , m_SerialNo ) ;

	BOOL bRtn = TRUE;
	if (m_bIsYking){
		printf("%s\n", (char *)"遥控重发");
		memcpy(buf, m_byYkSendBuf, m_byYkSendLen);
		len        = m_byYkSendLen;
		m_bIsYking = FALSE;
	}
	else if(m_bIsReSend || m_bIsSending){
		len = m_wReSendLen;
		memcpy(buf, m_byReSendBuf, len);
		m_byResendCount++;
		sprintf(DebugBuf, "重发 %d 次", m_byResendCount);
//		printf("%s\n", DebugBuf);
	}
//	else if(pBusMsg != NULL && m_bLinkStatus){
	else if(pBusMsg != NULL){
		printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		printf("%s\n", (char *)"总线消息");
		if(!ProcessBusMsg(pBusMsg, buf, len)){
			printf("%s\n", (char *)"总线消息处理失败");
			return FALSE;
		}
	}
	else{

		while(1){

			if(m_SPA_CfgInfo[iGLinePos].bLoop == SPALOOPOFF){
				iGLinePos = (iGLinePos + 1) % iGLineNum;
				continue;
			}

			if(m_SPA_CfgInfo[iGLinePos].bDataType == QUERY_CLOCK){
				if(iGTimeFlag > SPATIMEOUT){
					iGTimeFlag = 0;
					//	m_SendStatus = QUERY_CLOCK;
					OnTime(buf, len);
					time(&tGLastTime);
					iGLinePosLast = iGLinePos;
					iGLinePos = (iGLinePos + 1) % iGLineNum;

					printf("%s %d\n", __func__, __LINE__);
					char pritbuf[len + 1];
					memset(pritbuf, 0, sizeof(pritbuf));
					strncpy(pritbuf, (char *)buf, len);
					printf("%s\n", pritbuf);
					printf("##### ON TIME #####\n");

					break;
				}
				else{
					iGLinePos = (iGLinePos + 1) % iGLineNum;
					continue;
				}
			}
			else if(m_SPA_CfgInfo[iGLinePos].bDataType == QUERY_YK){
				iGLinePos = (iGLinePos + 1) % iGLineNum;
				continue;
			}

			bRtn = GetSendBuf(buf, len);
			if(bRtn){
				m_wReSendLen = len;
				memcpy(m_byReSendBuf, buf, m_wReSendLen);
				m_bIsSending = TRUE;
				if(!m_bIsNeedResend){
					m_bIsSending    = FALSE;
					m_bIsNeedResend = TRUE;
				}
			//	iGLinePos = (iGLinePos + 1) % iGLineNum;
			//	printf("GLinePos = %d\n" , iGLinePos );
// 				printf("%s %d\n", __func__, __LINE__);
// 				char pritbuf[len + 1];
// 				memset(pritbuf, 0, sizeof(pritbuf));
// 				strncpy(pritbuf, (char *)buf, len);
// 				printf("%s\n", pritbuf);

				break;
			}

		}
	}

	return bRtn;
}

// --------------------------------------------------------
/// \概要:	初始化协议数据
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CSPA::Init(BYTE byLineNo)
{

	if(!ReadCfgInfo()){
		printf("%s\n", (char *)"CSPA::ReadCfgInfo Err!!");
		DefaultCfgInfo();
		return FALSE;
	}


	if(!InitProtocolStatus()){
		printf("%s\n", (char *)"CSPA::InitProtocolStatus Err!!");
		return FALSE;
	}

	return TRUE;
}


