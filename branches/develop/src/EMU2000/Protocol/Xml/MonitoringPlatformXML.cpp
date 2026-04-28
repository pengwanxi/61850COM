/// \文件:	MonitoringPlatformXML.cpp
/// \概要:	XML 协议继承类定义
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-03-22
#include "MonitoringPlatformXML.h"
#include <map>
#include <vector>
using namespace std;
#define HANDSHAKE_DEFEAT 0
#define HANDSHAKE_SUCCESS 1

#define SEND_ALLOW 1
#define SEND_NOT_ALLOW 2

#define SEND_AGAIN_TIME 5

#define CMD_DZ_BIT 0x0010	 // 定值选择
#define CMD_COA_BIT 0x0020	 // 遥控选择
#define CMD_UNVARNISH 0x0040 // 透传命令
#define CMD_IDEN_DET 0x0100	 // 身份验证
#define CMD_IDEN_BIT 0x0200	 // 身份验证确认	开始数据传输
#define CMD_IDEN_END 0x0400	 // 身份验证确认	结束数据传输

#define CMD_NULL 0
#define CMD_TIME_CONFIRM 1	// 对时确认
#define CMD_TIME_END 2		// 对时结束
#define CMD_TOTAL_CONFIRM 3 // 总召确认
#define CMD_TOTAL_END 4		// 总召结束
#define CMD_YM_CONFIRM 5	// 遥脉确认
#define CMD_YM_END 6		// 遥脉结束
#define CMD_YM_ERROR 7		// 召唤遥脉数据出错
#define CMD_YK_ERROR 8		// 发送遥控否定认可
#define CMD_DZ_ERROR 9		// 发送定值否定认可
#define CMD_REG_REQUSET 10      //reg_request 请求

#define YX_START_ADDR 0x0001
#define YC_START_ADDR 0x4001
#define YK_START_ADDR 0x6001
#define YM_START_ADDR 0x6401

#define XML_T1 15	 // S
#define XML_T2 10	 // S
#define XML_T3 20	 // S
#define XML_T4 30	 // S
#define XML_YKTIME 5 // S
#define XML_DZTIME 8 // S

#define XML_START_T1 0x01
#define XML_START_T2 0x02
#define XML_START_T3 0x04
#define XML_START_T4 0x08
#define XML_START_YK 0x10
#define XML_START_DZ 0x20

#define XML_END_T1 0xFE
#define XML_END_T2 0xFD
#define XML_END_T3 0xFB
#define XML_END_T4 0xF7
#define XML_END_YK 0xEF
#define XML_END_DZ 0xDF

#define ONE_BUF_MOUNT_YX 1000
#define ONE_BUF_MOUNT_YM 500
#define ONE_BUF_MOUNT_YC 3000

extern "C" int SetCurrentTime(REALTIME *pRealTime);

struct pntval
{
	WORD wPnt;
	float fVal;
	QWORD dwVal;
};

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
MonitoringPlatformXML::MonitoringPlatformXML()
{
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memset(m_XmlDITimeOutBuf, 0, sizeof(m_XmlDITimeOutBuf));
	memset(&m_xml, 0, sizeof(m_xml));

	m_wDataIndex = 0;
	m_wCommand = 0;
	m_wExcmd = 0;
	m_bStartBit = FALSE;
	m_byTimeFlag = 0;
	m_byDITimeOut = 0;
	m_YkFlag = FALSE;
	m_iDataFlag = 0;
	m_wDeadVal = 3;
	m_wPreDevId = 0xFFFF;
	m_iAllCall_Update = 0;

	m_byStaticLineNo = 0;
	m_byStaticDevAddr = 0;

	m_t1 = 0;
	m_t2 = 0;
	m_t3 = 0;
	m_t4 = 0;
	m_YKTime = 0;
	m_DZTime = 0;
	m_DevStateTime = 0;
	m_VaryDevStateTime = 0;
	m_YxValueTime = 0;
	m_YcValueTime = 0;
	m_YmValueTime = 0;
	memset(m_bSequence, 0, sizeof(m_bSequence));
	memset(m_bDITimeOutSequence_YX, 0, sizeof(m_bDITimeOutSequence_YX));

	m_pTX_Buf = NULL;
	m_pRX_Buf = NULL;

	m_iDzDataNum = 0;
	m_dwDataType = 0;
	m_pDzData = NULL;
	memset(m_byDzType, 0, sizeof(m_byDzType));

	m_byDIBuf = new BYTE[XMLMAX_DI_LEN];
	m_wAIBuf = new float[XMLMAX_AI_LEN];
	m_dwPIBuf = new QWORD[XMLMAX_PI_LEN];
	memset(m_byDIBuf, 0, XMLMAX_DI_LEN);
	memset(m_wAIBuf, 0, XMLMAX_AI_LEN * 4);
	memset(m_dwPIBuf, 0, XMLMAX_PI_LEN * 8);
	modflag = 1;
	dataindex = 0;
	dataindex_zyx = 0;
	fenbaoflag = 1;
	last_ycbuf_issending = 0;
	last_ymbuf_issending = 0;
	last_yxbuf_issending = 0;

	time(&time_reboot_flag);
	reboot_flag=0;//重启标志默认不重启 当接收到accept后置为1	
	CanSendChange=false;
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
MonitoringPlatformXML::~MonitoringPlatformXML()
{
	m_bTaskRun = FALSE;
}

// --------------------------------------------------------
/// \概要:	初始化转发
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::InitRtuBase()
{
	UINT uPort;
	//	BOOL bOk = FALSE;
	char szCtrl[32] = {'\0'};

	CBasePort::GetCommAttrib(m_ComCtrl1, szCtrl, uPort);
	m_wPortNum = (WORD)uPort;

	/* 获取转发序号 */
	CreateTransTab();

	/* 从内存数据库中获取转发表默认数据 */
	m_pMethod->ReadAllYcData(m_wAIBuf);
	m_pMethod->ReadAllYmData(m_dwPIBuf);
	m_pMethod->ReadAllYxData(m_byDIBuf);

	m_bTaskRun = TRUE;

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	综合平台 XML 初始化
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::Init(BYTE byLineNo)
{
	char szFileName[256] = "";
	sprintf(szFileName, "%s%s", XMLPREFIXFILENAME, m_sTemplatePath);

	/* 初始设置为发送身份验证 */
	m_wCommand |= CMD_IDEN_DET;

	/* 读取需要转发的数据到该模块 */
	ReadMapConfig(szFileName);

	ReadMapConfig_transid(szFileName);

	/* 初始化该模块 */
	InitRtuBase();

	/* 初始化上传所有装置状态时间 */
	time(&m_DevStateTime);

	/* 初始化上传变化装置状态时间 */
	time(&m_VaryDevStateTime);

	/* 初始化实时上传遥信遥测遥脉时间 */
	time(&m_YxValueTime);
	time(&m_YcValueTime);
	time(&m_YmValueTime);

	char szLog[100] = {0};
	sprintf(szLog, "%s-XML", m_sDevName);
	m_log.setLogKey(szLog);

	m_log.writeLog("reboot");
	return TRUE;
}

void MonitoringPlatformXML::ReadMapConfig(LPCSTR lpszFile)
{
	CRtuBase::ReadMapConfig(lpszFile);
}
void MonitoringPlatformXML::ReadMapConfig_transid(LPCSTR lpszFile)
{
	FILE *fd;
	char strLine[256] = {'\0'};
	char strLine_xml[256] = {'\0'};
	int nType;

	fd = fopen(lpszFile, "r"); // rtux.txt
	if (fd == NULL)
	{
		LogPromptText("\n Open file %s failure!", lpszFile);
		return;
	}
	while (fgets(strLine, sizeof(strLine), fd))
	{
		int num = 0;
		ltrim(strLine);
		if (strLine[0] == ';' || strLine[0] == '#')
			continue;
		char *token = NULL;
		token = strtok(strLine, ",");
		/* 继续获取其他的子字符串 */
		while (token != NULL)
		{
			if (num == 2)
				trans_id.push_back(atoi(token));
			token = strtok(NULL, ",");
			num++;
		}
	}
	fclose(fd);
}

void MonitoringPlatformXML::ReadConfigOtherFuc(BYTE nType, char *strLine)
{
	//	printf("----FUNC = %s LINE = %d strLine = %s----\n", __func__, __LINE__, strLine);
	char *pItem, *pParam;
	pItem = strtok(strLine, "=");
	if (pItem == NULL)
		return;
	pParam = strtok(NULL, "=");
	if (pParam == NULL)
		return;
	//	printf("----FUNC = %s LINE = %d pItem = %s pParam = %s----\n", __func__, __LINE__, pItem, pParam);

	if (strstr(pItem, "upyxintervaltime"))
		m_wYxUploadInterval = (WORD)atoi(pParam);
	else if (strstr(pItem, "upycintervaltime"))
		m_wYcUploadInterval = (WORD)atoi(pParam);
	else if (strstr(pItem, "upymintervaltime"))
		m_wYmUploadInterval = (WORD)atoi(pParam);

	//	printf("----FUNC = %s LINE = %d yx = %d yc = %d ym = %d----\n", __func__, __LINE__, m_wYxUploadInterval, m_wYcUploadInterval, m_wYmUploadInterval);
}

void MonitoringPlatformXML::ReSetDataState()
{
	/* 重置超时时间 */
	m_byTimeFlag = 0;
	/* 重置时间 */
	SetTTimer(XML_T1, XML_END_T1);
	SetTTimer(XML_T2, XML_END_T2);
	SetTTimer(XML_T3, XML_END_T3);
	SetTTimer(XML_T4, XML_END_T4);
	SetTTimer(XML_YKTIME, XML_END_YK);
	SetTTimer(XML_DZTIME, XML_END_DZ);

	/* StartDI退出 */
	m_bStartBit = FALSE;

	/* 清除发送标志 */
	m_wCommand = 0;

	/* 重置传输所有装置状态的时间 */
	time(&m_DevStateTime);

	/* 重置传输变化装置状态的时间 */
	time(&m_VaryDevStateTime);

	/* 重置实时上传遥信遥测遥脉时间 */
	time(&m_YxValueTime);
	time(&m_YcValueTime);
	time(&m_YmValueTime);
}

// --------------------------------------------------------
/// \概要:	重置状态
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::ReSetState()
{
	/* 重置超时时间 */
	m_byTimeFlag = 0;
	/* 重置时间 */
	SetTTimer(XML_T1, XML_END_T1);
	SetTTimer(XML_T2, XML_END_T2);
	SetTTimer(XML_T3, XML_END_T3);
	SetTTimer(XML_T4, XML_END_T4);
	SetTTimer(XML_YKTIME, XML_END_YK);
	SetTTimer(XML_DZTIME, XML_END_DZ);

	/* StartDI退出 */
	m_bStartBit = FALSE;

	/* 清除发送标志 */
	m_wCommand = 0;

	/* 重新设置为发送身份验证 */
	m_wCommand |= CMD_IDEN_DET;

	/* 重置遥控标志 */

	/* 重置传输所有装置状态的时间 */
	time(&m_DevStateTime);

	/* 重置传输变化装置状态的时间 */
	time(&m_VaryDevStateTime);

	/* 重置实时上传遥信遥测遥脉时间 */
	time(&m_YxValueTime);
	time(&m_YcValueTime);
	time(&m_YmValueTime);
	reboot_flag=0;	
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	设置启动计时
///
/// \参数:	byTime
/// \参数:	byState
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::SetTTimer(BYTE byTime, BYTE byState)
{
	switch (byTime)
	{
	case XML_T1:
		SetXMLTime(&m_t1, byState, XML_START_T1, XML_END_T1);
		break;

	case XML_T2:
		SetXMLTime(&m_t2, byState, XML_START_T2, XML_END_T2);
		break;

	case XML_T3:
		SetXMLTime(&m_t3, byState, XML_START_T3, XML_END_T3);
		break;

	case XML_T4:
		SetXMLTime(&m_t4, byState, XML_START_T4, XML_END_T4);
		break;

	case XML_YKTIME:
		SetXMLTime(&m_YKTime, byState, XML_START_YK, XML_END_YK);
		break;

	case XML_DZTIME:
		SetXMLTime(&m_DZTime, byState, XML_START_DZ, XML_END_DZ);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	设置时间
///
/// \参数:	tTime
/// \参数:	byState
/// \参数:	byBeginT
/// \参数:	byEndT
// --------------------------------------------------------
void MonitoringPlatformXML::SetXMLTime(time_t *tTime, BYTE byState, BYTE byBeginT, BYTE byEndT)
{
	if (byState == byBeginT)
	{
		m_byTimeFlag |= byBeginT;
		time(tTime);
	}
	else if (byState == byEndT)
	{
		m_byTimeFlag &= byEndT;
	}
}

// --------------------------------------------------------
/// \概要:	获得时间标记
///
/// \返回:	BYTE
// --------------------------------------------------------
BYTE MonitoringPlatformXML::GetTimeFlag()
{
	return m_byTimeFlag;
}
#if 1
// --------------------------------------------------------
/// \概要:	获得通讯状态
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetDevCommState()
{
	if (m_bStartBit)
		return COM_DEV_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}
#endif
// --------------------------------------------------------
/// \概要:	获得通道通讯状态
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetComState(BYTE byLineNo)
{
	if (!m_pMethod)
		return COM_ABNORMAL;

	PBUSMANAGER pBus = m_pMethod->GetBus(byLineNo);
	if (!pBus)
		return COM_ABNORMAL;

	if (!pBus->m_Port)
		return COM_ABNORMAL;

	if (pBus->m_Port->IsPortValid())
		return COM_NORMAL;

	return COM_ABNORMAL;
}

// --------------------------------------------------------
/// \概要:	获得设备通讯状态
///
/// \参数:	byLineNo
/// \参数:	byAddr
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetDevCommState(BYTE byLineNo, BYTE byAddr)
{
	if (!m_pMethod)
		return COM_DEV_ABNORMAL;

	PBUSMANAGER pBus = m_pMethod->GetBus(byLineNo);
	if (!pBus)
		return COM_DEV_ABNORMAL;

	CProtocol *pProtocol = pBus->m_Protocol;
	if (!pProtocol)
		return COM_DEV_ABNORMAL;

	/* 判断双机冗余机制 */
	BOOL bStatus = COM_DEV_ABNORMAL;

	bStatus = m_pMethod->GetDevCommState(byLineNo, byAddr);

	return bStatus;
}

// --------------------------------------------------------
/// \概要:	获取真实值
///
/// \参数:	byType
/// \参数:	wPnt
/// \参数:	Value
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::GetRealVal(BYTE byType, WORD wPnt, void *Value)
{
	WORD wValue = 0;
	switch (byType)
	{
	case 0:
		if (wPnt >= XMLMAX_AI_LEN)
			return -2;
		memcpy(Value, &m_wAIBuf[wPnt], sizeof(float));
		break;
	case 1:
	{
		if (wPnt >= XMLMAX_DI_LEN)
			return -2;

		if (m_byDIBuf[wPnt] == 0)
			wValue = 0;
		else
			wValue = 1;
		memcpy(Value, &wValue, sizeof(WORD));
	}
	break;
	case 2:
	{
		if (wPnt >= XMLMAX_PI_LEN)
			return -2;
		memcpy(Value, &m_dwPIBuf[wPnt], sizeof(QWORD));
	}
	break;
	case 3:
	{
		if (wPnt >= XMLMAX_PI_LEN)
			return -2;
		memcpy(Value, &m_byDIBuf[wPnt], sizeof(BYTE));
	}
	break;
	default:
		return -1;
	}

	return 0;
}

// --------------------------------------------------------
/// \概要:	保存读出的定值数据
///
/// \参数:	dwDataType
/// \参数:	iDataNum
/// \参数:	pData
// --------------------------------------------------------
void MonitoringPlatformXML::SaveDzData(DWORD dwDataType, int iDataNum, DZ_DATA *pData)
{
	m_iDzDataNum = iDataNum;
	m_dwDataType = dwDataType;

	DZ_DATA *pDzData = new DZ_DATA[iDataNum + 1];
	memcpy(pDzData, pData, sizeof(DZ_DATA) * (iDataNum + 1));
	m_pDzData = pDzData;
	pDzData = NULL;
	// printf("----FUNC = %s LINE = %d----\n", __func__, __LINE__);
}

// --------------------------------------------------------
/// \概要:	处理缓冲队列信息
///
/// \参数:	pBusMsg
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::DealBusMsgInfo(PBUSMSG pBusMsg)
{
	int temp = 0;
	int iTempBufLen = 0;
	YK_DATA *pYkData = (YK_DATA *)(pBusMsg->pData);
	DZ_DATA *pDzData = (DZ_DATA *)(pBusMsg->pData);
	switch (pBusMsg->byMsgType)
	{
	case YK_PROTO:
		switch (pBusMsg->dwDataType)
		{
		case YK_SEL_RTN:
		case YK_EXCT_RTN:
		case YK_CANCEL_RTN:
			if ((pBusMsg->DataNum != 1) || (pBusMsg->DataLen != sizeof(YK_DATA)))
			{
				//	printf("----FUNC= %s LINE = %d Xml YK DataNum err----\n", __func__, __LINE__);
				return -1;
			}
			temp = m_pMethod->GetSerialNo(pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo);
			if (temp == -1)
				return -1;

			if (pYkData->byVal == YK_ERROR)
			{
				//	printf("----FUNC= %s LINE = %d YkbyVal is Err----\n", __func__, __LINE__);
				SetCommand(CMD_YK_ERROR);
			}
			else
			{
				//	printf("----FUNC= %s LINE = %d YkbyVal is Suc----\n", __func__, __LINE__);
				RelayProc((BYTE)pBusMsg->dwDataType, (WORD)temp, pYkData->wPnt, (BYTE)pYkData->byVal);
			}
			break;
		default:
			// printf("----FUNC= %s LINE = %d Xml can't find the YK_DATATYPE = %d----\n", __func__, __LINE__, (int)pBusMsg->dwDataType);
			break;
		}
		break;
	case DZ_PROTO:
		// printf("----FUNC= %s LINE = %d DZ!!----\n", __func__, __LINE__);
		switch (pBusMsg->dwDataType)
		{
		case DZ_CALL_RTN:
		case DZ_WRITE_EXCT_RTN:
			if (pBusMsg->DataLen != (int)sizeof(DZ_DATA) * pBusMsg->DataNum)
				return -1;
			temp = m_pMethod->GetSerialNo(pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo);
			if (temp == -1)
				return -1;

			if (pBusMsg->dwDataType == DZ_ERROR)
			{
				printf("----FUNC= %s LINE = %d DzbyVal is Err----\n", __func__, __LINE__);
				// SetCommand(CMD_DZ_ERROR);
				DzRelayEchoFrame(m_xml, iTempBufLen, (char *)"fail");
			}
			else
			{
				printf("----FUNC= %s LINE = %d DzbyVal is Suc----\n", __func__, __LINE__);
				SaveDzData(pBusMsg->dwDataType, pBusMsg->DataNum - 1, (DZ_DATA *)pBusMsg->pData);

				RelayDzProc((BYTE)pBusMsg->dwDataType, (WORD)temp, pDzData->wPnt, (BYTE *)pDzData->byVal);
			}

			break;
		default:
			printf("----FUNC= %s LINE = %d Xml can't find the DZ_DATATYPE = %d----\n", __func__, __LINE__, (int)pBusMsg->dwDataType);
			break;
		}
		break;
	case UNVARNISH_PROTO:
	{
		switch (pBusMsg->dwDataType)
		{
		case VARNISH_RTN:
		{
			char *pData = (char *)pBusMsg->pData;
			int dataLen = pBusMsg->DataLen - 10;
			bySource_BusNo = pBusMsg->SrcInfo.byBusNo;
			wSource_DevNo = pBusMsg->SrcInfo.wDevNo;

			m_wMsgDevID = GetDevidFromBusnoAndDevNo(bySource_BusNo, wSource_DevNo);
			// printf("xml varnish_rtn \n");

			memcpy(m_quest_id_forUnvarnish, &(((BYTE *)pData)[dataLen]), 10);

			char pTemp[100] = {0};

			memcpy(pTemp, pData, dataLen);

			if ((strcmp(pTemp, "fail") == 0) || (strcmp(pTemp, "succeed") == 0))
			{
				m_wCommand = CMD_UNVARNISH;
				strcpy(m_UnvarnishRtnBuf, pTemp);
			}
		}
		break;
		default:
			break;
		}
	}
	default:
		break;
	}

	return 1;
}

// --------------------------------------------------------
/// \概要:	遥控处理报文
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::YkPacket(XML TempXml, int &XmlBufLen)
{
	if (m_wYkFlag == YK_SEL_RTN)
	{
		YkRelayEchoFrame(TempXml, XmlBufLen, (char *)"succeed");
		Yk_RtnConfirm(TRUE);
	}
	else if (m_wYkFlag == YK_EXCT_RTN)
	{
		YkRelayEchoFrame(TempXml, XmlBufLen, (char *)"succeed");
		Yk_RtnConfirm(FALSE);
	}
	else if (m_wYkFlag == YK_CANCEL_RTN)
	{
		YkRelayEchoFrame(TempXml, XmlBufLen, (char *)"succeed");
		Yk_RtnConfirm(FALSE);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	遥控回复报文
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
/// \参数:	szBuf
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::YkRelayEchoFrame(XML TempXml, int &XmlBufLen, char *szBuf)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);

	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YK");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "result");

	xmlNodePtr xResult_Node;
	xResult_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "result", BAD_CAST szBuf);
	xmlNewProp(xResult_Node, BAD_CAST "errorcode", BAD_CAST "0");

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	定值处理报文
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::DzPacket(XML TempXml, int &XmlBufLen, PBUSMSG pBusMsg)
{
	if (m_wDzFlag == DZ_CALL_RTN)
	{
		//	DzRelayEchoFrame(TempXml, XmlBufLen, (char *)"succeed");
		DzReadRelayEchoFrame_new(TempXml, XmlBufLen, pBusMsg);
		SetTTimer(XML_DZTIME, XML_END_DZ);
	}
	else if (m_wDzFlag == DZ_WRITE_EXCT_RTN)
	{
		DzRelayEchoFrame(TempXml, XmlBufLen, (char *)"succeed");
		SetTTimer(XML_DZTIME, XML_END_DZ);
	}
	return TRUE;
}
BOOL MonitoringPlatformXML::DzReadRelayEchoFrame_new(XML TempXml, int &XmlBufLen, PBUSMSG pBusMsg)
{
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	xmlDocSetRootElement(xDoc, xRoot_Node);
	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "DZ");
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	xmlNodePtr xValue_Node;
	BYTE bDzVal = 0;
	short int sDzVal = 0;
	int iDzVal = 0;
	float fDzVal;
	WORD DEV_ID = GetDevidFromBusnoAndDevNo(pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo);
	DZ_DATA *pDzData = new DZ_DATA[pBusMsg->DataNum];
	memcpy(pDzData, (DZ_DATA *)(pBusMsg->pData), sizeof(DZ_DATA) * (pBusMsg->DataNum));
	// printf("--------pBusMsg->DataNum=%d-----\n", pBusMsg->DataNum-1);
	for (int i = 0; i < pBusMsg->DataNum - 1; i++)
	{
		pDzData++;
		memset(cTemp, 0, sizeof(cTemp));
		switch (pDzData->byType)
		{
		case 0:
			memcpy(&bDzVal, pDzData->byVal, 1);
			sprintf(cTemp, "%c", bDzVal);
			break;
		case 1:
			memcpy(&sDzVal, pDzData->byVal, 2);
			sprintf(cTemp, "%d", sDzVal);
			break;
		case 2:
			memcpy(&iDzVal, pDzData->byVal, 4);
			sprintf(cTemp, "%d", iDzVal);
			break;
		case 3:
			memcpy(&fDzVal, pDzData->byVal, 4);
			sprintf(cTemp, "%f", fDzVal);
			break;
		}
		m_byDzType[i] = pDzData->byType;
		xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST cTemp);
		// printf("-----------value=%s-----\n",cTemp);
		memset(cTemp, 0, sizeof(cTemp));
		sprintf(cTemp, "%d_%d", DEV_ID, pDzData->wPnt);
		xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
	}
	// printf("----------------byBusNo=%d     wDevNo=%d---------\n", pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo);
	// printf("-------------------------DEV_ID=%d-----------\n",DEV_ID);
	pDzData = NULL;
	delete[] m_pDzData;
	delete[] pDzData;
	xmlChar *xmlBuf;
	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	读定值回复报文
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::DzReadRelayEchoFrame(XML TempXml, int &XmlBufLen)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);

	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "DZ");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");

	xmlNodePtr xValue_Node;
	BYTE bDzVal = 0;
	short int sDzVal = 0;
	int iDzVal = 0;
	float fDzVal;
	DZ_DATA *pDzData = m_pDzData + 1;
	for (int i = 0; i < m_iDzDataNum; i++)
	{
		memset(cTemp, 0, sizeof(cTemp));
		switch (pDzData->byType)
		{
		case 0:
			memcpy(&bDzVal, pDzData->byVal, 1);
			sprintf(cTemp, "%c", bDzVal);
			break;
		case 1:
			memcpy(&sDzVal, pDzData->byVal, 2);
			sprintf(cTemp, "%d", sDzVal);
			break;
		case 2:
			memcpy(&iDzVal, pDzData->byVal, 4);
			sprintf(cTemp, "%d", iDzVal);
			break;
		case 3:
			memcpy(&fDzVal, pDzData->byVal, 4);
			sprintf(cTemp, "%f", fDzVal);
			break;
		}
		m_byDzType[i] = pDzData->byType;
		xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST cTemp);
		memset(cTemp, 0, sizeof(cTemp));
		// sprintf(cTemp, "%s_%d", m_pMethod->m_pRtuObj->m_sDevName, pDzData->wPnt);
		sprintf(cTemp, "%d_%d", m_wDZRecvAddr, pDzData->wPnt);
		xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);

		pDzData++;
	}
	pDzData = NULL;
	delete[] m_pDzData;

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	定值回复报文
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
/// \参数:	szBuf
///
/// \返回:	BOOL
// --------------------------------------------------------

BOOL MonitoringPlatformXML::DzRelayEchoFrame(XML TempXml, int &XmlBufLen, char *szBuf)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);

	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "DZ");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "result");

	xmlNodePtr xResult_Node;
	xResult_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "result", BAD_CAST szBuf);
	xmlNewProp(xResult_Node, BAD_CAST "errorcode", BAD_CAST "0");

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	获得命令状态
///
/// \返回:	WORD
// --------------------------------------------------------
WORD MonitoringPlatformXML::GetCommand()
{
	return m_wExcmd;
}

// --------------------------------------------------------
/// \概要:	设置命令状态
///
/// \参数:	wCommand
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::SetCommand(WORD wCommand)
{
	m_wExcmd = wCommand;
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	系统时间确认数据帧
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::SysClockConfirmPacket(XML TempXml, int &XmlBufLen)
{
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	总召唤确认帧
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::AllDataEchoPacket(XML TempXml, int &XmlBufLen, char *szTemp)
{
	//	printf("---FUNC = %s LINE = %d ----\n", __func__, __LINE__);
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	// printf("---FUNC = %s LINE = %d bSequence = %s----\n", __func__, __LINE__, TempXml.uId_Validate.bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST szTemp);

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	总召遥信数据（分包）
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadDigitalDataPacket(XML TempXml, int &XmlBufLen, int nSize, int *dataindex1)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	if (m_iDataFlag == 0)
		GetRandom();
	else
		m_iDataFlag = 0;
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	// printf("---FUNC = %s LINE = %d bSequence = %s----\n", __func__, __LINE__, TempXml.uId_Validate.bSequence);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);

	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YX");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");

	BYTE wVal = 0, wSerialNo = 0;
	DWORD wDevId = 0, wPnt;
	BOOL bDevState = FALSE;
	xmlNodePtr xValue_Node;
	int byCount = 0;
	BOOL bDevStateFlag = 0;

	for (int i = m_wDataIndex; i < nSize;)
	{
		// if (i >= GetPntSum(YX_SUM))
		// break;
		if (*dataindex1 >= GetPntSum(YX_SUM))
			break;
		if (m_pwDITrans[i] == 0xFFFF)
		{
			byCount++;
			*dataindex1++;
			i++;
			continue;
		}
		try
		{
			// wSerialNo =  GetSerialNoFromTrans(YX_TRANSTOSERIALNO, m_pwDITrans[i]);
			// wDevId =  GetDevIdFromTrans(YX_TRANSTOSERIALNO, m_pwDITrans[i]);
			// wPnt =  GetDevPntFromTrans(YX_TRANSTOSERIALNO, m_pwDITrans[i]);
			wSerialNo = GetSerialNoFromTrans(YX_TRANSTOSERIALNO, *dataindex1);
			wDevId = GetDevIdFromTrans(YX_TRANSTOSERIALNO, *dataindex1);
			wPnt = GetDevPntFromTrans(YX_TRANSTOSERIALNO, *dataindex1);

			bDevState = m_pMethod->GetDevCommState(wSerialNo);
			if (bDevState == COM_DEV_ABNORMAL)
				bDevStateFlag = 1;
			else
				bDevStateFlag = 0;

			// BYTE bySiq =  m_byDIBuf[wPnt];
			WORD bySiq = m_byDIBuf[*dataindex1];
			if (bySiq > 255)
				bySiq = 0;

			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", bySiq);

			BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			bflag = TRUE;

			if (bDevStateFlag == 0 && wDevId != 0 && bflag)
			{
				xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST cTemp);
				memset(cTemp, 0, sizeof(cTemp));
				sprintf(cTemp, "%d_%d", wDevId, wPnt);
				xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
				m_iDataFlag += 40;
			}
			byCount++;
			(*dataindex1)++;
			i++;
			if (m_iDataFlag > 1024000)
				break;
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
		}
	}

	xmlChar *xmlBuf = nullptr;
	XmlBufLen = 0;
	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	// 修改printf格式，添加长度控制
	printf("%.*s\n", XmlBufLen, (char *)m_XmlBuf);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	m_wDataIndex += byCount;

	if ((m_wDataIndex >= GetPntSum(YX_SUM)))
	{
		m_iDataFlag = 0;
	}
	return TRUE;
}
// --------------------------------------------------------
/// \概要:	总召遥信数据（未分包处理）
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadDigitalDataPacket(XML TempXml, int &XmlBufLen)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	if (m_iDataFlag == 0)
		GetRandom();
	else
		m_iDataFlag = 0;
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	// printf("---FUNC = %s LINE = %d bSequence = %s----\n", __func__, __LINE__, TempXml.uId_Validate.bSequence);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YX");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//	xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST "1");
	BYTE wVal = 0, wSerialNo = 0;
	DWORD wDevId = 0, wPnt;
	BOOL bDevState = FALSE;
	xmlNodePtr xValue_Node;
	int byCount = 0;
	BOOL bDevStateFlag = 0;

	int nSize = GetPntSum(YX_SUM);
	int dataindex = 0;

	for (int i = m_wDataIndex; i < nSize;)
	{
		if (m_pwDITrans[i] == 0xFFFF)
		{
			byCount++;
			i++;
			continue;
		}

		try
		{
			GetRealVal(3, (WORD)dataindex, &wVal);
			BYTE bySiq = wVal;
			if (bySiq > 255)
				bySiq = 0;

			wSerialNo = GetSerialNoFromTrans(YX_TRANSTOSERIALNO, dataindex);
			wDevId = GetDevIdFromTrans(YX_TRANSTOSERIALNO, dataindex);
			wPnt = GetDevPntFromTrans(YX_TRANSTOSERIALNO, dataindex);
			bDevState = m_pMethod->GetDevCommState(wSerialNo);
			if (bDevState == COM_DEV_ABNORMAL)
				bDevStateFlag = 1;
			else
				bDevStateFlag = 0;

			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", bySiq);

			BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			bflag = TRUE;

			// printf("i = %d val = %d wDevID = %d m_pwDITrans[%d] = %d \n",i, bySiq, wDevId , i , m_pwDITrans[i]);

			if (bDevStateFlag == 0 && wDevId != 0 && bflag)
			{
				xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST cTemp);
				memset(cTemp, 0, sizeof(cTemp));
				sprintf(cTemp, "%d_%d", wDevId, wPnt);
				xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
				m_iDataFlag += 40;
			}
			byCount++;
			dataindex++;
			i++;
			if (m_iDataFlag > 1024000)
				break;
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}
	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);
	// 修改printf格式，添加长度控制
	printf("%.*s\n", XmlBufLen, (char *)m_XmlBuf);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	m_wDataIndex += byCount;
	// printf("---FUNC = %s LINE = %d m_wDISum = %d m_wDataIndex = %d----\n", __func__, __LINE__, m_wDISum, m_wDataIndex);
	if (m_wDataIndex >= nSize)
	{
		m_iDataFlag = 0;
	}
	return TRUE;
}

// bflag 为true 即能发送数据 false不能发送数据
BOOL MonitoringPlatformXML::IsCanSendDataFromSerialNo(WORD wSerialNo)
{
	CProtocol *pProtocol = ((CPublicMethod *)m_pMethod)->GetProtocolMoudle(wSerialNo);
	if (NULL == pProtocol)
	{
		// printf("line=%d protocol is null wserialNo = %d\n", __LINE__, wSerialNo );
		return FALSE;
	}

	char *sDevName = pProtocol->m_sDevName;
	BOOL bflag = IsDevNameFillwithDight(sDevName);
	return bflag;
}

// --------------------------------------------------------
/// \概要:	总召遥测数据
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadAnalogDataPacket(XML TempXml, int &XmlBufLen)
{
	// printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);

	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	if (m_iDataFlag == 0)
		GetRandom();
	else
		m_iDataFlag = 0;
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YC");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//	WORD wSerialNo = 0;
	DWORD wDevId = 0, wPnt = 0;
	xmlNodePtr xValue_Node;
	int byCount = 0;
	float fVal;
	char byBuffer[8] = {'\0'};
	BOOL bDevState = FALSE;

	int nSize = GetPntSum(YC_SUM);
	//	printf("---FUNC = %s LINE = %d nSize = %d m_wDataIndex = %d----\n", __func__, __LINE__, nSize, m_wDataIndex);
	for (int i = m_wDataIndex; i < nSize; i++)
	{
		try
		{
			memset(byBuffer, 0, sizeof(byBuffer));
			fVal = m_wAIBuf[i];
			// printf("---FUNC = %s LINE = %d fVal = %f ----\n", __func__, __LINE__, fVal);
			if (m_pAIMapTab[i].wStn > 0 && m_pAIMapTab[i].wPntNum > 0)
				fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_wAIBuf[i]);
			//	printf("---FUNC = %s LINE = %d fVal = %f ----\n", __func__, __LINE__, fVal);
			sprintf(byBuffer, "%.2f", fVal);
			//	printf("---FUNC = %s LINE = %d byBuffer = %s ----\n", __func__, __LINE__, byBuffer);
			//	GlobalCopyByEndian(byBuffer1, byBuffer, 4);

			//	wSerialNo = GetSerialNoFromTrans(YC_TRANSTOSERIALNO, i);
			wDevId = GetDevIdFromTrans(YC_TRANSTOSERIALNO, i);
			wPnt = GetDevPntFromTrans(YC_TRANSTOSERIALNO, i);
			const int invalid_transnumber = 0;
			WORD wSerialNo = GetSerialNoFromTrans(YC_TRANSTOSERIALNO, i);

			bDevState = m_pMethod->GetDevCommState(wSerialNo);
			if (bDevState == COM_DEV_ABNORMAL)
				continue;

			// BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			// bflag = TRUE;
			// if (wDevId == invalid_transnumber || !bflag )
			if (wDevId == invalid_transnumber)
			{
				printf("---FUNC = %s LINE = %d wDevId = %d i = %d size = %d----data don't send to masterMachine\n", __func__, __LINE__, wDevId, i, nSize);
				continue;
			}

			xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST byBuffer);
			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d_%d", wDevId, wPnt);
			xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
			byCount++;
			m_iDataFlag += 40;
			if (m_iDataFlag > 1024000)
				break;
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}

	xmlChar *xmlBuf;
	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);
	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	m_wDataIndex += byCount;
	if (m_wDataIndex >= nSize)
	{
		m_iDataFlag = 0;
	}

	return TRUE;
}

BOOL MonitoringPlatformXML::LoadAnalogDataPacket_Batch(XML TempXml, int &XmlBufLen, int nSize)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	if (m_iDataFlag == 0)
		GetRandom();
	else
		m_iDataFlag = 0;
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);

	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "LXYC");

	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	Pack_BatchData(YC_SUM, YC_TRANSTOSERIALNO, xData_Node, nSize);
	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));

	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

void MonitoringPlatformXML::Pack_BatchData(BYTE dataType, BYTE dataTransType, xmlNodePtr xData_Node, int nSize)
{
	DWORD wDevId = 0, wSum = 0;
	xmlNodePtr xValue_Node;
	int byCount = 0;
	float fVal;
	QWORD dwVal = 0.0;
	char sBatchBuf[8192];
	char byBuffer[50] = {'\0'};

	map<int, vector<pntval> *> datatable;
	vector<pntval> dataVec;
	dataVec.reserve(20000);
	WORD wStartNo = 0xFFFF;
	DWORD wDeviceID = 0xFFFF;
	// m_wDataIndex = 0;
	int index = 0;
	int dataindex = 0;
	int trans = 0;
	BOOL bDevState = FALSE;

	for (int i = m_wDataIndex; i < nSize; i++)
	{
		m_wDataIndex++;
		if (m_wDataIndex > GetPntSum(dataType))
			break;
		try
		{
			memset(byBuffer, 0, sizeof(byBuffer));
			if (dataType == YC_SUM)
			{
				if (m_pwAITrans[i] == 0xFFFF)
					continue;
				trans = m_pwAITrans[i];
				fVal = m_wAIBuf[trans];
				if (m_pAIMapTab[dataindex].wStn > 0 && m_pAIMapTab[dataindex].wPntNum > 0)
					fVal = CalcAIRipeVal(m_pAIMapTab[dataindex].wStn, m_pAIMapTab[dataindex].wPntNum, m_wAIBuf[trans]);
				sprintf(byBuffer, "%.3f", fVal);
			}
			else if (dataType == YM_SUM)
			{
				if (m_pwPITrans[i] == 0xFFFF)
					continue;
				trans = m_pwPITrans[i];
				dwVal = m_dwPIBuf[trans];
				sprintf(byBuffer, "%llu", dwVal);
			}
			wDevId = GetDevIdFromTrans(dataTransType, trans);
			const WORD invalid_transnumber = 0;
			WORD wSerialNo = GetSerialNoFromTrans(dataTransType, trans);
			WORD wPnt = GetDevPntFromTrans(dataTransType, trans);
			index++;
			dataindex++;
			BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			bflag = TRUE;
			bDevState = m_pMethod->GetDevCommState(wSerialNo);

			if (bDevState == COM_DEV_ABNORMAL)
				continue;

			if (wSerialNo == 0xFFFF || wDevId == 0)
				continue;

			// printf("serialno = %d wdevID = %d , wpnt = %d\n", wSerialNo, wDevId, wPnt);
			if (0 == datatable.size())
			{
				wStartNo = wPnt;
				wDeviceID = wDevId;
				datatable[wDeviceID] = &dataVec;
			}
			if (datatable.end() == datatable.find(wDevId))
			{
				CalcRes(dataType, xData_Node, &datatable, wStartNo);
				wStartNo = wPnt;
				wDeviceID = wDevId;
				datatable[wDeviceID] = &dataVec;
				pntval pv;
				pv.wPnt = wPnt;
				if (dataType == YC_SUM)
				{
					pv.fVal = fVal;
				}
				else
					pv.dwVal = dwVal;

				dataVec.push_back(pv);
			}
			else if (dataVec.size() == 0)
			{
				pntval pv;
				pv.wPnt = wPnt;
				if (dataType == YC_SUM)
					pv.fVal = fVal;
				else
					pv.dwVal = dwVal;
				dataVec.push_back(pv);
			}
			else
			{
				pntval pv = dataVec.back();
				if (pv.wPnt + 1 == wPnt)
				{
					pntval pv;
					pv.wPnt = wPnt;
					if (dataType == YC_SUM)
						pv.fVal = fVal;
					else
						pv.dwVal = dwVal;
					dataVec.push_back(pv);
				}
				else
				{
					CalcRes(dataType, xData_Node, &datatable, wStartNo);
					wStartNo = wPnt;
					wDeviceID = wDevId;
					datatable[wDeviceID] = &dataVec;
					pntval pv;
					pv.wPnt = wPnt;
					if (dataType == YC_SUM)
						pv.fVal = fVal;
					else
						pv.dwVal = dwVal;
					dataVec.push_back(pv);
				}
			}
			if (i % 500 == 0)
				break;
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}

	if (datatable.size() != 0)
		fenbaoflag = 1;
	else
		fenbaoflag = 0;
	// 最后一个装置写到xml中
	CalcRes(dataType, xData_Node, &datatable, wStartNo);
}

void MonitoringPlatformXML::CalcRes(BYTE dataType, xmlNodePtr xData_Node, void *datatable, WORD wStartNo)
{
	map<int, vector<pntval> *> *pdataTable = (map<int, vector<pntval> *> *)datatable;
	map<int, vector<pntval> *>::iterator itor;
	vector<pntval> *pPntVal = NULL;
	DWORD wDevID = 0xFFFF;
	char sBatchBuf[8192 * 10] = {0};

	for (itor = pdataTable->begin(); itor != pdataTable->end(); itor++)
	{
		wDevID = itor->first;
		pPntVal = itor->second;
	}

	int size = 0;
	if (pPntVal)
		size = pPntVal->size();

	try
	{
		for (int i = 0; i < size; i++)
		{
			pntval pv = pPntVal->at(i);
			char buf[20] = {0};
			if (i != size - 1)
			{
				if (dataType == YC_SUM)
					sprintf(buf, "%.3f_", pv.fVal);
				else
					sprintf(buf, "%llu_", pv.dwVal);
			}
			else
			{
				if (dataType == YC_SUM)
					sprintf(buf, "%.3f", pv.fVal);
				else
					sprintf(buf, "%llu", pv.dwVal);
			}
			strcat(sBatchBuf, buf);
		}

		if (wDevID != 0xFFFF)
		{
			/* 创建一个节点，设置其内容和属性，然后加入根节点 */
			xmlNodePtr xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST sBatchBuf);
			memset(sBatchBuf, 0, sizeof(sBatchBuf));

			char cTemp[10] = {0};
			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", wDevID);
			xmlNewProp(xValue_Node, BAD_CAST "devID", BAD_CAST cTemp);
			char szStartBuf[10] = {0};
			sprintf(szStartBuf, "%d", wStartNo);

			xmlNewProp(xValue_Node, BAD_CAST "start", BAD_CAST szStartBuf);
			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", size);
			xmlNewProp(xValue_Node, BAD_CAST "no", BAD_CAST cTemp);
		}

		if (pdataTable != NULL)
			pdataTable->clear();

		if (pPntVal != NULL)
			pPntVal->clear();
	}
	catch (...)
	{
		char szLog[200] = {0};
		sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
		m_log.writeLog(szLog);
	}
}

// --------------------------------------------------------
/// \概要:	总召遥脉数据
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadPulseDataPacket(XML TempXml, int &XmlBufLen)
{
	//	printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[50] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	if (m_iDataFlag == 0)
		GetRandom();
	else
		m_iDataFlag = 0;
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YM");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//	xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST "1");
	//	WORD wSerialNo = 0;
	DWORD wDevId = 0, wPnt = 0;
	QWORD dwVal = 0;
	xmlNodePtr xValue_Node;
	int byCount = 0;
	BOOL bDevState = FALSE;

	int nSize = GetPntSum(YM_SUM);
	for (int i = m_wDataIndex; i < nSize; i++)
	{
		dwVal = m_dwPIBuf[i];
		wDevId = GetDevIdFromTrans(DD_TRANSTOSERIALNO, i);
		wPnt = GetDevPntFromTrans(DD_TRANSTOSERIALNO, i);
		WORD wSerialNo = GetSerialNoFromTrans(DD_TRANSTOSERIALNO, i);
		BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
		bDevState = m_pMethod->GetDevCommState(wSerialNo);
		if (bDevState == COM_DEV_ABNORMAL)
			continue;

		const int invalid_transnumber = 0;
		if (wDevId == invalid_transnumber)
		{
			continue;
		}
		memset(cTemp, 0, sizeof(cTemp));
		sprintf(cTemp, "%llu", dwVal);
		xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST cTemp);
		memset(cTemp, 0, sizeof(cTemp));
		sprintf(cTemp, "%d_%d", wDevId, wPnt);
		xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);

		byCount++;
		m_iDataFlag += 40;
		if (m_iDataFlag > 1024000)
			break;
	}

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	m_wDataIndex += byCount;
	if (m_wDataIndex >= nSize)
	{
		m_iDataFlag = 0;
	}

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	总召遥脉数据批量上传
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadPulseDataPacket_Batch(XML TempXml, int &XmlBufLen, int nSize)
{
	// printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);

	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	if (m_iDataFlag == 0)
		GetRandom();
	else
		m_iDataFlag = 0;
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);

	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "LXYM");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);
	Pack_BatchData(YM_SUM, DD_TRANSTOSERIALNO, xData_Node, nSize);

	xmlChar *xmlBuf;
	m_wPreDevId = 0xFFFF;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));

	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	// 添加XML内容打印
	//printf("Generated XML:\n%.*s\n", XmlBufLen, (char *)xmlBuf);
	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	// printf( m_XmlBuf ) ;
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	实时上传变位遥信数据
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadDIEFramePacket(XML TempXml, int &XmlBufLen)
{
	// printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();

	memset(m_bDITimeOutSequence_YX, 0, sizeof(m_bDITimeOutSequence_YX));
	//	memset(temp_quistid, '\0', sizeof(temp_quistid));
	//	printf("----FUNC = %s LINE = %d m_bSequence = %s m_bDITimeOutSequence_YX = %s size = %d----\n", __func__, __LINE__, m_bSequence, m_bDITimeOutSequence_YX, sizeof(m_bSequence));
	// memcpy((char *)m_bDITimeOutSequence_YX, (const char *)m_bSequence, sizeof(m_bSequence));
	strncpy((char *)m_bDITimeOutSequence_YX, (const char *)m_bSequence, sizeof(m_bSequence));

	// printf("----FUNC = %s LINE = %d  m_bDITimeOutSequence_YX =--%s--- size = %d----\n", __func__, __LINE__, m_bDITimeOutSequence_YX, sizeof(m_bSequence));
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);

	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bDITimeOutSequence_YX);

	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YX");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//	xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST "1");
	WORD wVal = 0, wSerialNo = 0, wPnt = 0, wNum = 0;
	BOOL bDevState = FALSE;
	xmlNodePtr xValue_Node;
	BOOL bDevStateFlag = 0;

	int nSize = m_dwDIEQueue_Xml.size();
	// printf("m_dwDIEQueue_Xml.size=%d\n", nSize);
	for (int i = 0; i < nSize; i++)
	{
		try
		{
			if (!GetDigitalEvt_Xml(wSerialNo, wPnt, wNum, wVal))
				break;

			// BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			// if( !bflag )
			//	continue;

			bDevState = m_pMethod->GetDevCommState(wSerialNo);
			if (bDevState == COM_DEV_ABNORMAL)
				bDevStateFlag = 1;
			else
				bDevStateFlag = 0;

			if (bDevStateFlag == 1)
				continue;

			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", wVal);
			if (bDevStateFlag == 1)
				xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST "-1");
			else
				xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST cTemp);
			memset(cTemp, 0, sizeof(cTemp));
            auto it = mapDevName.find(wSerialNo);
			if (it == mapDevName.end() || it->second == NULL || strcmp(it->second, "") == 0)					
				sprintf(cTemp, "%s_%d", "-1", wPnt);
			else
				sprintf(cTemp, "%s_%d", mapDevName[wSerialNo], wPnt);


			xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);
	memcpy(m_XmlDITimeOutBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	实时上传SOE信息
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadSOEFramePacket(XML TempXml, int &XmlBufLen)
{
	//	printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "SOE");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//	xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST "1");
	WORD wVal = 0, wPreVal = 0, wSerialNo = 0, wPnt = 0, wMiSecond = 0, wNum = 0;
	struct tm tmStruct;
	xmlNodePtr xValue_Node;
	while (m_iSOE_rd_p != m_iSOE_wr_p)
	{
		try
		{

			if (!GetSOEInfo_Xml(wSerialNo, &wPnt, &wNum, &wVal, &tmStruct, &wMiSecond))
				break;

			// BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			// if( !bflag )
			//	continue;

			wVal = wVal & 0x0001;
			if (wVal)
				wPreVal = 0;
			else
				wPreVal = 1;

			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%04d-%02d-%02d*%02d:%02d:%02d.%03d", tmStruct.tm_year + 1900, tmStruct.tm_mon, tmStruct.tm_mday, tmStruct.tm_hour, tmStruct.tm_min, tmStruct.tm_sec, wMiSecond);
			xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "time", BAD_CAST cTemp);
			memset(cTemp, 0, sizeof(cTemp));
            auto it = mapDevName.find(wSerialNo);
			if (it == mapDevName.end() || it->second == NULL || strcmp(it->second, "") == 0)
				sprintf(cTemp, "%s_%d", "-1", wPnt);
			else
				sprintf(cTemp, "%s_%d", mapDevName[wSerialNo], wPnt);

			xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", wPreVal);
			xmlNewProp(xValue_Node, BAD_CAST "pre_value", BAD_CAST cTemp);
			memset(cTemp, 0, sizeof(cTemp));
			sprintf(cTemp, "%d", wVal);
			xmlNewProp(xValue_Node, BAD_CAST "current_value", BAD_CAST cTemp);
			xmlNewProp(xValue_Node, BAD_CAST "extra_info", BAD_CAST "-123456");
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	实时上传变化遥测值
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadAIEFramePacket(XML TempXml, int &XmlBufLen)
{
	//	printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "YC");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	//	xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST "1");
	WORD wSerialNo = 0, wNum = 0, wPnt = 0;
	xmlNodePtr xValue_Node;
	float fVal, fGetVal = 0.0f;
	char byBuffer[8] = {'\0'};
	BOOL bDevState = FALSE;

	int nSize = m_dwAIEQueue_Xml.size();
	//	printf("---FUNC = %s LINE = %d nSize = %d----\n", __func__, __LINE__ , nSize );
	for (int i = 0; i < nSize; i++)
	{
		try
		{
			memset(byBuffer, 0, sizeof(byBuffer));
			if (!GetAnalogEvt_Xml(wSerialNo, wPnt, wNum, fGetVal))
				break;
			// BOOL bflag = IsCanSendDataFromSerialNo(wSerialNo);
			// if( !bflag )
			//	continue;

			// printf("---FUNC = %s LINE = %d  serialno = %d wpnt = %d wNum = %d val = %f ----\n", __func__, __LINE__ , wSerialNo , wPnt , wNum , fGetVal );

			bDevState = m_pMethod->GetDevCommState(wSerialNo);
			if (bDevState == COM_DEV_ABNORMAL)
				continue;

			fVal = fGetVal;
			// printf("---FUNC = %s LINE = %d fVal = %f ----\n", __func__, __LINE__, fVal);

			if (m_pAIMapTab[wNum].wStn > 0 && m_pAIMapTab[wNum].wPntNum > 0)
				fVal = CalcAIRipeVal(m_pAIMapTab[wNum].wStn, m_pAIMapTab[wNum].wPntNum, fVal);

			// printf("---FUNC = %s LINE = %d fVal = %f ----\n", __func__, __LINE__, fVal);
			sprintf(byBuffer, "%.3f", fVal);
			// printf("---FUNC = %s LINE = %d byBuffer = %s ----\n", __func__, __LINE__, byBuffer);

			xValue_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "value", BAD_CAST byBuffer);
			memset(cTemp, 0, sizeof(cTemp));
			auto it = mapDevName.find(wSerialNo);
			if (it == mapDevName.end() || it->second == NULL || strcmp(it->second, "") == 0)
				sprintf(cTemp, "%s_%d", "-1", wPnt);
			else
				sprintf(cTemp, "%s_%d", mapDevName[wSerialNo], wPnt);

			xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}

	try
	{
		xmlChar *xmlBuf;

		xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
		memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
		memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

		/* 释放文件内节点动态申请的内存 */
		xmlFree(xmlBuf);
		xmlFreeDoc(xDoc);
	}
	catch (...)
	{
		char szLog[200] = {0};
		sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
		m_log.writeLog(szLog);
	}
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	总招通讯状态
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadCommStatusPacket(XML TempXml, int &XmlBufLen)
{

	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[50] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	if (m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "comm_status");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");

	xmlNodePtr xCom_Node = xmlNewNode(NULL, BAD_CAST "com");
	xContent = xmlNewText(NULL);
	xmlAddChild(xData_Node, xCom_Node);
	xmlAddChild(xCom_Node, xContent);

	xmlNodePtr xDevice_Node = xmlNewNode(NULL, BAD_CAST "device");
	xContent = xmlNewText(NULL);
	xmlAddChild(xData_Node, xDevice_Node);
	xmlAddChild(xDevice_Node, xContent);

	BOOL byState = FALSE;
	WORD DevId = 0;
	xmlNodePtr xValue_Node;

	BYTE byBusNum = m_pMethod->GetToTalBusNum();
	if (byBusNum == 0)
		return FALSE;
	m_byStaticLineNo = 0;
	if (m_byStaticLineNo >= byBusNum)
		m_byStaticLineNo = 0;

	while (m_byStaticLineNo < byBusNum)
	{
		try
		{
			/* 如果是转发协议不传输通讯状态 */

			BYTE byType = m_pMethod->GetBusLineProtocolType(m_byStaticLineNo);
			if ((byType == PROTOCO_TRANSPROT) || (byType == 0xFF))
			{
				m_byStaticLineNo++;
				continue;
			}

			BYTE byTotalDevNum = m_pMethod->GetDevNum(m_byStaticLineNo);

			//	printf("---FUNC = %s LINE = %d m_byStaticDevAddr =%d byTotalDevNum = %d----\n", __func__, __LINE__, m_byStaticDevAddr, byTotalDevNum);
			for (int i = 0; m_byStaticDevAddr < byTotalDevNum; m_byStaticDevAddr++)
			{
				PBUSMANAGER pBus = m_pMethod->GetBus(m_byStaticLineNo);
				bool bSerial = FALSE;
				if (pBus)
					bSerial = strcmp("CSerialPort", pBus->m_Port->ClassName());

				if (i == 0 && !bSerial)
				{
					byState = GetComState(m_byStaticLineNo);
					if (byState == 1)
						byState = 0;
					else if (byState == 0)
						byState = 1;
					memset(cTemp, 0, sizeof(cTemp));
					sprintf(cTemp, "%d", (int)byState);
					xValue_Node = xmlNewTextChild(xCom_Node, NULL, BAD_CAST "com_status", BAD_CAST cTemp);
					memset(cTemp, 0, sizeof(cTemp));
					// sprintf(cTemp, "%d", m_byStaticLineNo + 1);
					sprintf(cTemp, "%d", pBus->m_Port->m_uThePort);
					xmlNewProp(xValue_Node, BAD_CAST "com_id", BAD_CAST cTemp);
					mapComId[m_byStaticLineNo + 1] = byState;
				}

				//	printf("---FUNC = %s LINE = %d----\n", __func__, __LINE__);
				WORD wDevAddr = m_pMethod->GetAddrByLineNoAndModuleNo(m_byStaticLineNo, (WORD)m_byStaticDevAddr);
				char *sDevName = m_pMethod->GetDevNameByLineNoAndModuleNo(m_byStaticLineNo, (WORD)m_byStaticDevAddr);
				WORD wSerialNo = m_pMethod->GetSerialNo(m_byStaticLineNo, wDevAddr);

				// printf("---FUNC = %s LINE = %d  wSerialNo = %d LineNo = %d DevAddr = %d----\n", __func__, __LINE__ , wSerialNo , m_byStaticLineNo , wDevAddr );
				int temp = 0;
				BOOL bflag = IsDevNameFillwithDight(sDevName);
				if (wDevAddr != 0 && bflag)
				{

					temp = atoi(sDevName);
					byState = GetDevCommState(m_byStaticLineNo, wDevAddr);
					if (byState == 1)
						byState = 0;
					else if (byState == 0)
						byState = 1;
					memset(cTemp, 0, sizeof(cTemp));
					sprintf(cTemp, "%d", (int)byState);

					//
					if (IsDevHaveData(wSerialNo))
					{

						if (IsTransData(sDevName))
						{

							xValue_Node = xmlNewTextChild(xDevice_Node, NULL, BAD_CAST "device_status", BAD_CAST cTemp);
							memset(cTemp, 0, sizeof(cTemp));
							// memcpy(cTemp, sDevName, sizeof(sDevName));
							memcpy(cTemp, sDevName, strlen(sDevName) + 1);

							//	sprintf(cTemp, "%d", DevId);
							xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
							// printf("-----------------%d %s %s\n", __LINE__, __FILE__, cTemp);
						}
					}
					mapDevName[DevId] = sDevName;
					mapDevId[DevId] = byState;
					// mapDevId_Com[wDevAddr] = m_byStaticLineNo;
					mapDevId_Com[temp] = m_byStaticLineNo;
				}
				i++;
				DevId++;
			}
			if (m_byStaticDevAddr >= byTotalDevNum)
			{
				m_byStaticDevAddr = 0;
				m_byStaticLineNo++;
			}
		}
		catch (...)
		{
			char szLog[200] = {0};
			sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
			m_log.writeLog(szLog);
		}
	}

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	// printf("---FUNC = %s LINE = %d XmlBufLen = %d----\n", __func__, __LINE__, XmlBufLen);
	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

BOOL MonitoringPlatformXML::IsDevHaveData(WORD wSerialNo)
{
	STNPARAM *pParam = &m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.StnUnit[wSerialNo];
	int i;
	// printf("%d %s %d,%d,%d,%d,%d\n", __LINE__, __FILE__ , wSerialNo ,pParam->wAnalogSum ,
	//	pParam->wDigitalSum , pParam->wPulseSum , pParam->wRelaySum );
	if (!pParam)
		return FALSE;

	if (pParam->wAnalogSum || pParam->wDigitalSum || pParam->wPulseSum || pParam->wRelaySum)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
BOOL MonitoringPlatformXML::IsTransData(char *sdevname)
{
	int flag = 0;
	for (int i = 0; i < trans_id.size(); i++)
	{
		if (trans_id[i] == atoi(sdevname))
		{
			flag = 1;
			break;
		}
	}
	if (flag == 1)
		return TRUE;
	else
		return FALSE;
}

// --------------------------------------------------------
/// \概要:	通讯状态变化上送
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadComEFramePacket(XML TempXml, int &XmlBufLen)
{
	// printf("*********%d %s************\n", __LINE__, __FILE__);
	//	printf("---FUNC = %s LINE = %d ----\n", __func__, __LINE__);
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[50] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
#if 0
	if(m_iAllCall_Update)
		xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST TempXml.uId_Validate.bSequence);
	else
#endif
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);

	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "comm_status");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlAttrPtr szAttr = xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");

	xmlNodePtr xCom_Node = xmlNewNode(NULL, BAD_CAST "com");
	xContent = xmlNewText(NULL);
	xmlAddChild(xData_Node, xCom_Node);
	xmlAddChild(xCom_Node, xContent);

	xmlNodePtr xDevice_Node = xmlNewNode(NULL, BAD_CAST "device");
	xContent = xmlNewText(NULL);
	xmlAddChild(xData_Node, xDevice_Node);
	xmlAddChild(xDevice_Node, xContent);

	BOOL byState = FALSE;
	WORD DevId = 0;
	xmlNodePtr xValue_Node;
	BYTE byBusNum = m_pMethod->GetToTalBusNum();
	WORD wStateVarySum = 0;
	if (byBusNum == 0)
	{
		// printf("xmldoc return line = %d\n", __LINE__);
		xmlFreeDoc(xDoc);
		return FALSE;
	}

	if (m_byStaticLineNo >= byBusNum)
		m_byStaticLineNo = 0;
	// printf("*********%d %s************\n", __LINE__, __FILE__);
	//	printf("---FUNC = %s LINE = %d m_byStaticLineNo = %d byBusNum = %d----\n", __func__, __LINE__, m_byStaticLineNo, byBusNum);
	while (m_byStaticLineNo < byBusNum)
	{
		/* 如果是转发协议不传输通讯状态 */
		BYTE byType = m_pMethod->GetBusLineProtocolType(m_byStaticLineNo);
		if ((byType == PROTOCO_TRANSPROT) || (byType == 0xFF))
		{
			m_byStaticLineNo++;
			continue;
		}

		BYTE byTotalDevNum = m_pMethod->GetDevNum(m_byStaticLineNo);
		// printf("*********%d %s************\n", __LINE__, __FILE__);
		//		printf("---FUNC = %s LINE = %d m_byStaticDevAddr =%d byTotalDevNum = %d----\n", __func__, __LINE__, m_byStaticDevAddr, byTotalDevNum);
		for (int i = 0; m_byStaticDevAddr < byTotalDevNum; m_byStaticDevAddr++)
		{
			try
			{
				PBUSMANAGER pBus = m_pMethod->GetBus(m_byStaticLineNo);
				bool bSerial = FALSE;
				if (pBus)
					bSerial = strcmp("CSerialPort", pBus->m_Port->ClassName());

				if (i == 0 && !bSerial)
				{
					byState = GetComState(m_byStaticLineNo);
					if (byState == 1)
						byState = 0;
					else if (byState == 0)
						byState = 1;
					if (mapComId[m_byStaticLineNo + 1] != byState)
					{
						memset(cTemp, 0, sizeof(cTemp));
						sprintf(cTemp, "%d", (int)byState);
						xValue_Node = xmlNewTextChild(xCom_Node, NULL, BAD_CAST "com_status", BAD_CAST cTemp);
						memset(cTemp, 0, sizeof(cTemp));
						// sprintf(cTemp, "%d", m_byStaticLineNo + 1);
						sprintf(cTemp, "%d", pBus->m_Port->m_uThePort);
						xmlAttrPtr szAttr = xmlNewProp(xValue_Node, BAD_CAST "com_id", BAD_CAST cTemp);
						mapComId[m_byStaticLineNo + 1] = byState;
						wStateVarySum++;
					}
				}
				//	printf("*********%d %s************\n", __LINE__, __FILE__);
				//	printf("---FUNC = %s LINE = %d----\n", __func__, __LINE__);
				WORD wDevAddr = m_pMethod->GetAddrByLineNoAndModuleNo(m_byStaticLineNo, (WORD)m_byStaticDevAddr);
				char *sDevName = m_pMethod->GetDevNameByLineNoAndModuleNo(m_byStaticLineNo, (WORD)m_byStaticDevAddr);
				BOOL bflag = IsDevNameFillwithDight(sDevName);
				// printf("line = %d res = %d ，devname = %s\n", __LINE__, bflag, sDevName );
				if (wDevAddr != 0 && bflag)
				{
					byState = GetDevCommState(m_byStaticLineNo, wDevAddr);
					if (byState == 1)
						byState = 0;
					else if (byState == 0)
						byState = 1;
					// printf("*********%d %s****bystate=%d-------m_byStaticLineNo=%d---------- wDevAddr=%d-----------********\n", __LINE__, __FILE__, byState, m_byStaticLineNo, wDevAddr);
					if (mapDevId[DevId] != byState)
					{
						memset(cTemp, 0, sizeof(cTemp));
						sprintf(cTemp, "%d", (int)byState);
						xValue_Node = xmlNewTextChild(xDevice_Node, NULL, BAD_CAST "device_status", BAD_CAST cTemp);
						memset(cTemp, 0, sizeof(cTemp));
						// memcpy(cTemp, sDevName, sizeof(sDevName));
						memcpy(cTemp, sDevName, strlen(sDevName) + 1);
						//	sprintf(cTemp, "%d", DevId);
						printf("*********%d %s****id=%s********\n", __LINE__, __FILE__, cTemp);
						xmlAttrPtr szAttr = xmlNewProp(xValue_Node, BAD_CAST "id", BAD_CAST cTemp);
						mapDevId[DevId] = byState;
						wStateVarySum++;
					}
				}
			}
			catch (...)
			{
				char szLog[200] = {0};
				sprintf(szLog, "%s %d try catch happen", __FUNCTION__, __LINE__);
				m_log.writeLog(szLog);
			}
			i++;
			DevId++;
		}
		if (m_byStaticDevAddr >= byTotalDevNum)
		{
			m_byStaticDevAddr = 0;
			m_byStaticLineNo++;
		}
	}
	//	printf("---FUNC = %s LINE = %d----\n", __func__, __LINE__);

	if (wStateVarySum == 0)
	{
		xmlFreeDoc(xDoc);
		return FALSE;
	}

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	//	printf("---FUNC = %s LINE = %d XmlBufLen = %d----\n", __func__, __LINE__, XmlBufLen);
	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	// printf(m_XmlBuf);
	return TRUE;
}

BOOL MonitoringPlatformXML::IsDevNameFillwithDight(char *szDevName)
{
	if (!szDevName)
		return FALSE;
	int len = strlen(szDevName);

	for (int i = 0; i < len; i++)
	{
		char pchar = szDevName[i];
		BOOL b = isdigit(pchar);
		if (b == 0)
			return FALSE;
	}

	return TRUE;
}
// --------------------------------------------------------
/// \概要:	上传总召唤数据
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadAllDataPacket(XML TempXml, int &XmlBufLen)
{

	switch (m_byDataStyle & 0xf0)
	{
	case 0x10:
	{
		m_iAllCall_Update = 1;
		int nSize = GetPntSum(YX_SUM);
		if (nSize < ONE_BUF_MOUNT_YX)
		{
			printf("call all yx\n");
			LoadDigitalDataPacket(TempXml, XmlBufLen);
		}
		else
		{
			printf("call all begin= %d\n", dataindex);
			LoadDigitalDataPacket(TempXml, XmlBufLen, m_wDataIndex + ONE_BUF_MOUNT_YX, &dataindex);
			SetTransIndex(m_wDataIndex, 0x10);
		}
		if (m_wDataIndex >= GetPntSum(YX_SUM))
		{
			dataindex = 0;
			SetTransIndex(0, 0x20);
		}
	}
	break;
	case 0x20:
	{
		m_iAllCall_Update = 1;
		int nSize_yc = GetPntSum(YC_SUM);
		if (nSize_yc < ONE_BUF_MOUNT_YC)
		{
			LoadAnalogDataPacket_Batch(TempXml, XmlBufLen, nSize_yc);
		}
		else
		{
			LoadAnalogDataPacket_Batch(TempXml, XmlBufLen, m_wDataIndex + ONE_BUF_MOUNT_YC);
			SetTransIndex(m_wDataIndex, 0x20);
		}
		if (m_wDataIndex >= GetPntSum(YC_SUM))
		{
			SetTransIndex(0, 0x30);
		}
	}
	break;
	case 0x30:
	{

		m_iAllCall_Update = 1;
		int nSize_ym = GetPntSum(YM_SUM);
		m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);
		if (nSize_ym < ONE_BUF_MOUNT_YM)
		{
			printf("line = %d fuc=%s\n", __LINE__, __FUNCTION__);
			LoadPulseDataPacket_Batch(TempXml, XmlBufLen, nSize_ym);
		}
		else
		{
			LoadPulseDataPacket_Batch(TempXml, XmlBufLen, m_wDataIndex + ONE_BUF_MOUNT_YM);
			SetTransIndex(m_wDataIndex, 0x30);
		}
		if (m_wDataIndex >= GetPntSum(YM_SUM))
			SetTransIndex(0, 0x40);
	}
	break;
	case 0x40:
	{

		m_iAllCall_Update = 1;
		LoadCommStatusPacket(TempXml, XmlBufLen);
		SetTransIndex(0, 0);
		SetCommand(CMD_TOTAL_CONFIRM);
	}
	break;
	}
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	实时上传变化数据帧
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::ChangeDataProcessPacket(XML TempXml, int &XmlBufLen)
{
	time_t tStateTimeTemp;
	time(&tStateTimeTemp);
	time_t tYxTimeTemp;
	time(&tYxTimeTemp);
	time_t tYcTimeTemp;
	time(&tYcTimeTemp);
	time_t tYmTimeTemp;
	time(&tYmTimeTemp);

	if ((m_byDITimeOut == 0) && (m_dwDIEQueue_Xml.size() > 0) && CanSendChange==true)
	{
		LoadDIEFramePacket(TempXml, XmlBufLen); // 实时上传变位遥信
		SetTTimer(XML_T3, XML_START_T3);
	}
	else if (m_byDITimeOut == 1) // 遥信超时未回复确认，重新发送一次
	{
		memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
		memcpy(m_XmlBuf, m_XmlDITimeOutBuf, strlen(m_XmlDITimeOutBuf));
		m_byDITimeOut = 0;
	}
	else if ((tYxTimeTemp - m_YxValueTime) > (m_wYxUploadInterval * 60) && last_ycbuf_issending == 0 && last_ymbuf_issending == 0)
	{
		m_iAllCall_Update = 0;
		m_pMethod->ReadAllYxData(m_byDIBuf);
		int nSize = GetPntSum(YX_SUM);
		if (nSize < ONE_BUF_MOUNT_YX)
		{
			LoadDigitalDataPacket(TempXml, XmlBufLen);
		}
		else
		{
			/*
			printf("yx send packet  begin %d\n", dataindex_zyx);
			struct timeval tv;
			struct tm *tm_info;
			char time_buffer[26];
			gettimeofday(&tv, NULL);
			tm_info = localtime(&tv.tv_sec);
			strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
			printf("[%s.%03ld] yx send packet begin %d\n", time_buffer, tv.tv_usec / 1000, dataindex_zyx);
			*/
			LoadDigitalDataPacket(TempXml, XmlBufLen, m_wDataIndex + ONE_BUF_MOUNT_YX, &dataindex_zyx);
			if (m_wDataIndex >= GetPntSum(YX_SUM) || dataindex_zyx >= GetPntSum(YX_SUM))
			{
				m_wDataIndex = 0;
				dataindex_zyx = 0;
				last_yxbuf_issending = 0;
				time(&m_YxValueTime);
			}
			else
			{
				last_yxbuf_issending = 1;
			}
		}
		if (m_wDataIndex >= GetPntSum(YX_SUM))
		{
			m_wDataIndex = 0;
			dataindex_zyx = 0;
			last_yxbuf_issending = 0;
			time(&m_YxValueTime);
		}
	}
	else if ((tYmTimeTemp - m_YmValueTime) > (m_wYmUploadInterval * 60) && last_yxbuf_issending == 0 && last_ycbuf_issending == 0)
	{

		m_iAllCall_Update = 0;
		m_pMethod->ReadAllYmData(&m_dwPIBuf[0]);
		int nSize_ym = GetPntSum(YM_SUM);
		if (nSize_ym < ONE_BUF_MOUNT_YM)
		{

			LoadPulseDataPacket_Batch(TempXml, XmlBufLen, nSize_ym);
		}
		else
		{
			LoadPulseDataPacket_Batch(TempXml, XmlBufLen, m_wDataIndex + ONE_BUF_MOUNT_YM);
			if (m_wDataIndex >= GetPntSum(YM_SUM))
			{
				m_wDataIndex = 0;
				last_ymbuf_issending = 0;
				time(&m_YmValueTime);
			}
			else
			{
				last_ymbuf_issending = 1;
			}
		}
		if (m_wDataIndex >= GetPntSum(YM_SUM))
		{
			m_wDataIndex = 0;
			last_ymbuf_issending = 0;
			time(&m_YmValueTime);
		}
	}
	else if ((m_iSOE_rd_p != m_iSOE_wr_p) && CanSendChange==true) // 传输SOE信息
	{
		LoadSOEFramePacket(TempXml, XmlBufLen);
	}
	else if ((tStateTimeTemp - m_DevStateTime) > 60)
	{
		m_iAllCall_Update = 0;
		LoadCommStatusPacket(TempXml, XmlBufLen);
		time(&m_DevStateTime);
		CanSendChange=true;
		//m_log.writeLog("tStateTimeTemp - m_DevStateTime");
	}
	else if ((tStateTimeTemp - m_VaryDevStateTime) > 60)
	{
		LoadComEFramePacket(TempXml, XmlBufLen);
		time(&m_VaryDevStateTime);
		//m_log.writeLog("tStateTimeTemp - m_VaryDevStateTime");
	}
	else if ((tYcTimeTemp - m_YcValueTime) >= m_wYcUploadInterval * 60 && last_yxbuf_issending == 0 && last_ymbuf_issending == 0)
	{
		m_iAllCall_Update = 0;
		//m_log.writeLog("yc real upload");
		int nSize_yc = GetPntSum(YC_SUM);
		if (nSize_yc < ONE_BUF_MOUNT_YC)
		{

			LoadAnalogDataPacket_Batch(TempXml, XmlBufLen, nSize_yc);
		}
		else
		{

			LoadAnalogDataPacket_Batch(TempXml, XmlBufLen, m_wDataIndex + ONE_BUF_MOUNT_YC);
			if (m_wDataIndex >= GetPntSum(YC_SUM))
			{
				m_wDataIndex = 0;
				last_ycbuf_issending = 0;
				time(&m_YcValueTime);
			}
			else
			{
				last_ycbuf_issending = 1;
			}
		}
		if (m_wDataIndex >= GetPntSum(YC_SUM))
		{
			m_wDataIndex = 0;
			last_ycbuf_issending = 0;
			time(&m_YcValueTime);
		}
		// m_log.writeLog("tYcTimeTemp - m_YcValueTime");
	}
	else if ((m_dwAIEQueue_Xml.size() > 0) && CanSendChange==true)
	{
		LoadAIEFramePacket(TempXml, XmlBufLen);
		// m_log.writeLog("m_dwAIEQueue_Xml.size");
	}
	else
	{
		;
	}
	return TRUE;
}
// --------------------------------------------------------
/// \概要:	组织身份确认数据帧
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::RequestPacket(XML TempXml, int &XmlBufLen)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "reg");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "update");
	xmlNewTextChild(xData_Node, NULL, BAD_CAST "gatewayID", BAD_CAST m_pMethod->m_pRtuObj->m_sDevName);

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	身份确认完成
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::RequestPacketReply(XML TempXml, int &XmlBufLen, char *szTemp)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);
	/* 生成4位随机数 */
	GetRandom();
	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_bSequence);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST szTemp);

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	组织存活通知帧
///
/// \参数:	TempXml
/// \参数:	XmlBufLen
// --------------------------------------------------------
BOOL MonitoringPlatformXML::NotifyPacket(XML TempXml, int &XmlBufLen)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");
	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST "1");
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST "0001");
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "notify");

	/* 创建一个节点，设置其内容和属性，然后加入根节点 */
	xmlNodePtr xData_Node = xmlNewNode(NULL, BAD_CAST "data");
	xmlNodePtr xContent = xmlNewText(NULL);
	xmlAddChild(xRoot_Node, xData_Node);
	xmlAddChild(xData_Node, xContent);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "notify");
	xmlNewTextChild(xData_Node, NULL, BAD_CAST "gatewayID", BAD_CAST "1");

	xmlChar *xmlBuf;
	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	预处理组织报文
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::PreProcess(PBUSMSG pBusMsg)
{
	int iTempBufLen = 0;
	if(GetCommand() == CMD_REG_REQUSET)
	{
		RequestPacket(m_xml, iTempBufLen); // 组织身份认证
		SetCommand(CMD_NULL);
		m_wCommand &= ~CMD_IDEN_DET;
		SetTTimer(XML_T2, XML_START_T2);
	}	
	else if ((m_wCommand & CMD_IDEN_DET) != 0)
	{
		RequestPacket(m_xml, iTempBufLen); // 组织身份认证
		m_wCommand &= ~CMD_IDEN_DET;
		SetTTimer(XML_T2, XML_START_T2);
	}
	else if ((m_wCommand & CMD_UNVARNISH) != 0)
	{
		RequestPacketUnvarnish(m_xml, iTempBufLen); // 透传
		m_wCommand &= ~CMD_UNVARNISH;
	}
	else if ((m_wCommand & CMD_IDEN_BIT) != 0)
	{
		RequestPacketReply(m_xml, iTempBufLen, (char *)"reg_confirm");
		m_wCommand &= ~CMD_IDEN_BIT;
	}
	else if ((m_wCommand & CMD_IDEN_END) != 0)
	{
		RequestPacketReply(m_xml, iTempBufLen, (char *)"reg_end");
		m_wCommand &= ~CMD_IDEN_END;
		m_bStartBit = FALSE;
	}
	else if ((m_wCommand & CMD_COA_BIT) != 0)
	{
		YkPacket(m_xml, iTempBufLen);
		m_wCommand &= ~CMD_COA_BIT;
	}
	else if (GetCommand() == CMD_YK_ERROR)
	{
		//	printf("----FUNC = %s LINE = %d CMD_YK_ERROR----\n", __func__, __LINE__);
		YkRelayEchoFrame(m_xml, iTempBufLen, (char *)"fail");
		SetCommand(CMD_NULL);
	}
	else if (m_wCommand & CMD_DZ_BIT)
	{
		//		printf("----FUNC = %s LINE = %d CMD_DZ_SUCCESS----\n", __func__, __LINE__);
		DzPacket(m_xml, iTempBufLen, pBusMsg);
		m_wCommand &= ~CMD_DZ_BIT;
	}
	else if (GetCommand() == CMD_DZ_ERROR)
	{
		//	printf("----FUNC = %s LINE = %d CMD_DZ_ERROR----\n", __func__, __LINE__);
		DzRelayEchoFrame(m_xml, iTempBufLen, (char *)"fail");
		SetCommand(CMD_NULL);
	}

	if (iTempBufLen > 0)
	{
		m_XmlBuf[iTempBufLen] = '\n';
		iTempBufLen++;

		m_pTX_Buf[0] = 0xEB;
		m_pTX_Buf[1] = 0x90;
		//		memcpy(m_pTX_Buf + 2, &iTempBufLen, 2);
		m_pTX_Buf[2] = (iTempBufLen >> 8) & 0xFF;
		m_pTX_Buf[3] = iTempBufLen & 0xFF;
		memcpy(m_pTX_Buf + 4, m_XmlBuf, iTempBufLen);
		iTempBufLen += 4;
	}

	return iTempBufLen;
}

void MonitoringPlatformXML::RequestPacketUnvarnish(XML TempXml, int &XmlBufLen)
{
	/* 定义文件和节点指针 */
	xmlDocPtr xDoc = xmlNewDoc(BAD_CAST "1.0");
	xmlNodePtr xRoot_Node = xmlNewNode(NULL, BAD_CAST "root");

	/* 设置根节点 */
	xmlDocSetRootElement(xDoc, xRoot_Node);

	char cTemp[30] = {'\0'};
	sprintf(cTemp, "%d", m_pMethod->m_pRtuObj->m_wDevAddr);

	/* 在根节点中直接创建节点 */
	xmlNodePtr xCommon_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "common", NULL);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "fac_no", BAD_CAST cTemp);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "quest_id", BAD_CAST m_quest_id_forUnvarnish);
	xmlNewTextChild(xCommon_Node, NULL, BAD_CAST "type", BAD_CAST "ESDSetting");

	xmlNodePtr xData_Node = xmlNewTextChild(xRoot_Node, NULL, BAD_CAST "data", NULL);
	xmlNewProp(xData_Node, BAD_CAST "operation", BAD_CAST "result");

	sprintf(cTemp, "%d", m_wMsgDevID);
	xmlNewTextChild(xData_Node, NULL, BAD_CAST "dev", BAD_CAST cTemp);
	xmlNodePtr xRes_Node = xmlNewTextChild(xData_Node, NULL, BAD_CAST "result", BAD_CAST m_UnvarnishRtnBuf);
	xmlNewProp(xRes_Node, BAD_CAST "errorcode", BAD_CAST "0");

	xmlChar *xmlBuf;

	xmlDocDumpFormatMemoryEnc(xDoc, &xmlBuf, &XmlBufLen, "UTF-8", 1);
	memset(m_XmlBuf, 0, sizeof(m_XmlBuf));
	memcpy(m_XmlBuf, xmlBuf, XmlBufLen);

	/* 释放文件内节点动态申请的内存 */
	xmlFree(xmlBuf);
	xmlFreeDoc(xDoc);
}

// --------------------------------------------------------
/// \概要:	开始报文组织
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::StartedProcess()
{
	int iTempBufLen = 0;
	if (GetCommand() == CMD_TIME_CONFIRM)
	{
		SysClockConfirmPacket(m_xml, iTempBufLen);
		SetCommand(CMD_NULL);
	}
	else if (m_byDataStyle != 0)
	{
		LoadAllDataPacket(m_xml, iTempBufLen); // 上传总招数据
	}
	else if (GetCommand() == CMD_TOTAL_CONFIRM)
	{
		AllDataEchoPacket(m_xml, iTempBufLen, (char *)"confirm");
		SetCommand(CMD_NULL);
	}
	else
	{
		ChangeDataProcessPacket(m_xml, iTempBufLen);
	}

	if (iTempBufLen > 0)
	{

		m_XmlBuf[iTempBufLen] = '\n';
		iTempBufLen++;

		m_pTX_Buf[0] = 0xEB;
		m_pTX_Buf[1] = 0x90;

		m_pTX_Buf[2] = (iTempBufLen >> 8) & 0xFF;
		m_pTX_Buf[3] = iTempBufLen & 0xFF;

		memcpy(m_pTX_Buf + 4, m_XmlBuf, iTempBufLen);

		iTempBufLen += 4;
	}
	return iTempBufLen;
}

// --------------------------------------------------------
/// \概要:	组织发送报文
///
/// \参数:	pBuf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::LoadXMLMessage(BYTE *pBuf, int &len, PBUSMSG pBusMsg)
{
	fenbaoflag = 1;
	if (pBuf == NULL)
		return FALSE;
	m_pTX_Buf = pBuf;
	len = PreProcess(pBusMsg);
	if (len > 0)
		;
	else
	{
		if (m_bStartBit)
		{
			len = StartedProcess();
		}
	}
	if (len <= 0)
		return FALSE;
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	组织报文
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	if (buf == NULL)
		return FALSE;

	if (pBusMsg)
		DealBusMsgInfo(pBusMsg);
	if (!m_pMethod->m_pPort->IsPortValid())
	{
		struct timeval tv;
		struct tm *tm_info;
		char time_buffer[26];
		gettimeofday(&tv, NULL);
		tm_info = localtime(&tv.tv_sec);
		strftime(time_buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
		printf("[%s.%03ld] Port is not valid!\n", time_buffer, tv.tv_usec / 1000);	
		ReSetState();
		return FALSE;
	}
	BOOL bRtn = LoadXMLMessage(buf, len, pBusMsg);
	time(&time_reboot_flag);

#if 1	
	if (len>0)
	{
		printf("-------------Send Buf---------%d----------!!!!\n", m_byLineNo);
		for (int i = 0; i < 4; i++)
		{
			printf("%02x", buf[i]);
		}
		for (int i = 4; i < len; i++)
		{
			printf("%c", buf[i]);
		}
		printf("\n");
	}
#endif
	return bRtn;
}

BOOL MonitoringPlatformXML::XmlCommandProc(BYTE *pRecvBuf, int nlen)
{
	WORD len = 0;
	m_pRX_Buf = pRecvBuf;
	if (m_pRX_Buf[0] != 0xEB)
	{
		return FALSE;
	}
	if (m_pRX_Buf[1] != 0x90)
	{
		return FALSE;
	}

	len = ((m_pRX_Buf[2] << 8) | m_pRX_Buf[3]);

	if ((len + 4) != nlen)
		return FALSE;
	if (!AnalyticMessage(&m_xml, (char *)m_pRX_Buf, len))
	{
		printf("\n");
		for (int i = 0; i < len; i++)
			printf("%02x ", m_pRX_Buf[i]);
		printf("\n");
		OutPromptText("Failed Analytic.\n");
		return FALSE;
	}

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
BOOL MonitoringPlatformXML::ProcessProtocolBuf(BYTE *buf, int len)
{
	BOOL bRtn = TRUE;
	if (len == 0 || buf == NULL)
		return FALSE;
	time(&time_reboot_flag);	
#if 0
	if (len > 0)
	{
		printf("-------------Recv Buf---------%d----------!!!!\n", m_byLineNo);
		for (int i = 0; i < 4; i++)
		{
			printf("%02x", buf[i]);
		}
		for (int i = 4; i < len; i++)
		{
			printf("%c", buf[i]);
		}
		printf("\n");
	}

#endif

	// 	char pbuf[1024] =
	// 	{
	// 		"<\?xml version =\"1.0\" encoding =\"utf-8\"\?>"
	// 		"<root>"
	// 			"<common>"
	// 				"<fac_no>1 </fac_no>"
	// 				"<quest_id>7546</quest_id>"
	// 				"<type>ESDSetting</type>"
	// 			"</common>"
	// 			"<data operation =\"write\">"
	// 				"<devcount>2</devcount>"
	// 				"<dev>185_186</dev>"
	// 				"<textbody>10 08 21 aa b1 01 02 03 04 05 06</textbody>"
	// 			"</data>"
	// 		"</root>"
	// 	};

	// 	char sbuf[1024] = {};
	// 	sbuf[0] = 0xEB;
	// 	sbuf[1] = 0x90;
	// 	WORD slen = (WORD)string(pbuf).length();
	// 	sbuf[2] = HIBYTE(slen);
	// 	sbuf[3] = LOBYTE(slen);
	// 	strcat(sbuf + 4, pbuf);
	//
	// 	len = slen + 4 ;
	// 	buf = (BYTE*)sbuf;
	//
	// 	XmlCommandProc((BYTE*)sbuf, len);

	int iLen = len;
	int iNLen = ((buf[2] << 8) | buf[3]) + 4;
	if (iLen > iNLen)
	{
		XmlCommandProc(buf, iNLen);
		int iLen1 = len - iNLen;
		int iNLen1 = ((buf[iNLen + 2] << 8) | buf[iNLen + 3]) + 4;
		if (iLen1 > iNLen1)
		{
			bRtn = XmlCommandProc(buf + iNLen, iNLen1);
			int iNLen2 = ((buf[iNLen + iNLen1 + 2] << 8) | buf[iNLen + iNLen1 + 3]) + 4;
			bRtn = XmlCommandProc(buf + iNLen + iNLen1, iNLen2);
		}
		else if (iLen1 == iNLen1)
			bRtn = XmlCommandProc(buf + iNLen, iLen1);
	}
	else if (iLen == iNLen)
		bRtn = XmlCommandProc(buf, len);

	return bRtn;
}

// --------------------------------------------------------
/// \概要:	处理XML时间
// --------------------------------------------------------
void MonitoringPlatformXML::ProcessXMLTime()
{
	BYTE byTimeFlag = GetTimeFlag();
	time_t tTimeTemp;
	time(&tTimeTemp);
	if ((byTimeFlag & XML_START_T1) == XML_START_T1)
	{
		if (difftime(tTimeTemp, m_t1) > XML_T1)
		{
			/* 处理XML所有状态 */
			//	ReSetState();
			/* 关闭网络连接 */
			//	if(m_pMethod)
			//	m_pMethod->CloseSocket(m_byLineNo);
		}
	}

	if ((byTimeFlag & XML_START_T2) == XML_START_T2)
	{
		if (difftime(tTimeTemp, m_t2) > XML_T2)
		{
			/* 发送身份确认 */
			m_wCommand |= CMD_IDEN_DET;
		}
	}

	if ((byTimeFlag & XML_START_T3) == XML_START_T3)
	{
		if (difftime(tTimeTemp, m_t3) > XML_T3)
		{
			/* 实时遥信上传处理 */
			m_byDITimeOut = 1;
		}
	}
	if ((byTimeFlag & XML_START_T4) == XML_START_T4)
	{
		if (difftime(tTimeTemp, m_t4) > XML_T4)
		{
			/* 拒绝重新身份验证 */
			m_wCommand |= CMD_IDEN_DET;
		}
	}

	if ((byTimeFlag & XML_START_YK) == XML_START_YK)
	{
		if (difftime(tTimeTemp, m_YKTime) > XML_YKTIME)
		{
			//		printf("----FUNC= %s LINE = %d YkTime is Over----\n", __func__, __LINE__);
			SetCommand(CMD_YK_ERROR);
			SetTTimer(XML_YKTIME, XML_END_YK);
		}
	}

	if ((byTimeFlag & XML_START_DZ) == XML_START_DZ)
	{
		if (difftime(tTimeTemp, m_DZTime) > XML_DZTIME)
		{
			//		printf("----FUNC= %s LINE = %d DzTime is Over----\n", __func__, __LINE__);
			SetCommand(CMD_DZ_ERROR);
			SetTTimer(XML_DZTIME, XML_END_DZ);
		}
	}
}

// --------------------------------------------------------
/// \概要:	时间处理函数
// --------------------------------------------------------
void MonitoringPlatformXML::TimerProc()
{
	/* 从内存中读取变化遥信和遥测数据 */
	ReadChangData();

	/* 将变化遥信写入数据库 */
	//	WriteChangedDI();
	/* 处理XML T1, T2, T3, T4, YK_TIME */
	ProcessXMLTime();

	time_t tTimeTemp;
	time(&tTimeTemp);

	if (difftime(tTimeTemp, time_reboot_flag) > 60&&reboot_flag==1)
	{
		m_log.writeLog("超时60秒 and reboot");
		reboot_flag=0;	
		system("reboot");
		system("sync");
	}
}

// --------------------------------------------------------
/// \概要:	设置报文发送序列
///
/// \参数:	wIndex
/// \参数:	wDataStyle
// --------------------------------------------------------
void MonitoringPlatformXML::SetTransIndex(WORD wIndex, WORD wDataStyle)
{
	m_wDataIndex = wIndex;
	m_byDataStyle = wDataStyle;
}

// --------------------------------------------------------
/// \概要:	生成4 位随机数
// --------------------------------------------------------
void MonitoringPlatformXML::GetRandom()
{
	memset(m_bSequence, '\0', sizeof(m_bSequence));
	srand(time(NULL));
	WORD wRandNum = rand() % 10000;
	sprintf((char *)m_bSequence, "%04d", wRandNum);
}

// --------------------------------------------------------
/// \概要:	获得节点设置
///
/// \参数:	xDoc
/// \参数:	xPath
///
/// \返回:	BOOL
// --------------------------------------------------------
xmlXPathObjectPtr MonitoringPlatformXML::GetNodeSet(xmlDocPtr xDoc, const xmlChar *xPath)
{
	xmlXPathContextPtr xContext;
	xmlXPathObjectPtr xResult;

	xContext = xmlXPathNewContext(xDoc);
	if (xContext == NULL)
	{
		// printf("------ file = %s context is NULL ------\n", __FILE__);
		return NULL;
	}

	xResult = xmlXPathEvalExpression(xPath, xContext);
	xmlXPathFreeContext(xContext);
	if (xResult == NULL)
	{
		// printf("------ file = %s xmlXPathEvalExpression return NULL -----\n", __FILE__);
		return NULL;
	}

	if (xmlXPathNodeSetIsEmpty(xResult->nodesetval))
	{
		xmlXPathFreeObject(xResult);
		// printf("------ file = %s nodeset is empty ------\n", __FILE__);
		return NULL;
	}

	return xResult;
}

// --------------------------------------------------------
/// \概要:	解析服务器回复帧
///
/// \参数:	TempXml
/// \参数:	XmlBuf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::AnalyticMessage(XML *TempXml, char *XmlBuf, int len)
{

	char szTempType[20] = {'\0'};

	/* 过滤'<' 前方的乱码 */
	while (XmlBuf[0] != '<')
		XmlBuf = XmlBuf + 1;

	/* 将buf字符串转化为xmlDocPtr格式的xml结构 */
	xmlKeepBlanksDefault(0);
	xmlDocPtr xDoc = xmlParseMemory(XmlBuf, len);
	if (xDoc == NULL)
	{
		printf("Fail to parse XML buffer.\n");
		return FALSE;
	}

	xmlNodePtr xCur = xmlDocGetRootElement(xDoc);
	if (xmlStrcmp(xCur->name, (const xmlChar *)"root"))
	{
		printf("Don't found root.\n");
		xmlFreeDoc(xDoc);
		return FALSE;
	}

	xmlChar *xPath = (xmlChar *)"/root/common/type";
	xmlXPathObjectPtr xApp_Result = GetNodeSet(xDoc, xPath);
	if (xApp_Result == NULL)
	{
		printf("xApp_Result is NULL\n");
		xmlXPathFreeObject(xApp_Result);
		xmlFreeDoc(xDoc);
		return FALSE;
	}

	xmlNodeSetPtr xNodeSet = xApp_Result->nodesetval;
	xCur = xNodeSet->nodeTab[0];
	xmlChar *xValue = xmlNodeGetContent(xCur);

	memcpy(szTempType, xValue, strlen((char *)xValue));

	xmlFree(xValue);

	// printf("%s %d %s\n", __FILE__, __LINE__, TempXml->uId_Validate.bSequence);
	// printf("---recv=%s\n", szTempType);
	if (memcmp(szTempType, "reg_request", strlen("reg_request")) == 0) // 重新登录请求
	{
		// printf("reg_request aommand\n");
		m_dwAIEQueue_Xml.clear();
		ReSetDataState();
		SetTTimer(XML_T4, XML_START_T4);
		xmlXPathFreeObject(xApp_Result);
		xmlFreeDoc(xDoc);
		SetCommand(CMD_REG_REQUSET);
		return TRUE;
	}
	else if (memcmp(szTempType, "reg", strlen("reg")) == 0)
	{
		m_dwAIEQueue_Xml.clear();

		ReSetDataState();

		memset(TempXml, 0, sizeof(XML));
		memcpy(TempXml->bType, szTempType, strlen(szTempType));
		xPath = (xmlChar *)"/root/common/quest_id";

		xmlXPathFreeObject(xApp_Result);
		xApp_Result = GetNodeSet(xDoc, xPath);
		xNodeSet = xApp_Result->nodesetval;
		xCur = xNodeSet->nodeTab[0];
		xValue = xmlNodeGetContent(xCur);

		memcpy(TempXml->uId_Validate.bSequence, xValue, strlen((char *)xValue));

		xmlFree(xValue);
		if (memcmp(TempXml->uId_Validate.bSequence, m_bSequence, sizeof(m_bSequence)) == 0)
		{
			xPath = (xmlChar *)"/root/data/result";

			xmlXPathFreeObject(xApp_Result);
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];
			xValue = xmlNodeGetContent(xCur);

			memcpy(TempXml->uId_Validate.bResult, xValue, strlen((char *)xValue));
			xmlFree(xValue);
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
			if (memcmp(TempXml->uId_Validate.bResult, "accept", strlen((char *)"accept")) == 0)
			{
				
				SetTTimer(XML_T2, XML_END_T2);
				SetTTimer(XML_T4, XML_END_T4);
				m_bStartBit = TRUE;
				reboot_flag=1;
			}
			else if (memcmp(TempXml->uId_Validate.bResult, "refuse", strlen((char *)"refuse")) == 0)
			{
				//			m_wCommand |= CMD_IDEN_END;
				ReSetState();
				SetTTimer(XML_T4, XML_START_T4);
				OutPromptText("peer refused\n");
				return TRUE;
			}
		}
		else
		{
			//	m_wCommand |= CMD_IDEN_DET;
			// printf("quest_id is Failed!!\n");
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
			return FALSE;
		}
	}

	else if (memcmp(szTempType, "reboot", strlen("reboot")) == 0) // 重启命令
	{
		
		system("reboot");
		system("sync");
		return TRUE;
	}
	else if (memcmp(szTempType, "call_all", strlen((char *)"call_all")) == 0)
	{

		// printf("---FUNC = %s LINE = %d ----\n", __func__, __LINE__);
		memset(TempXml, 0, sizeof(XML));

		memcpy(TempXml->bType, szTempType, strlen(szTempType));

		xPath = (xmlChar *)"/root/common/quest_id";
		xmlXPathFreeObject(xApp_Result);
		xApp_Result = GetNodeSet(xDoc, xPath);
		xNodeSet = xApp_Result->nodesetval;
		xCur = xNodeSet->nodeTab[0];
		xValue = xmlNodeGetContent(xCur);

		memcpy(TempXml->uId_Validate.bSequence, xValue, strlen((char *)xValue));

		xmlFree(xValue);
		dataindex = 0;
		SetTransIndex(0, 0x10);
		m_dwAIEQueue_Xml.clear();

		xmlXPathFreeObject(xApp_Result);
		xmlFreeDoc(xDoc);
	}
	else if (memcmp(szTempType, "YK", strlen((char *)"YK")) == 0)
	{
		//	printf("---FUNC = %s LINE = %d YK Start----\n", __func__, __LINE__);
		//	memset(TempXml, 0, sizeof(XML));
		memcpy(TempXml->bType, szTempType, strlen(szTempType));
		xPath = (xmlChar *)"/root/common/fac_no";

		xmlXPathFreeObject(xApp_Result);
		xApp_Result = GetNodeSet(xDoc, xPath);
		xNodeSet = xApp_Result->nodesetval;
		xCur = xNodeSet->nodeTab[0];

		xValue = xmlNodeGetContent(xCur);
		TempXml->uId_Validate.wFacNo = (WORD)atoi((char *)xValue);
		xmlFree(xValue);
		if (m_pMethod->m_pRtuObj->m_wDevAddr == TempXml->uId_Validate.wFacNo)
		{
			//	printf("---FUNC = %s LINE = %d FacNo True----\n", __func__, __LINE__);
			xPath = (xmlChar *)"/root/common/quest_id";

			xmlXPathFreeObject(xApp_Result);
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];

			xValue = xmlNodeGetContent(xCur);
			memcpy(TempXml->uId_Validate.bSequence, xValue, strlen((char *)xValue));
			xmlFree(xValue);
			xPath = (xmlChar *)"/root/data/code";

			xmlXPathFreeObject(xApp_Result);
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];
			xValue = xmlGetProp(xCur, (const xmlChar *)"id");
			CharacterSplit(TempXml, (char *)xValue);
			xmlFree(xValue);

			xValue = xmlNodeGetContent(xCur);
			TempXml->uId_Validate.bYkValue = (BYTE)atoi((char *)xValue);
			xmlFree(xValue);

			int nSize = GetPntSum(YK_SUM);
			WORD wSerialNo = 0xFFFF;
			for (int i = 0; i < nSize; i++)
			{
				if ((TempXml->uId_Validate.wDevId == GetDevIdFromTrans(YK_TRANSTOSERIALNO, i)) &&
					(TempXml->uId_Validate.wYkOrder == GetDevPntFromTrans(YK_TRANSTOSERIALNO, i)))
				{
					wSerialNo = GetSerialNoFromTrans(YK_TRANSTOSERIALNO, i);
					break;
				}
			}

			if (wSerialNo == 0xFFFF)
			{
				SetTTimer(XML_YKTIME, XML_START_YK);
				printf("file = %s , line = %d func = %s wDevID = %d ykorder = %d\n", __FILE__, __LINE__, __FUNCTION__,
					   TempXml->uId_Validate.wDevId, TempXml->uId_Validate.wYkOrder);

				xmlFreeDoc(xDoc);
				return FALSE;
			}

			xPath = (xmlChar *)"/root/data";
			xmlXPathFreeObject(xApp_Result);
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];
			xValue = xmlGetProp(xCur, (const xmlChar *)"operation");
			//	printf("---FUNC = %s LINE = %d xValue = %s----\n", __func__, __LINE__, (char *)xValue);

			if (memcmp((char *)xValue, "pre_op", strlen((char *)"pre_op")) == 0)
			{
				//	printf("---FUNC = %s LINE = %d pre_op----\n", __func__, __LINE__);
				RelaySelectProc(wSerialNo, TempXml->uId_Validate.wDevId, TempXml->uId_Validate.wYkOrder, TempXml->uId_Validate.bYkValue);
				SetTTimer(XML_YKTIME, XML_START_YK);
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xmlFreeDoc(xDoc);
			}

			else if (memcmp((char *)xValue, "op", strlen((char *)"op")) == 0)
			{
				if (Yk_IsCanSend())
				{
					//	printf("---FUNC = %s LINE = %d op----\n", __func__, __LINE__);
					RelayExecuteProc(wSerialNo, TempXml->uId_Validate.wDevId, TempXml->uId_Validate.wYkOrder, TempXml->uId_Validate.bYkValue);
					SetTTimer(XML_YKTIME, XML_START_YK);
				}
				else
					SetCommand(CMD_YK_ERROR);

				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xmlFreeDoc(xDoc);
			}

			else if (memcmp((char *)xValue, "cancel", strlen((char *)"cancel")) == 0)
			{
				if (Yk_IsCanSend())
				{
					//	printf("---FUNC = %s LINE = %d cancel----\n", __func__, __LINE__);
					RelayCancelProc(wSerialNo, TempXml->uId_Validate.wDevId, TempXml->uId_Validate.wYkOrder, TempXml->uId_Validate.bYkValue);
					SetTTimer(XML_YKTIME, XML_START_YK);
				}
				else
					SetCommand(CMD_YK_ERROR);
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xmlFreeDoc(xDoc);
			}
			else
			{
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xmlFreeDoc(xDoc);
			}
		}
		else
		{
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
		}
	}
	else if (memcmp(szTempType, "DZ", strlen((char *)"DZ")) == 0)
	{

		// memset(TempXml, 0, sizeof(XML));
		memcpy(TempXml->bType, szTempType, strlen(szTempType));
		xPath = (xmlChar *)"/root/common/fac_no";

		xmlXPathFreeObject(xApp_Result);
		xApp_Result = GetNodeSet(xDoc, xPath);
		xNodeSet = xApp_Result->nodesetval;
		xCur = xNodeSet->nodeTab[0];
		xValue = xmlNodeGetContent(xCur);
		TempXml->uId_Validate.wFacNo = (WORD)atoi((char *)xValue);

		// printf("----FUNC = %s LINE = %d  FacNo = %d read DZ ----\n", __func__, __LINE__, TempXml->uId_Validate.wFacNo);
		xmlFree(xValue);
		if (m_pMethod->m_pRtuObj->m_wDevAddr == TempXml->uId_Validate.wFacNo)
		{
			xPath = (xmlChar *)"/root/common/quest_id";
			xmlXPathFreeObject(xApp_Result);
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];
			xValue = xmlNodeGetContent(xCur);
			memcpy(TempXml->uId_Validate.bSequence, xValue, strlen((char *)xValue));
			xmlFree(xValue);
			xPath = (xmlChar *)"/root/data";
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];
			xValue = xmlGetProp(xCur, (const xmlChar *)"operation");

			BYTE byBusNo = 0;
			int iDzDataNum = 0;
			int Datamod;
			if (memcmp((char *)xValue, "read", strlen((char *)"read")) == 0)
			{
				xPath = (xmlChar *)"/root/data/DZ";
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xApp_Result = GetNodeSet(xDoc, xPath);
				xNodeSet = xApp_Result->nodesetval;
				xCur = xNodeSet->nodeTab[0];
				xValue = xmlGetProp(xCur, (const xmlChar *)"dev_id");
				TempXml->uId_Validate.wDevId = (WORD)atoi((char *)xValue);
				// printf("----FUNC = %s LINE = %d  dev_id = %d read DZ ----\n", __func__, __LINE__, TempXml->uId_Validate.wDevId);
				m_wDZRecvAddr = atoi((char *)xValue);
				m_wDzDevAddr = GetDevAddrFromDev_id(m_wDZRecvAddr);
				m_wDzRecvName = GetDevNameFromDev_id(m_wDZRecvAddr);

				if (m_wDzDevAddr == 0xFFFF || m_wDzRecvName == 0xFFFF)
				{
					// printf("----FUNC = %s LINE = %d  can't find dev_id = %d ----\n", __func__, __LINE__, TempXml->uId_Validate.wDevId);
					xmlFree(xValue);
					xmlXPathFreeObject(xApp_Result);
					xmlFreeDoc(xDoc);
					return FALSE;
				}
				// printf("m_wDZRecvAddr =%d m_wDzDevAddr=%d\n", m_wDZRecvAddr, m_wDzDevAddr);
				/*byBusNo = mapDevId_Com[m_wDzDevAddr];*/
				byBusNo = mapDevId_Com[m_wDzRecvName];

				xmlFree(xValue);
				xValue = xmlGetProp(xCur, (const xmlChar *)"dz_num");
				iDzDataNum = (int)atoi((char *)xValue);
				xmlFree(xValue);
				/********************************************************/
				xValue = xmlGetProp(xCur, (const xmlChar *)"mod");
				Datamod = (int)atoi((char *)xValue);
				xmlFree(xValue);
				if (Datamod == 1)
				{
					xValue = xmlGetProp(xCur, (const xmlChar *)"usergroup");
					TempXml->uId_Validate.byDzZoneNo = (BYTE)atoi((char *)xValue);
					xmlFree(xValue);
					modflag = 1;
					// printf("*********%d %s****** byBusNo=%d ** m_wDzDevAddr=%d****\n", __LINE__, __FILE__, byBusNo, m_wDzDevAddr);
					m_pMethod->SetDzCall(this, byBusNo, m_wDzDevAddr, TempXml->uId_Validate.byDzZoneNo, (DZ_DATA *)NULL, iDzDataNum);
				}
				else
				{
					modflag = 2;
					xValue = xmlGetProp(xCur, (const xmlChar *)"start_order");
					TempXml->uId_Validate.byDZStartOrder = (BYTE)atoi((char *)xValue);
					xmlFree(xValue);
					// printf("************%d start_order*****\n", TempXml->uId_Validate.byDZStartOrder);
					// printf("*********%d %s****** byBusNo=%d ** m_wDzDevAddr=%d****\n", __LINE__, __FILE__, byBusNo, m_wDzDevAddr);
					m_pMethod->SetDzCall_By_StartOrder(this, byBusNo, m_wDzDevAddr, TempXml->uId_Validate.byDZStartOrder, (DZ_DATA *)NULL, iDzDataNum);
					//	printf("*********%d %s****** byBusNo=%d ** m_wDzDevAddr=%d****\n", __LINE__, __FILE__, byBusNo, m_wDzDevAddr);
				}
				/*************************************************************/
				SetTTimer(XML_DZTIME, XML_START_DZ);
				// printf("----FUNC = %s LINE = %d  devAddr = %d read DZ ----\n", __func__, __LINE__, m_wDzDevAddr);
				xmlXPathFreeObject(xApp_Result);
				xmlFreeDoc(xDoc);
			}
			else if (memcmp((char *)xValue, "write", strlen((char *)"write")) == 0)
			{

				xPath = (xmlChar *)"/root/data";
				xmlFree(xValue);
				xApp_Result = GetNodeSet(xDoc, xPath);
				xNodeSet = xApp_Result->nodesetval;
				xCur = xNodeSet->nodeTab[0];
				xValue = xmlGetProp(xCur, (const xmlChar *)"mod");
				if (atoi((char *)xValue) == 1)
				{
					modflag = 1;
					// xmlFree(xValue);
				}
				else if (atoi((char *)xValue) == 2)
				{
					modflag = 2;
					// xmlFree(xValue);
				}
				else
				{
					// xmlFree(xValue);
					return FALSE;
				}
				xPath = (xmlChar *)"/root/data/device";
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xApp_Result = GetNodeSet(xDoc, xPath);
				xNodeSet = xApp_Result->nodesetval;
				xCur = xNodeSet->nodeTab[0];
				xValue = xmlGetProp(xCur, (const xmlChar *)"id");
				TempXml->uId_Validate.wDevId = (WORD)atoi((char *)xValue);
				m_wDzDevAddr = GetDevAddrFromDev_id(atoi((char *)xValue));
				m_wDzRecvName = GetDevNameFromDev_id(atoi((char *)xValue)); /* GetDevAddrFromDev_id(atoi((char *)xValue));*/
				printf("----FUNC = %s LINE = %d  dev_id = %d write DZ ----\n", __func__, __LINE__, TempXml->uId_Validate.wDevId);
				if (m_wDzDevAddr == 0xFFFF || m_wDzRecvName == 0xFFFF)
				{
					// printf("----FUNC = %s LINE = %d  can't find dev_id = %d ----\n", __func__, __LINE__, TempXml->uId_Validate.wDevId);
					xmlFree(xValue);
					xmlXPathFreeObject(xApp_Result);
					xmlFreeDoc(xDoc);
					return FALSE;
				}
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				// byBusNo = mapDevId_Com[m_wDzDevAddr];
				byBusNo = mapDevId_Com[m_wDzRecvName];
				// xValue = xmlGetProp(xCur, (const xmlChar *)"dz_num");
				printf("*********%d %s****** byBusNo=%d ** m_wDzDevAddr=%d****\n", __LINE__, __FILE__, byBusNo, m_wDzDevAddr);
				/*BYTE bDzNum = (BYTE)atoi((char *)xValue);*/

				/*xmlFree(xValue);*/
				DWORD bDzNum = 300;
				DZ_DATA DzData[bDzNum];
				BYTE bDzZoneNo = 0;
				BYTE bDzVal = 0;
				short int sDzVal = 0;
				UINT uiDzVal = 0;
				float fDzVal = 0;
				char cPath[30] = {"\0"};
				printf("*********%d %s*****bDzNum=%d*******\n", __LINE__, __FILE__, bDzNum);
				for (int i = 0; i < bDzNum; i++)
				{
					memset(cPath, 0, 30);
					printf("*********%d %s************\n", __LINE__, __FILE__);
					sprintf(cPath, "%s", (char *)"/root/data/device/value");
					xPath = (xmlChar *)cPath;
					printf("*********%d %s************\n", __LINE__, __FILE__);
					xApp_Result = GetNodeSet(xDoc, xPath);
					if (xApp_Result == NULL)
						break;
					printf("*********%d %s************\n", __LINE__, __FILE__);
					xNodeSet = xApp_Result->nodesetval;
					/*if (xmlXPathNodeSetlsEmpty(xNodeSet))
					{
					xmlXPathFreeObject(xApp_Result);
					break;
					}*/
					xCur = xNodeSet->nodeTab[i];
					if (xCur == NULL)
						break;
					printf("----modflag=%d----\n", modflag);
					if (modflag == 1)
					{
						xValue = xmlGetProp(xCur, (const xmlChar *)"usergroup");
						bDzZoneNo = (BYTE)atoi((char *)xValue);
						xmlFree(xValue);
						xValue = xmlGetProp(xCur, (const xmlChar *)"dz_no");
						DzData[i].wPnt = (WORD)atoi((char *)xValue);
						DzData[i].byType = m_byDzType[i];
						xmlFree(xValue);
					}
					else if (modflag == 2)
					{
						xValue = xmlGetProp(xCur, (const xmlChar *)"order");
						DzData[i].wPnt = (WORD)atoi((char *)xValue);
						DzData[i].byType = m_byDzType[i];
						xmlFree(xValue);
						bDzZoneNo = 0;
					}
					else
					{
						return FALSE;
					}
					printf("*********%d %s*******%d*****\n", __LINE__, __FILE__, modflag);
					xValue = xmlNodeGetContent(xCur);
					printf("----FUNC = %s LINE = %d xValue = %s--type=%d--\n", __func__, __LINE__, (char *)xValue, m_byDzType[i]);
					switch (m_byDzType[i])
					{
					case 0:
						bDzVal = (BYTE)atoi((char *)xValue);
						memcpy(DzData[i].byVal, &bDzVal, 1);
						break;
					case 1:
						sDzVal = (short int)atoi((char *)xValue);
						memcpy(DzData[i].byVal, &sDzVal, 2);
						break;
					case 2:
						uiDzVal = (UINT)atoi((char *)xValue);
						memcpy(DzData[i].byVal, &uiDzVal, 4);
						break;
					case 3:
						fDzVal = (float)atoi((char *)xValue);
						memcpy(DzData[i].byVal, &fDzVal, 4);
						break;
					}
					xCur = xCur->xmlChildrenNode;
					xmlFree(xValue);
					xmlXPathFreeObject(xApp_Result);
					iDzDataNum++;
				}
				printf("*********%d %s***byBusNo=%d **m_wDzDevAddr=%d*** bDzZoneNo=%d**iDzDataNum=%d**\n", __LINE__, __FILE__, byBusNo, m_wDzDevAddr, bDzZoneNo, iDzDataNum);
				m_pMethod->SetDzWriteExct(this, byBusNo, m_wDzDevAddr, bDzZoneNo, (DZ_DATA *)DzData, iDzDataNum);
				printf("*********%d %s************\n", __LINE__, __FILE__);
				SetTTimer(XML_DZTIME, XML_START_DZ);
				xmlFreeDoc(xDoc);
				modflag = 1;
			}
			else
			{
				xmlFree(xValue);
				xmlXPathFreeObject(xApp_Result);
				xmlFreeDoc(xDoc);
			}
		}
		else
		{
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
		}
	}
	else if (memcmp(szTempType, "confirm", strlen((char *)"confirm")) == 0)
	{
		// memset(TempXml, 0, sizeof(XML));
		memcpy(TempXml->bType, szTempType, strlen(szTempType));
		xPath = (xmlChar *)"/root/common/quest_id";

		xmlXPathFreeObject(xApp_Result);
		xApp_Result = GetNodeSet(xDoc, xPath);
		xNodeSet = xApp_Result->nodesetval;
		xCur = xNodeSet->nodeTab[0];
		xValue = xmlNodeGetContent(xCur);
		if (!memcmp(xValue, m_bDITimeOutSequence_YX, sizeof(m_bDITimeOutSequence_YX)))
		{
			SetTTimer(XML_T3, XML_END_T3);
			m_byDITimeOut = 0;
			xmlFree(xValue);
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
		}
		else
		{
			xmlFree(xValue);
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
		}
	}
	else if (memcmp(szTempType, "time", strlen((char *)"time")) == 0)
	{
		// memset(TempXml, 0, sizeof(XML));
		memcpy(TempXml->bType, szTempType, strlen(szTempType));
		xPath = (xmlChar *)"/root/common/fac_no";

		// printf("---FUNC = %s LINE = %d ----\n", __func__, __LINE__);
		xmlXPathFreeObject(xApp_Result);
		xApp_Result = GetNodeSet(xDoc, xPath);
		xNodeSet = xApp_Result->nodesetval;
		xCur = xNodeSet->nodeTab[0];
		xValue = xmlNodeGetContent(xCur);
		TempXml->uId_Validate.wFacNo = (WORD)atoi((char *)xValue);

		// printf("----FUNC = %s LINE = %d time = %s----\n", __func__, __LINE__, "time");

		if (m_pMethod->m_pRtuObj->m_wDevAddr == TempXml->uId_Validate.wFacNo)
		{
			xPath = (xmlChar *)"/root/data/time";

			xmlFree(xValue);
			xmlXPathFreeObject(xApp_Result);
			xApp_Result = GetNodeSet(xDoc, xPath);
			xNodeSet = xApp_Result->nodesetval;
			xCur = xNodeSet->nodeTab[0];
			xValue = xmlNodeGetContent(xCur);

			// printf("----FUNC = %s LINE = %d time = %s recvClockFlag = %d ----\n", __func__, __LINE__, (char *)xValue, m_wRecvClock);

			if (m_wRecvClock)
				SysClockProc((BYTE *)xValue);

			xmlFree(xValue);
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
		}
		else
		{
			xmlFree(xValue);
			xmlXPathFreeObject(xApp_Result);
			xmlFreeDoc(xDoc);
		}
	}
	else if (memcmp(szTempType, "ESDSetting", strlen((char *)"ESDSetting")) == 0)
	{
		xmlXPathFreeObject(xApp_Result);
		m_payment_info.removeAll();

		// printf("*********%d %s************\n", __LINE__, __FILE__);
		xPath = (xmlChar *)"/root/data/devcount";
		GetXMLPathDataToPaymentInfo(xDoc, (char *)xPath, m_payment_info);

		xPath = (xmlChar *)"/root/data/dev";
		GetXMLPathDataToPaymentInfo(xDoc, (char *)xPath, m_payment_info);

		xPath = (xmlChar *)"/root/data/textbody";
		GetXMLPathDataToPaymentInfo(xDoc, (char *)xPath, m_payment_info);

		xPath = (xmlChar *)"/root/common/quest_id";
		GetXMLPathDataToPaymentInfo(xDoc, (char *)xPath, m_payment_info);

		xmlFreeDoc(xDoc);

		sendUnvarnishCmd(m_payment_info);
		// printf("xml ESDSetting\n");
	}

	return TRUE;
}

void MonitoringPlatformXML::sendUnvarnishCmd(paymentInfo &payment_info)
{
	int size = payment_info.wDevCount;
	for (int i = 0; i < size; i++)
	{
		WORD wData = GetDevAddrAndBusFromDev_id(payment_info.dev_addr[i]);
		BYTE byBus = HIBYTE(wData);
		BYTE byAddr = LOBYTE(wData);
		WORD len = payment_info.dev_cmd_len;

		memcpy(&payment_info.dev_cmd[len], payment_info.quest_id, 10);

		m_pMethod->Unvarnished(this, byBus, byAddr, payment_info.dev_cmd, len + 11, VARNISH_CALL);
	}
}

void MonitoringPlatformXML::GetXMLPathDataToPaymentInfo(xmlDocPtr xDoc, char *pPath, paymentInfo &info)
{
	xmlNodePtr xCur;
	xmlXPathObjectPtr xApp_Result = GetNodeSet(xDoc, (xmlChar *)pPath);
	xmlNodeSetPtr xNodeSet = xApp_Result->nodesetval;
	xCur = xNodeSet->nodeTab[0];
	xmlChar *xValue = xmlNodeGetContent(xCur);
	if (strcmp("/root/data/devcount", pPath) == 0)
		info.wDevCount = atoi((char *)xValue);
	else if (strcmp("/root/data/dev", pPath) == 0)
	{
		// printf("*********%d %s************\n", __LINE__, __FILE__);
		BYTE index = 0;
		char *pch = NULL;
		pch = strtok((char *)xValue, "_");
		while (pch != NULL)
		{
			info.dev_addr[index++] = (WORD)atoi(pch);
			pch = strtok(NULL, "_");
		}
	}
	else if (strcmp("/root/data/textbody", pPath) == 0)
	{
		// printf("*********%d %s************\n", __LINE__, __FILE__);
		WORD index = 0;
		char *pch = NULL;
		pch = strtok((char *)xValue, " ");
		while (pch != NULL)
		{
			info.dev_cmd[index++] = convertToHex(pch, 2);

			pch = strtok(NULL, " ");
		}
		info.dev_cmd_len = index;
	}
	else if (strcmp("/root/common/quest_id", pPath) == 0)
	{
		// printf("*********%d %s************\n", __LINE__, __FILE__);
		char *pData = (char *)xValue;
		int index = 0;
		while (pData[index])
		{
			info.quest_id[index] = pData[index];
			index++;
		}
	}
	xmlFree(xValue);
	xmlXPathFreeObject(xApp_Result);
}

WORD MonitoringPlatformXML::convertToHex(char *pData, BYTE size)
{
	if (pData == NULL)
		return 0;

	pData[0] = tolower(pData[0]);
	pData[1] = tolower(pData[1]);
	WORD wData1 = 0, wData2 = 0, wData = 0;
	if (pData[0] >= '0' && pData[0] <= '9')
		wData1 = pData[0] - '0';
	else if (pData[0] >= 'a' && pData[0] <= 'f')
		wData1 = pData[0] - 'a' + 10;

	if (pData[1] >= '0' && pData[1] <= '9')
		wData2 = pData[1] - '0';
	else if (pData[1] >= 'a' && pData[1] <= 'f')
		wData2 = pData[1] - 'a' + 10;

	wData = wData1 * 16 + wData2;
	return wData;
}

WORD MonitoringPlatformXML::GetDevAddrFromDev_id(DWORD wDevId)
{
	CBusManger *pBusManager = m_pMethod->m_pBusManager;
	if (!pBusManager)
		return -1;

	BUSARRAY &bArray = pBusManager->m_sbus;
	int sz = bArray.size();
	for (int i = 0; i < sz; i++)
	{
		PBUSMANAGER pBusManager = bArray[i];
		if (!pBusManager || !pBusManager->m_Protocol)
			continue;
		if (pBusManager->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)
			continue;
		CPROTO_ARRAY &pProtoArray = pBusManager->m_Protocol->m_module;
		int protoSize = pProtoArray.size();
		for (int m = 0; m < protoSize; m++)
		{
			CProtocol *pProto = pProtoArray[m];
			if (!pProto)
				continue;
			if (atoi(pProto->m_sDevName) == wDevId)
				return pProto->m_wDevAddr;
		}
	}
	return 0xFFFF;
}
WORD MonitoringPlatformXML::GetDevNameFromDev_id(DWORD wDevId)
{
	CBusManger *pBusManager = m_pMethod->m_pBusManager;
	if (!pBusManager)
		return -1;

	BUSARRAY &bArray = pBusManager->m_sbus;
	int sz = bArray.size();
	for (int i = 0; i < sz; i++)
	{
		PBUSMANAGER pBusManager = bArray[i];
		if (!pBusManager || !pBusManager->m_Protocol)
			continue;
		if (pBusManager->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)
			continue;
		CPROTO_ARRAY &pProtoArray = pBusManager->m_Protocol->m_module;
		int protoSize = pProtoArray.size();
		for (int m = 0; m < protoSize; m++)
		{
			CProtocol *pProto = pProtoArray[m];
			// printf("------------------%d------------\n", atoi(pProto->m_sDevName));
			if (!pProto)
				continue;
			if (atoi(pProto->m_sDevName) == wDevId)
				return atoi(pProto->m_sDevName);
		}
	}
	return 0xFFFF;
}
WORD MonitoringPlatformXML::GetDevAddrAndBusFromDev_id(DWORD wDevId)
{
	CBusManger *pBusManager = m_pMethod->m_pBusManager;
	if (!pBusManager)
		return -1;

	BUSARRAY &bArray = pBusManager->m_sbus;
	int sz = bArray.size();
	for (int i = 0; i < sz; i++)
	{
		PBUSMANAGER pBusManager = bArray[i];
		if (!pBusManager || !pBusManager->m_Protocol)
			continue;
		if (pBusManager->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)
			continue;
		CPROTO_ARRAY &pProtoArray = pBusManager->m_Protocol->m_module;
		int protoSize = pProtoArray.size();
		for (int m = 0; m < protoSize; m++)
		{
			CProtocol *pProto = pProtoArray[m];
			if (!pProto)
				continue;
			if (atoi(pProto->m_sDevName) == wDevId)
				return MAKEWORD(pProto->m_wDevAddr, pProto->m_byLineNo);
		}
	}
	return 0xFFFF;
}

WORD MonitoringPlatformXML::GetDevidFromBusnoAndDevNo(BYTE byBusNo, WORD wDevAddr)
{
	CBusManger *pCBusManager = m_pMethod->m_pBusManager;
	if (!pCBusManager)
		return -1;

	BUSARRAY &bArray = pCBusManager->m_sbus;
	int sz = bArray.size();

	PBUSMANAGER pBusManager = bArray[byBusNo];
	if (!pBusManager || !pBusManager->m_Protocol)
		return -1;
	if (pBusManager->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)
		return -1;

	CPROTO_ARRAY &pProtoArray = pBusManager->m_Protocol->m_module;
	int protoSize = pProtoArray.size();
	for (int m = 0; m < protoSize; m++)
	{
		CProtocol *pProto = pProtoArray[m];
		if (!pProto)
			continue;

		if (pProto->m_wDevAddr == wDevAddr && pProto->m_byLineNo == byBusNo)
			return atoi(pProto->m_sDevName);
	}
}

// --------------------------------------------------------
/// \概要:	同步系统时间
///
/// \参数:	pDataBuf
// --------------------------------------------------------
void MonitoringPlatformXML::SysClockProc(BYTE *pDataBuf)
{
	REALTIME t;
	int pos = 0;
	char *str = NULL;
	char *pchStrTmpIn = NULL;

	str = strtok_r((char *)pDataBuf, "-", &pchStrTmpIn);
	mapTime[pos++] = str;
	while (str != NULL)
	{
		str = strtok_r(NULL, "-", &pchStrTmpIn);
		mapTime[pos++] = str;
		// printf("----FUNC = %s LINE = %d str = %s----\n", __func__, __LINE__, str);
	}
	pos = 0;

	t.wYear = (unsigned short)atoi(mapTime[pos++]);
	t.wMonth = (unsigned short)atoi(mapTime[pos++]);
	t.wDay = (unsigned short)atoi(mapTime[pos++]);
	t.wHour = (unsigned short)atoi(mapTime[pos++]);
	t.wMinute = (unsigned short)atoi(mapTime[pos++]);
	t.wSecond = (unsigned short)atoi(mapTime[pos++]);
	t.wMilliSec = (unsigned short)atoi(mapTime[pos++]);

	// printf("----FUNC = %s LINE = %d %04d-%02d-%02d-%02d-%02d-%02d-%03d----\n", __func__, __LINE__, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliSec);
	SetCurrentTime(&t);
	// system("hwclock -w -u");
	m_DevStateTime = 0;

	/* 初始化上传变化装置状态时间 */
	m_VaryDevStateTime = 0;

	/* 初始化实时上传遥信遥测遥脉时间 */
	m_YxValueTime = 0;
	m_YcValueTime = 0;
	m_YmValueTime = 0;
}

// --------------------------------------------------------
/// \概要:	将变位遥信写入队列
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wVal
///  wNum:全局顺序号
//    m_pwDITrans[wNum] 或得转发序号
//    m_byDIBuf[wTransNo]  wTransNo从转发序号0开始获取数据
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::WriteDIVal(WORD wSerialNo, WORD wNum, WORD wVal)
{
	// printf("m_pwDITrans[%d] = %d \n", wNum , m_pwDITrans[wNum]);
	if (m_pwDITrans[wNum] == 0xFFFF)
		return FALSE;
	WORD wTransNo = m_pwDITrans[wNum];

	WORD wPnt = GetDevPntFromTrans(YX_TRANSTOSERIALNO, wTransNo);
	// printf("wPnt = %d wTransNo = %d\n", wPnt,wTransNo );
	if (m_pwDITrans == NULL || wPnt == 0xFFFF)
		return FALSE;

	// WORD wNum1 = m_pwDITrans[wPnt] & 0x7FFF;
	//	printf("wNum = %d m_wDISum = %d\n", wNum, m_wDISum);
	if (wNum > m_wDISum)
		return FALSE;

	if (wNum < XMLMAX_DI_LEN)
	{
		if (m_byDIBuf[wTransNo] != wVal)
		{
			m_byDIBuf[wTransNo] = wVal;
			AddDigitalEvt_Xml(wSerialNo, wPnt, wNum, wVal);
		}
	}
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	将变位遥测输入队列
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	fVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::WriteAIVal(WORD wSerialNo, WORD wNum, float fVal)
{
	if (m_pwAITrans == NULL || m_pwAITrans[wNum] == 0xFFFF)
		return FALSE;

	WORD wNumTrans = m_pwAITrans[wNum];

	WORD wPnt = GetDevPntFromTrans(YC_TRANSTOSERIALNO, wNumTrans);
	if (wPnt == 0xFFFF)
		return FALSE;

	// printf("---FUNC = %s LINE = %d  serialno = %d wNum = %d val = %f wNumTrans = %d wPnt = %d----\n", __func__, __LINE__, wSerialNo, wNum, fVal , wNumTrans , wPnt );

	if (wNumTrans > m_wAISum)
		return FALSE;

	if (wNum < XMLMAX_AI_LEN)
	{
		float nDelt = fVal - m_wAIBuf[wNumTrans];
		if (abs((int)nDelt) >= m_wDeadVal)
		{
			// printf("---FUNC = %s LINE = %d nDelt = %d fVal = %f m_wAIBuf[wNum] = %f----\n", __func__, __LINE__, abs((int)nDelt), fVal, m_wAIBuf[wNum]);
			m_wAIBuf[wNumTrans] = fVal;
			/*lel*/
#if 0
			AddAnalogEvt(wSerialNo, wNum, fVal);
#else
			AddAnalogEvt_Xml(wSerialNo, wPnt, wNum, fVal);
#endif
			/*end*/
		}
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	将变位遥脉输入队列
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	dwVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::WritePIVal(WORD wSerialNo, WORD wPnt, QWORD dwVal)
{
	if (m_pwPITrans == NULL || wPnt == 0xFFFF)
		return FALSE;

	WORD wNum = m_pwPITrans[wPnt];

	if (wNum > m_wPISum || wNum == 0xFFFF)
		return FALSE;

	if (wNum < XMLMAX_PI_LEN)
	{
		m_dwPIBuf[wNum] = dwVal;
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	将SOE输入队列
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wVal
/// \参数:	lTime
/// \参数:	wMiSecond
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::WriteSOEInfo(WORD wSerialNo, WORD wNum, WORD wVal, LONG lTime, WORD wMiSecond)
{
	if (m_pwDITrans[wNum] == 0xFFFF)
		return FALSE;

	WORD wPnt = GetDevPntFromTrans(YX_TRANSTOSERIALNO, wNum);
	if (m_pwDITrans == NULL || wPnt == 0xFFFF)
		return FALSE;

	WORD wNum1 = m_pwDITrans[wPnt] & 0x7FFF;

	if (wNum > m_wDISum)
		return FALSE;

	if (wNum < XMLMAX_DI_LEN)
	{
		AddSOEInfo_Xml(wSerialNo, wPnt, wNum1, wVal, lTime, wMiSecond);
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	拆分字符串，第一个是设备ID，第二个是顺序号
///
/// \参数:	szBuf
// --------------------------------------------------------
void MonitoringPlatformXML::CharacterSplit(Xml *TempXml, char *szBuf)
{
	char *szTok = NULL;
	//	printf("----FUNC= %s LINE = %d szBuf = %s----\n", __func__, __LINE__, szBuf);
	if (strchr(szBuf, '_'))
	{
		szTok = strtok(szBuf, "_");
		TempXml->uId_Validate.wDevId = (DWORD)atoi(szTok);
		szTok = strtok(NULL, "_");
		TempXml->uId_Validate.wYkOrder = (WORD)atoi(szTok);
	}
}

// --------------------------------------------------------
/// \概要:	遥控预置
///
/// \参数:	wStn
/// \参数:	wDevId
/// \参数:	wPnt
/// \参数:	byStatus
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::RelaySelectProc(WORD wStn, WORD wDevId, WORD wPnt, BYTE byStatus)
{
	BYTE byBusNo;
	WORD wDevAddr;

	if (m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkSel(this, byBusNo, wDevAddr, wPnt, byStatus);
	else
		printf("----FUNC = %s LINE = %d Xml RelaySelect is failed----\n", __func__, __LINE__);

	//	printf("----FUNC = %s LINE = %d wStn = %d wDevId = %d wPnt = %d byStatus = %d byBusNo = %d wDevAddr = %d----\n", __func__, __LINE__, wStn, wDevId, wPnt, byStatus, byBusNo, wDevAddr);
	return 0;
}

// --------------------------------------------------------
/// \概要:	遥控执行
///
/// \参数:	wStn
/// \参数:	wDevId
/// \参数:	wPnt
/// \参数:	byStatus
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::RelayExecuteProc(WORD wStn, WORD wDevId, WORD wPnt, BYTE byStatus)
{
	BYTE byBusNo;
	WORD wDevAddr;

	if (m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkExe(this, byBusNo, wDevAddr, wPnt, byStatus);
	else
		printf("----FUNC = %s LINE = %d Xml RelayExecute is failed----\n", __func__, __LINE__);

	//	printf("----FUNC = %s LINE = %d wStn = %d wDevId = %d wPnt = %d byStatus = %d byBusNo = %d wDevAddr = %d----\n", __func__, __LINE__, wStn, wDevId, wPnt, byStatus, byBusNo, wDevAddr);
	return 0;
}

// --------------------------------------------------------
/// \概要:	遥控取消
///
/// \参数:	wStn
/// \参数:	wDevId
/// \参数:	wPnt
/// \参数:	byStatus
///
/// \返回:	int
// --------------------------------------------------------
int MonitoringPlatformXML::RelayCancelProc(WORD wStn, WORD wDevId, WORD wPnt, BYTE byStatus)
{
	BYTE byBusNo;
	WORD wDevAddr;

	if (m_pMethod->GetBusLineAndAddr(wStn, byBusNo, wDevAddr))
		m_pMethod->SetYkCancel(this, byBusNo, wDevAddr, wPnt, byStatus);
	else
		printf("----FUNC = %s LINE = %d Xml RelayCancel is failed----\n", __func__, __LINE__);

	return 0;
}

// --------------------------------------------------------
/// \概要:	判断是否可以进行遥控设置
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::Yk_IsCanSend()
{
	return m_YkFlag;
}

// --------------------------------------------------------
/// \概要:	遥控设置
///
/// \参数:	bPreSetFlag
// --------------------------------------------------------
void MonitoringPlatformXML::Yk_PreSet(BOOL bPreSetFlag)
{
	m_YkFlag = bPreSetFlag;
}

// --------------------------------------------------------
/// \概要:	遥控回复应答
///
/// \参数:	bFlag
// --------------------------------------------------------
void MonitoringPlatformXML::Yk_RtnConfirm(BOOL bFlag)
{
	Yk_PreSet(bFlag);
	SetTTimer(XML_YKTIME, XML_END_YK);
}

// --------------------------------------------------------
/// \概要:	判断遥控返回状态
///
/// \参数:	byCommand
/// \参数:	wIndex
/// \参数:	byResult
// --------------------------------------------------------
void MonitoringPlatformXML::RelayEchoProc(BYTE byCommand, WORD wIndex, BYTE byResult)
{
	//	printf("----FUNC = %s LINE = %d RelayEchoProc Runing----\n", __func__, __LINE__);
	switch (byCommand)
	{
	case YK_SEL_RTN:
		m_wYkFlag = YK_SEL_RTN;
		m_wCommand |= CMD_COA_BIT;
		break;
	case YK_EXCT_RTN:
		m_wYkFlag = YK_EXCT_RTN;
		m_wCommand |= CMD_COA_BIT;
		break;
	case YK_CANCEL_RTN:
		m_wYkFlag = YK_CANCEL_RTN;
		m_wCommand |= CMD_COA_BIT;
		break;
	default:
		break;
	}
}

// --------------------------------------------------------
/// \概要:	判断定值返回状态
///
/// \参数:	byCommand
/// \参数:	wIndex
/// \参数:	byResult
// --------------------------------------------------------
void MonitoringPlatformXML::RelayDzEchoProc(BYTE byCommand, WORD wIndex, BYTE *byResult)
{
	// printf("----FUNC = %s LINE = %d RelayDzEchoProc Runing----\n", __func__, __LINE__);
	switch (byCommand)
	{
	case DZ_CALL_RTN:
		m_wDzFlag = DZ_CALL_RTN;
		m_wCommand |= CMD_DZ_BIT;
		break;
	case DZ_WRITE_EXCT_RTN:
		m_wDzFlag = DZ_WRITE_EXCT_RTN;
		m_wCommand |= CMD_DZ_BIT;
		break;
	default:
		break;
	}
}

// --------------------------------------------------------
/// \概要:	将遥信插入容器
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wNum
/// \参数:	wVal
// --------------------------------------------------------
void MonitoringPlatformXML::AddDigitalEvt_Xml(WORD wSerialNo, WORD wPnt, WORD wNum, WORD wVal)
{ /*{{{*/
	if ((wSerialNo >= m_pMethod->GetGatherDevCount()) || (wPnt > 4095))
	{
		return;
	}
	pthread_mutex_lock(&mutex_yx);
	if (m_dwDIEQueue_Xml.size() > RTUMAX_DIE_LEN)
		m_dwDIEQueue_Xml.pop_front();

	SETDATA_XML data;
	memset(&data, 0, sizeof(SETDATA_XML));

	data.wPnt = wPnt;
	data.wNum = wNum;
	data.wVal = wVal;
	data.wSerialNo = wSerialNo;
	m_dwDIEQueue_Xml.push_back(data);

	pthread_mutex_unlock(&mutex_yx);
}

// --------------------------------------------------------
/// \概要:	获得容器遥信
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wNum
/// \参数:	wVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetDigitalEvt_Xml(WORD &wSerialNo, WORD &wPnt, WORD &wNum, WORD &wVal)
{ /*{{{*/
	// m_semDIEMutex.semTake();
	pthread_mutex_lock(&mutex_yx);

	if (m_dwDIEQueue_Xml.empty())
	{
		pthread_mutex_unlock(&mutex_yx);
		return FALSE;
	}

	SETDATA_XML data = m_dwDIEQueue_Xml.front();
	if ((data.wSerialNo >= m_pMethod->GetGatherDevCount()) || (data.wPnt > 4095))
	{
		pthread_mutex_unlock(&mutex_yx);
		return FALSE;
	}
	printf("--- du qu  m_dwDIEQueue_Xmlsize=%d serialNo = %d wPnt=%d wVal = %d wNum= %d\n", m_dwDIEQueue_Xml.size(), data.wSerialNo, data.wPnt, data.wVal, data.wNum);
	wSerialNo = data.wSerialNo;
	wVal = data.wVal;
	wPnt = data.wPnt;
	wNum = data.wNum;

	m_dwDIEQueue_Xml.pop_front();
	// m_semDIEMutex.semGive();
	pthread_mutex_unlock(&mutex_yx);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	将遥测插入容器
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wNum
/// \参数:	fVal
// --------------------------------------------------------
void MonitoringPlatformXML::AddAnalogEvt_Xml(WORD wSerialNo, WORD wPnt, WORD wNum, float fVal)
{
	if ((wSerialNo >= m_pMethod->GetGatherDevCount()) || (wPnt > 4095))
	{
		// printf("\nserialno:%d	pnt:%d  line:%d file=%s \n", wSerialNo, wPnt, __LINE__, __func__ );
		return;
	}
	pthread_mutex_lock(&mutex_yc);
	// m_semAIEMutex.semTake();

	if (m_dwAIEQueue_Xml.size() > RTUMAX_AIE_LEN)
		m_dwAIEQueue_Xml.pop_front();
	SETDATA_XML data;
	data.wPnt = wPnt;
	data.wNum = wNum;
	data.fVal = fVal;
	data.wSerialNo = wSerialNo;
	m_dwAIEQueue_Xml.push_back(data);

	pthread_mutex_unlock(&mutex_yc);
	// m_semAIEMutex.semGive();
}

// --------------------------------------------------------
/// \概要:	获得容器遥测
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wNum
/// \参数:	fVal
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetAnalogEvt_Xml(WORD &wSerialNo, WORD &wPnt, WORD &wNum, float &fVal)
{ /*{{{*/
	pthread_mutex_lock(&mutex_yc);
	// m_semAIEMutex.semTake();
	if (m_dwAIEQueue_Xml.empty())
	{
		pthread_mutex_unlock(&mutex_yc);
		// m_semAIEMutex.semGive();
		return FALSE;
	}

	SETDATA_XML data = m_dwAIEQueue_Xml.front();
	if ((data.wSerialNo >= m_pMethod->GetGatherDevCount()) || (data.wPnt > 4095))
	{
		// printf("\nserialno:%d	pnt:%d  line:%d\n", data.wSerialNo, data.wPnt, __LINE__);
		pthread_mutex_unlock(&mutex_yc);
		// m_dwAIEQueue.pop_front();
		// m_semAIEMutex.semGive();
		return FALSE;
	}
	wSerialNo = data.wSerialNo;
	wPnt = data.wPnt;
	wNum = data.wNum;
	fVal = data.fVal;

	m_dwAIEQueue_Xml.pop_front();
	pthread_mutex_unlock(&mutex_yc);
	// m_semAIEMutex.semGive();

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	将SOE插入到容器
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wNum
/// \参数:	wVal
/// \参数:	lTime
/// \参数:	wMiSecond
// --------------------------------------------------------
void MonitoringPlatformXML::AddSOEInfo_Xml(WORD wSerialNo, WORD wPnt, WORD wNum, WORD wVal, LONG lTime, WORD wMiSecond)
{ /*{{{*/
	// m_semSOEMutex.semTake();
	pthread_mutex_lock(&mutex_soe);
	m_soeBuffer_Xml[m_iSOE_wr_p].lTime = lTime;
	m_soeBuffer_Xml[m_iSOE_wr_p].wMiSecond = wMiSecond;
	m_soeBuffer_Xml[m_iSOE_wr_p].wPntNum = wPnt;
	m_soeBuffer_Xml[m_iSOE_wr_p].wNum = wNum;
	m_soeBuffer_Xml[m_iSOE_wr_p].wStatus = wVal;
	m_soeBuffer_Xml[m_iSOE_wr_p].wSerialNo = wSerialNo;
	m_iSOE_wr_p = (m_iSOE_wr_p + 1) % RTUMAX_SOE_LEN;
	if (m_iSOE_wr_p == m_iSOE_rd_p)
		m_iSOE_rd_p = (m_iSOE_rd_p + 1) % RTUMAX_SOE_LEN;
	// m_semSOEMutex.semGive();
	pthread_mutex_unlock(&mutex_soe);
}

// --------------------------------------------------------
/// \概要:	获得容器SOE
///
/// \参数:	wSerialNo
/// \参数:	wPnt
/// \参数:	wNum
/// \参数:	wVal
/// \参数:	pTime
/// \参数:	wMiSecond
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL MonitoringPlatformXML::GetSOEInfo_Xml(WORD &wSerialNo, WORD *wPnt, WORD *wNum, WORD *wVal, void *pTime, WORD *wMiSecond)
{ /*{{{*/
	if (m_iSOE_rd_p == m_iSOE_wr_p)
		return FALSE;

	// m_semSOEMutex.semTake();
	pthread_mutex_lock(&mutex_soe);
	GetOwnStructTm(m_soeBuffer_Xml[m_iSOE_rd_p].lTime, (struct tm *)pTime);
	*wMiSecond = m_soeBuffer_Xml[m_iSOE_rd_p].wMiSecond;
	*wPnt = m_soeBuffer_Xml[m_iSOE_rd_p].wPntNum;
	*wNum = m_soeBuffer_Xml[m_iSOE_rd_p].wNum;
	*wVal = m_soeBuffer_Xml[m_iSOE_rd_p].wStatus;
	wSerialNo = m_soeBuffer_Xml[m_iSOE_rd_p].wSerialNo;
	m_iSOE_rd_p = (m_iSOE_rd_p + 1) % RTUMAX_SOE_LEN;
	// m_semSOEMutex.semGive();
	pthread_mutex_unlock(&mutex_soe);
	return TRUE;
}

