/*
 * =====================================================================================
 *
 *       Filename:  Cjt188_2004.cpp
 *
 *    Description:  Cjt188_2004 �汾Э��
 *
 *        Version:  1.0
 *        Created:  2015��03��12�� 10ʱ31��07��
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
#include <assert.h>
#include "Cjt188_2004.h"

#define	CJT188_2004_SYNC_INTERVAL	60*20			/* ��ʱ���  ��λs*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  CCjt188_2004
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CCjt188_2004::CCjt188_2004 ()
{
	InitProtocolStatus(  );
}  /* -----  end of method CCjt188_2004::CCjt188_2004  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ~CCjt188_2004
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CCjt188_2004::~CCjt188_2004 ()
{
}  /* -----  end of method CCjt188_2004::~CCjt188_2004  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ProcessDataT1
 * Description:  ����ˮ������ 10H-19H
 *       Input:  buf:68��ͷ�ı��Ļ�����
 *				 len���������ĳ���
 *		Return:  TRUE����ȷ���� FALSE��δ��ȷ����
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::ProcessDataT1 ( const BYTE *buf, int len  )
{
	BYTE byYxVal;
	WORD wYcPnt = 0;
	WORD wYxPnt = 0;
	WORD wYmPnt = 0;
	int i ;
	const BYTE *pointer = buf;
	pointer += 10;

	//�ж����ݳ����Ƿ���ȷ
	if( 0x16 != ( *pointer ) )
	{
		print( "���ݳ����쳣" );	
		return FALSE;
	}

	pointer += 4;  //�л���������
	
	/* yc ���� */
	// ��ǰ�ۻ�����  �����յ�ǰ�ۻ�����
	//5byte 
	for( i=0; i<1; i++ )
	{
		DWORD dwYcVal;
		float fYcVal;
		dwYcVal = HexToBcd( *pointer ) 
			+ HexToBcd( *(pointer + 1) ) * 100	
			+ HexToBcd( *(pointer + 2) ) * 10000 
			+ HexToBcd( *(pointer + 3) ) * 1000000;


		fYcVal = (float)(dwYcVal);
		sprintf( m_szPrintBuf, "%d ym%d update%f",m_SerialNo, wYmPnt, fYcVal );
		print( m_szPrintBuf );
		m_pMethod->SetYmData( m_SerialNo, wYmPnt, static_cast<QWORD>(dwYcVal));

		wYmPnt++;
		pointer += 5;
	}

	for (i = 0; i < 1; i++)
	{
		DWORD dwYcVal;
		float fYcVal;
		dwYcVal = HexToBcd(*pointer)
			+ HexToBcd(*(pointer + 1)) * 100
			+ HexToBcd(*(pointer + 2)) * 10000
			+ HexToBcd(*(pointer + 3)) * 1000000;


		fYcVal = (float)(dwYcVal);
		sprintf(m_szPrintBuf, "%d yc%d update%f", m_SerialNo, wYcPnt, fYcVal);
		print(m_szPrintBuf);

		m_pMethod->SetYcData(m_SerialNo, wYcPnt, fYcVal);

		wYcPnt++;
		pointer += 5;
	}

	//ʵʱʱ�� ������
	//7byte
	pointer += 7;

	/* yx ���� */
	//����״̬ ��λ 00���� 01���� 11���쳣
	//�˴�������yx���� ����״̬���� 
	if(  0 == ( *pointer & 0x03 ) )	
	{
		byYxVal = 0;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
		print( m_szPrintBuf );
	}
	else if( 1 == (*pointer & 0x03) )
	{
		byYxVal = 1;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
		print( m_szPrintBuf );
	}
	wYxPnt ++;

	if(  3 == ( *pointer & 0x03 ) )	
	{
		byYxVal = 0;
	}
	else 
	{
		byYxVal = 1;
	}
	m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
	sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
	print( m_szPrintBuf );
	wYxPnt ++;

	//��ص�ѹ������
	for( i=0; i<6; i++ )	
	{
		byYxVal = ( *pointer >> (2 + i) ) & 0x01;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
		print( m_szPrintBuf );
		wYxPnt ++;
	}
	pointer++;
	for( i=0; i<8; i++ )	
	{
		byYxVal = ( *pointer >> i ) & 0x01;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo ,wYxPnt, byYxVal );
		print( m_szPrintBuf );
		wYxPnt ++;
	}
	
	return TRUE;
}		/* -----  end of method CCjt188_2004::ProcessDataT1  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ProcessDataT2
 * Description:  �������������� 20H-29H
 *               buf:68��ͷ�ı��Ļ�����
 *       Input:  len���������ĳ���
 *		Return:  TRUE����ȷ���� FALSE��δ��ȷ����
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::ProcessDataT2 ( const BYTE *buf, int len  )
{
	DWORD dwYcVal;
	float fYcVal;
	BYTE byYxVal;
	WORD wYcPnt = 0;
	WORD wYxPnt = 0;
	int i ;
	const BYTE *pointer = buf;
	pointer += 10;

	//�ж����ݳ����Ƿ���ȷ
	if( 0x2e != ( *pointer ) )
	{
		print( "���ݳ����쳣" );	
		return FALSE;
	}

	pointer += 4;  //�л���������
	/* yc ���� */
	// ���������� ��ǰ���� �ȹ��� ���� ��ǰ�ۻ�����
	//5byte 
	for( i=0; i<5; i++ )
	{
		dwYcVal = HexToBcd( *pointer ) 
			+ HexToBcd( *(pointer + 1) ) * 100	
			+ HexToBcd( *(pointer + 2) ) * 10000 
			+ HexToBcd( *(pointer + 3) ) * 1000000;
		fYcVal = (float)(dwYcVal) ;
		m_pMethod->SetYcData( m_SerialNo, wYcPnt, fYcVal );
		sprintf( m_szPrintBuf, "%d yc%d update%f",m_SerialNo, wYcPnt, fYcVal );
		print( m_szPrintBuf );

		wYcPnt ++;
		pointer += 5;
	}

	//��ˮ�¶� ��ˮ�¶�
	for ( i=0; i<2; i++ )
	{
		dwYcVal = HexToBcd( *pointer ) 
			+ HexToBcd( *(pointer + 1) ) * 100	
			+ HexToBcd( *(pointer + 2) ) * 10000 ;

		fYcVal = (float)(dwYcVal) / 100;
		m_pMethod->SetYcData( m_SerialNo, wYcPnt, fYcVal );
		sprintf( m_szPrintBuf, "%d yc%d update%f",m_SerialNo, wYcPnt, fYcVal );
		print( m_szPrintBuf );
		wYcPnt ++;
		pointer += 3;
	}

	//���۹���ʱ��
	//3byte
	dwYcVal = HexToBcd( *pointer ) 
		+ HexToBcd( *(pointer + 1) ) * 100	
		+ HexToBcd( *(pointer + 2) ) * 10000 ;

	fYcVal = (float)(dwYcVal);
	m_pMethod->SetYcData( m_SerialNo, wYcPnt, fYcVal );
	sprintf( m_szPrintBuf, "%d yc%d update%f",m_SerialNo, wYcPnt, fYcVal );
	print( m_szPrintBuf );
	wYcPnt ++;
	pointer += 3;

	//ʵʱʱ�� ������
	//7byte
	pointer += 7;

	/* yx ���� */
	//����״̬ ��λ 00���� 01���� 11���쳣
	//�˴�������yx���� ����״̬���� 
	if(  0 == ( *pointer & 0x03 ) )	
	{
		byYxVal = 0;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
		print( m_szPrintBuf );
	}
	else if( 1 == (*pointer & 0x03) )
	{
		byYxVal = 1;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
		print( m_szPrintBuf );
	}
	wYxPnt ++;

	if(  3 == ( *pointer & 0x03 ) )	
	{
		byYxVal = 0;
	}
	else 
	{
		byYxVal = 1;
	}
	m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
	sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
	print( m_szPrintBuf );
	wYxPnt ++;

	//��ص�ѹ������
	for( i=0; i<6; i++ )	
	{
		byYxVal = ( *pointer >> (2 + i) ) & 0x01;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo,wYxPnt, byYxVal );
		print( m_szPrintBuf );
		wYxPnt ++;
	}
	pointer++;
	for( i=0; i<8; i++ )	
	{
		byYxVal = ( *pointer >> i ) & 0x01;
		m_pMethod->SetYxData( m_SerialNo, wYxPnt, byYxVal );		
		sprintf( m_szPrintBuf, "%d yx%d update%d",m_SerialNo, wYxPnt, byYxVal );
		print( m_szPrintBuf );
		wYxPnt ++;
	}
	
	return TRUE;
}		/* -----  end of method CCjt188_2004::ProcessDataT2  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ProcessDataT3
 * Description:  ����ȼ�������� 30H-39H
 *               buf:68��ͷ�ı��Ļ�����
 *       Input:  len���������ĳ���
 *		Return:  TRUE����ȷ���� FALSE��δ��ȷ����
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::ProcessDataT3 ( const BYTE *buf, int len  )
{
	ProcessDataT1( buf, len );
	return TRUE;
}		/* -----  end of method CCjt188_2004::ProcessDataT3  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ProcessReadData
 * Description:  ������ʽ��ȷ�����ݣ� ���������� �жϺʹ���
 *       Input: buf:68��ͷ�ı��Ļ�����
 *              len���������ĳ���
 *		Return: TRUE����ȷ���� FALSE��δ��ȷ����
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::ProcessReadData ( const BYTE *buf, int len )
{
	const BYTE *pointer = buf;
	pointer += 9;
	/* �ж����� */
	//�ж�������Ч�� D6 ��ʾͨѶ�Ƿ��쳣
	if( 0x40 == ( 0x40 & *pointer )  )	
	{
		print( "ͨѶ�쳣" );	
		return FALSE;
	}

	//�ж����ݳ����Ƿ���ȷ
	pointer += 2;
	
	//�ж�d0 d1 �Ƿ���ȷ
	if( m_CfgInfo[m_bySendPos].byDI0 != *(pointer) 
		|| m_CfgInfo[m_bySendPos].byDI1 != *(pointer+1) )
	{
		sprintf( m_szPrintBuf,
				"����d0 �� d1����ȷrecv d0=%x d1=%x local d0=%x d1=%x",
				*pointer, *(pointer+1), m_CfgInfo[m_bySendPos].byDI0,m_CfgInfo[m_bySendPos].byDI1 );
		print( m_szPrintBuf );	
		return FALSE;
	}
	pointer += 2;

	//�ж�ser ����ȷ��
	if( m_bySer != *pointer )
	{
		sprintf( m_szPrintBuf,
				"������Ų���ȷrecv ser=%x local ser=%x",
				*pointer, m_bySer );
		print( m_szPrintBuf );	
		return FALSE;
	}

	/* �������� */
	if( m_byMeterType >= 0x10 && m_byMeterType <= 0x19 )
	{
		print( "ˮ������ " );
		return ProcessDataT1( buf, len );   //ˮ������
	}
	else if( m_byMeterType >= 0x20 && m_byMeterType <= 0x29 )
	{
		print( "���������� " );
		return ProcessDataT2( buf, len );   //����������
	}
	else if( m_byMeterType >= 0x30 && m_byMeterType <= 0x39 )
	{
		print( "ȼ�������� " );
		return ProcessDataT3( buf, len );   //ȼ��������
	}

	return FALSE;
}		/* -----  end of method CCjt188_2004::ProcessReadData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ProcessBuf
 * Description:  �������ձ���
 *       Input:	 ���ջ����� ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::ProcessBuf ( const BYTE *buf, int len )
{
	switch ( m_byDataType )
	{
		case CJT188_READDATA_DATATYPE:	
			print( "�����ݴ���" );
			return ProcessReadData( buf, len );
			break;

		case CJT188_TIME_DATATYPE:	
			print( "��ʱ����" );
			break;

		default:	
			sprintf( m_szPrintBuf, "δ�ҵ�����������%d", m_byDataType );
			print( m_szPrintBuf );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CCjt188_2004::ProcessBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  IsTimeToSync
 * Description:  �Ƿ��ʱ  ������ʱһ�Σ� �Ժ�CJT188_2004_SYNC_INTERVAL�����ʱ
 *       Input:  void
 *		Return:  TRUE:��ʱ FALSE:����ʱ
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::IsTimeToSync ( void )
{
	static time_t oldTime;
	time_t curTime;

	//�����Ƕ�ʱһ��
	if( m_bLinkStatus && m_bLinkTimeSyn )
	{
		oldTime = time( NULL );
		m_bLinkTimeSyn = FALSE;
		return TRUE;
	}

	//��ȡ��ǰʱ�� �Ƚ�ʱ��� ��ʱ	
	curTime = time(NULL);
	if( difftime( curTime, oldTime ) > CJT188_2004_SYNC_INTERVAL )
	{
		oldTime =  curTime;
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CCjt188_2004::IsTimeToSync  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  RequestReadData
 * Description:  ��������
 *       Input:  buf:��֯���ĵĻ����� 
 *				 len:���ĵĳ���
 *		Return:  TRUE
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::RequestReadData ( BYTE *buf, int &len )
{
	len = 0;
	for ( int i=0; i<m_byFENum; i++)
	{
		buf[len++] = 0xfe;
	}
	buf[len++] = 0x68;
	buf[len++] = m_byMeterType;
	//��ַλ
	for ( int i=0; i<7; i++)
	{
		buf[len++] = m_bySlaveAddr[i];
	}
	buf[len++] = 0x01;  //������ ������
	buf[len++] = 0x03;	//���ݳ���
	//2004Ϊ2����ʶ��
	buf[len++] = m_CfgInfo[m_bySendPos].byDI0;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI1;
	buf[len++] = ( ++m_bySer ) % 0xff;	//˳���
	buf[len++] = GetCs( buf + m_byFENum, len - m_byFENum );
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CCjt188_2004::RequestReadData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  TimeSync
 * Description:  ��ʱ����
 *       Input:	 buf:��֯���ĵĻ����� 
 *               len:���ĵĳ���
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::TimeSync ( BYTE *buf, int &len )
{
	REALTIME curTime;
	len = 0;
	for ( int i=0; i<m_byFENum; i++)
	{
		buf[len++] = 0xfe;
	}
	buf[len++] = 0x68;
	buf[len++] = m_byMeterType;
	//��ַλ
	for ( int i=0; i<7; i++)
	{
		buf[len++] = m_bySlaveAddr[i];
	}
	buf[len++] = 0x04;	//д����
	buf[len++] = 0x0a;	//���ݳ���

	//2004Ϊ2����ʶ��
	buf[len++] = m_CfgInfo[m_bySendPos].byDI0;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI1;

	buf[len++] = ( ++m_bySer ) % 0xff;	//˳���
	//ʱ��
	GetCurrentTime( &curTime );
	buf[len++] = BcdToHex( (BYTE)curTime.wSecond );
	buf[len++] = BcdToHex( (BYTE)curTime.wMinute );
	buf[len++] = BcdToHex( (BYTE)curTime.wHour);
	buf[len++] = BcdToHex( (BYTE)curTime.wDay);
	buf[len++] = BcdToHex( (BYTE)curTime.wMonth ) ;
	buf[len++] = BcdToHex( (BYTE)(curTime.wYear%100));
	buf[len++] = BcdToHex( (BYTE)(curTime.wYear/100));

	buf[len++] = GetCs( buf + m_byFENum, len - m_byFENum );
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CCjt188_2004::TimeSync  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  GetSendBuf
 * Description:	 ��ȡ���ͱ��ĺͳ���	  
 *       Input:	 ���ͻ����� ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::GetSendBuf ( BYTE *buf, int &len )
{
	switch ( m_byDataType )
	{
		case CJT188_READDATA_DATATYPE:	
			print( "��������" );
			return RequestReadData( buf, len );
			break;

		case CJT188_TIME_DATATYPE:	
			print( "��ʱ" );
			return TimeSync( buf, len );
			break;

		default:	
			sprintf( m_szPrintBuf, "Cjt188_2004 ��%d�����������������ô���", m_bySendPos );
			print( m_szPrintBuf );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CCjt188_2004::GetSendBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  InitProtocolStatus
 * Description:  ��ʼ��Э��״̬���� ������ʼ��ͨѶ�����쳣ʱ���ô˺���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::InitProtocolStatus ( void )
{
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
	m_bySer = 0;				//������0

	//�ط�����������
	m_byReSendLen = 0;
	memset( m_byReSendBuf, 0, CJT188_MAX_BUF_LEN );

	return TRUE;
}		/* -----  end of method CCjt188_2004::InitProtocolStatus  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  InitSendCfgInfo
 * Description:  ��ʼ��������Ϣ�����㷢����Ϣ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CCjt188_2004::InitSendCfgInfo ( void )
{
	Cjt188CfgInfo tCfgInfo;

	//��������Ϣ
	tCfgInfo.byDataType = CJT188_READDATA_DATATYPE;
	tCfgInfo.byDI0 = 0x1f;
	tCfgInfo.byDI1 = 0x90;
	tCfgInfo.byCycle = 1;
	m_CfgInfo.push_back( tCfgInfo );

	//д��׼ʱ��
	tCfgInfo.byDataType = CJT188_TIME_DATATYPE;
	tCfgInfo.byDI0 = 0x15;
	tCfgInfo.byDI1 = 0xA0;
	tCfgInfo.byCycle = 0;
	m_CfgInfo.push_back( tCfgInfo );
}		/* -----  end of method CCjt188_2004::InitSendCfgInfo  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_Cjt188
 *      Method:  GetDevNameToAddr
 * Description:  ͨ��װ�õ����ֶ�ȡͨѶ��ַ
 *       Input:  void
 *		Return:  
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::GetDevNameToAddr ( void )
{
	printf("%s %d %s\n", __FILE__, __LINE__, m_sDevName);
	int len = strlen( m_sDevName );
	if( len < 14)
	{
		return FALSE;
	}

	m_bySlaveAddr[6] = atoh( m_sDevName + len - 14, 2, 1 );
	m_bySlaveAddr[5] = atoh( m_sDevName + len - 12, 2, 1 );
	m_bySlaveAddr[4] = atoh( m_sDevName + len - 10, 2, 1 );
	m_bySlaveAddr[3] = atoh( m_sDevName + len - 8, 2, 1 );
	m_bySlaveAddr[2] = atoh( m_sDevName + len - 6, 2, 1 );
	m_bySlaveAddr[1] = atoh( m_sDevName + len - 4, 2, 1 );
	m_bySlaveAddr[0] = atoh( m_sDevName + len - 2, 2, 1 );
	
	printf("%s %d %s\n", __FILE__, __LINE__, m_sDevName);

	return TRUE;

}		/* -----  end of method CProtocol_Cjt188::GetDevNameToAddr  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  TimerProc
 * Description:  ʱ�䴦������ ��Ҫ����һЩ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CCjt188_2004::TimerProc ( void )
{
	if( ! m_bLinkStatus )
	{
		return;
	}
	//ʱ���ж�
	//
	//�ط���������
	if ( m_bIsReSend && m_byResendCount >= CJT188_MAX_RESEND_COUNT )
	{
		sprintf( m_szPrintBuf, "resend count %d >= %d InitProtocolStatus", m_byResendCount, CJT188_MAX_RESEND_COUNT );
		print( m_szPrintBuf );
		InitProtocolStatus(  );
	}

	//���մ����������
	if ( m_byRecvErrorCount >= CJT188_MAX_RECV_ERR_COUNT )
	{
		sprintf( m_szPrintBuf, "recv err count %d >= %d InitProtocolStatus", m_byRecvErrorCount, CJT188_MAX_RECV_ERR_COUNT );
		print( m_szPrintBuf );
		InitProtocolStatus(  );
	}
}		/* -----  end of method CCjt188_2004::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  ProcessProtocolBuf
 * Description:	 �����յ������ݻ��� 
 *       Input:  ���յ������ݻ��� ���泤��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::ProcessProtocolBuf ( BYTE *buf, int len )
{
	int pos = 0;
	BOOL bRtn = TRUE;

	print( "ProcessProtocolBuf" );
	//�жϱ��ĺ�����
	if( !WhetherBufValue( buf, len , pos ) )
	{
		//���Ĵ�����
		print ( "Cjt1886 WhetherBufValue buf Recv err!!!\n" );
		m_byRecvErrorCount ++;
		m_bIsReSend = TRUE;
		return FALSE;
	}

	//��������
	bRtn = ProcessBuf( buf+m_byFENum, len );
	if( !bRtn )
	{
		print( "�������ķ��������δ����" );
	}

	//����״̬
	m_byRecvErrorCount = 0;
	m_bLinkStatus = TRUE;
	m_bIsReSend = FALSE;
	m_byResendCount = 0;
	m_bIsSending = FALSE;

	//������ȷ����
	return TRUE;
}		/* -----  end of method CCjt188_2004::ProcessProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�����ݻ���
 *       Input:  ������ ���������ݳ��� ������Ϣ
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	BOOL bRtn = TRUE;
	//�ط�����
	if (  m_bIsReSend && m_bIsSending )
	{
		len = m_byReSendLen; 
		memcpy( buf, m_byReSendBuf, len );
		m_byResendCount ++;
	}
	else if( m_bLinkStatus && pBusMsg != NULL )
	{
		print( "������Ϣ" );
		//cjt188Ŀǰδ���κ���Ϣ���� ֱ�ӷ���
		return FALSE;
	}
	else
	{
		print( "GetSendBuf" );		
		//����λ���ƶ�
		ChangeSendPos(  );
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;

		//���ж�ʱ ����Ƕ�ʱ ��ʱ����
		if( IsTimeToSync() )
		{
			int i;
			print( "time to sync" );
			m_byDataType = CJT188_TIME_DATATYPE;	
			for( i=0; i<(int)m_CfgInfo.size(); i++ )
			{
				if( CJT188_TIME_DATATYPE == m_CfgInfo[i].byDataType )	
				{
					m_bySendPos = i;
					break;
				}
			}

			m_bIsNeedResend = FALSE;
		}

		//��֯���ͱ���
		bRtn = GetSendBuf( buf, len );

		if ( bRtn && len > 0)
		{
			//��֯�ط�����
			m_byReSendLen = len;	
			memcpy( m_byReSendBuf, buf, m_byReSendLen );
			m_bIsReSend = TRUE;

			m_bIsSending = TRUE;
			//��Բ���Ҫ�ط�����Ϣ���ô˱�ʶλ
			if( !m_bIsNeedResend )
			{
				m_bIsSending = FALSE;
				m_bIsNeedResend = TRUE;
			}
		}
	}

	return bRtn;
}		/* -----  end of method CCjt188_2004::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  Init
 * Description:	 ��ʼ��Э������  
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::Init ( BYTE byLineNo )
{
	assert( byLineNo <= 22 );

	if( !GetDevNameToAddr(  ) )
	{
		print ( "CCjt188_2004:Addr Err!!!\n" );	
		return FALSE;
	}

	//��ȡ�����ļ�
	if( !ReadCfgInfo() )
	{
		print ( "CCjt188_2004:ReadCfgInfo Err!!!\n" );	
		return FALSE;
	}
	printf("%s %d\n", __FILE__, __LINE__);
	//��ʼ��Э��״̬
	if( !InitProtocolStatus() ) 
	{
		print ( "CCjt188_2004:InitProtocolStatus Err\n" );
		return FALSE;
	}
	printf("%s %d\n", __FILE__, __LINE__);

	//��ʼ����������
	InitSendCfgInfo(  );
	if( m_CfgInfo.empty() )
	{
		printf ( "CCjt188_2004:can't find the InitSendCfgInfo\n" );
		return FALSE;
	}
	printf("%s %d\n", __FILE__, __LINE__);

	print( "Cjt188 Init OK" );
	return TRUE;
}		/* -----  end of method CCjt188_2004::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CCjt188_2004
 *      Method:  GetDevCommState
 * Description:	 ����װ������״̬  
 *       Input:
 *		Return:	 BOOL 
 *--------------------------------------------------------------------------------------
 */
BOOL CCjt188_2004::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}		/* -----  end of method CCjt188_2004::GetDevCommState  ----- */

