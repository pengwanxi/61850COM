/// \文件:	main.cpp
/// \概要:	XML 传输存储协议主函数
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-03-22

#include <cstdio>
#include "CProtocol_XML.h"


// --------------------------------------------------------
/// \概要:	引用全局函数
// --------------------------------------------------------
extern "C"
{
	CProtocol *CreateProtocol(CMethod *pMethod);
}

CProtocol *CreateProtocol(CMethod *pMethod)
{
	CProtocol *pProtocol = NULL;
	pProtocol = new CProtocol_XML;
	if(pProtocol)
		pProtocol->m_pMethod = pMethod;

	return pProtocol;
}
