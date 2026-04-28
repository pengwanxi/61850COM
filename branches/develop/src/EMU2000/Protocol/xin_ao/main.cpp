
#include <stdio.h>
#include "Protocol_Xin_ao_Slave.h"

extern "C" {
	CProtocol *CreateProtocol(CMethod *pMethod);
}

CProtocol *CreateProtocol(CMethod *pMethod)
{
	CProtocol *pProtocol = NULL;
	pProtocol = new CProtocol_Xin_ao_Slave;
	if (pProtocol)
		pProtocol->m_pMethod = pMethod;
	return pProtocol;
}