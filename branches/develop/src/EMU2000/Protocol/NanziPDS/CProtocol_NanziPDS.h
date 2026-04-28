/// \文件:	CProtocol_NanziPDS.h
/// \概要:	NanziPDS协议头文件
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-09-11

#ifndef _CPROTOCOL_NANZIPDS_H
#define _CPROTOCOL_NANZIPDS_H

#include <time.h>
#include <sys/time.h>
#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"

// --------------------------------------------------------
/// \概要:	NanziPDS协议类，继承CProtocol 类
// --------------------------------------------------------
class CProtocol_NanziPDS : public CProtocol
{
	public:
		CProtocol_NanziPDS();
		~CProtocol_NanziPDS();
		virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);
		virtual BOOL Init(BYTE byLineNo);
		/* 判断报文有效性 */
		virtual BOOL WhetherBufValue(BYTE *buf, int &len, int &pos);
	protected:
		BOOL GetDevData();
		BOOL ProcessFileData(CProfile &profile);
		BOOL CreateModule(int iModule, int iSerialNo, WORD wAddr, char *sName, char *sTemplatePath);
};

#endif
