#include "CProtocol_ModBusMaster.h"
#include "ModBusWenKongyi.h"
#include "ModBusControl.h"
#include "ModBusEPS.h"
#include "ModBusDSE7320FDJ.h"
#include "ModBusXiaoDianLiu.h"
#include "ModBusXiaoHuMCU.h"
#include "ModBusLekuThermometer.h"
#include "ModBusZhenZhouWenkongyi.h"

#include "PMC_530.h"
#include "ModBusSD96E3.h"
#include "ModBusCJZ9SY.h"
#include "ModBusCJZ9SY_U.h"
#include "ModBusCJZ9SY_N.h"
#include "ModBusDiqiuzhan.h"
#include "ModBusEM600.h"
#include "ModBusWSCGQ.h"
#include "ModBusxzPMW2000.h"
#include "ModBusLGT8000EAS4.h"
#include "ModBusLGT8000E9S4 .h"
#include "ModBusEatonACB.h"

#define MODULE_CONTROL 1		 // 百色电容控制补偿
#define MODULE_WENKONGYI 2		 /* 百色干式变压器温控器Mosbus协议  */
#define MODULE_EPS 3			 /* 百色UPS和EPS通讯协议*/
#define MODULE_DSE7320FDJ 4		 /* 贵州松桃高速DSE7320发电机*/
#define MODULE_XDL 5			 /* 小电流接地选线保护装置  厦门正新集美厂 */
#define MODULE_XHMCU 6			 /* 消弧控制器  厦门正新集美厂 */
#define MODULE_LEKUTHERMOMETER 7 /* 乐酷变压器用电子温度计 */
#define MODULE_SD96E3 9			 /*苏州玛努利项目仪表SD96E3*/

#define MODULE_CJZ9SY 10			 /*8英寸项目CJZ‐9SY 液晶多功能表*/
#define MODULE_CJZ9SY_U 11			 /*CJZ‐9SY 液晶多功能表 3个相电压，3个线电压*/
#define MODULE_CJZ9SY_N 12			 /*特殊开发解析方式不同*/
#define MODULE_ZHENZHOU_WENKONGYI 13 // 郑州温控仪
#define MODULE_DIQIUZHAN 15			 /*卫通地球站*/
#define MODULE_EMU600 16			 /*EM600*/
#define MODULE_WSCGQ 17				 /*地球站设备*/
#define MODULE_XZPMW2000 18			 /*地球站设备*/
#define MODULE_LGT8000E_AS4 19		 /* 潮项目LGT8000E-AS4（小）*/
#define MODULE_LGT8000E_9S4 20		 /* 潮项目LGT8000E-9S4（大）*/

#define MODULE_PXR20_TCP 30
#define MODULE_PXR20_RTU 31
#define MODULE_PXR25_TCP 32
#define MODULE_PXR25_RTU 33

const int _PMC_530 = 8;
/*///////////////////////////////////////////////////////////////////////////*/
/* CRC-16, G(X)=X16+X15+X2+1*/
/* CRC 高位字节值表 */
unsigned char AuchCRCHi[256] = {/*{{{*/
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
								0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
								0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
								0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
								0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
								0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
								0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
								0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
								0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
								0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
								0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
								0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
								0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
								0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
								0x80, 0x41, 0x00, 0xC1, 0x81, 0x40}; /*}}}*/
/* CRC低位字节值表*/
unsigned char AuchCRCLo[256] = {/*{{{*/
								0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
								0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
								0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
								0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
								0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
								0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
								0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
								0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
								0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
								0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
								0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
								0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
								0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
								0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
								0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
								0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
								0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
								0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
								0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
								0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
								0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
								0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
								0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
								0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
								0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
								0x43, 0x83, 0x41, 0x81, 0x80, 0x40}; /*}}}*/

CProtocol_ModBusMaster::CProtocol_ModBusMaster()
{ /*{{{*/
	// ctor
	memset(m_sTemplatePath, 0, sizeof(m_sTemplatePath));

} /*}}}*/

CProtocol_ModBusMaster::~CProtocol_ModBusMaster()
{ /*{{{*/
	// dtor
	int size = m_module.size();
	for (int i = 0; i < size; i++)
	{
		delete m_module[i];
	}
	m_module.clear();
	printf("Delete All CProtocol_ModBus OK . \n");
} /*}}}*/

BOOL CProtocol_ModBusMaster::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{ /*{{{*/
	return FALSE;
} /*}}}*/

BOOL CProtocol_ModBusMaster::ProcessProtocolBuf(BYTE *buf, int len)
{ /*{{{*/
	return FALSE;
} /*}}}*/

BOOL CProtocol_ModBusMaster::Init(BYTE byLineNo)
{ /*{{{*/
	// 增加ModBus 采集模块数据
	// 通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo;
	// 读取模板文件
	m_ProtoType = PROTOCO_GATHER;

	return GetDevData();
} /*}}}*/

BOOL CProtocol_ModBusMaster::GetDevData()
{ /*{{{*/
	memset(m_sDevPath, 0, sizeof(m_sDevPath));
	sprintf(m_sDevPath, "%s/ModBusMaster/%s%02d.ini", SYSDATAPATH, DEVNAME, m_byLineNo + 1);
	CProfile profile(m_sDevPath);

	return ProcessFileData(profile);
} /*}}}*/

BOOL CProtocol_ModBusMaster::ProcessFileData(CProfile &profile)
{ /*{{{*/
	BOOL bRtn = FALSE;
	if (!profile.IsValid())
	{
		printf("Open file %s Failed ! \n ", profile.m_szFileName);
		return FALSE;
	}

	char sSect[200] = "DEVNUM";
	char sKey[20][100] = {"module", "serialno", "addr", "name", "template"};

	WORD wModule = 0;
	int serialno = 1;
	WORD addr = 3;
	char sName[50] = {0};
	char stemplate[200] = {0};
	int iNum = 0;

	iNum = profile.GetProfileInt(sSect, (char *)"NUM", 0);
	if (iNum == 0)
	{
		printf("Get DEVNUM Failed ! \n ");
		return FALSE;
	}

	for (int i = 0; i < iNum; i++)
	{
		sprintf(sSect, "%s%03d", "DEV", i + 1);

		wModule = profile.GetProfileInt(sSect, sKey[0], 0);
		serialno = profile.GetProfileInt(sSect, sKey[1], 0);
		addr = profile.GetProfileInt(sSect, sKey[2], 0);
		profile.GetProfileString(sSect, sKey[3], (char *)"NULL", sName, sizeof(sName));
		profile.GetProfileString(sSect, sKey[4], (char *)"NULL", stemplate, sizeof(stemplate));

		// 创建相应模块子类
		bRtn = CreateModule(wModule, serialno, addr, sName, stemplate);
		if (!bRtn)
		{
			printf("Create ModBus Module=%d serialno=%d addr=%d sName=%s stemplate=%s \
					Error \n",
				   wModule, serialno, addr, sName, stemplate);
			return FALSE;
		}
	}

	return TRUE;
} /*}}}*/

BOOL CProtocol_ModBusMaster::CreateModule(int iModule, int iSerialNo, WORD iAddr, char *sName, char *stplatePath)
{ /*{{{*/
	CProtocol_ModBusMaster *pProtocol = NULL;

	switch (iModule)
	{
	case MODULE_CONTROL:
	{
		pProtocol = new CModBusControl;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;

	case MODULE_WENKONGYI:
	{
		pProtocol = new CModbusWenKongYi;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;

	case MODULE_EPS:
	{
		pProtocol = new CModbusEPS;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;

	case MODULE_DSE7320FDJ:
		pProtocol = new CModbusDSE7320FDJ;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		break;

	case MODULE_XDL:
		pProtocol = new CModBusXDL;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		break;

	case MODULE_XHMCU:
		pProtocol = new CModBusXHMCU;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		break;

	case MODULE_LEKUTHERMOMETER:
		pProtocol = new ModBusLekuThermometer;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		break;

	case _PMC_530:
		pProtocol = new PMC_530;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf("- Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		break;

	case MODULE_SD96E3:
	{
		pProtocol = new CModBusSD96E3;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_SD96E3\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_ZHENZHOU_WENKONGYI:
	{
		pProtocol = new CModBusZhenZhouWenkongyi;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_ZhenZhouWenKongYi\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
#ifdef WISE_EEM
	case MODULE_IRTU:
	{
		pProtocol = new CModBusSCJZ9SY;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_SD96E3\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
#endif
	case MODULE_CJZ9SY_U:
	{
		pProtocol = new CModBusSCJZ9SY_U;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_SD96E3\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_CJZ9SY_N:
	{
		pProtocol = new CModBusSCJZ9SY_N;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_SD96E3\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_DIQIUZHAN:
	{
		pProtocol = new CModBusDiqiuzhan;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_Diqiuzhan\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_EMU600:
	{
		pProtocol = new CModBusEM600;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  CModBusEM600\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_WSCGQ:
	{
		pProtocol = new CModBusWSCGQ;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_WSCGQ\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_XZPMW2000:
	{
		pProtocol = new CModBusxzPMW2000;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;
		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_XZPMW2000\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_LGT8000E_AS4:
	{
		pProtocol = new CModBusGT8000EAS4;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;

		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_LGT8000E_AS4\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;

	case MODULE_LGT8000E_9S4:
	{
		pProtocol = new CModBusGT8000E9S4;
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;

		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  MODULE_LGT8000E_AS4\n", m_byLineNo, iAddr, iSerialNo);
	}
	break;
	case MODULE_PXR20_TCP:
	case MODULE_PXR20_RTU:
	case MODULE_PXR25_TCP:
	case MODULE_PXR25_RTU:
	{
		pProtocol = new CModBusEatonACB();
		pProtocol->m_byLineNo = m_byLineNo;
		pProtocol->m_wModuleType = iModule;
		pProtocol->m_wDevAddr = iAddr;
		pProtocol->m_SerialNo = iSerialNo;
		strcpy(pProtocol->m_sDevName, sName);
		strcpy(pProtocol->m_sTemplatePath, stplatePath);
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod;
		pProtocol->m_ProtoType = PROTOCO_GATHER;

		// 初始化模板数据
		if (!pProtocol->Init(m_byLineNo))
			return FALSE;
		printf(" Add bus = %d Addr = %d serialno = %d  iModule = %d\n", m_byLineNo, iAddr, iSerialNo, iModule);
	}
	break;
	default:
	{
		printf("ModBus don't contain this module Failed .\n");
		return FALSE;
	}
	}
	m_module.push_back(pProtocol);

	return TRUE;
} /*}}}*/

WORD CProtocol_ModBusMaster::GetCrc(BYTE *pBuf, int len)
{								   /*{{{*/
	unsigned char uchCRCHi = 0xFF; /* 高CRC字节初始化 */
	unsigned char uchCRCLo = 0xFF; /* 低CRC 字节初始化 */
	unsigned uIndex;			   /* CRC循环中的索引 */

	while (len--) /* 传输消息缓冲区 */
	{
		uIndex = uchCRCHi ^ *pBuf++; /* 计算CRC */
		uchCRCHi = uchCRCLo ^ AuchCRCHi[uIndex];
		uchCRCLo = AuchCRCLo[uIndex];
	}
	return (uchCRCHi << 8 | uchCRCLo);
} /*}}}*/

BOOL CProtocol_ModBusMaster::BroadCast(BYTE *buf, int &len)
{ /*{{{*/
	// 组织该模块的广播报文
	int index = 0;
	buf[index++] = 0xFF;
	buf[index++] = 0x02;
	buf[index++] = 0x03;
	buf[index++] = 0x04;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);

	len = index;

	printf("\n CProtocol_ModBus  TestBroadCast \n ");
	return TRUE;
} /*}}}*/
