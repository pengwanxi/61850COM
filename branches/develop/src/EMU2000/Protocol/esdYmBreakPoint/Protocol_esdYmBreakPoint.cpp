#include "Protocol_esdYmBreakPoint.h"
#include "YmBreakPoint.h"
#define MODULE_ESD_YM_BREAK_POINT		1

CProtocol_esdYmBreakPoint::CProtocol_esdYmBreakPoint()
{
}


CProtocol_esdYmBreakPoint::~CProtocol_esdYmBreakPoint()
{
}

BOOL CProtocol_esdYmBreakPoint::GetDevData()
{
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/esdBreakPoint/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
	CProfile profile(m_sDevPath);

	return ProcessFileData(profile);
}

BOOL CProtocol_esdYmBreakPoint::ProcessFileData(CProfile &profile)
{
	if (!profile.IsValid())
	{
		printf("Open file %s Failed ! \n ", profile.m_szFileName);
		return FALSE;
	}

	char sSect[200] = "DEVNUM";
	char sKey[20][50] = { "module" , "addr" , "name" , "masteraddr" , "template" , "ycdead" , "ycProperty" , "timing" };


	char sName[50] = { 0 };//模块名字
	char stemplate[200] = { 0 };//模板路径
	int iNum = 0;//站数量
	char sMasterAddr[200] = { 0 };//主站IP地址和端口

	iNum = profile.GetProfileInt(sSect, (char *)"NUM", 0);
	if (iNum == 0)
	{
		printf("Get DEVNUM Failed ! \n ");
		return FALSE;
	}

	BYTE byIndex = 0;
	for (int i = 0; i < iNum; i++)
	{
		BOOL bRtn;
		WORD wModule = 0;//模块标识
		WORD addr = 0;//装置地址
		sprintf(sSect, "%s%03d", "DEV", i + 1);

		wModule = profile.GetProfileInt(sSect, sKey[byIndex++], 0);
		addr = profile.GetProfileInt(sSect, sKey[byIndex++], 0);

		profile.GetProfileString(sSect, sKey[byIndex++], (char *)"NULL", sName, sizeof(sName));
		profile.GetProfileString(sSect, sKey[byIndex++], (char *)"NULL", sMasterAddr, sizeof(stemplate));
		profile.GetProfileString(sSect, sKey[byIndex++], (char *)"NULL", stemplate, sizeof(stemplate));

		//创建相应模块子类
		bRtn = CreateModule(wModule, sMasterAddr, addr, sName, stemplate);
		if (!bRtn)
		{
			printf("Create IEC101S Module=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, addr, sName, stemplate);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CProtocol_esdYmBreakPoint::CreateModule(int iModule, char * sMasterAddr, WORD iAddr, char * sName, char * stplatePath)
{
	CYmBreakPoint * pProtocol = NULL;

	switch (iModule)
	{
		case MODULE_ESD_YM_BREAK_POINT:
		{
			pProtocol = new CYmBreakPoint;
			if (!InitEsdYmBreakpoint_Module(pProtocol, iModule, sMasterAddr, iAddr, sName, stplatePath))
				return FALSE;
		}
		break;
		default:
		{
			printf("%s don't contain this module Failed .\n", "CProtocol_esdYmBreakPoint");
			return FALSE;
		}
	}

	m_module.push_back(pProtocol);
	return TRUE;
}


BOOL CProtocol_esdYmBreakPoint::InitEsdYmBreakpoint_Module(CYmBreakPoint * pProtocol, int iModule, char * sMasterAddr, WORD iAddr, char * sName, char * stplatePath)
{
	if (pProtocol == NULL)
		return FALSE;

	pProtocol->m_byLineNo = m_byLineNo;
	pProtocol->m_wModuleType = iModule;
	pProtocol->m_wDevAddr = iAddr;
	printf("iddr=%d m_byLineNo=%d\n", iAddr, m_byLineNo);
	strcpy(pProtocol->m_sDevName, sName);
	strcpy(pProtocol->m_sTemplatePath, stplatePath);
	m_pMethod->m_pRtuObj = pProtocol;
	pProtocol->m_pMethod = m_pMethod;
	pProtocol->m_ProtoType = PROTOCO_TRANSPROT;
	//初始化模板数据
	if (!pProtocol->Init(m_byLineNo))
		return FALSE;
	printf(" Add bus = %d Addr = %d ProtocolName = %s \n", m_byLineNo, iAddr, sName);

	return TRUE;
}



BOOL CProtocol_esdYmBreakPoint::Init(BYTE byLineNo)
{
	m_byLineNo = byLineNo;
	m_ProtoType = PROTOCO_TRANSPROT;
	//读取模板文件

	return GetDevData();
}
