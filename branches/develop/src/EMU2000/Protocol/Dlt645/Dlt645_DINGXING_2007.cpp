#include "Dlt645_DINGXING_2007.h"



Dlt645_DINGXING_2007::Dlt645_DINGXING_2007()
{
}


Dlt645_DINGXING_2007::~Dlt645_DINGXING_2007()
{
}

BOOL Dlt645_DINGXING_2007::ProcessYxData(const BYTE *buf, int len)
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	DWORD dwYmVal = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;
	if (len < 16)
	{
		print((char *)"len < 16");
		return FALSE;
	}

	if (buf[8] != 0x91)
	{
		sprintf(m_szPrintBuf, "buf[8]=%.2x", buf[8]);
		print(m_szPrintBuf);
		return FALSE;
	}

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;

	pointer = buf + 14;
	//��ȡ��7λ
	BYTE bData = pointer[0] & 0x40;
	if (bData)
		m_pMethod->SetYxData(m_SerialNo, wPnt,1);
	else
		m_pMethod->SetYxData(m_SerialNo, wPnt, 0);

	sprintf(m_szPrintBuf, "yx pnt:%d value:%lu", wPnt,bData);
	print(m_szPrintBuf);
}
