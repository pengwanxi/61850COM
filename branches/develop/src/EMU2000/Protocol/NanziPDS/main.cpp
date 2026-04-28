/// \文件:	main.cpp
/// \概要:	协议主函数
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-09-11

#include <stdio.h>
#include "CProtocol_NanziPDS.h"

extern "C" CProtocol *CreateProtocol(CMethod *pMethod);

// --------------------------------------------------------
/// \概要:	创建协议
///
/// \参数:	pMethod
///
/// \返回:	返回协议
// --------------------------------------------------------
CProtocol *CreateProtocol(CMethod *pMethod)
{
	CProtocol *pProtocol = NULL;
	pProtocol = new CProtocol_NanziPDS;
	if(pProtocol){
		pProtocol->m_pMethod = pMethod;
		printf("NanziPDS DLL OK.\n");
	}

	return pProtocol;
}
