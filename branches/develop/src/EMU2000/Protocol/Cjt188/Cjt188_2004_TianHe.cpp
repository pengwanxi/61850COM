#include "Cjt188_2004_TianHe.h"



CCjt188_2004_TianHe::CCjt188_2004_TianHe()
{
}


CCjt188_2004_TianHe::~CCjt188_2004_TianHe()
{
}

BOOL CCjt188_2004_TianHe::ProcessDataT1(const BYTE *buf, int len)
{
	BYTE byYxVal;
	WORD wYcPnt = 0;
	WORD wYxPnt = 0;
	int i;
	const BYTE *pointer = buf;
	pointer += 10;

	//�ж����ݳ����Ƿ���ȷ
	if (9 != (*pointer))
	{
		print("���ݳ����쳣");
		return FALSE;
	}

	pointer += 4;  //�л���������

				   /* yc ���� */
				   // ��ǰ�ۻ�����  �����յ�ǰ�ۻ�����
				   //5byte 
	for (i = 0; i < 1; i++)
	{
		DWORD dwYcVal;
		//float fYcVal;
		dwYcVal = HexToBcd(*pointer)
			+ HexToBcd(*(pointer + 1)) * 100
			+ HexToBcd(*(pointer + 2)) * 10000
			+ HexToBcd(*(pointer + 3)) * 1000000;

		//fYcVal = (float)(dwYcVal);
		sprintf(m_szPrintBuf, "%d ym%d update%llu", m_SerialNo, wYcPnt, (QWORD)dwYcVal);
		print(m_szPrintBuf);

		m_pMethod->SetYmData(m_SerialNo, wYcPnt, static_cast<QWORD>(dwYcVal));
	}
	
	return TRUE;
}
