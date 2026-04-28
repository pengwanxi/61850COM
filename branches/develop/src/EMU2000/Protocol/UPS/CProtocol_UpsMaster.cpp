/// \文件:	CProtocol_UpsMaster.cpp
/// \概要:	UPS 协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-07-02

#include "CProtocol_UpsMaster.h"
#include "UpsBaiSe.h"
#include "UpsKeShiDa.h"
#include "HipulseU.h"
#include "UpsPssentr.h"
#include "Speciall_UPS.h"


#define MODULE_BAISE		1	//百色UPS通讯协议
#define MODULE_KESHIDA		2	//科士达UPS通讯协议
#define MODULE_SPECIALL_UPS 3  // UPS 通讯协议（内部）
#define MODULE_HIPULSEU     4   //HIPULSE U系列
#define MODULE_PSSENTR      5   //SENTRY HPS  HTS



extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CProtocol_UpsMaster::CProtocol_UpsMaster()
{
	memset(m_sTemplatePath, 0, sizeof(m_sTemplatePath));
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CProtocol_UpsMaster::~CProtocol_UpsMaster()
{
	int size = m_module.size();
	for(int i = 0; i < size; i++)
	{
		delete m_module[i];
	}
	m_module.clear();
	printf("Delete All CProtocol_UpsMaster OK . \n");
}

// --------------------------------------------------------
/// \概要:	获得报文
///
/// \参数:	buf
/// \参数:	len
/// \参数:	pBusMsg
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_UpsMaster::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
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
BOOL CProtocol_UpsMaster::ProcessProtocolBuf(BYTE *buf, int len)
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
BOOL CProtocol_UpsMaster::Init(BYTE byLineNo)
{
	//增加UPS 采集模块数据
	//通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo;
	//读取模板文件
	m_ProtoType = PROTOCO_GATHER;
	return GetDevData();
}

// --------------------------------------------------------
/// \概要:	得到地址数据
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_UpsMaster::GetDevData()
{
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/UPS/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
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
BOOL CProtocol_UpsMaster::ProcessFileData(CProfile &profile)
{
	BOOL bRtn = FALSE;
	if(!profile.IsValid())
	{
		printf("Open file %s Failed ! \n ", profile.m_szFileName);
		return FALSE;
	}

	char sSect[ 200 ] = "DEVNUM";
	char sKey[ 20 ][ 100 ]={ "module", "serialno", "addr", "name", "template" };

	WORD wModule = 0;
	int  serialno=1;
	WORD addr =3;
	char sName[ 50 ] = { 0 };
	char stemplate[ 200 ] = { 0 };
	int iNum = 0;

	iNum = profile.GetProfileInt(sSect, (char *)"NUM", 0);
	if(iNum == 0)
	{
		printf("Get DEVNUM Failed ! \n ");
		return FALSE;
	}

	for(int i = 0; i < iNum; i++)
	{
		sprintf(sSect, "%s%03d",  "DEV", i + 1);

		wModule = profile.GetProfileInt(sSect, sKey[ 0 ], 0);
		serialno = profile.GetProfileInt(sSect, sKey[ 1 ], 0);
		addr = profile.GetProfileInt(sSect, sKey[ 2 ], 0);
		profile.GetProfileString(sSect, sKey[ 3 ], (char *)"NULL", sName, sizeof(sName));
		profile.GetProfileString(sSect, sKey[ 4 ], (char *)"NULL", stemplate, sizeof(stemplate));

		//创建相应模块子类
		bRtn = CreateModule(wModule, serialno, addr, sName, stemplate);
		if (!bRtn)
		{
			printf ("Create UPS Module=%d serialno=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, serialno, addr, sName, stemplate);
			return FALSE;
		}
	}
	return TRUE;
}

// --------------------------------------------------------
/// \概要:	创建相应模板子类
///
/// \参数:	iModule
/// \参数:	iSerialNo
/// \参数:	iAddr
/// \参数:	sName
/// \参数:	stplatePath
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_UpsMaster::CreateModule(int iModule, int iSerialNo, WORD iAddr, char *sName, char *stplatePath)
{
	CProtocol_UpsMaster *pProtocol = NULL;

	switch(iModule){
		case MODULE_BAISE:
			pProtocol = new CUpsBaiSe;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			pProtocol->m_SerialNo = iSerialNo;
			strcpy(pProtocol->m_sDevName, sName);
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_GATHER;
			//初始化模板数据
			if(!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);

			break;
		case MODULE_KESHIDA:
			pProtocol = new CUpsKeShiDa;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			pProtocol->m_SerialNo = iSerialNo;
			strcpy(pProtocol->m_sDevName, sName);
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_GATHER;
			//初始化模板数据
			if(!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
			break;
		case MODULE_SPECIALL_UPS:
			pProtocol = new SPECIALLUPS;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			pProtocol->m_SerialNo = iSerialNo;
			strcpy(pProtocol->m_sDevName, sName);
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_GATHER;
			//初始化模板数据
			if (!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
			break;
		case MODULE_HIPULSEU:
			pProtocol = new CUpsHipulse;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			pProtocol->m_SerialNo = iSerialNo;
			strcpy(pProtocol->m_sDevName, sName);
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_GATHER;
			//初始化模板数据
			if (!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
			break;
		case MODULE_PSSENTR:
			pProtocol = new CUpsSentry;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			pProtocol->m_SerialNo = iSerialNo;
			strcpy(pProtocol->m_sDevName, sName);
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_GATHER;
			//初始化模板数据
			if (!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
			break;

		default:
			printf("UPS don't contain this module Failed .\n");
			return FALSE;
	}

	m_module.push_back(pProtocol);

	return TRUE;
}

