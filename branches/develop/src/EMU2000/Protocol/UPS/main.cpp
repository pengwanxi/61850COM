/// \文件:	main.cpp
/// \概要:	UPS 协议
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-07-02

#include <stdio.h>
#include "CProtocol_UpsMaster.h"

extern "C"
{
	CProtocol *CreateProtocol(CMethod *pMethod);
}

CProtocol *CreateProtocol(CMethod *pMethod)
{
	CProtocol *pProtocol = NULL;
	pProtocol = new CProtocol_UpsMaster;
	if(pProtocol)
	{
		pProtocol->m_pMethod = pMethod;
		printf( "UPS DLL OK.\n");
	}

	return pProtocol;
}

