// TcpClientShort.h: interface for the CTcpClientShort class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCPCLIENTSHORT_H__1D95CAB6_9303_46D6_BE27_EDD7D0FCE928__INCLUDED_)
#define AFX_TCPCLIENTSHORT_H__1D95CAB6_9303_46D6_BE27_EDD7D0FCE928__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TcpClient.h"

//1. 先connet
//2. 再read or write
//3. 最后在关闭
//4. 关于该类去连接谁，均已经在配置设置好了
class CTcpClientShort : public CTcpClient  
{
public:
	CTcpClientShort();
	virtual ~CTcpClientShort();
	virtual char* ClassName(){return (char *)"CTcpClientShort";}
};

#endif // !defined(AFX_TCPCLIENTSHORT_H__1D95CAB6_9303_46D6_BE27_EDD7D0FCE928__INCLUDED_)
