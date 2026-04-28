/// \文件:	CProtocol_SPA.cpp
/// \概要:	ABB Spa协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2017-09-25

#include "CProtocol_Spa.h"
#include "Spa.h"

#define MODULE_RTU 1

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CProtocol_SPA::CProtocol_SPA()
{
	memset(m_sTemplatePath, 0, sizeof(m_sTemplatePath));
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CProtocol_SPA::~CProtocol_SPA()
{
	int size = m_module.size();
	for(int i = 0; i < size; i++)
		delete m_module[i];
	m_module.clear();
	printf("Delete All CProtocol_SPA OK.\n");
}

// --------------------------------------------------------
/// \概要:	得到协议报文
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	FALSE
// --------------------------------------------------------
BOOL CProtocol_SPA::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	return FALSE;
}

// --------------------------------------------------------
/// \概要:	处理协议报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	FALSE
// --------------------------------------------------------
BOOL CProtocol_SPA::ProcessProtocolBuf(BYTE *buf, int len)
{
	return FALSE;
}

// --------------------------------------------------------
/// \概要:	初始化
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_SPA::Init(BYTE byLineNo)
{
	/*增加ModBus 采集模块数据*/
	/*通过总线号获取读取的装置文件路径*/
	m_byLineNo = byLineNo;
	/*读取模板文件*/
	m_ProtoType = PROTOCO_GATHER;

	return GetDevData();
}

// --------------------------------------------------------
/// \概要:	打开配置文件(.ini)
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_SPA::GetDevData()
{
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/SPA/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
	CProfile profile(m_sDevPath);

	return ProcessFileData(profile);
}

// --------------------------------------------------------
/// \概要:	处理文件数据
///
/// \参数:	profile
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_SPA::ProcessFileData(CProfile &profile)
{
	BOOL bRtn = FALSE;
	if(!profile.IsValid()){
		printf("Open file %s Failed!\n", profile.m_szFileName);
		return FALSE;
	}

	char sDevnum[50]   = "DEVNUM";
	char sDev[50]      = "DEV";
	char sKey[20][100] = {
		"module", "serialno", "addr", "name", "template"
	};
	int iNum      = 0;
	WORD wModule  = 0;
	int iSerialNo = 0;
	WORD iAddr    = 0;
	char sName[50];
	char sStemplate[50];

	memset(sName, 0, sizeof(sName));
	memset(sStemplate, 0, sizeof(sStemplate));

	if((iNum = profile.GetProfileInt(sDevnum, (char *)"NUM", 0)) == 0){
		printf("Get DEVNUM Failed!!!\n");
		return FALSE;
	}

	for(int i = 0; i < iNum; i++){
		sprintf(sDev, "%s%03d", "DEV", i + 1);

		wModule = profile.GetProfileInt(sDev, sKey[0], 0);
		iSerialNo = profile.GetProfileInt(sDev, sKey[1], 0);
		iAddr = profile.GetProfileInt(sDev, sKey[2], 0);
		profile.GetProfileString(sDev, sKey[3], (char *)"NULL", sName, sizeof(sName));
		profile.GetProfileString(sDev, sKey[4], (char *)"NULL", sStemplate, sizeof(sStemplate));

		/*创建相应模块子类*/
		bRtn = CreateModule(wModule, iSerialNo, iAddr, sName, sStemplate);
		if(!bRtn){
			printf("Create ModBus module = %d serialno = %d addr = %d name = %s stemplate = %s Error!!!\n", wModule, iSerialNo, iAddr, sName, sStemplate);
			return FALSE;
		}
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	创建模块
///
/// \参数:	iModule
/// \参数:	iSerialNo
/// \参数:	wAddr
/// \参数:	sName
/// \参数:	sTemplatePath
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_SPA::CreateModule(int iModule, int iSerialNo, WORD wAddr, char *sName, char *sTemplatePath)
{
	CProtocol_SPA *pProtocol = NULL;
	switch(iModule){
		case MODULE_RTU:
			pProtocol                = new CSPA;
			pProtocol->m_byLineNo    = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr    = wAddr;
			pProtocol->m_SerialNo    = iSerialNo;
			strcpy(pProtocol->m_sTemplatePath, sTemplatePath);
			m_pMethod->m_pRtuObj     = pProtocol;
			pProtocol->m_pMethod     = m_pMethod;
			pProtocol->m_ProtoType   = PROTOCO_GATHER;
			strcpy(pProtocol->m_sDevName, sName);
			/*初始化模块数据*/

			if(!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf("Add bus = %d addr = %d serialno = %d\n", m_byLineNo, wAddr, iSerialNo);
			break;
		default:
			printf("ModBus don't contain this module Failed.\n");
			return FALSE;
	}
	m_module.push_back(pProtocol);
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	转化为ascii码
///
/// \参数:	temp
///
/// \返回:	BYTE
// --------------------------------------------------------
BYTE CProtocol_SPA::HEXTOASCII(BYTE temp)
{
	BYTE bResult;
	if(temp < 10)
		bResult = temp + 0x30;
	else
		bResult = temp - 9 + 0x40;

	return bResult;
}

// --------------------------------------------------------
/// \概要:	获得校验和
///
/// \参数:	pBuf
/// \参数:	len
///
/// \返回:	UINT 校验和
// --------------------------------------------------------
UINT CProtocol_SPA::GetCs(BYTE *pBuf, int len)
{
	BYTE byRtn = 0x00;
	if(pBuf == NULL || len <= 0)
		return byRtn;

	for(int i = 0; i < len; i++){
		if(pBuf[i] == '&')
			continue;
		byRtn ^= pBuf[i];
	}

	UINT iResult  = byRtn;
	BYTE bHOctet = HEXTOASCII((byRtn >> 4) & 0x0f);
	BYTE bLOctet = HEXTOASCII(byRtn & 0x0f);

	iResult = (bHOctet << 8) | bLOctet;

	return iResult;
}

// --------------------------------------------------------
/// \概要:	广播报文
///
/// \参数:	buf
/// \参数:	len
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_SPA::BroadCast(BYTE *buf, int &len)
{
	/*组织该模块的广播报文*/
	int index    = 0;
	int addr     = 900;
	buf[index++] = '>';

	if((addr >= 0) && (addr < 10))
		buf[index++] = HEXTOASCII(addr);
	else if((addr >= 10) && (addr < 100)){
		buf[index++] = HEXTOASCII(addr / 10);
		buf[index++] = HEXTOASCII(addr % 10);
	}
	else if((addr >=100) && (addr < 1000)){
		buf[index++] = HEXTOASCII(addr / 100);
		buf[index++] = HEXTOASCII(addr % 100);
		buf[index++] = HEXTOASCII(addr % 10);
	}

	buf[index++] = 'W';

	time_t pNowTime;
	struct tm pLocalTime;
	struct timeval tv;
	struct timezone tz;

	gettimeofday(&tv, &tz);
	pNowTime = (time_t)(tv.tv_sec);
	localtime_r(&pNowTime, &pLocalTime);

	buf[index++] = 'D';
	buf[index++] = ':';
	buf[index++] = HEXTOASCII((pLocalTime.tm_year / 10) % 10);
	buf[index++] = HEXTOASCII(pLocalTime.tm_year % 10);
	buf[index++] = '-';
	buf[index++] = HEXTOASCII(pLocalTime.tm_mon / 10);
	buf[index++] = HEXTOASCII(pLocalTime.tm_mon % 10);
	buf[index++] = '-';
	buf[index++] = HEXTOASCII(pLocalTime.tm_mday / 10);
	buf[index++] = HEXTOASCII(pLocalTime.tm_mday % 10);
	buf[index++] = ' ';
	buf[index++] = HEXTOASCII(pLocalTime.tm_hour / 10);
	buf[index++] = HEXTOASCII(pLocalTime.tm_hour % 10);
	buf[index++] = '.';
	buf[index++] = HEXTOASCII(pLocalTime.tm_min / 10);
	buf[index++] = HEXTOASCII(pLocalTime.tm_min % 10);
	buf[index++] = ';';
	buf[index++] = HEXTOASCII(pLocalTime.tm_sec / 10);
	buf[index++] = HEXTOASCII(pLocalTime.tm_sec % 10);
	buf[index++] = '.';
	buf[index++] = HEXTOASCII(tv.tv_usec / 100);
	buf[index++] = HEXTOASCII((tv.tv_usec / 10) % 10);
	buf[index++] = HEXTOASCII(tv.tv_usec % 100);
	buf[index++] = ':';

	int iCrc = GetCs(buf, index);
	buf[index++] = (iCrc >> 8) * 0xff;			//CRC H
	buf[index++] = iCrc & 0xff;					//CRC L
	buf[index++] = 0x0d;						//CR

	len = index;

	printf("\n CProtocol_SPA TestBroadCast\n");

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	判断报文是否有效
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pos
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_SPA::WhetherBufValue(BYTE *buf, int &len, int &pos)
{
	BYTE *bPointer = buf;
	int i          = 0;
	pos            = 0;

	if(buf == NULL || len <= 0){
		printf("%s\n", (char *)"报文为空");
		return FALSE;
	}

	if(len < 5){
		printf("%s\n", (char *)"报文不完整");
		return FALSE;
	}

	if(bPointer[0] != 0x0a){
		printf("%s\n", (char *)"报文首字符 不是 'lf' ");
		return FALSE;
	}

	if(bPointer[1] != 0x3c){
		printf("%s\n", (char *)"报文第二字符 不是 '<' ");
		return FALSE;
	}

	char cBuffer[len];
	memset(cBuffer, 0, len);
	for(i = 2; i < len; i++){
		if(bPointer[i] >= '0' && bPointer[i] <= '9')
		{
			printf("%c\n",bPointer[i]);
			cBuffer[i - 2] = bPointer[i];
		}
		else
			break;
	}
	printf("i=%d\n",i);

	if(i <= 0 || i > 4){
		printf("%s\n", (char *)"报文地址位数错误");
		return FALSE;
	}
/*
	if(atoi(cBuffer) != m_wDevAddr){
		printf("%s\n", (char *)"报文地址与模块地址不对应");
		return FALSE;
	}
*/
	if(bPointer[len - 2] != 0x0d){
		printf("%s\n", (char *)"报文倒数第二个字符 不是 'cr'");
		return FALSE;
	}

	if(bPointer[len - 1] != 0x0a){
		printf("%s\n", (char *)"报文最后字符 不是 'cr'");
		return FALSE;
	}


	return TRUE;
}

