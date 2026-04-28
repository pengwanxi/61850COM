/// \文件:	CProtocol_NanziPDS.cpp
/// \概要:	NanziPDS协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-09-11

#include "CProtocol_NanziPDS.h"
#include "NanziPDS.h"

#define MODULE_RTU 1

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CProtocol_NanziPDS::CProtocol_NanziPDS()
{
	memset(m_sTemplatePath, 0, sizeof(m_sTemplatePath));
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CProtocol_NanziPDS::~CProtocol_NanziPDS()
{
	int size = m_module.size();
	for(int i = 0; i < size; i++)
		delete m_module[i];
	m_module.clear();
	printf("Delete All CProtocol_NanziPDS OK.\n");
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
BOOL CProtocol_NanziPDS::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
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
BOOL CProtocol_NanziPDS::ProcessProtocolBuf(BYTE *buf, int len)
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
BOOL CProtocol_NanziPDS::Init(BYTE byLineNo)
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
BOOL CProtocol_NanziPDS::GetDevData()
{
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/nanzipds/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
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
BOOL CProtocol_NanziPDS::ProcessFileData(CProfile &profile)
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
BOOL CProtocol_NanziPDS::CreateModule(int iModule, int iSerialNo, WORD wAddr, char *sName, char *sTemplatePath)
{
#if 1
	CProtocol_NanziPDS *pProtocol = NULL;
	switch(iModule){
		case MODULE_RTU:
			pProtocol                = new CNanziPDS;
			pProtocol->m_byLineNo    = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr    = wAddr;
			pProtocol->m_SerialNo    = iSerialNo;
			strcpy(pProtocol->m_sTemplatePath, sTemplatePath);
			m_pMethod->m_pRtuObj     = pProtocol;
			pProtocol->m_pMethod     = m_pMethod;
			strcpy(pProtocol->m_sDevName, sName);
			pProtocol->m_ProtoType   = PROTOCO_GATHER;
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
#endif
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
BOOL CProtocol_NanziPDS::WhetherBufValue(BYTE *buf, int &len, int &pos)
{

	return TRUE;
}

