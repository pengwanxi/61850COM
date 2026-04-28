/// \文件:	CProtocol_XML.h
/// \概要:	XML 协议基类声明
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-03-22

#ifndef CPROTOCOL_H_
#define CPROTOCOL_H_

#include "../../share/Rtu.h"
#include "../../share/CMethod.h"

#define	XMLPREFIXFILENAME					"/mynand/config/IMP_XML/"		// XML 固定路径

#define MODULE_MONITORING_PLATFORMXML		1							//综合监控平台
#define XMLMAXPOINTNUM						256 * 256

typedef struct xmlmaplist
{
	BYTE Type;						// F_PType		测点类型
	int DevSerialNo;				// F_DevID		设备ID
	int DevTypePoint;				// F_DevOrder	测点顺序序号
	int DevXmlPoint;				//				XML顺序序号
	BYTE DevCode[10];
	BYTE DevName[50];
}XMLMAP;

class CProtocol_XML : public CRtuBase
{
	public:
		CProtocol_XML();
		virtual ~CProtocol_XML();

		WORD m_wYxUploadInterval;
		WORD m_wYcUploadInterval;
		WORD m_wYmUploadInterval;

		BOOL GetDevData();
		BOOL ProcessFileData(CProfile &profile);
		BOOL CreateModule(int iModule, char *sMasterAddr, WORD iAddr, char *sName, char *stplatePath);
		BOOL InitXML_Module(CProtocol_XML *pProtocol, int iModule, char *sMasterAddr, WORD iAddr, char *sName, char *stplatePath);
		virtual BOOL Init(BYTE byLineNo);

	protected:
		char m_sMasterAddr[200] ;			//网络参数保存
};
#endif
