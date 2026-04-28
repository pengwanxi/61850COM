/// \文件:	CProtocol_XML.cpp
/// \概要:	XML 协议基类定义
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-03-22

#include <cstring>
#include "CProtocol_XML.h"
#include "../../share/CProtocol.h"
//#include "../../librtdb/rdbObj.h"
#include "MonitoringPlatformXML.h"

// --------------------------------------------------------
/// \概要:	构造函数
// --------------------------------------------------------
CProtocol_XML::CProtocol_XML()
{
	m_wYxUploadInterval = 10;
	m_wYcUploadInterval = 15;
	m_wYmUploadInterval = 5;
}

// --------------------------------------------------------
/// \概要:	析构函数
// --------------------------------------------------------
CProtocol_XML::~CProtocol_XML()
{

}

BOOL CProtocol_XML::InitXML_Module(CProtocol_XML *pProtocol, int iModule, char *sMasterAddr, WORD iAddr, char *sName, char *stplatePath)
{
	if(pProtocol == NULL)
		return FALSE;

	BOOL bRtn = FALSE;
	pProtocol->m_byLineNo = m_byLineNo;
	pProtocol->m_wModuleType = iModule;
	pProtocol->m_wDevAddr = iAddr;
//	printf("iddr = %d m_byLineNo = %d\n", iAddr, m_byLineNo);

	strcpy(pProtocol->m_sDevName, sName);
	strcpy(pProtocol->m_sTemplatePath, stplatePath);
	strcpy(pProtocol->m_sMasterAddr, sMasterAddr);
	m_pMethod->m_pRtuObj = pProtocol;
	pProtocol->m_pMethod = m_pMethod;
	pProtocol->m_ProtoType = PROTOCO_TRANSPROT;

	/* 初始化模板数据 */
	bRtn = pProtocol->Init(m_byLineNo);
	if(!bRtn)
	{
		printf("Init Error!\n");
		return FALSE;
	}

	printf("Add bus = %d Addr = %d xProtocolName = %s\n", m_byLineNo, iAddr, sName);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	创建相应模块子类
///
/// \参数:	iModule		:模板标识
/// \参数:	sMasterAddr	:主站IP地址和端口
/// \参数:	iAddr		:装置地址
/// \参数:	sName		:模板名称
/// \参数:	stplatePath	:模板路径
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_XML::CreateModule(int iModule, char *sMasterAddr, WORD iAddr, char *sName, char *stplatePath)
{
	CProtocol_XML *pProtocol = NULL;
	switch(iModule)
	{
		case MODULE_MONITORING_PLATFORMXML:
			pProtocol = new MonitoringPlatformXML;
			if(!InitXML_Module(pProtocol, iModule, sMasterAddr, iAddr, sName, stplatePath))
				return FALSE;
			break;
		default:
			printf("%s don't contain this module Failed.\n", (char *)"XML");
			return FALSE;
	}

	m_module.push_back(pProtocol);

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	处理协议文件数据
///
/// \参数:	profile
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_XML::ProcessFileData(CProfile &profile)
{
	if(!profile.IsValid())
	{
		printf("Open file %s Failed!\n", profile.m_szFileName);
	}

	char sSect[200] = "DEVNUM";
	char sKey[20][50] =
	{
		"module", "addr", "name", "masteraddr", "template", "ycdead", "ycProperty", "timing"
	};

	BYTE byIndex = 0;
	BOOL bRtn = 0;
	WORD wModule = 0;						//模板标识
	WORD addr = 0;							//装置地址
	int iNum = 1;							//站数量
	char sName[50] = {0};					//模板名字
	char stemplate[200] = {0};				//模板路径
	char sMasterAddr[200] = {0};			//主站IP地址和端口

	iNum = profile.GetProfileInt(sSect, (char *)"NUM", 0);
	if(iNum == 0)
	{
		printf("Get DEVNUM Failed!\n");
		return FALSE;
	}

	for(int i = 0; i < iNum; i++)
	{
		sprintf(sSect, "%s%03d", "DEV", i + 1);
		wModule = profile.GetProfileInt(sSect, sKey[byIndex++], 0);
		addr = profile.GetProfileInt(sSect, sKey[byIndex++], 0);
//		printf("---FUNC = %s LINE = %d wYxUploadInterval = %d wYcUploadInterval = %d wYmUploadInterval = %d----\n", __func__, __LINE__, wYxUploadInterval, wYcUploadInterval, wYmUploadInterval);
		profile.GetProfileString(sSect, sKey[byIndex++], (char *)"NULL", sName, sizeof(sName));
		profile.GetProfileString(sSect, sKey[byIndex++], (char *)"NULL", sMasterAddr, sizeof(sMasterAddr));
		profile.GetProfileString(sSect, sKey[byIndex++], (char *)"NULL", stemplate, sizeof(stemplate));

		/* 创建相应模块子类 */
		bRtn = CreateModule(wModule, sMasterAddr, addr, sName, stemplate);
		if(!bRtn)
		{
			printf("Create XML Module = %d addr = %d sName = %s stemplate = %s ERROR!\n", wModule, addr, sName, stemplate);
			return FALSE;
		}
	}

	return TRUE;
}

// --------------------------------------------------------
/// \概要:	获取设备数据
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_XML::GetDevData()
{
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/IMP_XML/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
	CProfile profile(m_sDevPath);

	return ProcessFileData(profile);
}

// --------------------------------------------------------
/// \概要:	协议初始化开始
///
/// \参数:	byLineNo
///
/// \返回:	BOOL
// --------------------------------------------------------
BOOL CProtocol_XML:: Init(BYTE byLineNo)
{
	m_byLineNo = byLineNo;
	m_ProtoType = PROTOCO_TRANSPROT;

	/* 读取模板文件 */
	return GetDevData();
}


