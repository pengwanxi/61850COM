/*
 * =====================================================================================
 *
 *       Filename:  Dlt645_2007.cpp
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
#include "Dlt645_2007.h"


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  CDlt645_2007
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_2007::CDlt645_2007 ()
{/*{{{*/
	InitProtocolStatus(  );
}  /* -----  end of method CDlt645_2007::CDlt645_2007  (constructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  ~CDlt645_2007
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_2007::~CDlt645_2007 ()
{
}  /* -----  end of method CDlt645_2007::~CDlt645_2007  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  ProcessYcData
 * Description:  ң�⴦��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::ProcessYcData ( const BYTE *buf, int len )
{/*{{{*/
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	DWORD dwYcVal = 0;
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

	pointer = buf + 14;
	while( byDataNum > 0 )
	{
		float fYcVal;

		CalFormatData( pointer, byDataFormat, byDataLen, dwYcVal);
		fYcVal = (float)dwYcVal;
		m_pMethod->SetYcData( m_SerialNo, wPnt, fYcVal );

		pointer += byDataLen;
		wPnt++;
		byDataNum--;
		sprintf( m_szPrintBuf, "yc pnt:%d value:%f", wPnt, fYcVal );
		print( m_szPrintBuf );
	}

	return TRUE;
}		/* -----  end of method CDlt645_2007::ProcessYcData  ----- *//*}}}*/


BOOL CDlt645_2007::ProcessYxData(const BYTE *buf, int len)
{
	return FALSE;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  ProcessYmData
 * Description:  ң������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::ProcessYmData ( const BYTE *buf, int len )
{/*{{{*/
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	DWORD dwYmVal = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;
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
	while( byDataNum > 0 )
	{
		CalFormatData( pointer, byDataFormat, byDataLen, dwYmVal );
		m_pMethod->SetYmData( m_SerialNo, wPnt, (QWORD)dwYmVal );

		pointer += byDataLen;
		wPnt++;
		byDataNum--;

		sprintf( m_szPrintBuf, "ym pnt:%d value:%lu", wPnt, dwYmVal );
		print( m_szPrintBuf );
	}

	return TRUE;
}		/* -----  end of method CDlt645_2007::ProcessYmData  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  ProcessBuf
 * Description:  �������ձ���
 *       Input:	 ���ջ���������
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::ProcessBuf ( const BYTE *buf, int len )
{/*{{{*/
	switch ( m_byDataType )
	{
		case DLT645_YC_DATATYPE:
			print( "ң������" );
			ProcessYcData( buf, len );
			break;

		case DLT645_YM_DATATYPE:
			print( "ң������" );
			ProcessYmData( buf, len );
			break;

		case DLT645_YX_DATATYPE:
			print("Yx process");
			ProcessYxData(buf, len);
		default:
			sprintf( m_szPrintBuf, "δ�ҵ�����������%d", m_byDataType );
			print( m_szPrintBuf );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CDlt645_2007::ProcessBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  IsTimeToSync
 * Description:  �Ƿ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::IsTimeToSync ( void )
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
}		/* -----  end of method CDlt645_2007::IsTimeToSync  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  RequestReadData
 * Description:  ��������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::RequestReadData ( BYTE *buf, int &len )
{/*{{{*/
	len = 0;
	for ( int i=0; i<m_CfgInfo[m_bySendPos].byFENum; i++)
	{
		buf[len++] = 0xfe;
	}
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
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CDlt645_2007::RequestReadData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  TimeSync
 * Description:  ��ʱ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::TimeSync ( BYTE *buf, int &len )
{/*{{{*/
	REALTIME curTime;
	len = 0;
	for ( int i=0; i<m_CfgInfo[m_bySendPos].byFENum; i++)
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
	buf[len++] = (BYTE)(curTime.wSecond + 0x33);
	buf[len++] = (BYTE)curTime.wMinute + 0x33;
	buf[len++] = (BYTE)curTime.wHour + 0x33;
	buf[len++] = (BYTE)curTime.wDay + 0x33;
	buf[len++] = (BYTE)curTime.wMonth + 0x33;
	buf[len++] = (BYTE)(curTime.wYear-2000)+ 0x33;
	buf[len++] = GetCs( buf, 16 );
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CDlt645_2007::TimeSync  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  GetSendBuf
 * Description:	 ��ȡ���ͱ��ĺͳ���
 *       Input:	 ���ͻ����� ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::GetSendBuf ( BYTE *buf, int &len )
{/*{{{*/
	switch ( m_byDataType )
	{
		case DLT645_YC_DATATYPE:

		case DLT645_YM_DATATYPE:
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
}		/* -----  end of method CDlt645_2007::GetSendBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  InitProtocolStatus
 * Description:  ��ʼ��Э��״̬����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::InitProtocolStatus ( void )
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
}		/* -----  end of method CDlt645_2007::InitProtocolStatus  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  TimerProc
 * Description:  ʱ�䴦������ ��Ҫ����һЩ��ʱ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDlt645_2007::TimerProc ( void )
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
}		/* -----  end of method CDlt645_2007::TimerProc  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  ProcessProtocolBuf
 * Description:	 �����յ������ݻ���
 *       Input:  ���յ������ݻ��� ���泤��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::ProcessProtocolBuf ( BYTE *buf, int len )
{/*{{{*/
	print( "ProcessProtocolBuf" );
	char sbuf[500] = { 0 };
	int i = 0;
	int index = 0;
	for (i = 0; i < len; i++)
	{
		sprintf(sbuf + index++, "%02x", buf[i]);
		strcat(sbuf + index++, " ");
	}
	
	strcat(sbuf+index++,"   ProcessBuf\n");
	printf(sbuf);

	int pos = 0;
	BOOL bRtn = TRUE;
	if( !WhetherBufValue( buf, len , pos ) )
	{
		//���Ĵ�����
		print ( "Dlt6456 WhetherBufValue buf Recv err!!!\n" );
		m_byRecvErrorCount ++;
		m_bIsReSend = TRUE;
		return FALSE;
	}

	bRtn = ProcessBuf( buf+pos, len );
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
}		/* -----  end of method CDlt645_2007::ProcessProtocolBuf  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�����ݻ���
 *       Input:  ������ ���������ݳ��� ������Ϣ
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{/*{{{*/
	BOOL bRtn = TRUE;
	//�ط�����
	if ( m_bLinkStatus && m_bIsReSend )
	{
		len = m_byReSendLen;
		memcpy( buf, m_byReSendBuf, len );
		m_byResendCount ++;
	}
	else if( m_bLinkStatus && pBusMsg != NULL )
	{
		print( "������Ϣ" );
		//dlt645Ŀǰδ���κ���Ϣ���� ֱ�ӷ���
		return FALSE;
	}
	else
	{
		print( "GetSendBuf" );
		ChangeSendPos(  );
		m_byDataType = m_CfgInfo[m_bySendPos].byDataType;
		if( IsTimeToSync() )
		{
			m_byDataType = DLT645_TIME_DATATYPE;
			m_bIsNeedResend = FALSE;
		}

		bRtn = GetSendBuf( buf, len );

		if ( bRtn && len > 0)
		{
			m_byReSendLen = len;
			memcpy( m_byReSendBuf, buf, m_byReSendLen );
			m_bIsSending = TRUE;
			if( !m_bIsNeedResend )
			{
				m_bIsSending = FALSE;
				m_bIsNeedResend = TRUE;
			}
			else
			{
			    m_bIsReSend = TRUE;
			}
				
				
			char sbuf[500] = { 0 };	
			int i = 0;
			int index = 0;
			for (i = 0; i < len; i++)
			{
				sprintf(sbuf + index++, "%02x", buf[i]);
				strcat(sbuf + index++, " ");
			}
			
			strcat(sbuf+index++,"   sendBuf\n");
			printf(sbuf);
		}
	}

	return bRtn;
}		/* -----  end of method CDlt645_2007::GetProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_Cjt188
 *      Method:  GetDevNameToAddr
 * Description:  ͨ��װ�õ����ֶ�ȡͨѶ��ַ
 *       Input:  void
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::GetDevNameToAddr ( void )
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
 *       Class:  CDlt645_2007
 *      Method:  Init
 * Description:	 ��ʼ��Э������
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::Init ( BYTE byLineNo )
{/*{{{*/
	if( byLineNo > 22 )
		return FALSE;

	// if( !GetDevNameToAddr(  ) )
	// {
	// 	print ( "CDlt645_2007:Addr Err!!!\n" );
	// 	return FALSE;
	// }

	if( !ReadCfgInfo() )
	{
		print ( "CDlt645_2007:ReadCfgInfo Err!!!\n" );
		return FALSE;
	}

	if( !InitProtocolStatus() )
	{
		print ( "CDlt645_2007:InitProtocolStatus Err\n" );
		return FALSE;
	}

	print( "Dlt645 Init OK" );
	return TRUE;
}		/* -----  end of method CDlt645_2007::Init  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_2007
 *      Method:  GetDevCommState
 * Description:	 ����װ������״̬
 *       Input:
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_2007::GetDevCommState ( void )
{/*{{{*/

	if ( m_bLinkStatus )
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}		/* -----  end of method CDlt645_2007::GetDevCommState  ----- *//*}}}*/
