/// \文件:	CProtocol_UpsMaster.h
/// \概要:	UPS 协议基类声明
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-07-02

#ifndef CPROTOCOL_UPSMASTER_H_
#define CPROTOCOL_UPSMASTER_H_

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"

class CProtocol_UpsMaster : public CProtocol
{
	public:
		CProtocol_UpsMaster();
		virtual ~CProtocol_UpsMaster();

		virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);

		BOOL UPSQueryStatePack(BYTE *buf, int &len);

		BOOL UPSQueryStateDeal(BYTE *buf, int len);

		virtual BOOL Init(BYTE byLineNo);
	protected:
		BOOL GetDevData();
	protected:
		BOOL ProcessFileData(CProfile &profile);
		BOOL CreateModule(int iModule, int iSerialNo, WORD iAddr, char *sName, char *stplatePath);
};

#endif
