#include "LoraPort.h"
#include "include/lora.h"
#include <iostream>
using namespace std ;


CLoraPort::CLoraPort()
{
	
}


CLoraPort::~CLoraPort()
{
}

BOOL CLoraPort::IsPortValid(void)
{
	return TRUE;
}

BOOL CLoraPort::OpenPort(char* lpszError /*= NULL*/)
{
	int ret = lora_init();
	cout << __FILE__ << __LINE__ << __FUNCTION__ << "  return val====" << ret << endl;
	return true;
}

void CLoraPort::ClosePort(void)
{
	return;
}

int CLoraPort::AsyReadData(BYTE *pBuf, int nRead)
{
	if (pBuf == NULL || !nRead )
		return 0;

	int ret = lora_read(( char *)pBuf, nRead);
	return ret ;
}

int CLoraPort::WritePort(BYTE *pBuf, int nWrite)
{
	if (pBuf == NULL || !nWrite)
		return 0;

	int ret = lora_write(( char *)pBuf, nWrite);
	return 	ret;
}
