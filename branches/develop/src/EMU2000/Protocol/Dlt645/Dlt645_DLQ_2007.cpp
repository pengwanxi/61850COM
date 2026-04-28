/*
 * =====================================================================================
 *
 *       Filename:  Dlt645_DLQ_2007.cpp
 *
 *    Description:  dlt645 2007�汾Э��
 *
 *        Version:  1.0
 *        Created:  2014��11��10�� 14ʱ12��10��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *   Organization:
 *
 *		  history:
 * =====================================================================================
 */
#include <stdio.h>
#include "Dlt645_DLQ_2007.h"


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  CDlt645_DLQ
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_DLQ::CDlt645_DLQ ()
{/*{{{*/
	m_dayflag = -1;
	InitProtocolStatus(  );
}  /* -----  end of method CDlt645_DLQ::CDlt645_DLQ  (constructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  ~CDlt645_DLQ
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_DLQ::~CDlt645_DLQ ()
{
}  /* -----  end of method CDlt645_DLQ::~CDlt645_DLQ  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  ProcessYcData
 * Description:  ң�⴦��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::ProcessYcData ( const BYTE *buf, int len )
{/*{{{*/
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	DWORD dwYcVal = 0;
	BYTE byflag = 0;
	const BYTE *pointer;

	if (atoi(m_sDevName) == 463 || atoi(m_sDevName) == 464)
		printf("devName = %s byDataLen =%d \n", m_sDevName, byDataLen);
	else
	{
		printf("devName = %s\n", m_sDevName);
	}

	if ( len < 16 )
		return FALSE;

	if ( buf[8] != 0x91 )
		return FALSE;

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	byflag = m_CfgInfo[m_bySendPos].byflag;

	pointer = buf + 14;
	while( byDataNum > 0 )
	{
	
			float fYcVal;	
			CalFormatData(pointer, byDataFormat, byDataLen, dwYcVal);
			fYcVal = (float)dwYcVal;
			m_pMethod->SetYcData(m_SerialNo, wPnt, fYcVal);
			printf("----m_SerialNo=%d, wPnt=%d, fYcVal=%f\n", m_SerialNo, wPnt, fYcVal);
			pointer += byDataLen;
			wPnt++;
			byDataNum--;
	
	/*	sprintf( m_szPrintBuf, "yc pnt:%d value:%f", wPnt, fYcVal );
		print( m_szPrintBuf );*/
	}

	return TRUE;
}		/* -----  end of method CDlt645_DLQ::ProcessYcData  ----- *//*}}}*/


BOOL CDlt645_DLQ::ProcessYxData(const BYTE *buf, int len)
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	pointer = buf + 14;

	BYTE byYxByte;
	BYTE byYxValue[8] = {0};
	BYTE byYxValue1[8] = { 0 };
	printf("\n------yx---------------\n");
	for (int i = 0; i<8; i++)
	{
		
		BYTE byYxBit = 0;
		byYxBit = ((*pointer - 0x33) >> i) & 0x01;
		if (byYxBit == 0x01)
		{
			byYxValue[i] = 1;
		}
		else
		{
			byYxValue[i] = 0;
		}
		printf("%d  ", byYxValue[i]);
		
	}
	printf("\n--------xxxxx-------------\n");
	/*for (int j = 0,t=7; j < 8; j++,t--)
	{
		byYxValue[j] = byYxValue1[t];
	}*/

	BYTE data = 0;

	/*for (int j = 0; j < byDataNum; j++)
	{*/
	if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 0 && byYxValue[4] == 0)//00000+++0
	{
		m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
		printf("yx wPnt= %d-------%d------\n", wPnt,1);
		wPnt++;
	}
	else
	{
		m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
		printf("yx wPnt= %d-------%d------\n", wPnt, 0);
		wPnt++;
	}


		if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0)//00100++1
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d-------%d------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d-------%d------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 1 && byYxValue[1] == 0 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0)//10100++2
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d-------%d------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d------%d-------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 0 && byYxValue[1] == 1 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0 )//01100++3
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d------%d-------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d------%d-------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 1 && byYxValue[1] == 1 && byYxValue[2] == 1 && byYxValue[3] == 0 && byYxValue[4] == 0)//11100---++4
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d-------%d------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d------%d-------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0)//00010--++5
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d-------%d------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d------%d-------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 1 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] ==1 && byYxValue[4] == 0)//10010--++6
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d------%d-------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d-----%d--------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 0 && byYxValue[1] == 1 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0)//01010++7
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d------%d-------\n", wPnt, 1);

			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d-----%d--------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] == 1 && byYxValue[1] == 1 && byYxValue[2] == 0 && byYxValue[3] == 1 && byYxValue[4] == 0)//11010++8
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d-------%d------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d------%d-------\n", wPnt, 0);
			wPnt++;
		}

		if (byYxValue[0] ==1&& byYxValue[1] == 1 && byYxValue[2] == 1 && byYxValue[3] == 1 && byYxValue[4] == 0)//11110++9
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d------%d-------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d------%d-------\n", wPnt, 0);
			wPnt++;
		}

	   if (byYxValue[0] == 0 && byYxValue[1] == 1&& byYxValue[2] ==0 && byYxValue[3] == 0 && byYxValue[4] == 1)//01001++10
	   {
		   m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
		   printf("yx wPnt= %d--------%d-----\n", wPnt, 1);
		   wPnt++;
	   }
	   else
	   {
		   m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
		   printf("yx wPnt= %d------%d-------\n", wPnt, 0);
		   wPnt++;
	   }

	  if (byYxValue[0] == 0 && byYxValue[1] == 0 && byYxValue[2] == 0 && byYxValue[3] == 0 && byYxValue[4] == 1)//00001++11
	  {
		  m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
		  printf("yx wPnt= %d------%d-------\n", wPnt, 1);
		  wPnt++;
	  }
	  else
	  {
		  m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
		  printf("yx wPnt= %d----%d---------\n", wPnt, 0);
		  wPnt++;
	  }

	  if (byYxValue[5] == 0 && byYxValue[6] == 0)//00++12
	  {
		  m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
		  printf("yx wPnt= %d------%d-------\n", wPnt, 1);
		  wPnt++;
	  }
	  else
	  {
		  m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
		  printf("yx wPnt= %d----%d---------\n", wPnt, 0);
		  wPnt++;
	  }

		if (byYxValue[5] == 1 && byYxValue[6] == 0)//10++13
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 1);
			printf("yx wPnt= %d------%d-------\n", wPnt, 1);
			wPnt++;
		}
		else
		{
			m_pMethod->SetYxData(m_SerialNo, wPnt, 0);
			printf("yx wPnt= %d-----%d--------\n", wPnt, 0);

			wPnt++;
		}

	
	return FALSE;
}
BOOL CDlt645_DLQ::ProcessSOEData(const BYTE *buf, int len)//û�� ƽ����ĿһЩ�¼��ǵ�����ͨң�⴦����
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	DWORD dwYcVal = 0;
	BYTE  bySpecialflag = 0;
	const BYTE *pointer;

	if (len < 16)
		return FALSE;

	if (buf[8] != 0x91)
		return FALSE;

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	bySpecialflag = m_CfgInfo[m_bySendPos].byflag;

	pointer = buf + 14;
	while (byDataNum > 0)
	{

		BYTE soeflag = 0;
		TIMEDATA ptmData;
		if (bySpecialflag == 1)//��·���Լ��¼���¼
		{
			if (0x00 == *pointer)
			{
				soeflag = 1;
			}
			else if (0x11 == *pointer)
			{
				soeflag = 0;
			}
			ptmData.MiSec = 0;
			ptmData.Second = 0;
			ptmData.Minute = 0;
			ptmData.Hour = 0;
			ptmData.Day = 0;
			ptmData.Month = 0;
			ptmData.Year = 0;
			m_pMethod->SetYxDataWithTime(m_SerialNo, wPnt, soeflag, &ptmData);

		}
		else if (bySpecialflag == 2)//���������¼���¼
		{

		}
		else if (bySpecialflag == 3)//��������Ͷ���¼���¼
		{

		}
		else if (bySpecialflag == 4)//բλ�仯�¼���¼
		{

		}
		else if (bySpecialflag == 5)//�澯�¼���¼
		{

		}
		else if (bySpecialflag == 6)//��ѹʧ/�����¼���¼
		{

		}

		pointer += byDataLen;
		wPnt++;
		byDataNum--;
		
	}
	return TRUE;
	return FALSE;
}
BOOL CDlt645_DLQ::ProcessMaxMinData(const BYTE *buf, int len)
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	DWORD dwYcVal = 0;

	const BYTE *pointer;

	if (len < 16)
		return FALSE;

	if (buf[8] != 0x91)
		return FALSE;

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;

	pointer = buf + 14;
	while (byDataNum > 0)
	{
		
		float fYcVal;
		//ʣ��������ֵ������ʱ�̡�Ӱ�쵱ǰʣ�������������
		//��һ��һ�����ϴ���
		//ʣ����������1���ֽ�;ʣ�����ֵ 2���ֽ�;���ֵ�ʱ�� 6���ֽ�
		CalFormatData(pointer, byDataFormat, byDataLen, dwYcVal);

		fYcVal = (float)dwYcVal;
		m_pMethod->SetYcData(m_SerialNo, wPnt, fYcVal);

		pointer += byDataLen;
		wPnt++;
		byDataNum--;
		sprintf(m_szPrintBuf, "yc pnt:%d value:%f", wPnt, fYcVal);
		print(m_szPrintBuf);

	}
	return TRUE;
}
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  ProcessYmData
 * Description:  ң������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::ProcessYmData ( const BYTE *buf, int len )
{/*{{{*/
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	DWORD dwYmVal = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;
	REALTIME curTime;
	GetCurrentTime(&curTime);

	if ( len < 16 )
  {
      print((char *)"len < 16");
      return FALSE;
  }
	if ( buf[8] != 0x91 )
  {
      sprintf( m_szPrintBuf, "buf[8]=%.2x", buf[8] );
      print(m_szPrintBuf);
      return FALSE;
  }

	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;

	pointer = buf + 14;
	while (byDataNum > 0)
	{
		CalFormatData(pointer, byDataFormat, byDataLen, dwYmVal);
		m_pMethod->SetYmData(m_SerialNo, wPnt, static_cast<QWORD>(dwYmVal));
		printf("--ym--m_SerialNo=%d, wPnt=%d, dwYmVal=%d\n", m_SerialNo, wPnt, dwYmVal);
		pointer += byDataLen;
		wPnt++;
		byDataNum--;
		sprintf(m_szPrintBuf, "ym pnt:%d value:%lu", wPnt, dwYmVal);
		print(m_szPrintBuf);
	}
	return TRUE;
	
	
}		/* -----  end of method CDlt645_DLQ::ProcessYmData  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  ProcessBuf
 * Description:  �������ձ���
 *       Input:	 ���ջ���������
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::ProcessBuf ( const BYTE *buf, int len )
{/*{{{*/
	printf("----------recv---------\n");
	for (int i = 0; i < len; i++)
	{
		printf("%02x ",buf[i]);
	}
	switch ( m_byDataType )
	{
		case DLT645_YC_DATATYPE:
			print( "ң������\n" );
			ProcessYcData( buf, len );
			break;

		case DLT645_YM_DATATYPE:
			print( "ң������\n" );
			ProcessYmData( buf, len );
			break;

		case DLT645_YX_DATATYPE:
			print("ң������\n");
			ProcessYxData(buf, len);
			break;

		case DLT645_MAX_MIN_DATATYPE:
			print("�����Сֵ����\n");
			ProcessMaxMinData(buf,len);
			break;

		case DLT645_SOE_DATATYPE:
			print("SOE�¼���¼");
			ProcessSOEData(buf,len);
			break;

		default:
			sprintf( m_szPrintBuf, "δ�ҵ�����������%d", m_byDataType );
			print( m_szPrintBuf );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CDlt645_DLQ::ProcessBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  IsTimeToSync
 * Description:  �Ƿ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::IsTimeToSync ( void )
{/*{{{*/
	if( m_bLinkStatus && m_bLinkTimeSyn )
	{
		m_bLinkTimeSyn = FALSE;
		return TRUE;
	}
	REALTIME curTime;
	GetCurrentTime( &curTime );

	if( 12 == curTime.wHour )
	{
		if( 1 > curTime.wMinute && 10 > curTime.wSecond )
		{
			if( m_bTimeSynFlag )
				return FALSE;
			else
				return TRUE;
		}
		else
		{
			m_bTimeSynFlag = FALSE;
		}
	}

	return FALSE;
}		/* -----  end of method CDlt645_DLQ::IsTimeToSync  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  RequestReadData
 * Description:  ��������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::RequestReadData ( BYTE *buf, int &len )
{/*{{{*/
	len = 0;

	for (int i = 0; i<m_CfgInfo[m_bySendPos].byFENum; i++)//ǰ����ַ
	{
		buf[len++] = 0xfe;
	}

	//��ʼ��
	buf[len++] = 0x68;
	//��ַλ
	for ( int i=0; i<6; i++)
	{
		buf[len++] = m_bySlaveAddr[i];
	}
	buf[len++] = 0x68;
	buf[len++] = 0x11;	//������
	buf[len++] = 0x04;	//���ݳ���

	//2007Ϊ4����ʶ��
	buf[len++] = m_CfgInfo[m_bySendPos].byDI0 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI1 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI2 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI3 + 0x33;

	buf[len++] = GetCs( buf + m_CfgInfo[m_bySendPos].byFENum, 14 );				//by cyz!

	buf[len++] = 0x16;//������

	

	return TRUE;
}		/* -----  end of method CDlt645_DLQ::RequestReadData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  TimeSync
 * Description:  ��ʱ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::TimeSync ( BYTE *buf, int &len )
{/*{{{*/
	REALTIME curTime;
	len = 0;

	for (int i = 0; i<m_CfgInfo[0].byFENum; i++)
	/*for ( int i=0; i<m_CfgInfo[m_bySendPos].byFENum; i++)*/
	{
		buf[len++] = 0xfe;
	}

	buf[len++] = 0x68;
	
	//��ַλ
	for ( int i=0; i<6; i++)
	{
		buf[len++] = 0x99;
	}
	buf[len++] = 0x68;
	buf[len++] = 0x08;	//������
	buf[len++] = 0x06;	//���ݳ���

	GetCurrentTime( &curTime );

	//2007Ϊ4����ʶ��
	/*buf[len++] = (BYTE)(curTime.wSecond + 0x33);
	buf[len++] = (BYTE)curTime.wMinute + 0x33;
	buf[len++] = (BYTE)curTime.wHour + 0x33;
	buf[len++] = (BYTE)curTime.wDay + 0x33;
	buf[len++] = (BYTE)curTime.wMonth + 0x33;
	buf[len++] = (BYTE)(curTime.wYear-2000)+ 0x33;*/

	buf[len++] = DEC_TO_BCD(curTime.wSecond);
	buf[len++] = DEC_TO_BCD(curTime.wMinute);
	buf[len++] = DEC_TO_BCD(curTime.wHour);
	buf[len++] = DEC_TO_BCD(curTime.wDay);
	buf[len++] = DEC_TO_BCD(curTime.wMonth);
	buf[len++] = DEC_TO_BCD((curTime.wYear - 2000));

	buf[len++] = GetCs( buf, 16 );

	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CDlt645_DLQ::TimeSync  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  GetSendBuf
 * Description:	 ��ȡ���ͱ��ĺͳ���
 *       Input:	 ���ͻ����� ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::GetSendBuf ( BYTE *buf, int &len )
{/*{{{*/
	switch ( m_byDataType )
	{
		case DLT645_YC_DATATYPE:

		case DLT645_YM_DATATYPE:

		case DLT645_YX_DATATYPE:

		case DLT645_MAX_MIN_DATATYPE:
			print( "��������" );
			RequestReadData( buf, len );
			break;
		case DLT645_TIME_DATATYPE:
			print( "��ʱ" );
			TimeSync( buf, len );
			break;

		default:
			sprintf( m_szPrintBuf, "Dlt645_2007 ��%d�����������������ô���", m_bySendPos );
			print( m_szPrintBuf );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CDlt645_DLQ::GetSendBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  InitProtocolStatus
 * Description:  ��ʼ��Э��״̬����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::InitProtocolStatus ( void )
{/*{{{*/
	m_bLinkStatus = FALSE;		//����״̬Ϊ��
	m_bySendPos = 0;			//����λ����0
	m_byDataType = 0;			//��������Ϊ��
	m_byRecvErrorCount = 0;     //���մ������0
	m_bIsReSend = FALSE;		//�ط���ʶλ0
	m_byResendCount = 0;		//�ط���������
	m_bIsSending = FALSE;		//���ͺ���1 ���պ�ֵ0
	m_bIsNeedResend = TRUE;		//�Ƿ���Ҫ�ط�
	m_bTimeSynFlag = FALSE;		//��ʱ��ʶ
	m_bLinkTimeSyn = TRUE;		//װ����ͨ���ʱһ��
	//�ط�����������
	m_byReSendLen = 0;
	memset( m_byReSendBuf, 0, DLT645_MAX_BUF_LEN );

	return TRUE;
}		/* -----  end of method CDlt645_DLQ::InitProtocolStatus  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  TimerProc
 * Description:  ʱ�䴦������ ��Ҫ����һЩ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDlt645_DLQ::TimerProc ( void )
{/*{{{*/
	//ʱ���ж�
	//
	//�ط���������
	if ( m_bIsReSend && m_byResendCount > DLT645_MAX_RESEND_COUNT )
	{
		sprintf( m_szPrintBuf, "resend count %d > %d InitProtocolStatus", m_byResendCount, DLT645_MAX_RESEND_COUNT );
		print( m_szPrintBuf );
		InitProtocolStatus(  );
	}

	//���մ����������
	if ( m_byRecvErrorCount > DLT645_MAX_RECV_ERR_COUNT )
	{
		sprintf( m_szPrintBuf, "recv err count %d > %d InitProtocolStatus", m_byRecvErrorCount, DLT645_MAX_RECV_ERR_COUNT );
		print( m_szPrintBuf );
		InitProtocolStatus(  );
	}
}		/* -----  end of method CDlt645_DLQ::TimerProc  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  ProcessProtocolBuf
 * Description:	 �����յ������ݻ���
 *       Input:  ���յ������ݻ��� ���泤��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::ProcessProtocolBuf ( BYTE *buf, int len )
{/*{{{*/
	print( "ProcessProtocolBuf" );	
	int pos = 0;
	BOOL bRtn = TRUE;
	if( !WhetherBufValue( buf, len , pos ) )
	{
		//���Ĵ�����
		printf ( "%s\n","Dlt6456 WhetherBufValue buf Recv err!!!\n" );
		m_byRecvErrorCount ++;
		m_bIsReSend = TRUE;
		return FALSE;
	}
	bRtn = ProcessBuf( buf+pos, len );
	if( !bRtn )
	{
		printf("%s\n", "�������ķ��������δ����");
	}
	//����״̬
	m_byRecvErrorCount = 0;
	m_bLinkStatus = TRUE;
	m_bIsReSend = FALSE;
	m_byResendCount = 0;
	m_bIsSending = FALSE;

	//������ȷ����
	return TRUE;
}		/* -----  end of method CDlt645_DLQ::ProcessProtocolBuf  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�����ݻ���
 *       Input:  ������ ���������ݳ��� ������Ϣ
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{/*{{{*/
	BOOL bRtn = TRUE;
	REALTIME curTime;
	GetCurrentTime(&curTime);
	//printf("------bus=%d-----time=%d\n", curTime.wMinute, m_SerialNo);
	if (m_dayflag != curTime.wDay&&curTime.wSecond != 0) //ÿ���ʱһ�� ���ұܿ�������
	{
		printf("-----------Dlq Curtime One  Time  Everyday--------devname=%s----------------\n", m_sDevName);
		m_byDataType = 0;//����Ҫ����  1 yx 2yc 4ym
		TimeSync(buf, len);// ��ʱ 
		m_dayflag = curTime.wDay;
	}
	else if ((curTime.wMinute == 1) || (curTime.wMinute == 16) || (curTime.wMinute == 31) || (curTime.wMinute == 46) )//��ʱ��ң��
	{
		ChangeSendPos_YM();
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		printf("---------------Dlq YM on time----------m_byDataType=%d-----------m_bySendPos=%d----------\n", m_byDataType, m_bySendPos);
		if (DLT645_YM_DATATYPE == m_byDataType)
		{
			bRtn = GetSendBuf(buf, len);
		}
	}
	else
	{
		ChangeSendPos(  );
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		//printf("SendBuf YC YX  pos=%d  m_byDataType=%d\n", m_bySendPos, m_byDataType);
		if (m_byDataType!=DLT645_YM_DATATYPE)//�˴����ڷ���ң��
		{
			bRtn = GetSendBuf(buf, len);
		}
		else
		{
			return FALSE;
		}
		
		if ( bRtn && len > 0)
		{
			
			char sbuf[500] = { 0 };	
			int i = 0;
			int index = 0;
			for (i = 0; i < len; i++)
			{
				sprintf(sbuf + index++, "%02x", buf[i]);
				strcat(sbuf + index++, "  ");
			}
			strcat(sbuf+index++,"   sendBuf\n");
		}
	}
	printf("----DLQ--sendbuf-------pos=%d----------\n", m_bySendPos);
	for (int j = 0; j < len; j++)
	{
		printf("%02x ", buf[j]);
	}
	printf("\n");

	return bRtn;
}		/* -----  end of method CDlt645_DLQ::GetProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_Cjt188
 *      Method:  GetDevNameToAddr
 * Description:  ͨ��װ�õ����ֶ�ȡͨѶ��ַ
 *       Input:  void
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::GetDevNameToAddr ( void )
{/*{{{*/
	int len = strlen( m_sDevName );
	if( len < 12)
	{
		return FALSE;
	}

	m_bySlaveAddr[0] = atoh( m_sDevName + len - 2, 2, 1 );
	m_bySlaveAddr[1] = atoh( m_sDevName + len - 4, 2, 1 );
	m_bySlaveAddr[2] = atoh( m_sDevName + len - 6, 2, 1 );
	m_bySlaveAddr[3] = atoh( m_sDevName + len - 8, 2, 1 );
	m_bySlaveAddr[4] = atoh( m_sDevName + len - 10, 2, 1 );
	m_bySlaveAddr[5] = atoh( m_sDevName + len - 12, 2, 1 );

	return TRUE;

}		/* -----  end of method CProtocol_Cjt188::GetDevNameToAddr  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  Init
 * Description:	 ��ʼ��Э������
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::Init ( BYTE byLineNo )
{/*{{{*/
	if( byLineNo > 22 )
		return FALSE;

	// if( !GetDevNameToAddr(  ) )
	// {
	// 	print ( "CDlt645_DLQ:Addr Err!!!\n" );
	// 	return FALSE;
	// }

	if( !ReadCfgInfo() )
	{
		print ( "CDlt645_DLQ:ReadCfgInfo Err!!!\n" );
		return FALSE;
	}

	if( !InitProtocolStatus() )
	{
		print ( "CDlt645_DLQ:InitProtocolStatus Err\n" );
		return FALSE;
	}

	print( "Dlt645 Init OK" );
	return TRUE;
}		/* -----  end of method CDlt645_DLQ::Init  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_DLQ
 *      Method:  GetDevCommState
 * Description:	 ����װ������״̬
 *       Input:
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_DLQ::GetDevCommState ( void )
{/*{{{*/

	if ( m_bLinkStatus )
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}		/* -----  end of method CDlt645_DLQ::GetDevCommState  ----- *//*}}}*/
