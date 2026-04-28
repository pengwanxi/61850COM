
#ifndef  __XIN_AO_H__
#define  __XIN_AO_H__

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
#include "../../share/Rtu.h"
#define  MODULE_XIN_AO_MQTT_JSON  1

class CProtocol_Xin_ao_Slave :
	public CRtuBase
{
public:
	CProtocol_Xin_ao_Slave();
	virtual ~CProtocol_Xin_ao_Slave();

	BOOL GetDevData();
	BOOL ProcessFileData(CProfile &profile);
	BOOL CreateModule(int iModule, char * sMasterAddr, WORD iAddr, char * sName, char * stplatePath);
	virtual BOOL Init(BYTE byLineNo);
};

#endif   
