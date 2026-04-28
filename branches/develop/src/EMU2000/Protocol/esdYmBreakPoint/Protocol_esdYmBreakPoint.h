#pragma once
#include "../../share/Rtu.h"
#include "../../share/CMethod.h"
#include "../../share/typedef.h"
class CYmBreakPoint;
class CProtocol_esdYmBreakPoint : public CRtuBase
{
public:
	CProtocol_esdYmBreakPoint();
	~CProtocol_esdYmBreakPoint();
	BOOL GetDevData();
	BOOL ProcessFileData(CProfile &profile);
	BOOL CreateModule(int iModule, char * sMasterAddr, WORD iAddr, char * sName, char * stplatePath);
	BOOL InitEsdYmBreakpoint_Module(CYmBreakPoint * pProtocol, int iModule, char * sMasterAddr, WORD iAddr, char * sName, char * stplatePath);
	virtual BOOL Init(BYTE byLineNo);
};

