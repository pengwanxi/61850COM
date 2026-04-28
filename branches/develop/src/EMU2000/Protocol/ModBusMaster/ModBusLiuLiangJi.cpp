/*
 * =====================================================================================
 *
 *       Filename:  ModBusLiuLiangJi.cpp
 *
 *    Description:   �����˼ҼҾ��̳���Ŀ����ͬ�ţ�SM-16-XM-QD007�����н�������
				
				4��485Э�����˵����
							modbusЭ��
					������:9600
					У��λ:��
					����λ:8
					ֹͣλ:1
					Ѱַ��ʽ�������ţ�H +������� 01H+�����ݳ��ȣ�11H
					ͨѶ��ʽ���첽����
					���ݳ��ȣ���11Hλ
					1λ������H���Ǳ����Ϊ����������b���Χ0~255�������䣩
					2λ��������01H
					3λ: ���ݳ���11H
					4λ��������λ�ͽ��ʣ���˵����
					5��6��7λ������ֵ
					 ��ʽ��16λ������ACCBHI��bit15Ϊ����λ����ACCBLO��EXPB����ͬ
					8��9��10λ���¶�ֵ����λ��(��ʽͬ��)
					11��12��13λ��ѹ��ֵ����λMPa(��ʽͬ��) 
					14��15��16λ���ܶ�ֵ����λKg/ m3(��ʽͬ��)
					17��18��19��20λ���ۼ�����
					��ʽ���ɸߵ���Ϊ17λ��X8 ,X7��,18λ��X6 ,X5��,19λ��X4, X3��,
					20λ��X2, X1����С����ʼ��Ĭ����X2��X1֮�䡣
					21λ��CRC���ֽ� 
					22λ��CRC���ֽ�
 *        Version:  1.0
 *        Created:  2016��05��20�� 09ʱ45��00��
 *       Revision:  none
 *       Compiler:  
 *
 *         Author:  ykk
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */


#include "ModBusLiuLiangJi.h"
#include <math.h>

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  ModBusLiuLiangJi
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
ModBusLiuLiangJi::ModBusLiuLiangJi ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
}  /* -----  end of method CModBusXHMCU::CModBusXHMCU  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  ~ModBusLiuLiangJi
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
ModBusLiuLiangJi::~ModBusLiuLiangJi ()
{
}  /* -----  end of method CModBusXHMCU::~CModBusXHMCU  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  print
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void ModBusLiuLiangJi::print ( char *buf ) const
{

#ifdef  XHMCU_PRINT
	OutBusDebug( m_byLineNo, (BYTE *)buf, strlen( buf ), 2 );
#endif     /* -----  not XHMCU_PRINT  ----- */
}		/* -----  end of method CModBusXHMCU::print  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  WhetherBufValue
 * Description:  �鿴���ձ�����Ч�� 
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLiuLiangJi::WhetherBufValue ( BYTE *buf, int &len, int &pos )
{
	BYTE *pointer = buf;
	pos = 0;

	while( len >= 17 )
	{
		//�ж�ͬ����
		if( m_wDevAddr != *pointer
				|| 0x01 != *( pointer + 1 )
				|| 0x11 != *( pointer + 2 ) )
		{
			print( (char *)"����ͷ" );
			pointer ++;
			len --;
			pos ++;
			continue;
		}
		//�жϵ�ַ
		WORD wCRC = GetCrc( buf, buf[2] + 3 );
		if( ( HIBYTE(wCRC) == buf[ buf[2] + 3 ] ) && ( LOBYTE(wCRC) == buf[ buf[2] + 4 ] ) )
		{
			return TRUE;
		}
		else
		{
			print( (char *)"У��δͨ��" );
			pointer ++;
			len --;
			pos ++;
			continue;
		}
	}

	print( (char *) "ModBusLiuLiangJi WhetherBufValue not find right buf ");

	return FALSE ;
}		/* -----  end of method ModBusLiuLiangJi::WhetherBufValue  ----- */

	
/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  ProcessRecvBuf
 * Description:  ��������  ��������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLiuLiangJi::ProcessRecvBuf ( BYTE *buf, int len )
{
	BYTE i;
	double wVal;
	
	m_pMethod->SetYcData ( m_SerialNo , 0 , (float)buf[3] );
	for( i=1;i<5;i++ )
	{
		wVal = ((float)(buf[4+((i-1)*3)]<<8 & buf[5+((i-1)*3)])/(float)0x8000)*pow( 2 , buf[6+((i-1)*3)] );
		m_pMethod->SetYcData ( m_SerialNo , i , wVal );
		
	}
	wVal = (buf[16]>>4)*1000000 + (buf[16]&0x0F)*100000 + (buf[17]>>4)*10000 + (buf[17]&0x0F)*1000 /
			(buf[18]>>4)*100 + (buf[18]&0x0F)*10 + (buf[19]>>4) + (float)(buf[19]&0x0F)*0.1;
	m_pMethod->SetYmData( m_SerialNo , 5 , (QWORD)wVal );

	
	return TRUE;
}		/* -----  end of method ModBusLiuLiangJi::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  GetProtocolBuf
 * Description:  ��ȡ���ͱ���  
 *       Input:  ������ ���� ��Ϣ����ң�ص���Ϣ���� ʼ��ΪNULL��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLiuLiangJi::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	len = 0;

	//��֯ͬ��ͷ
	buf[len++] = m_wDevAddr;
	buf[len++] = 0x01;
	buf[len++] = 0x00;
	buf[len++] = 0x00;// �Ĵ�����ַ 
	buf[len++] = 0x00; 
	buf[len++] = 0x11; // �Ĵ�������

	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);
	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method ModBusLiuLiangJi::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  ProcessProtocolBuf
 * Description:  �������ձ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLiuLiangJi::ProcessProtocolBuf ( BYTE *buf, int len )
{
	int pos = 0;
	if ( !WhetherBufValue( buf, len, pos ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusLiuLiangJi recv buf err !!!\n" );
		print( szBuf );
		m_byRecvCount ++;
		return FALSE;	
	}
	
	ProcessRecvBuf( buf + pos, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method ModBusLiuLiangJi::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  Init
 * Description:  ��ʼ��Э��
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLiuLiangJi::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method ModBusLiuLiangJi::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  TimerProc
 * Description:  ʱ�Ӵ���
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void ModBusLiuLiangJi::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			print( (char *) "ModBusLiuLiangJi:unlink\n");
		}
	}
}		/* -----  end of method ModBusLiuLiangJi::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLiuLiangJi
 *      Method:  GetDevCommState
 * Description:  ��ȡװ��״̬
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLiuLiangJi::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method ModBusLiuLiangJi::GetDevCommState  ----- */

