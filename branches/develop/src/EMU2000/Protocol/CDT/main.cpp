	//#include <fstream>				//加上各种错误，
#include <stdio.h>
#include "CProtocol_Cdt.h"

extern "C"{
	CProtocol *CreateProtocol(CMethod *pMethod);
}

CProtocol *CreateProtocol(CMethod *pMethod)
{
	CProtocol *pProtocol = NULL;
	pProtocol = new CProtocol_Cdt;
	if(pProtocol)
		pProtocol->m_pMethod = pMethod;
	return pProtocol;
}
