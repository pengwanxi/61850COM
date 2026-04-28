#include "Protocol_Xin_ao_Slave.h"
#include "CProtocol_Xin_ao_Slave_Module.h"


CProtocol_Xin_ao_Slave::CProtocol_Xin_ao_Slave()
{
}


CProtocol_Xin_ao_Slave::~CProtocol_Xin_ao_Slave()
{
}

BOOL CProtocol_Xin_ao_Slave::GetDevData()
{
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/xinaoSlave/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
	CProfile profile(m_sDevPath);

	return ProcessFileData(profile);
}

BOOL CProtocol_Xin_ao_Slave::ProcessFileData(CProfile &profile)
{
	if (!profile.IsValid())
	{
		printf("Open file %s Failed ! \n ", profile.m_szFileName);
		return FALSE;
	}

	char sSect[200] = "DEVNUM";
	char sKey[20][50] = { "module" , "addr" , "name" , "masteraddr" , "template" , "ycdead" , "ycProperty" , "timing" };

	BOOL bRtn;
	WORD wModule = 0;//模块标识

	WORD addr = 0;//装置地址
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
			printf("Create xinao Module=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, addr, sName, stemplate);
			return FALSE;
		}
	}

	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave::CreateModule(int iModule, char * sMasterAddr, WORD iAddr, char * sName, char * stplatePath)
{
    CProtocol_Xin_ao_Slave * pProtocol = NULL ;
	bool bRtn = FALSE;
    switch( iModule )
   {
    	case MODULE_XIN_AO:
		{
			pProtocol = new CProtocol_Xin_ao_Slave_Module ;
			pProtocol->m_byLineNo = m_byLineNo ;
			pProtocol->m_wModuleType = iModule ;
			pProtocol->m_wDevAddr = iAddr ;
			strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
			strcpy(pProtocol->m_sDevName, sName);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod ;
			pProtocol->m_ProtoType = PROTOCO_TRANSPROT;
			//初始化模板数据
			bRtn = pProtocol->Init( m_byLineNo ) ;
			if ( !bRtn )
			{
				printf ( "Init Error \n");
				return FALSE;
			}
			printf( " Add bus = %d Addr = %d " , m_byLineNo , iAddr ) ;
		}
		break;
		case MODULE_XIN_AO_MQTT_JSON:
		{
			pProtocol = new CProtocol_Xin_ao_Slave;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			strcpy(pProtocol->m_sDevName, sName);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_TRANSPROT;
			//初始化模板数据
			bRtn = pProtocol->Init(m_byLineNo);
			if (!bRtn)
			{
				printf("Init Error \n");
				return FALSE;
			}
			printf(" Add bus = %d Addr = %d ", m_byLineNo, iAddr);
		}
		break;
	default:
		{
			printf( "ModBus don't contain this module Failed .\n" );
			return FALSE ;
		}
    }

	m_module.push_back(pProtocol);

	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave::Init(BYTE byLineNo)
{
	m_byLineNo = byLineNo;
	m_ProtoType = PROTOCO_TRANSPROT;

	return GetDevData();
}
