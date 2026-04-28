/*
 * =====================================================================================
 *
 *       Filename:  ModBusSD96E3.cpp
 *
 *    Description: SD ϵ���Ǳ��Ĳ���ֵ������ 03H ���������
	SD ϵ�в�����ַ����
	����Ϊ������Ϣ������ַ�� 03H ��������� 10H ������д
	��ַ ���� ��д
	���� ˵�� ��������
	0000H
	��� UA
	(��������)
	���ѹ UAB
	(��������)
	R word
	0001H
	���ѹ UB
	(��������)
	���ѹ UBC
	(��������)
	R word
	0002H
	���ѹ UC
	(��������)
	���ѹ UAC
	(��������)
	R word
	0003H ���� IA R word
	0004H ���� IB R word
	0005H ���� IC R word
	0006H �й����� P R word
	0007H �޹����� Q R word
	0008H �������� �� R word
	0009H Ƶ�� F R
	���ݸ�ʽ����
	�� �� �� �� �� �� ��
	��������ȣ����ö���
	����ʾ�� ������ǰ����
	���ں� ����ϸ�������
	���·�����˵����
	һ �� �� �� �� �� ��
	������ȣ����ñ�׼��
	IEEE754 ������������
	�ȣ���ʽ��ʾ��
	ע���������ݵ���
	С�̶�Ϊ 0.1Wh������
	�� �� �� �� �� �� �� Ϊ
	10000(2710H)ʱ�� ����
	�� ʱ �� �� Ϊ 10000 ��
	0.1Wh��1KWh��
	word
	000AH
	��ѹ����
	������Ϣ R/W word
	000BH
	����
	������Ϣ R/W
	��ϸ����������·���
	����Ϣ˵��
	word
	000CH-0
	00DH
	���β�
	���й����� R/W Dword
	000EH-0
	00FH
	���β�
	���޹����� R/W
	��������ʾ��
	������ǰ�������ں�
	Dword
	0010H-0
	011H
	һ�β�
	���й����� R Dword
	0012H-0
	013H
	һ�β�
	���޹����� R
	IEEE754 ������
	�������ȣ���ʽ
	Dword
	SD ϵ�ж๦�ܵ�������Ǳ��û��ֲ� V1.4 ��
	17
	����˵������ 16 ���Ʊ�ʾ��
	��λ��������� 01 03 00 00 00 14 45 C5
	���� 01H Ϊ�Ǳ���ַ�� 03H Ϊ��� 0000H ΪҪ��ѯ
	��������ʼ��ַ�� 0014H ָҪ��ѯ�����ݳ���Ϊ 20 ���֣� 45C5H
	Ϊ CRC У���롣
	�Ǳ���Ӧ�� 01 03 28 XX�� 20 ���֣� CRC ��
	����յ���ѹ����Ϊ�� 03 E8H����ת��Ϊʮ�������� 1000
	����յ����ε�������Ϊ�� 00 00 2710��������� 2710H��
	ת��Ϊʮ��������Ϊ 10000��
	ע���������ݵ���С�̶�Ϊ 0.1Wh�������������������Ϊ
	10000(2710H)ʱ��������ʱ����Ϊ 10000��0.1Wh��1KWh��
	������Ϣ˵������ 16 ���Ʊ�ʾ��
	���ֽ� ���ֽ�
	��ѹ����������Ϣ ����ָ������ ��ѹָ������
	���ʸ�����Ϣ ����ָ������ ���ʷ���
	��ѹ�������ʵ�ָ�����ֺ���Ϊ��
	�����ѹ����������Ϣ�Ĵ����ķ�������Ϊ 02 03H�������ָ��
	���ֶ���Ϊ 02H����ѹָ�����ֶ���Ϊ 03H���� A ������� 0003H����
	��Ϊ 1388H��ʮ������ 5000������ A �����ʵ��ֵΪ�� 5000/10000����
	102=50A����ѹ�����ʶ���Ҳ��֮���ơ�
	���ʷ��Ų��ֺ���Ϊ��
	���ʷ��Ŷ��� 00H 01H 10H 11H
	�й����ʷ��� �� �� �� ��
	�޹����ʷ��� �� �� �� ��
 *
 *        Version:  1.0
 *        Created:  2017��1��9�� 11ʱ40��18��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  ykk (), 

 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */

#include <math.h>
#include "ModBusSD96E3.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  CModBusSD96E3
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CModBusSD96E3::CModBusSD96E3 ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
}  /* -----  end of method CModBusSD96E3::ModBusSD96E3  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  ~CModBusSD96E3
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CModBusSD96E3::~CModBusSD96E3 ()
{
}  /* -----  end of method CModBusSD96E3::~CModBusSD96E3  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  WhetherBufValue
 * Description:  �鿴���ձ�����Ч�� 
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusSD96E3::WhetherBufValue ( BYTE *buf, int &len )
{
	BYTE *pointer = buf;
	WORD wCrc;
	int pos = 0;

	while( len >= 4 )
	{
		//�жϵ�ַ
		if ( *pointer != m_wDevAddr )
		{
			goto DEFAULT;
		}

		//�жϹ�����
		if ( *( pointer + 1 ) != 0x03 )
		{
			goto DEFAULT;
		}

		//�ж�У��
		wCrc = GetCrc( pointer, ( *( pointer + 2 ) + 3 ) );
		if( *( pointer + ( *( pointer + 2 ) + 3 ) ) !=  HIBYTE(wCrc)
		 || *( pointer + ( *( pointer + 2 ) + 4 ) ) !=  LOBYTE(wCrc))
		{
			goto DEFAULT;
		}
			
		buf = buf + pos;
		len = *(pointer + 2) + 5;
		return TRUE;
DEFAULT:
		pointer ++;
		len --;
		pos ++;
	}
	return FALSE ;
}		/* -----  end of method CModBusSD96E3::WhetherBufValue  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  ProcessRecvBuf
 * Description:  ��������  ��������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusSD96E3::ProcessRecvBuf ( BYTE *buf, int len )
{
	if( len < 15 )
	{
		return FALSE;
	}
	BYTE IPower=0 , UPower=0 , FPower=0 , FSignBit=0;
	IPower = buf[23];
	UPower = buf[24];
	FSignBit = buf[25];
	FPower = buf[26];
	//printf("iuff %d %d %d %d \n ",IPower,UPower,FPower,FSignBit);
	for( int i=0; i<10; i++ )
	{	//printf("yc i =%d\n",i);
		float fYcValue = 0.0;	
		fYcValue = (float)( MAKEWORD( buf[4 + 2 * i], buf[ 3 + 2 * i ] ) );
		if( i < 3 )
			fYcValue = (fYcValue/10000)*pow(10,UPower);
		else if( i > 2 && i < 6 )
			fYcValue = (fYcValue/10000)*pow(10,IPower);
		else if( i > 5 && i < 8 )
		{
			fYcValue = (fYcValue/10000)*pow(10,FPower)/1000;
			switch( FSignBit )
			{
				case 0x00:
				break;
				case 0x01:
					if( i == 6 )
						fYcValue = -fYcValue; 
				break;
				case 0x10:
					if( i == 7 )
						fYcValue = -fYcValue; 
				break;
				case 0x11:
					fYcValue = -fYcValue; 
				break;
			}
		}
		else if( i == 8 )
			fYcValue = fYcValue*0.001;
		else if( i == 9 )
			fYcValue = fYcValue*0.01;
		//printf("yc m_SerialNo=%d fYcValue =%f\n",m_SerialNo,fYcValue);
		m_pMethod->SetYcData( m_SerialNo, i, fYcValue );
		//printf("SetYcData m_SerialNo=%d fYcValue =%f\n",m_SerialNo,fYcValue);
		// printf ( "yc pnt=%d val=%f\n", i, fYcValue );
	}
	for( int i=0; i<2; i++ )
	{//printf("ym i =%d\n",i);
		float fYmValue = 0.0;	
		fYmValue = *(float *)(buf+35 + 4*i );
		fYmValue = fYmValue/10000;
		//printf("ym m_SerialNo=%d fYmValue =%f\n",m_SerialNo,fYmValue);
		m_pMethod->SetYmData( m_SerialNo, i, (QWORD)fYmValue );
		//printf("SetYmData m_SerialNo=%d fYmValue =%f\n",m_SerialNo,fYmValue);
	}

	return TRUE;
}		/* -----  end of method CModBusSD96E3::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  GetProtocolBuf
 * Description:  ��ȡ���ͱ���  
 *       Input:  ������ ���� ��Ϣ����ң�ص���Ϣ���� ʼ��ΪNULL��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusSD96E3::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	len = 0;

	buf[len++] = m_wDevAddr;
	buf[len++] = 0x03;

	buf[len++] = 0x00;
	buf[len++] = 0x00;
	buf[len++] = 0x00;
	buf[len++] = 0x14;
	
	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);

	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method CModBusSD96E3::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  ProcessProtocolBuf
 * Description:  �������ձ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusSD96E3::ProcessProtocolBuf ( BYTE *buf, int len )
{
	if ( !WhetherBufValue( buf, len ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "CModBusSD96E3 recv buf err !!!\n" );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );

		m_byRecvCount ++;
		return FALSE;	
	}
	ProcessRecvBuf( buf, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method CModBusSD96E3::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  Init
 * Description:  ��ʼ��Э��
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusSD96E3::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method CModBusSD96E3::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  TimerProc
 * Description:  ʱ�Ӵ���
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModBusSD96E3::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			OutBusDebug( m_byLineNo, (BYTE *)"CModBusSD96E3:unlink\n", 30, 2 );
		}
	}
}		/* -----  end of method CModBusSD96E3::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusSD96E3
 *      Method:  GetDevCommState
 * Description:  ��ȡװ��״̬
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusSD96E3::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModBusSD96E3::GetDevCommState  ----- */

