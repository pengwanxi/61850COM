/*
 * =====================================================================================
 *
 *       Filename:  IEC101S_2002.c
 *
 *    Description:  IEC101��վ 2002 ��
 *
 *        Version:  1.0
 *        Created:  2014��11��18�� 13ʱ32��45��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *   Organization:
 *
 *		  history:
 * =====================================================================================
 */


#include "IEC101S_2002.h"
#include <assert.h>
#include "../../share/global.h"


extern "C" void GetCurrentTime( REALTIME *pRealTime );
extern "C" int  SetCurrentTime( REALTIME *pRealTime );

typedef struct _CP56TIME2A
{/*{{{*/
	BYTE byLoMis;
	BYTE byHiMis;
	BYTE byMin;
	BYTE byHour;
	BYTE byDay;
	BYTE byMon;
	BYTE byYear;
}CP56TIME2A;/*}}}*/
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  CIEC101S_2002
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CIEC101S_2002::CIEC101S_2002 ()
{/*{{{*/
	//����װ��������ʼ��ַ
	m_wYxStartAddr = 0x0001;
	m_wYcStartAddr = 0x4001;
	m_wYkStartAddr = 0x6001;
	m_wYmStartAddr = 0x6401;
	m_wComStateAddr = 50000;

	//�ɱ仯�Ĵ���ԭ�� ������ַ �� ��Ϣ���ֽڳ���
	m_byCotLen = 1;
	m_byAddrLen = 1;
	m_byInfoAddrLen = 2;

	//������������
	m_byTotalCallYx = 1;//����ң��
	m_byTotalCallYc = 11;//����ֵ ��Ȼ�ֵ
	m_byTotalCallYm = 15;//�ۻ���

	m_byChangeYx = 1;	//������Ϣ
	m_bySoeYx = 30;    //cp56time2a ������Ϣ
	m_byChangeYc = 11; //��Ȼ�ֵ
	m_byYkType = IEC101S_2002_YKSINGLE_TYPE;//����ң��

	//��ջ�����
	memset( m_fYcBuf, 0, sizeof( m_fYcBuf ) );
	memset( m_byYxBuf, 0, sizeof( m_byYxBuf ) );
	memset( m_dwYmBuf, 0, sizeof( m_dwYmBuf ) );

	//��ʼ��Э��״̬
	InitProtocolState(  );
}  /* -----  end of method CIEC101S_2002::CIEC101S_2002  (constructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ~CIEC101S_2002
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CIEC101S_2002::~CIEC101S_2002 ()
{}  /* -----  end of method CIEC101S_2002::~CIEC101S_2002  (destructor)  ----- */

/* ====================  OtherBegin    ======================================= */
static void GetCp56Time2a( CP56TIME2A *pCp56Time )
{/*{{{*/
	REALTIME curTime;
	WORD wMilliSec;

	GetCurrentTime( &curTime );
	wMilliSec = curTime.wSecond*1000 + curTime.wMilliSec;
	pCp56Time->byLoMis = LOBYTE( wMilliSec );
	pCp56Time->byHiMis = HIBYTE( wMilliSec );
	pCp56Time->byMin = (BYTE)curTime.wMinute;
	pCp56Time->byHour = (BYTE)curTime.wHour;
	pCp56Time->byDay = LOBYTE(curTime.wDay + ( curTime.wDayOfWeek << 5 ));
	pCp56Time->byMon = (BYTE)curTime.wMonth;
	pCp56Time->byYear = curTime.wYear-2000;
}/*}}}*/

static CP56TIME2A GetTmToCp56Time2a( struct tm t, WORD wMiSec )
{/*{{{*/
	WORD wMilliSec;
	CP56TIME2A tt;

	wMilliSec = t.tm_sec*1000 + wMiSec;
	tt.byLoMis = LOBYTE( wMilliSec );
	tt.byHiMis = HIBYTE( wMilliSec );
	tt.byMin = (BYTE)t.tm_min;
	tt.byHour = (BYTE)t.tm_hour;
	tt.byDay = LOBYTE(t.tm_mday + ( t.tm_wday << 5 ));
	tt.byMon = (BYTE)t.tm_mon;
	tt.byYear = t.tm_year-100;

	return tt;
}/*}}}*/

/*{{{*/
// static float CalcAIRipeVal(BYTE wStn, WORD wPnt, float fYcVal)
// {
// float fVal = 0;
// const ANALOGITEM *pItem = Get_RTDB_Analog(byStn, wPnt);
// if( pItem  )
// {
// fVal = fYcVal * pItem->fRatio + pItem->fOffset;

// }
// return fVal;

// }
/*}}}*/

int CIEC101S_2002::GetRealVal(BYTE byType, WORD wPnt, void *v)
{/*{{{*/
	WORD  wValue = 0 ;
	switch(byType)
	{
	case 0:
		if(wPnt>=IEC101S_2002_MAX_YC_NUM) return -2;
		memcpy(v, &m_fYcBuf[wPnt], sizeof(WORD));
		break;
	case 1:
		{
			if(wPnt>=IEC101S_2002_MAX_YX_NUM)
				return -2;

			if( m_byYxBuf[ wPnt ] ==0 )
				wValue = 0;
			else
				wValue = 1;

			memcpy(v, &wValue, sizeof(WORD));
		}
		break;
	case 2:
		if(wPnt>=IEC101S_2002_MAX_YM_NUM) return -2;
		memcpy(v, &m_dwYmBuf[wPnt], sizeof(QWORD));
		break;
	default:
		return -1;
	}
	return 0;
}/*}}}*/

BOOL CIEC101S_2002::WriteAIVal(WORD wSerialNo ,WORD wPnt, float fVal)
{/*{{{*/
	if(m_pwAITrans==NULL) return FALSE;
	WORD wNum = m_pwAITrans[wPnt];
	if(wNum>m_wAISum) return FALSE;
	if(wNum<IEC101S_2002_MAX_YC_NUM)//mengqp ��<=��Ϊ< ����m_wAIBuf[4096]Խ��
	{
		float fDelt = fVal - m_fYcBuf[wNum];
		if(abs((int)fDelt)>=m_wDeadVal)
		{
			m_fYcBuf[wNum] = fVal;
			if(m_bDataInit)
			{
				AddAnalogEvt( wSerialNo , wNum, fVal);
			}
		}
	}
	return TRUE ;
}/*}}}*/

BOOL CIEC101S_2002::WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if(wNum>m_wDISum) return FALSE;
	if( wNum<IEC101S_2002_MAX_YX_NUM)//mengqp ��<= ��Ϊ<
	{
		if( m_byYxBuf[ wNum ] != wVal )
		{
			m_byYxBuf[ wNum ] = wVal ;
			if(m_bDataInit)
			{
				AddDigitalEvt( wSerialNo , wNum, wVal);
			}
		}
	}
	return TRUE ;
}/*}}}*/

BOOL CIEC101S_2002::WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal)
{/*{{{*/
	if(m_pwPITrans==NULL) return FALSE;
	WORD wNum = m_pwPITrans[wPnt];
	if(wNum>m_wPISum) return FALSE;
	if(wNum<IEC101S_2002_MAX_YM_NUM)//mengqp ��<= ��Ϊ<
	{
		m_dwYmBuf[wNum] = dwVal;
	}
	return TRUE ;
}/*}}}*/

BOOL CIEC101S_2002::WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{/*{{{*/
	if(m_pwDITrans==NULL) return FALSE;
	WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
	if(wNum>=m_wDISum) return FALSE;
	if(wNum<IEC101S_2002_MAX_YX_NUM)
	{
		AddSOEInfo(wSerialNo , wNum, wVal, lTime, wMiSecond);
	}
	return TRUE ;
}/*}}}*/
/* ====================  OtherEnd    ======================================= */

/* ====================  RecvBegin     ======================================= */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIE101S_2002
 *      Method:  SetRecvParam
 * Description:  ���ý��ղ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::SetRecvParam ( void )
{/*{{{*/
	m_bSending = FALSE;
	m_bReSending = FALSE;

	m_bySendCount = 0;
	m_byRecvCount ++;
	m_byResendCount = 0;

	m_bLinkStatus = TRUE;
}		/* -----  end of method CIE101S_2002::SetRecvParam  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessCtlBit
 * Description:  ���������� 0xf
 *       Input:  �ַ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessCtlBit ( BYTE c )
{/*{{{*/
	switch ( c & 0x0f )
	{
	case 0: //����/ȷ��֡ Զ����·��λ
		m_SendStatus = RECOGNITION;
		InitProtocolState(  );		 //��ʼ��Э�����
		m_dwSendFlag |= IEC101S_SPECIAL_DATA;
		print( "����/ȷ��֡ Զ����·��λ" );
		break;

	case 1:	//����/ȷ��֡ �û����̸�λ
		print( "����/ȷ��֡ �û����̸�λ" );
		break;

	case 2:	//����/ȷ��֡ ƽ�⴫�䱣��
		print( "����/ȷ��֡ ƽ�⴫�䱣��" );
		break;

	case 3:	//����/ȷ��֡ �û�����
		print( "����/ȷ��֡ �û�����" );
		m_SendStatus = USER_DATA;
		break;

	case 4: //����/�޻ش�֡ �û�����
		print( "����/�޻ش�֡ �û�����" );
		m_SendStatus = NONE_USER_DATA;
		break;

	case 8:	//����/��Ӧ Ҫ�����λ��Ӧ
		print( "����/��Ӧ Ҫ�����λ��Ӧ" );
		break;

	case 9:	//����/��Ӧ ������·״̬
		print( "����/��Ӧ ������·״̬" );
		m_SendStatus = LINK_STATUS;
		m_dwSendFlag |= IEC101S_SPECIAL_DATA;
		break;

	case 10://����/��Ӧ ����һ������
		print( "����/��Ӧ ����һ������" );
		m_SendStatus = LEVEL1_DATA;
		break;

	case 11://����/��Ӧ �����������
		print( "����/��Ӧ �����������" );
		m_SendStatus = LEVEL2_DATA;
		break;

	default:
		print( "������ΪЭ�̵�����Ӧ�ñ���" );
		return FALSE;
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessCtlBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  SetYkParam
 * Description:  ����ң�ز���
 *       Input:  ASDU���� ԭ�� վ�� ��� ֵ
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::SetYkParam ( BYTE byType, BYTE byCot, WORD wStn, WORD wPnt, BYTE byStatus )
{/*{{{*/
	m_byYKAsduType = byType;
	m_byYkCot = byCot;
	m_wYkStn = wStn;
	m_wYkPnt = wPnt;
	m_byYkStatus = byStatus;
}		/* -----  end of method CIEC101S_2002::SetYkParam  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsYkParamTrue
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsYkParamTrue ( BYTE byType, BYTE byCot, WORD wStn, WORD wPnt, BYTE byStatus ) const
{/*{{{*/
	BYTE byTmp;
	if( 0x2d == byType )
	{
		byTmp = IEC101S_2002_YKSINGLE_TYPE;
	}
	else if( 0x2e == byType )
	{
		byTmp = IEC101S_2002_YKDOUBLE_TYPE;
	}
	else
	{
		printf ( "IEC101S yk type=%x is err!!!\n", byType );
		return FALSE;
	}

	if( byTmp != m_byYKAsduType
			|| byCot != m_byYkCot
			|| wStn != m_wYkStn
			|| wPnt != m_wYkPnt
			|| byStatus != m_byYkStatus)
	{
		return TRUE;
	}
	else
	{
		printf ( "IEC101S YK type=%x %x cot=%x %x stn=%d %d pnt=%d %d status=%d %d\n" ,
				byType, m_byYkType, byCot, m_byYkCot, wStn, m_wYkStn, wPnt, m_wYkPnt , byStatus, m_byYkStatus);

		return FALSE;
	}
}		/* -----  end of method CIEC101S_2002::IsYkParamTrue  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessYkBuf
 * Description:  ����ң�ر���
 *       Input:
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessYkBuf ( const BYTE *buf, int len )
{/*{{{*/
	const BYTE *pointer = &buf[8];

	//����ԭ��
	BYTE byCot = *pointer++;
	if( 2 == m_byCotLen )
	{
		pointer++;
	}

	// ������ַ
	WORD wAddr = *pointer++;
	if ( 2 == m_byAddrLen )
	{
		pointer ++;
	}
	if( wAddr != m_wDevAddr )
	{
		printf ( "IEC101S ��ַ����ȷ\n" );
		return FALSE;
	}

	WORD wInfoAddr = MAKEWORD( *pointer, *(	pointer + 1 ) );
	pointer += 2;
	if( 3 == m_byInfoAddrLen )
	{
		pointer ++;
	}
	// if( wInfoAddr < 6001 || wInfoAddr > 6200 )
	// {
	// return FALSE;
	// }

	//���
	WORD wNum = wInfoAddr - m_wYkStartAddr;
	if ( wNum > m_wDOSum )
	{
		printf ( "IEC101S λ�ò���ȷ\n" );
		return FALSE;
	}
	WORD byStn = m_pDOMapTab[wNum].wStn - 1;
	WORD wPnt = m_pDOMapTab[wNum].wPntNum - 1;

	//ң��ֵ
	BYTE bySdco = *pointer;
	BYTE byStatus = 0xff;
	if( IEC101S_2002_YKSINGLE_TYPE == m_byYkType )
	{
		byStatus = ( bySdco & 0x01 );
	}
	else if( IEC101S_2002_YKDOUBLE_TYPE == m_byYkType )
	{
		byStatus = ( bySdco & 0x03 ) -1;
	}
	if(0 != byStatus && 1 != byStatus)
	{
		printf ( "IEC101S ״̬����ȷ\n" );
		return FALSE;
	}

	//ң������
	if( 0x06 == byCot )
	{
		if( 0 != (0x80 & bySdco ) )
		{
			BYTE byBusNo;
			WORD wDevAddr;
			printf ( "IEC101S ң��ѡ��\n" );
			if( m_pMethod->GetBusLineAndAddr( byStn, byBusNo, wDevAddr ) )
			{
				m_dwSendFlag = IEC101S_YK_SEL;
				SetYkParam( buf[6], byCot, byStn, wPnt, byStatus  );
				m_pMethod->SetYkSel(this, byBusNo, wDevAddr, wPnt, byStatus);
			}
		}
		else
		{
			if ( IsYkParamTrue( buf[6], byCot, byStn, wPnt, byStatus ) )
			{
				BYTE byBusNo;
				WORD wDevAddr;
				printf ( "IEC101S ң��ִ��\n" );
				if( m_pMethod->GetBusLineAndAddr( byStn, byBusNo, wDevAddr ) )
				{
					m_dwSendFlag = IEC101S_YK_EXE;
					m_pMethod->SetYkExe(this, byBusNo, wDevAddr, wPnt, byStatus);
				}
			}
		}
	}
	else if( 0x08 == byCot )
	{
		BYTE byBusNo;
		WORD wDevAddr;
		if( m_pMethod->GetBusLineAndAddr( byStn, byBusNo, wDevAddr ) )
		{
			m_dwSendFlag = IEC101S_YK_CANCEL;
			SetYkParam( buf[6], byCot, byStn, wPnt, byStatus  );
			m_pMethod->SetYkCancel(this, byBusNo, wDevAddr, wPnt, byStatus);
		}
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessYkBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessTotalCallBuf
 * Description:  ������������
 *       Input:
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessTotalCallBuf ( const BYTE *buf, int len )
{/*{{{*/
	const BYTE *pointer = &buf[6];
	pointer ++;
	//�ж��޶���
	BYTE byVsq = *pointer++;
	if( 1 != byVsq )
	{
		sprintf( m_szPrintBuf, "IEC101S totalcall VSQ=%d err", byVsq );
		print( m_szPrintBuf );
		return FALSE;
	}

	//����ԭ��
	BYTE byCot = *pointer++;
	if( 2 == m_byCotLen )
	{
		pointer++;
	}
	if( 6 != byCot )
	{
		sprintf( m_szPrintBuf, "IEC101S totalcall COT=%d err", byCot );
		print( m_szPrintBuf );
		return FALSE;
	}

	// ������ַ
	WORD wAddr = *pointer++;
	if ( 2 == m_byAddrLen )
	{
		pointer ++;
	}
	if( wAddr != m_wDevAddr )
	{
		sprintf( m_szPrintBuf, "IEC101S totalcall ADDR=%d err", wAddr );
		print( m_szPrintBuf );
		return FALSE;
	}

	//��Ϣ�����ַ
	WORD wInfoAddr = MAKEWORD( *pointer, *(	pointer + 1 ) );
	pointer += 2;
	if( 3 == m_byInfoAddrLen )
	{
		pointer ++;
	}
	if( 0 != wInfoAddr )
	{
		sprintf( m_szPrintBuf, "IEC101S totalcall INFOADDR=%d err", wInfoAddr );
		print( m_szPrintBuf );
		return FALSE;
	}

	//�ٻ��޶���
	BYTE byQoi = *pointer++;
	if( 0x14 != byQoi )
	{
		sprintf( m_szPrintBuf, "IEC101S totalcall QOI=%d err", byQoi );
		print( m_szPrintBuf );
		return FALSE;
	}
	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessTotalCallBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessYMCallBuf
 * Description:	 ����ң���л�����j
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessYMCallBuf ( const BYTE *buf, int len )
{
	const BYTE *pointer = &buf[6];
	pointer ++;
	//�ж��޶���
	BYTE byVsq = *pointer++;
	if( 1 != byVsq )
	{
		sprintf( m_szPrintBuf, "IEC101S ymcall VSQ=%d err", byVsq );
		print( m_szPrintBuf );
		return FALSE;
	}

	//����ԭ��
	BYTE byCot = *pointer++;
	if( 2 == m_byCotLen )
	{
		pointer++;
	}
	if( 6 != byCot )
	{
		sprintf( m_szPrintBuf, "IEC101S ymcall COT=%d err", byCot );
		print( m_szPrintBuf );
		return FALSE;
	}

	// ������ַ
	WORD wAddr = *pointer++;
	if ( 2 == m_byAddrLen )
	{
		pointer ++;
	}
	if( wAddr != m_wDevAddr )
	{
		sprintf( m_szPrintBuf, "IEC101S ymcall ADDR=%d err", wAddr );
		print( m_szPrintBuf );
		return FALSE;
	}

	//��Ϣ�����ַ
	WORD wInfoAddr = MAKEWORD( *pointer, *(	pointer + 1 ) );
	pointer += 2;
	if( 3 == m_byInfoAddrLen )
	{
		pointer ++;
	}
	if( 0 != wInfoAddr )
	{
		sprintf( m_szPrintBuf, "IEC101S ymcall INFOADDR=%d err", wInfoAddr );
		print( m_szPrintBuf );
		return FALSE;
	}

	//�ٻ��޶���
	BYTE byQoi = *pointer++;
	if( 0x14 != byQoi )
	{
		sprintf( m_szPrintBuf, "IEC101S ymcall QOI=%d err", byQoi );
		print( m_szPrintBuf );
		return FALSE;
	}
	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessYMCallBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessTimeSyncBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessTimeSyncBuf ( const BYTE *buf, int len )
{
	const BYTE *pointer = &buf[6];
	pointer ++;
	//�ж��޶���
	BYTE byVsq = *pointer++;
	if( 1 != byVsq )
	{
		sprintf( m_szPrintBuf, "IEC101S timesync VSQ=%d err", byVsq );
		print( m_szPrintBuf );
		return FALSE;
	}

	//����ԭ��
	BYTE byCot = *pointer++;
	if( 2 == m_byCotLen )
	{
		pointer++;
	}
	if( 6 != byCot )
	{
		sprintf( m_szPrintBuf, "IEC101S timesync COT=%d err", byCot );
		print( m_szPrintBuf );
		return FALSE;
	}

	// ������ַ
	WORD wAddr = *pointer++;
	if ( 2 == m_byAddrLen )
	{
		pointer ++;
	}
	if( wAddr != m_wDevAddr && wAddr != 0xff)
	{
		sprintf( m_szPrintBuf, "IEC101S timesync ADDR=%d err", wAddr );
		print( m_szPrintBuf );
		return FALSE;
	}

	//��Ϣ�����ַ
	WORD wInfoAddr = MAKEWORD( *pointer, *(	pointer + 1 ) );
	pointer += 2;
	if( 3 == m_byInfoAddrLen )
	{
		pointer ++;
	}
	if( 0 != wInfoAddr )
	{
		sprintf( m_szPrintBuf, "IEC101S timesync INFOADDR=%d err", wInfoAddr );
		print( m_szPrintBuf );
		return FALSE;
	}

	//cp56time2a
	REALTIME curTime;
	WORD wMiSecond = MAKEWORD( *(pointer), *(pointer + 1) );
	pointer += 2;
	curTime.wMilliSec = wMiSecond % 1000;
	curTime.wSecond = wMiSecond / 1000;
	curTime.wMinute = (*pointer++) & 0x3f;
	curTime.wHour = (*pointer++) & 0x1f;
	curTime.wDay = (*pointer++) & 0x1f;
	curTime.wMonth = (*pointer++) & 0x0f;
	curTime.wYear = ( (*pointer++) + 2000 );

	if( curTime.wSecond < 60
			&& curTime.wMinute < 60
			&& curTime.wHour < 24
			&& curTime.wDay <= 31
			&& curTime.wMonth <= 12
			&& curTime.wYear < 2030)
	{
		SetCurrentTime( &curTime );
	}
	else
	{
		sprintf( m_szPrintBuf, "IEC101 timesync SetCurrentTime err!!!time=%d-%d-%d %d:%d:%d",
				curTime.wYear, curTime.wMonth, curTime.wDay, curTime.wHour, curTime.wMinute,curTime.wSecond);
		print( m_szPrintBuf );
	}
	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessTimeSyncBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessHead68Buf
 * Description:  ������һ����������0x68 ������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessHead68Buf ( const BYTE *pBuf, int len )
{
	//�жϿ����ֵı�ʾλ�Ƿ���Ч
	if ( !ProcessJudgeFlag( pBuf[4] & 0xf0 ) )
	{
		print( "ProcessJudgeFlag" );
		return FALSE;
	}

	//����������
	if ( !ProcessCtlBit( pBuf[4] & 0x0f ) )
	{
		print( "ProcessCtlBit" );
		return FALSE;
	}

	switch ( pBuf[6] )
	{
	case 45:
		print( "����ң��" );
		m_byYkType = IEC101S_2002_YKSINGLE_TYPE;
		ProcessYkBuf( pBuf, len );
		break;

	case 46:
		print( "˫��ң��" );
		m_byYkType = IEC101S_2002_YKDOUBLE_TYPE;
		ProcessYkBuf( pBuf, len );
		break;

	case 47:
		print( "������" );
		break;

	case 48:
		print( "�趨ֵ ��һ��ֵ" );
		break;

	case 49:
		print( "�趨ֵ ��Ȼ�ֵ" );
		break;

	case 50:
		print( "�趨ֵ �̸�����" );
		break;

	case 100:
		print( "���ٻ�����" );
		if ( ProcessTotalCallBuf( pBuf, len ) )
		{
			m_dwSendFlag |= (IEC101S_TOTAL_CALL | IEC101S_TOTAL_YX | IEC101S_TOTAL_YC| IEC101S_TOTAL_CALL_END);
			m_dwSendFlag |= IEC101S_SPECIAL_DATA;
		}
		else
		{
			print( "���ٻ��������" );
		}
		break;

	case 101:
		// m_SendStatus = TOTAL_CALL_YM_BEGIN;
		print( "�������ٻ�����" );
		if ( ProcessYMCallBuf( pBuf, len ) )
		{
			m_dwSendFlag |= IEC101S_CALL_YM | IEC101S_TOTAL_YM | IEC101S_CALL_YM_END;
			m_dwSendFlag |= IEC101S_SPECIAL_DATA;
		}
		else
		{
			print( "�������ٻ��������" );
		}
		break;

	case 102:
		print( "������" );
		break;

	case 103:
		print( "ʱ��ͬ������" );
		if( ProcessTimeSyncBuf( pBuf, len ) )
		{
			m_SendStatus = TIME_SYNC;
			m_dwSendFlag |= IEC101S_TIME_SYNC ;
			m_dwSendFlag |= IEC101S_SPECIAL_DATA;
		}
		else
		{
			print( "��ʱ����" );
		}
		break;

	case 104:
		print( "��������" );
		break;

	case 105:
		print( "��λ��������" );
		break;

	case 106:
		print( "��ʱ�������" );
		break;

	default:
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessHead68Buf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessHead10Buf
 * Description:  ������һ����������0x10 ������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessHead10Buf ( const BYTE *pBuf, int len )
{
	//�жϿ����ֵı�ʾλ�Ƿ���Ч
	if ( !ProcessJudgeFlag( pBuf[1] & 0xf0 ) )
	{
		print( "IEC101S:ProcessJudgeFlag err" );
		return FALSE;
	}

	//����������
	if ( !ProcessCtlBit( pBuf[1] & 0x0f ) )
	{
		print( "IEC101S:ProcessCtlBit err" );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessHead10Buf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessRecvBuf
 * Description:  ������������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessRecvBuf ( const BYTE *pBuf, int len )
{
	if ( pBuf[0] == 0x10 )  //�̶�֡���ݴ���
	{
		return ProcessHead10Buf( pBuf, len );
	}
	else if( pBuf[0] == 0x68 ) //�仯֡���ݴ���
	{
		return ProcessHead68Buf( pBuf, len );
	}
	else//δ�ҵ�����֡
	{
		print( "ProcessRecvBuf err" );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::ProcessRecvBuf  ----- */

/* ====================  RecvEnd     ======================================= */
/* ====================  SendBegin     ======================================= */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsYkBusMsgValue
 * Description:  ң�ط�����Ϣ�Ƿ���Ч
 *       Input:  ��Ϣָ��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsYkRtnBusMsgValid ( PBUSMSG pBusMsg, DWORD dwYkType )
{
	assert ( pBusMsg != NULL );

	//�ж���Ϣ��������
	if( 1 != pBusMsg->DataNum
			|| pBusMsg->DataLen != sizeof(YK_DATA))
	{
		printf ( "IEC101S Yk Msg err\n" );
		return FALSE;
	}

	//�жϱ����Ƿ�����ң��״̬
	if( ( m_dwSendFlag & IEC101S_YK_SEL ) == 0
			&& (m_dwSendFlag & IEC101S_YK_EXE) == 0
			&& (m_dwSendFlag & IEC101S_YK_CANCEL) == 0)
	{
		printf ( "IEC101S None Yk Status\n" );
		SetYkParam( 0, 0, 0, 0, 0 );
		return FALSE;
	}

	//�жϷ��ص�ң�غͱ������ڿ��Ƶ�ң���Ƿ���ͬһ��
	YK_DATA *pData = (YK_DATA *)pBusMsg->pData;
	if( !IsYkParamTrue( m_byYKAsduType, m_byYkCot, pBusMsg->SrcInfo.wDevNo, pData->wPnt, pData->byVal ) )
	{
		printf ( "IEC101S MsgData is err\n" );
		return FALSE;
	}

	if( !( m_dwSendFlag & dwYkType ) )
	{
		printf ( "IEC101S None Yk Status\n" );
		SetYkParam( 0, 0, 0, 0, 0 );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::IsYkBusMsgValue  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  DealBusMsg
 * Description:  ����������Ϣ
 *       Input:  ��Ϣָ��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::DealBusMsg ( PBUSMSG pBusMsg )
{
	switch ( pBusMsg->byMsgType )
	{
	case YK_PROTO:
		{
			switch ( pBusMsg->dwDataType  )
			{
			case YK_SEL_RTN:
				if( IsYkRtnBusMsgValid( pBusMsg, IEC101S_YK_SEL ) )
				{
					printf ( "IEC101S YK_SEL_RTN\n" );
					m_SendStatus = YK_RTN_DATA;
				}
				break;

			case YK_EXCT_RTN:
				if( IsYkRtnBusMsgValid( pBusMsg, IEC101S_YK_EXE ) )
				{
					printf ( "IEC101S YK_EXCT_RTN\n" );
					m_SendStatus = YK_RTN_DATA;
				}
				break;

			case YK_CANCEL_RTN:
				if( IsYkRtnBusMsgValid( pBusMsg, IEC101S_YK_CANCEL ) )
				{
					printf ( "IEC101S YK_CANCEL_RTN\n" );
					m_SendStatus = YK_RTN_DATA;
				}
				break;

			default:
				print( "IEC101S:DealBusMsg can't find the datatype" );
				return FALSE;
				break;
			}				/* -----  end switch  ----- */
		}
		break;

	default:
		print( "IEC101S:DealBusMsg can't find the msgtype" );
		return FALSE;
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CIEC101S_2002::DealBusMsg  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Add68HeadAndTail
 * Description:	 Ϊ��õ�ASDU��������68��ͷβ
 *       Input:  ��֯�õ�ASDU���� ���� Ҫ��ȡ�ı��� ����
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
int  CIEC101S_2002::Add68HeadAndTail ( const BYTE *byAsduBuf, int iAsduLen, BYTE *buf )
{
	//TYPE:1 + VSQ:1 + COT:(1-2) + ADDR:(1-2) + INFOADDR(2-3) >= 6
	if( iAsduLen < 6 )
	{
		return -1;
	}
	//68Head
	buf[0] = 0x68;
	buf[1] = iAsduLen + 2;
	buf[2] = iAsduLen + 2;
	buf[3] = 0x68;
	if ( IsHaveLevel1Data(  ) )
	{
		buf[4] = 0x28;
	}
	else
	{
		buf[4] = 0x08;
	}
	buf[5] = m_wDevAddr;

	//data
	memcpy( buf+6, byAsduBuf, iAsduLen );

	//68tail
	buf[iAsduLen + 6] = GetCs( buf+4, iAsduLen + 2 );
	buf[iAsduLen + 7] = 0x16;

	return iAsduLen + 8;
}		/* -----  end of method CIEC101S_2002::Add68HeadAndTail  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetLinkStatusBuf
 * Description:  ��ȡͨѶ״̬��������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetLinkStatusBuf ( BYTE *buf, int &len )
{/*{{{*/
	len = 0;
	buf[len++] = 0x10;
	buf[len++] = 0x0b;
	buf[len++] = m_wDevAddr;
	buf[len++] = GetCs( buf + 1, 2 );		//У��ֻ��һλ!
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CIEC101S_2002::GetLinkStatusBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetReconitionBuf
 * Description:  ��ȡ��·��λ
 *       Input:  ������ ����
 *		Return:  TRUE
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetReconitionBuf ( BYTE *buf, int &len )
{/*{{{*/
	len = 0;
	buf[len++] = 0x10;
	if ( IsHaveLevel1Data(  ) )
	{
		buf[len++] = 0x20;
	}
	else
	{
		buf[len++] = 0x00;
	}
	buf[len++] = m_wDevAddr;
	buf[len++] = GetCs( buf + 1, 2 );
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CIEC101S_2002::GetReconitionBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetUserDataBuf
 * Description:  �û�����ȷ��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetUserDataBuf ( BYTE *buf, int &len )
{
	return GetLevel1Data( buf, len );
}		/* -----  end of method CIEC101S_2002::GetUserDataBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetNoneDataBuf
 * Description:  ��ȡ�����ݱ��Ļظ�
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetNoneDataBuf ( BYTE *buf, int &len )
{
	len = 0;
	buf[len++] = 0x10;
	buf[len++] = 0x09;
	buf[len++] = m_wDevAddr;
	buf[len++] = GetCs( buf + 1, 2 );
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CIEC101S_2002::GetNoneDataBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetToTalCallRecoBuf
 * Description:  ��ȡ���ٻ�ȷ�ϱ���  ASDU100
 *       Input:  ������ ���� ��־λ ����ԭ��7����ȷ�� 10������ֹ
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetTotalCallRecoBuf ( BYTE *buf, int &len, BYTE byCot )
{
	print( "����ȷ��֡" );
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x64;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = byCot;	//7����ȷ�� 10������ֹ
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��Ϣ�����ַ
	byAsduBuf[iAsduLen++] = 0x00;
	byAsduBuf[iAsduLen++] = 0x00;
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//�ٻ��޶��� AOI
	byAsduBuf[iAsduLen++] = 0x14;	//վ�ٻ�

	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::GetToTalCallRecoBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetCallYmRecoBuf
 * Description:  ��ȡ�ۼ����Ͽɱ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetCallYmRecoBuf ( BYTE *buf, int &len, BYTE byCot )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x65;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = byCot;	//7����ȷ�� 10������ֹ
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��Ϣ�����ַ
	byAsduBuf[iAsduLen++] = 0x00;
	byAsduBuf[iAsduLen++] = 0x00;
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//�ٻ��޶��� QCC
	byAsduBuf[iAsduLen++] = 0x05;	//�ܵ����������

	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::GetCallYmRecoBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetTimeSyncRecoBuf
 * Description:  ��ʱȷ�ϱ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetTimeSyncRecoBuf ( BYTE *buf, int &len, BYTE byCot )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x67;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = byCot;	//7����ȷ�� 10������ֹ
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��Ϣ�����ַ
	byAsduBuf[iAsduLen++] = 0x00;
	byAsduBuf[iAsduLen++] = 0x00;
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//CP56Time2a
	CP56TIME2A t;
	GetCp56Time2a( &t );

	byAsduBuf[iAsduLen++] = t.byLoMis;
	byAsduBuf[iAsduLen++] = t.byHiMis;
	byAsduBuf[iAsduLen++] = t.byMin;
	byAsduBuf[iAsduLen++] = t.byHour;
	byAsduBuf[iAsduLen++] = t.byDay;
	byAsduBuf[iAsduLen++] = t.byMon;
	byAsduBuf[iAsduLen++] = t.byYear;

	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::GetTimeSyncRecoBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NA_1_TotalFrame
 * Description:  ��ȡ���ٲ����� ��һ��ֵ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NA_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x09;			//TYPE
	byAsduBuf[iAsduLen++] = 0x81;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x14;			// ��Ӧվ�ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ
	//��Ϣ�����ַ
	WORD wPnt = m_wDataIndex + m_wYcStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// NVA��һ��ֵ QDSƷ�������� δ�������
	BYTE byCount = 0;
	for ( int i=m_wDataIndex; i<m_wAISum; i++)
	{
		BYTE byQds = 0;
		float fVal = m_fYcBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0 )
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_fYcBuf[i]);
				//��֯ң��ֵ
				short sVal = ( short )fVal;
		byAsduBuf[iAsduLen++] = LOBYTE(sVal);
		byAsduBuf[iAsduLen++] = HIBYTE(sVal);

		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( YC_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			byQds |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byQds;

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wAISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YC;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NA_1_TotalFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NA_1_ChangeFrame
 * Description:  ��ȡ�仯������ ��һ��ֵ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NA_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x09;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ NVA QDS
	BYTE byCount = 0;
	int iSize = m_dwAIEQueue.size();
	for ( int i=0; i<iSize; i++)
	{
		//��ȡ˳��ţ� �ڲ���ţ� ֵ
		WORD wSerialNo;
		WORD wNum;
		WORD wVal;
		float fVal;
		if( !GetAnalogEvt( wSerialNo, wNum, fVal ) )
		{
			break;
		}
		wVal = (WORD)fVal;

		//��Ϣ�����ַ
		WORD wPnt = wNum + m_wYcStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//��֯ NVA
		byAsduBuf[iAsduLen++] = LOBYTE(wVal);
		byAsduBuf[iAsduLen++] = HIBYTE(wVal);

		//��֯ QDS  δ�������
		BYTE byQds = 0;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			byQds |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byQds;

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NA_1_ChangeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NA_1_Frame
 * Description:  //��ȡ����ֵ ��һ��ֵ  ASDU9
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_ME_NA_1_TotalFrame( buf, len );
	}

	if( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_ME_NA_1_ChangeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NA_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TA_1_SoeFrame
 * Description:  //��ȡ��ʱ��ͻ�������� ��һ��ֵ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TA_1_SoeFrame ( BYTE *buf, int &len )
{
	print( "δ��֯ASDU 10 M_ME_TA_1 ͻ������" );
	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TA_1_SoeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TA_1_Frame
 * Description:  //��ȡ��ʱ�����ֵ ��һ��ֵ ASDU10
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_ME_TA_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TA_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NB_1_TotalFrame
 * Description:   //��ȡ���ٲ���ֵ ��Ȼ�ֵ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NB_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x0b;			//TYPE
	byAsduBuf[iAsduLen++] = 0x81;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x14;			// ��Ӧվ�ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ
	//��Ϣ�����ַ
	WORD wPnt = m_wDataIndex + m_wYcStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// SVA��Ȼ�ֵ QDSƷ��������
	BYTE byCount = 0;
	for ( int i=m_wDataIndex; i<m_wAISum; i++)
	{
		BYTE byQds = 0;
		float fVal = m_fYcBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0 )
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_fYcBuf[i]);
				//��֯ң��ֵ
				short sVal = (short)fVal;
		byAsduBuf[iAsduLen++] = LOBYTE(sVal);
		byAsduBuf[iAsduLen++] = HIBYTE(sVal);

		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( YC_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			byQds |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byQds;

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wAISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YC;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NB_1_TotalFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NB_1_ChangeFrame
 * Description:  //��ȡ�仯������ ��Ȼ�ֵ
 *       Input: ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NB_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x0b;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ SVA QDS
	BYTE byCount = 0;
	int iSize = m_dwAIEQueue.size();
	for ( int i=0; i<iSize; i++)
	{
		//��ȡ˳��ţ� �ڲ���ţ� ֵ
		WORD wSerialNo;
		WORD wNum;
		WORD wVal;
		float fVal;
		if( !GetAnalogEvt( wSerialNo, wNum, fVal ) )
		{
			break;
		}
		wVal = (WORD)fVal;

		//��Ϣ�����ַ
		WORD wPnt = wNum + m_wYcStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//��֯ SVA
		byAsduBuf[iAsduLen++] = LOBYTE(wVal);
		byAsduBuf[iAsduLen++] = HIBYTE(wVal);

		//��֯ QDS  δ�������
		BYTE byQds = 0;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			byQds |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byQds;

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NB_1_ChangeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NB_1_Frame
 * Description:  //��ȡ����ֵ ��Ȼ�ֵ  ASDU11
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NB_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_ME_NB_1_TotalFrame( buf, len );
	}

	if( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_ME_NB_1_ChangeFrame(buf, len);
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NB_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TB_1_SoeFrame
 * Description:  //��ȡ��ʱ��ͻ�������� ��Ȼ�ֵ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TB_1_SoeFrame ( BYTE *buf, int &len )
{
	print( "not organize M_ME_TB_1 ASDU12" );
	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TB_1_SoeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TB_1_Frame
 * Description: ��ȡ��ʱ�����ֵ ��Ȼ�ֵ ASDU12
 *       Input:  ������  ����
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TB_1_Frame ( BYTE *buf, int &len , int iFlag )
{
	if( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_ME_TB_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TB_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NC_1_TotalFrame
 * Description:  //��ȡ���ٲ���ֵ �̸�����
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NC_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x0d;			//TYPE
	byAsduBuf[iAsduLen++] = 0x81;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x14;			// ��Ӧվ�ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ
	//��Ϣ�����ַ
	WORD wPnt = m_wDataIndex + m_wYcStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// STD754�̸����� QDSƷ��������
	BYTE byCount = 0;
	for ( int i=m_wDataIndex; i<m_wAISum; i++)
	{
		//��֯ң��ֵ
		float fVal = m_fYcBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0  )
		{
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, (int)m_fYcBuf[i]);
		}

		//����fVal˳�� linux��window�ڴ��͸�����ʱ���ߵ��ֽڲ���
		BYTE byBuffer[ 4  ] ;
		BYTE byBuffer1[ 4  ] ;
		memcpy( byBuffer , &fVal , 4  ) ;
		GlobalCopyByEndian(byBuffer1,byBuffer,4);
		// byBuffer1[ 0  ] = byBuffer[ 3  ] ;
		// byBuffer1[ 1  ] = byBuffer[ 2  ] ;
		// byBuffer1[ 2  ] = byBuffer[ 1  ] ;
		// byBuffer1[ 3  ] = byBuffer[ 0  ] ;

		memcpy(&byAsduBuf[iAsduLen], byBuffer1, 4);
		iAsduLen += 4;

		// ��֯QDS
		BYTE byQds = 0;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( YC_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			byQds |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byQds;

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wAISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YC;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NC_1_TotalFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NC_1_ChangeFrame
 * Description:  //��ȡ�仯������ �̸�����
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NC_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x0d;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ STD QDS
	BYTE byCount = 0;
	int iSize = m_dwAIEQueue.size();
	for ( int i=0; i<iSize; i++)
	{
		//��ȡ˳��ţ� �ڲ���ţ� ֵ
		WORD wSerialNo;
		WORD wNum;
		// WORD wVal;
		float fYcVal;
		if( !GetAnalogEvt( wSerialNo, wNum, fYcVal ) )
		{
			break;
		}

		//��Ϣ�����ַ
		WORD wPnt = wNum + m_wYcStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//��֯ SVA
		float fVal = fYcVal;
		BYTE byBuffer1[4];
		BYTE byBuffer[4];
		if( m_pAIMapTab[wNum].wStn>0 &&
				m_pAIMapTab[wNum].wPntNum>0 )
		{
			fVal = CalcAIRipeVal(m_pAIMapTab[wNum].wStn-1,
					m_pAIMapTab[wNum].wPntNum-1,
					fVal);
			memcpy( byBuffer , &fVal , 4  ) ;
			GlobalCopyByEndian(byBuffer1,byBuffer,4);
			// byBuffer1[ 0  ] = byBuffer[ 3  ] ;
			// byBuffer1[ 1  ] = byBuffer[ 2  ] ;
			// byBuffer1[ 2  ] = byBuffer[ 1  ] ;
			// byBuffer1[ 3  ] = byBuffer[ 0  ] ;
		}
		memcpy(&byAsduBuf[iAsduLen], &byBuffer1, 4);
		iAsduLen += 4;

		//��֯ QDS  δ�������
		BYTE byQds = 0;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			byQds |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byQds;

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NC_1_ChangeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_NC_1_Frame
 * Description:  //��ȡ����ֵ �̸����� ASDU13
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_NC_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_ME_NC_1_TotalFrame( buf, len );
	}

	if( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_ME_NC_1_ChangeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_NC_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TC_1_SoeFrame
 * Description:  //��ȡ��ʱ��ͻ�������� �̸�����
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TC_1_SoeFrame ( BYTE *buf, int &len )
{
	print( "not organize M_ME_TC_1 ASDU14 soeFrame" );
	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TC_1_SoeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TC_1_Frame
 * Description:  //��ȡ��ʱ�����ֵ �̸����� ASDU14
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TC_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_ME_TC_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TC_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_ND_1_TotalFrame
 * Description:  ����ֵ�� ����Ʒ�������ʵĹ�һ��ֵ����
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_ND_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x15;			//TYPE
	byAsduBuf[iAsduLen++] = 0x81;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x14;			// ��Ӧվ�ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ
	//��Ϣ�����ַ
	WORD wPnt = m_wDataIndex + m_wYcStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// NVA��һ��ֵ
	BYTE byCount = 0;
	for ( int i=m_wDataIndex; i<m_wAISum; i++)
	{
		float fVal = m_fYcBuf[i];
		if( m_pAIMapTab[i].wStn>0 && m_pAIMapTab[i].wPntNum>0 )
			fVal = CalcAIRipeVal(m_pAIMapTab[i].wStn, m_pAIMapTab[i].wPntNum, m_fYcBuf[i]);
				//��֯ң��ֵ
				short sVal = ( short )fVal;
		byAsduBuf[iAsduLen++] = LOBYTE(sVal);
		byAsduBuf[iAsduLen++] = HIBYTE(sVal);

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wAISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YC;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_ND_1_TotalFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_ND_1_ChangeFrame
 * Description:   ����ֵ�� ����Ʒ�������ʵĹ�һ��ֵ�仯
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_ND_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x15;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ NVA
	BYTE byCount = 0;
	int iSize = m_dwAIEQueue.size();
	for ( int i=0; i<iSize; i++)
	{
		//��ȡ˳��ţ� �ڲ���ţ� ֵ
		WORD wSerialNo;
		WORD wNum;
		WORD wVal;
		float fVal;
		if( !GetAnalogEvt( wSerialNo, wNum, fVal ) )
		{
			break;
		}
		wVal = (WORD)fVal;

		//��Ϣ�����ַ
		WORD wPnt = wNum + m_wYcStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//��֯ NVA
		byAsduBuf[iAsduLen++] = LOBYTE(wVal);
		byAsduBuf[iAsduLen++] = HIBYTE(wVal);

		byCount ++;
		//ÿ������ϴ�40��ң��
		if ( byCount > 40 )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_ND_1_ChangeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_ND_1_Frame
 * Description:	 ����ֵ�� ����Ʒ�������ʵĹ�һ��ֵ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_ND_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_ME_ND_1_TotalFrame( buf, len );
	}

	if( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_ME_ND_1_ChangeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_ND_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TD_1_SoeFrame
 * Description:  //��ȡ��CP56Time2aͻ�������� ��һ��ֵ
 *       Input:  ������ ����
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TD_1_SoeFrame ( BYTE *buf, int &len )
{
	print( "not organize M_ME_TD_1 ASDU34 SoeFrame" );
	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TD_1_SoeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TD_1_Frame
 * Description:  //��ȡ��CP56Time2a����ֵ ��һ��ֵ  ASDU34
 *       Input:  ������ ���� ��ʶ
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TD_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_ME_TD_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TD_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TE_1_SoeFrame
 * Description:  //��ȡ��CP56Time2aͻ�������� ��Ȼ�ֵ
 *       Input:  ������ ����
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TE_1_SoeFrame ( BYTE *buf, int &len )
{
	print( "not organize M_ME_TE_1 ASDU35 SoeFrame" );

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TE_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TE_1_Frame
 * Description:  //��ȡ��CP56Time2a����ֵ ��Ȼ�ֵ ASDU35
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TE_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_ME_TE_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TE_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TF_1_SoeFrame
 * Description:  //��ȡ��CP56Time2aͻ�������� �̸�����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TF_1_SoeFrame ( BYTE *buf, int &len )
{
	print( "not organize M_ME_TE_1 ASDU36 SoeFrame" );

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TF_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_ME_TF_1_Frame
 * Description:  //��ȡ��CP56Time2a����ֵ �̸�����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_ME_TF_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_ME_TF_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_ME_TF_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_NA_1_TotalFrame
 * Description:  ��ȡ������Ϣ���ٱ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_NA_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x01;			//TYPE
	byAsduBuf[iAsduLen++] = 0x81;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x14;			// վ�ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��Ϣ�����ַ
	WORD wPnt;
	wPnt = m_wDataIndex + m_wYxStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// ��֯bysiq����
	BYTE byCount = 0;
	for(int i=m_wDataIndex; i<m_wDISum; i++)
	{
		WORD wVal;
		//��ȡң��ֵ
		GetRealVal(1, (WORD)i, &wVal);
		BYTE bySiq = 0 ;

		if( wVal )
			bySiq = 1 ;
		else
			bySiq = 0 ;

		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( YX_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			bySiq |= 0xC0 ;
		}

		//���
		byAsduBuf[iAsduLen++] = bySiq ;

		//������ ÿ����ഫ��128��ң��
		byCount++;
		if ( byCount >= 127 )
		{
			break;
		}
	}
	//��֯vsq
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	// printf ( "m_wDataIndex=%d m_wDISum=%d\n", m_wDataIndex, m_wDISum );
	if ( m_wDataIndex >= m_wDISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YX;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_NA_1_TotalFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_NA_1_ChangeFrame
 * Description:    //��ȡ����ʱ��ı仯������Ϣ
 *       Input:  ����������
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_NA_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x01;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ �仯ֵ
	BYTE byCount = 0;
	int iSize = m_dwDIEQueue.size();
	for(int i=0; i<iSize; i++)
	{
		WORD wSerialNo;
		WORD wPnt;
		WORD wVal;
		if(!GetDigitalEvt( wSerialNo , wPnt, wVal ))
		{
			break;
		}

		//��Ϣ�����ַ
		wPnt += m_wYxStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//�仯ֵ
		BYTE bySiq = 0;
		if ( wVal & 0x01  )
		{
			bySiq = 0x01 ;
		}
		else
		{
			bySiq = 0 ;
		}
		BOOL bDevState = FALSE ;
		bDevState = m_pMethod->GetDevCommState( wSerialNo  ) ;
		if( bDevState == COM_DEV_ABNORMAL  )
		{
			bySiq |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = bySiq;

		//�������
		byCount++;
		if ( byCount > 20 )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;			//�ɱ�ṹ�޶���VSQ

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_NA_1_ChangeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_NA_1_Frame
 * Description:  ��ȡ����ʱ��ĵ�����Ϣ֡  ASDU1
 *       Input:  ������ ���� ��־λ��0�������ٻظ��� 1�����仯 2����ͻ��ظ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_NA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	// ��������ң��
	if( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_SP_NA_1_TotalFrame( buf, len );
	}

	if( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_SP_NA_1_ChangeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_NA_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_TA_1_SoeFrame
 * Description:  //��ȡ��ʱ���ͻ�䵥����Ϣ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_TA_1_SoeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x02;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ siq cp24time2a
	BYTE byCount = 0;
	while(m_iSOE_rd_p != m_iSOE_wr_p)
	{
		WORD wSerialNo;
		WORD wPnt;
		WORD wVal;
		struct tm tmStruct;
		WORD wMiSecond;
		WORD wTime;

		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond );

		//��Ϣ�����ַ
		wPnt += m_wYxStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//siq
		wVal  = wVal & 0x0001;
		byAsduBuf[iAsduLen++] = LOBYTE(wVal);

		//cp24time2a
		wTime = tmStruct.tm_sec*1000 + wMiSecond;
		byAsduBuf[iAsduLen++] = LOBYTE(wTime);
		byAsduBuf[iAsduLen++] = HIBYTE(wTime);
		byAsduBuf[iAsduLen++] = (BYTE)tmStruct.tm_min;

		//�������
		byCount++;
		if( byCount > 20  )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;			//�ɱ�ṹ�޶���VSQ

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_TA_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_TA_1_Frame
 * Description:  //��ȡ��ʱ��ĵ�����Ϣ  ASDU2
 *       Input:  ��־λ��0�������ٻظ��� 1�����仯 2����soe�ظ�
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_TA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_SOE_TYPE ==  iFlag)
	{
		return Get_M_SP_TA_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_TA_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TA_1_TotalFrame
 * Description:  //��ȡ����ʱ�������˫����Ϣ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_NA_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;
	WORD wPnt;
	BYTE byCount = 0;

	byAsduBuf[iAsduLen++] = 0x03;			//TYPE
	byAsduBuf[iAsduLen++] = 0x81;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x14;			// վ�ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��Ϣ�����ַ
	wPnt = m_wDataIndex + m_wYxStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// ��֯bysiq����
	for(int i=m_wDataIndex; i<m_wDISum; i++)
	{
		WORD wVal;
		//��ȡң��ֵ
		GetRealVal(1, (WORD)i, &wVal);
		BYTE bySiq = 0 ;

		if( wVal )
			bySiq = 2 ;
		else
			bySiq = 1 ;

		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( YX_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			bySiq |= 0xC0 ;
		}

		//���
		byAsduBuf[iAsduLen++] = bySiq ;

		//������ ÿ����ഫ��128��ң��
		byCount++;
		if ( byCount >= 127 )
		{
			break;
		}
	}
	//��֯vsq
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wDISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YX;
		m_wDataIndex = 0;
	}

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wDISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YX;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}
	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TA_1_TotalFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TA_1_ChangeFrame
 * Description:  //��ȡ����ʱ��ı仯˫����Ϣ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_NA_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x03;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ DIQ
	BYTE byCount = 0;
	int iSize = m_dwDIEQueue.size();
	for(int i=0; i<iSize; i++)
	{
		WORD wSerialNo;
		WORD wPnt;
		WORD wVal;
		if(!GetDigitalEvt( wSerialNo , wPnt, wVal ))
		{
			break;
		}

		//��Ϣ�����ַ
		wPnt += m_wYxStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//�仯ֵ
		BYTE byDiq = 0;
		if ( wVal & 0x01  )
		{
			byDiq = 0x02 ;
		}
		else
		{
			byDiq = 0x01 ;
		}
		BOOL bDevState = FALSE ;
		bDevState = m_pMethod->GetDevCommState( wSerialNo  ) ;
		if( bDevState == COM_DEV_ABNORMAL  )
		{
			byDiq |= 0xC0 ;
		}
		byAsduBuf[iAsduLen++] = byDiq;

		//�������
		byCount++;
		if ( byCount > 20 )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;			//�ɱ�ṹ�޶���VSQ

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TA_1_ChangeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TA_1_Frame
 * Description:  //����ʱ���˫����Ϣ  ASDU3
 *       Input:  ��־λ��0�������ٻظ��� 1�����仯 2����soe�ظ�
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_NA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_DP_NA_1_TotalFrame( buf, len );
	}

	if( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_DP_NA_1_ChangeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TA_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TA_1_SoeFrame
 * Description:  ��ȡ��ʱ���˫��soe��Ϣ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_TA_1_SoeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x04;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ diq cp24time2a
	BYTE byCount = 0;
	while(m_iSOE_rd_p != m_iSOE_wr_p)
	{
		WORD wSerialNo;
		WORD wPnt;
		WORD wVal;
		struct tm tmStruct;
		WORD wMiSecond;
		WORD wTime;

		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond );

		//��Ϣ�����ַ
		wPnt += m_wYxStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//diq
		wVal  = wVal & 0x0001;
		byAsduBuf[iAsduLen++] = LOBYTE(wVal) + 1;

		//cp24time2a
		wTime = tmStruct.tm_sec*1000 + wMiSecond;
		byAsduBuf[iAsduLen++] = LOBYTE(wTime);
		byAsduBuf[iAsduLen++] = HIBYTE(wTime);
		byAsduBuf[iAsduLen++] = (BYTE)tmStruct.tm_min;

		//�������
		byCount++;
		if( byCount > 20  )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;			//�ɱ�ṹ�޶���VSQ


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TA_1_SoeFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TA_1_Frame
 * Description:  //��ȡ��ʱ���˫����Ϣ  ASDU4
 *       Input:  ������ ����
 *		Return:	  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_TA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_DP_TA_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TA_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_TB_1_SoeFrame
 * Description:  //��ȡ��ʱ��CP56Time2a��ͻ�䵥����Ϣ
 *       Input:  ������ ����
 *		Return:  BOOl
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_TB_1_SoeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x1e;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ siq cp56time2a
	BYTE byCount = 0;
	while(m_iSOE_rd_p != m_iSOE_wr_p)
	{
		WORD wSerialNo;
		WORD wPnt;
		WORD wVal;
		struct tm tmStruct;
		WORD wMiSecond;

		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond );

		//��Ϣ�����ַ
		wPnt += m_wYxStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//siq
		wVal  = wVal & 0x0001;
		byAsduBuf[iAsduLen++] = LOBYTE(wVal) ;

		//cp56time2a
		CP56TIME2A t = GetTmToCp56Time2a( tmStruct, wMiSecond );
		byAsduBuf[iAsduLen++] = t.byLoMis;
		byAsduBuf[iAsduLen++] = t.byHiMis;
		byAsduBuf[iAsduLen++] = t.byMin;
		byAsduBuf[iAsduLen++] = t.byHour;
		byAsduBuf[iAsduLen++] = t.byDay;
		byAsduBuf[iAsduLen++] = t.byMon;
		byAsduBuf[iAsduLen++] = t.byYear;

		//�������
		byCount++;
		if( byCount > 20  )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;			//�ɱ�ṹ�޶���VSQ

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_TB_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_SP_TB_1_Frame
 * Description:  //��ȡ��ʱ��CP56Time2a�ĵ�����Ϣ  ASDU30
 *       Input:  ������ ����
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_SP_TB_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_SP_TB_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_SP_TB_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TB_1_SoeFrame
 * Description:  //��ȡ��ʱ��CP56Time2a��ͻ��˫����Ϣ
 *       Input:  ������ ����
 *		Return:   BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_TB_1_SoeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x1f;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ diq cp56time2a
	BYTE byCount = 0;
	while(m_iSOE_rd_p != m_iSOE_wr_p)
	{
		WORD wSerialNo;
		WORD wPnt;
		WORD wVal;
		struct tm tmStruct;
		WORD wMiSecond;

		GetSOEInfo( wSerialNo , &wPnt, &wVal, &tmStruct, &wMiSecond );

		//��Ϣ�����ַ
		wPnt += m_wYxStartAddr;
		byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
		byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
		if ( 3 == m_byInfoAddrLen )
		{
			byAsduBuf[iAsduLen++] = 0x00;
		}

		//diq
		wVal  = wVal & 0x0001;
		byAsduBuf[iAsduLen++] = LOBYTE(wVal) + 1 ;

		//cp56time2a
		CP56TIME2A t = GetTmToCp56Time2a( tmStruct, wMiSecond );
		byAsduBuf[iAsduLen++] = t.byLoMis;
		byAsduBuf[iAsduLen++] = t.byHiMis;
		byAsduBuf[iAsduLen++] = t.byMin;
		byAsduBuf[iAsduLen++] = t.byHour;
		byAsduBuf[iAsduLen++] = t.byDay;
		byAsduBuf[iAsduLen++] = t.byMon;
		byAsduBuf[iAsduLen++] = t.byYear;

		//�������
		byCount++;
		if( byCount > 20  )
		{
			break;
		}
	}
	byAsduBuf[1] = byCount;			//�ɱ�ṹ�޶���VSQ

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TB_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_DP_TB_1_Frame
 * Description:  //��ȡ��ʱ��CP56Time2a��˫����Ϣ ASDU31
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_DP_TB_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_DP_TB_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_DP_TB_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_NA_1_TotalFrame
 * Description:  //��ȡ����ʱ��������ۼ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_NA_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x0f;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x25;			// ��Ӧ���������ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ
	WORD wPnt = m_wDataIndex + m_wYmStartAddr;
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if ( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	//��֯���ݰ� BCR �����Ƽ���������
	BYTE byCount = 0;
	for(int i=m_wDataIndex; i<m_wDISum; i++)
	{
		DWORD dwVal = (DWORD)(m_dwYmBuf[i]);
		// BCR��ʽ��24ҳ 6.4.7 ǰ�ĸ��ֽ�
		byAsduBuf[iAsduLen++]  = LOBYTE(LOWORD(dwVal));
		byAsduBuf[iAsduLen++] = HIBYTE(LOWORD(dwVal));
		byAsduBuf[iAsduLen++] = LOBYTE(HIWORD(dwVal));
		byAsduBuf[iAsduLen++] = HIBYTE(HIWORD(dwVal));


		//BCR ������ֽ�
		BYTE bySiq = 0 ;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( DD_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			bySiq |= 0xC0 ;
		}
		//���
		byAsduBuf[iAsduLen++] = bySiq ;

		//������ ÿ����ഫ��128��ң��
		byCount++;
		if ( byCount >= 20 )
		{
			break;
		}
	}
	//��֯vsq
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wPISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YM;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_NA_1_TotalFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_NA_1_ChangeFrame
 * Description:  //��ȡ����ʱ��ı仯�ۻ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_NA_1_ChangeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x0f;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ BCR


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_NA_1_ChangeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_NA_1_Frame
 * Description: //����ʱ����ۼ���  ASDU15
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_NA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_IT_NA_1_TotalFrame(buf, len);
	}

	if ( IEC101S_2002_CHANGE_TYPE == iFlag )
	{
		return Get_M_IT_NA_1_ChangeFrame(buf, len);
	}
	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_NA_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_TA_1_TotalFrame
 * Description:  //��ȡ��ʱ��������ۼ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_TA_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x10;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x25;			// ��Ӧ���������ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	//��֯���ݰ� ��Ϣ��ַ BCR CP24time2a
	BYTE byCount = 0;
	for(int i=m_wDataIndex; i<m_wDISum; i++)
	{
		DWORD dwVal = (DWORD)(m_dwYmBuf[i]);
		// BCR��ʽ��24ҳ 6.4.7 ǰ�ĸ��ֽ�
		byAsduBuf[iAsduLen++]  = LOBYTE(LOWORD(dwVal));
		byAsduBuf[iAsduLen++] = HIBYTE(LOWORD(dwVal));
		byAsduBuf[iAsduLen++] = LOBYTE(HIWORD(dwVal));
		byAsduBuf[iAsduLen++] = HIBYTE(HIWORD(dwVal));


		//BCR ������ֽ�
		BYTE bySiq = 0 ;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( DD_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			bySiq |= 0xC0 ;
		}
		//���
		byAsduBuf[iAsduLen++] = bySiq ;

		//cp24Time2a
		CP56TIME2A t;
		GetCp56Time2a( &t );
		byAsduBuf[iAsduLen++] = t.byLoMis;
		byAsduBuf[iAsduLen++] = t.byHiMis;
		byAsduBuf[iAsduLen++] = t.byMin;

		//������ ÿ����ഫ��128��ң��
		byCount++;
		if ( byCount >= 20 )
		{
			break;
		}
	}
	//��֯vsq
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wPISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YM;
		m_wDataIndex = 0;
	}


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_TA_1_TotalFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_TA_1_SoeFrame
 * Description:  //��ȡ��ʱ���ͻ���ۻ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_TA_1_SoeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x10;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ BCR CP24time2a


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_TA_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_TA_1_Frame
 * Description:  //��ʱ����ۼ���  ASDU16
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_TA_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_IT_TA_1_TotalFrame( buf, len );
	}

	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_IT_TA_1_SoeFrame( buf, len );
	}

	return FALSE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_TA_1_Frame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_TB_1_TotalFrame
 * Description:  //��ȡ��CP56Time2aʱ��������ۼ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_TB_1_TotalFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x25;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x25;			// ��Ӧ���������ٻ�
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	//��֯���ݰ� ��Ϣ��ַ BCR CP56time2a
	BYTE byCount = 0;
	for(int i=m_wDataIndex; i<m_wDISum; i++)
	{
		DWORD dwVal = (DWORD)(m_dwYmBuf[i]);
		// BCR��ʽ��24ҳ 6.4.7 ǰ�ĸ��ֽ�
		byAsduBuf[iAsduLen++]  = LOBYTE(LOWORD(dwVal));
		byAsduBuf[iAsduLen++] = HIBYTE(LOWORD(dwVal));
		byAsduBuf[iAsduLen++] = LOBYTE(HIWORD(dwVal));
		byAsduBuf[iAsduLen++] = HIBYTE(HIWORD(dwVal));


		//BCR ������ֽ�
		BYTE bySiq = 0 ;
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( DD_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		if( bDevState == COM_DEV_ABNORMAL )
		{
			bySiq |= 0xC0 ;
		}
		//���
		byAsduBuf[iAsduLen++] = bySiq ;

		//cp24Time2a
		CP56TIME2A t;
		GetCp56Time2a( &t );
		byAsduBuf[iAsduLen++] = t.byLoMis;
		byAsduBuf[iAsduLen++] = t.byHiMis;
		byAsduBuf[iAsduLen++] = t.byMin;
		byAsduBuf[iAsduLen++] = t.byHour;
		byAsduBuf[iAsduLen++] = t.byDay;
		byAsduBuf[iAsduLen++] = t.byMon;
		byAsduBuf[iAsduLen++] = t.byYear;

		//������ ÿ����ഫ��128��ң��
		byCount++;
		if ( byCount >= 20 )
		{
			break;
		}
	}
	//��֯vsq
	byAsduBuf[1] = 0x80 | byCount;

	//���ң�������ѷ����꣬ ��ȡ������ң��״̬
	m_wDataIndex += byCount;
	if ( m_wDataIndex >= m_wPISum )
	{
		m_dwSendFlag &= ~IEC101S_TOTAL_YM;
		m_wDataIndex = 0;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_TB_1_TotalFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_TB_1_SoeFrame
 * Description:  //��ȡ��CP56Time2aʱ���ͻ���ۻ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_TB_1_SoeFrame ( BYTE *buf, int &len )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	byAsduBuf[iAsduLen++] = 0x10;			//TYPE
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	byAsduBuf[iAsduLen++] = 0x03;			// ͻ�����Է���
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��֯���ݰ� ��Ϣ��ַ BCR CP56time2a


	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_TB_1_SoeFrame  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Get_M_IT_TB_1_Frame
 * Description:  //��CP56Time2aʱ����ۼ���  ASDU37
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Get_M_IT_TB_1_Frame ( BYTE *buf, int &len, int iFlag )
{
	if ( IEC101S_2002_TOTAL_TYPE == iFlag )
	{
		return Get_M_IT_TB_1_TotalFrame( buf, len );
	}

	if ( IEC101S_2002_SOE_TYPE == iFlag )
	{
		return Get_M_IT_TB_1_SoeFrame( buf, len );
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Get_M_IT_TB_1_Frame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIE101S_2002
 *      Method:  GetYkRtnDataFrame
 * Description:  ң�ط��ر���
 *       Input:  ������ ���� ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetYkRtnDataFrame ( BYTE *buf, int &len, int byYkRtnType )
{
	BYTE byAsduBuf[256];
	int iAsduLen = 0;

	if( m_byYKAsduType == 45 || m_byYKAsduType == 46 )
	{
		byAsduBuf[iAsduLen++] = m_byYKAsduType;			//TYPE
	}
	else
	{
		return FALSE;
	}
	byAsduBuf[iAsduLen++] = 0x01;			//VSQ

	//����ԭ��
	if( m_dwSendFlag & IEC101S_YK_CANCEL )
	{
		byAsduBuf[iAsduLen++] = 0x09;			// ֹͣ����ȷ��
		m_dwSendFlag &= ~IEC101S_YK_CANCEL;
	}
	else
	{
		byAsduBuf[iAsduLen++] = 0x07;			// ����ȷ��
		if( m_dwSendFlag & IEC101S_YK_SEL )
		{
			m_dwSendFlag &= ~IEC101S_YK_SEL;
		}
		else if( m_dwSendFlag & IEC101S_YK_EXE )
		{
			m_dwSendFlag &= ~IEC101S_YK_EXE;
		}
		else
		{
			return FALSE;
		}
	}
	if( 2 == m_byCotLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	//������ַ
	byAsduBuf[iAsduLen++] = LOBYTE(m_wDevAddr);	//��λ��ַ
	if ( 2 == m_byAddrLen )
	{
		byAsduBuf[iAsduLen++] = HIBYTE(m_wDevAddr); //��λ������ַ
	}

	//��Ϣ�����ַ
	WORD wPnt = m_wYkPnt + m_wYkStartAddr;
	if( wPnt < m_wYkStartAddr || wPnt > ( m_wYkStartAddr + 0x400 ) )
	{
		printf ( "IEC101S Yk pnt=%d err\n", wPnt );
		return FALSE;
	}
	byAsduBuf[iAsduLen++] = LOBYTE( wPnt );
	byAsduBuf[iAsduLen++] = HIBYTE( wPnt );
	if( 3 == m_byInfoAddrLen )
	{
		byAsduBuf[iAsduLen++] = 0x00;
	}

	// SDCS
	BYTE byStatus = 0;
	switch ( byYkRtnType )
	{
	case IEC101S_YK_SEL:
		byStatus = m_byYkStatus | 0x80;
		break;

	case IEC101S_YK_EXE:
		byStatus = m_byYkStatus;
		break;

	case IEC101S_YK_CANCEL:
		byStatus = m_byYkStatus;
		break;

	default:
		printf ( "IEC101S SDCS err\n" );
		return FALSE;
		break;
	}				/* -----  end switch  ----- */
	if( m_byYKAsduType == 46 )
	{
		byAsduBuf[iAsduLen++] = byStatus + 1;
	}
	else
	{
		byAsduBuf[iAsduLen++] = byStatus;
	}

	//���ӱ���ͷβ
	len = Add68HeadAndTail( byAsduBuf, iAsduLen, buf );
	if(len <= 0)
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIE101S_2002::GetYkRtnDataFrame  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetChangeYcData
 * Description:  ��ȡ�仯ң��
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetChangeYcData ( BYTE *buf, int &len )
{
	switch ( m_byChangeYc )
	{
	case 9: //����ֵ ��һ��ֵ
		return Get_M_ME_NA_1_Frame( buf, len, IEC101S_2002_CHANGE_TYPE );
		break;

	case 11://����ֵ ��Ȼ�ֵ
		return Get_M_ME_NB_1_Frame( buf, len, IEC101S_2002_CHANGE_TYPE );
		break;

	case 13://����ֵ �̸�����
		return Get_M_ME_NC_1_Frame( buf, len, IEC101S_2002_CHANGE_TYPE );
		break;

	case 21://����ֵ ����Ʒ�������εĹ�һ��ֵ
		return Get_M_ME_ND_1_Frame( buf, len, IEC101S_2002_CHANGE_TYPE );
		break;

	default:
		print( "IEC101S:GetTotalYcData can't find the changyc type" );
		break;
	}				/* -----  end switch  ----- */

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetChangeYcData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetLevel2Data
 * Description:  ��ȡ��������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetLevel2Data ( BYTE *buf, int &len )
{
	//���������·�׶� ��Ӧ������
	if( IsHaveSpecialData(  ) )
	{
		return GetSpecialData( buf, len );
	}

	if ( IsHaveLevel1Data(  ) )
	{
		return GetReconitionBuf( buf, len );
	}

	//ң��soe����
	if( IsHaveYxSoeData(  ) )
	{
		print( "�仯SOE" );
		//��ȡsoe����
		return GetSoeYxData (buf, len);
	}

	//�Ƿ��б仯ң������
	if( IsHaveChangeYcData(  ) )
	{
		print( "�仯ң��" );
		return GetChangeYcData( buf, len );
	}

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetLevel2Data  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsHaveLevel1Data
 * Description:  �鿴�Ƿ���1������
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsHaveLevel1Data (  )
{
	// if ( (m_dwSendFlag & IEC101S_TOTAL_YX) || IsHaveYxSoeData(  ) || IsHaveChangeYxData(  ) )
	if ( (m_dwSendFlag & IEC101S_TOTAL_YX) || IsHaveChangeYxData(  ) )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}		/* -----  end of method CIEC101S_2002::IsHaveLevel1Data  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsHaveSpecialData
 * Description:  �鿴�Ƿ����������� ��ָ������· ���� ��ʱ�Ƚ׶�
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsHaveSpecialData ( void ) const
{/*{{{*/
	if ( m_dwSendFlag & IEC101S_SPECIAL_DATA )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}		/* -----  end of method CIEC101S_2002::IsHaveSpecialData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsHaveYxSoeData
 * Description:  �Ƿ���YXSOE����
 *       Input:  void
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsHaveYxSoeData ( void ) const
{
	if ( m_iSOE_rd_p != m_iSOE_wr_p )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}		/* -----  end of method CIEC101S_2002::IsHaveYxSoeData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsHaveChangeYxData
 * Description:  �Ƿ��б仯ң��
 *       Input:	 void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsHaveChangeYxData ( void )
{/*{{{*/
	if ( m_dwDIEQueue.size( ) > 0 )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}		/* -----  end of method CIEC101S_2002::IsHaveChangeYxData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  IsHaveChangeYcData
 * Description:  �Ƿ��б仯ң��
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::IsHaveChangeYcData ( void )
{
	if ( m_dwAIEQueue.size(  ) > 0 )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}		/* -----  end of method CIEC101S_2002::IsHaveChangeYcData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetTotalYmData
 * Description:	 ����ң������
 *       Input:  ������ ����
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetTotalYmData ( BYTE *buf, int &len )
{
	switch ( m_byTotalCallYm )
	{
	case 15://�ۻ���
		print( "�ۼ���" );
		return Get_M_IT_NA_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 16://��ʱ����ۼ���
		print( "��ʱ����ۼ���" );
		return Get_M_IT_TA_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 37://��CP56Time2aʱ����ۻ���
		print( "��CP56TIME2aʱ����ۼ���" );
		return Get_M_IT_TB_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	default:
		sprintf( m_szPrintBuf, "ȫң���������ô��� m_byTotalCallYm=%d", m_byTotalCallYm );
		print( m_szPrintBuf );
		break;
	}				/* -----  end switch  ----- */
	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetTotalYmData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetTotalYcData
 * Description:	 ����ң������
 *       Input:  ������ ����
 *		Return: BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetTotalYcData ( BYTE *buf, int &len )
{
	switch ( m_byTotalCallYc )
	{
	case 9:	//��һ��ֵ
		print( "��һ��ֵ" );
		return Get_M_ME_NA_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 10://��ʱ��Ĺ�һ��ֵ
		print( "��ʱ��Ĺ�һ��ֵ" );
		break;

	case 11://��Ȼ�ֵ
		print( "��Ȼ�ֵ" );
		return Get_M_ME_NB_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 12://��ʱ��ı�Ȼ�ֵ
		print( "��ʱ��ı�Ȼ�ֵ" );
		break;

	case 13://�̸�����
		print( "�̸�����" );
		return Get_M_ME_NC_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 14://��ʱ��Ķθ�����
		print( "��ʱ��Ķ̸�����" );
		break;

	case 21://����ֵ ����Ʒ�������ʵĹ�һ��ֵ
		print( "����ֵ ����Ʒ�������ʵĹ�һ��ֵ" );
		return Get_M_ME_ND_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 34://��CP55Time2a�Ĺ�һ��ֵ
		print( "��CP55Time2a�Ĺ�һ��ֵ" );
		break;

	case 35://��CP55Time2a�ı�Ȼ�ֵ
		print( "��CP55Time2a�ı�Ȼ�ֵ" );
		break;

	case 36://��CP55Time2a�Ķ̸�����
		print( "��CP55Time2a�Ķ̸�����" );
		break;

	default:
		sprintf( m_szPrintBuf, "ȫң���������ô��� m_byTotalCallYc=%d", m_byTotalCallYc );
		print( m_szPrintBuf );
		break;
	}				/* -----  end switch  ----- */

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetTotalYcData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetTotalYxData
 * Description:  ����ң������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetTotalYxData ( BYTE *buf, int &len )
{
	switch ( m_byTotalCallYx )
	{
	case 1://������Ϣ
		print( "ȫң�ŵ�����Ϣ" );
		return Get_M_SP_NA_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 2://��ʱ��ĵ�����Ϣ
		print( "ȫң�Ŵ�ʱ��ĵ�����Ϣ" );
		break;

	case 3://˫����Ϣ
		print( "ȫң�Ŵ�ʱ��ĵ�����Ϣ" );
		return Get_M_DP_NA_1_Frame( buf, len, IEC101S_2002_TOTAL_TYPE );
		break;

	case 4://��ʱ���˫����Ϣ
		print( "ȫң��˫����Ϣ" );
		break;

	case 30://��CP56Time2aʱ��ĵ�����Ϣ
		print( "ȫң�Ŵ�CP56Time2aʱ��ĵ�����Ϣ" );
		break;

	case 31://��CP56Time2aʱ���˫����Ϣ
		print( "ȫң�Ŵ�CP56Time2aʱ��ĵ�����Ϣ" );
		break;

	default:
		sprintf( m_szPrintBuf, "ȫң���������ô��� m_byTotalCallYx=%d", m_byTotalCallYx );
		print( m_szPrintBuf );
		break;
	}				/* -----  end switch  ----- */

	return FALSE;
}		/* -----  end of method CIEC101S_2002::GetTotalYxData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetSpecialData
 * Description:  ��ȡ ���� ң�� ��ʱ ʱ�����ݱ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetSpecialData ( BYTE *buf, int &len )
{/*{{{*/
	if( m_dwSendFlag & IEC101S_TOTAL_CALL )			//����ȷ��
	{
		if ( GetTotalCallRecoBuf( buf, len, 0x07 ) )
		{
			m_dwSendFlag &= ~IEC101S_TOTAL_CALL;
			return TRUE;
		}
	}
	else if( m_dwSendFlag & IEC101S_TOTAL_YX )		//����ң������
	{
		return GetTotalYxData( buf, len );
	}
	else if( m_dwSendFlag & IEC101S_TOTAL_YC )		//����ң������
	{
		return  GetTotalYcData( buf, len );
	}
	else if( m_dwSendFlag & IEC101S_TOTAL_CALL_END ) //������ֹ
	{
		if( GetTotalCallRecoBuf( buf, len, 0x0a ) )
		{
			m_dwSendFlag &= ~IEC101S_TOTAL_CALL_END;
			m_dwSendFlag &= ~IEC101S_SPECIAL_DATA;
			return TRUE;
		}
	}
	else if( m_dwSendFlag & IEC101S_CALL_YM )		//�ٻ�ң��ȷ��
	{
		if( GetCallYmRecoBuf( buf, len, 0x07 ) )
		{
			m_dwSendFlag &= ~IEC101S_CALL_YM;
			return TRUE;
		}
	}
	else if( m_dwSendFlag & IEC101S_TOTAL_YM )      //�ٻ�ң������
	{
		return GetTotalYmData( buf, len );
	}
	else if( m_dwSendFlag & IEC101S_CALL_YM_END )    //�ٻ�ң����ֹ
	{
		if( GetCallYmRecoBuf( buf, len, 0x0a ) )
		{
			m_dwSendFlag &= ~IEC101S_CALL_YM_END;
			m_dwSendFlag &= ~IEC101S_SPECIAL_DATA;
			return TRUE;
		}
	}
	else if( m_dwSendFlag & IEC101S_TIME_SYNC )		//��ʱȷ��
	{
		if( GetTimeSyncRecoBuf( buf, len, 0x07 ) )
		{
			m_dwSendFlag &= ~IEC101S_TIME_SYNC;
			return TRUE;
		}
	}
	else if( m_dwSendFlag & IEC101S_TIME_SYNC_END ) //��ʱ��ֹ
	{
		if( GetTimeSyncRecoBuf( buf,len,0x0a ) )
		{
			m_dwSendFlag &= ~IEC101S_TIME_SYNC_END;
			m_dwSendFlag &= ~IEC101S_SPECIAL_DATA;
			return TRUE;
		}
	}

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetSpecialData  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetChangeYxData
 * Description:  ��ȡ�仯ң�ű���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetChangeYxData ( BYTE *buf, int &len )
{/*{{{*/
	switch ( m_byChangeYx )
	{
	case 1:	//������Ϣ
		return Get_M_SP_NA_1_Frame( buf, len, IEC101S_2002_CHANGE_TYPE );
		break;

	case 3:	//˫����Ϣ
		return Get_M_DP_NA_1_Frame( buf, len, IEC101S_2002_CHANGE_TYPE );
		break;

	default:
		print( "IEC101S:GetChangeYxData err" );
		break;
	}				/* -----  end switch  ----- */

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetChangeYxData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetSoeYxData
 * Description:  ��ȡsoe��Ϣ
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetSoeYxData ( BYTE *buf, int &len )
{/*{{{*/
	switch ( m_bySoeYx  )
	{
	case 2:	//��ʱ��ĵ�����Ϣ
		return Get_M_SP_TA_1_Frame( buf, len, IEC101S_2002_SOE_TYPE );
		break;

	case 4:	//��ʱ���˫����Ϣ
		return Get_M_DP_TA_1_Frame( buf, len, IEC101S_2002_SOE_TYPE );
		break;

	case 30://��CP56Time2aʱ��ĵ�����Ϣ
		return Get_M_SP_TB_1_Frame( buf, len, IEC101S_2002_SOE_TYPE );
		break;

	case 31://��CP56Time2aʱ���˫����Ϣ
		return Get_M_DP_TB_1_Frame( buf, len, IEC101S_2002_SOE_TYPE );
		break;

	default:
		print( "IEC101S GetSoeYxData err" );
		break;
	}				/* -----  end switch  ----- */

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetSoeYxData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetLevel1Data
 * Description:  ��ȡ��������
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetLevel1Data ( BYTE *buf, int &len )
{/*{{{*/
	//��������
	if( IsHaveSpecialData(  ) )
	{
		//�ܷ��ȡ����
		return GetSpecialData( buf, len );
	}

	//�仯ң������
	if( IsHaveChangeYxData(  ) )
	{
		print( "�仯ң��" );
		//��ȡ�仯ң������
		return GetChangeYxData( buf, len );
	}


	//���������� ���������ݱ���
	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIEC101S_2002::GetLevel1Data  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIE101S_2002
 *      Method:  GetYkRtnData
 * Description:  ��ȡң�ط�������
 *       Input:  buf, len
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetYkRtnData ( BYTE *buf, int &len )
{/*{{{*/
	if( m_dwSendFlag & IEC101S_YK_CANCEL )
	{
		return GetYkRtnDataFrame( buf, len, IEC101S_YK_CANCEL );
	}
	else if( m_dwSendFlag & IEC101S_YK_SEL )
	{
		return GetYkRtnDataFrame( buf, len, IEC101S_YK_SEL );
	}
	else if( m_dwSendFlag & IEC101S_YK_EXE )
	{
		return GetYkRtnDataFrame( buf, len, IEC101S_YK_EXE );
	}

	printf ( "IEC101S GetYkRtnData err\n" );

	return GetNoneDataBuf( buf, len );
}		/* -----  end of method CIE101S_2002::GetYkRtnData  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetSendBuf
 * Description:  ��ȡ���ͱ���
 *       Input:  ������ ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetSendBuf ( BYTE *buf, int &len )
{/*{{{*/
	BOOL bRtn = FALSE;

	if( m_bReSending && m_bLinkStatus )
	{
		m_SendStatus = RESEND;
	}
	// sprintf( m_szPrintBuf, "m_SendStatus=%d m_dwSendFlag=%x",m_SendStatus, m_dwSendFlag );
	// print( m_szPrintBuf );

	switch ( m_SendStatus )
	{
	case LINK_STATUS:
		print( "��Ӧ֡ ��·״̬��Ҫ�����" );
		bRtn = GetLinkStatusBuf( buf, len );
		break;

	case RECOGNITION:
		print( "ȷ��֡ �Ͽ�" );
		bRtn = GetReconitionBuf( buf, len );
		break;

	case USER_DATA:
		print( "�û�ȷ��֡ �Ͽ�" );
		bRtn = GetUserDataBuf( buf, len );
		break;

	case LEVEL2_DATA:
		print( "��������" );
		bRtn = GetLevel2Data( buf, len );
		break;

	case LEVEL1_DATA:
		print( "һ������" );
		bRtn = GetLevel1Data( buf, len );
		break;

	case YK_RTN_DATA:
		print( "ң�ط�������" );
		bRtn = GetYkRtnData( buf, len );
		break;

	case RESEND:
		print( "�ط�����" );
		memcpy( buf, m_byResendBuf, m_byResendLen );
		len = m_byResendLen;
		m_byResendCount ++;
		bRtn = TRUE;
		break;

	default:
		sprintf( m_szPrintBuf, "can't find m_SendStatus=%d", m_SendStatus );
		if( m_bLinkStatus )
		{
			print( m_szPrintBuf );
		}
		break;
	}				/* -----  end switch  ----- */

	//�����ط�
	if( bRtn )
	{
		m_byResendLen = (BYTE)len;
		memcpy( m_byResendBuf, buf, m_byResendLen );
	}

	return bRtn;
}		/* -----  end of method CIEC101S_2002::GetSendBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIE101S_2002
 *      Method:  SetSendParam
 * Description:  ���÷��Ͳ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::SetSendParam ( void )
{/*{{{*/
	m_bSending = TRUE;
	m_bySendCount ++;

	if( m_bySendCount > 1 )
	{
		m_bReSending = TRUE;
	}

	m_SendStatus = NULL_STATUS;
}		/* -----  end of method CIE101S_2002::SetSendParam  ----- *//*}}}*/

/* ====================  SendEnd     ======================================= */
/* ====================  InitBegin     ======================================= */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ReadCfgTemplate
 * Description:  ��ȡģ����Ϣ  ��ʱδ�� ��ȡ�ӿ�
 *       Input:	 void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ReadCfgTemplate ( void )
{/*{{{*/
	FILE *fp ;
	char szFileName[256] = "";
	sprintf( szFileName, "%s%s" , IEC101SPREFIXFILENAME, m_sTemplatePath);
	// print( szFileName );
	char szLineBuf[256];
	int iLineNum = 0;

	fp = fopen( szFileName, "r" );
	if( fp == NULL )
	{
		printf ( "open file %s err!!!\n", szFileName );
		return FALSE;
	}

	while ( fgets(szLineBuf, sizeof(szLineBuf), fp) != NULL && iLineNum < 30 )
	{
		iLineNum ++;
		if( strncmp( szLineBuf, "COTLEN=", 7 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[7] );
			if( 2 == byCfgVal || 1 == byCfgVal )
			{
				//1��2
				m_byCotLen = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "ADDRLEN=", 8 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[8] );
			if( 2 == byCfgVal || 1 == byCfgVal )
			{
				//1��2
				m_byAddrLen = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "INFOADDRLEN=", 12 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[12] );
			if( 2 == byCfgVal || 3 == byCfgVal )
			{
				//3��2
				m_byInfoAddrLen = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "TOTALYCTYPE=", 12 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[12] );
			if( 1 == byCfgVal || 3 == byCfgVal )
			{
				//1��3
				m_byTotalCallYx = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "TOTALYXTYPE=", 12 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[12] );
			if( 9 == byCfgVal || 11 == byCfgVal || 13==byCfgVal || 21==byCfgVal )
			{
				//9 11 13 21
				m_byTotalCallYc = byCfgVal;
			}

		}
		else if( strncmp( szLineBuf, "TOTALYMTYPE=", 12 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[12] );
			if( 15 == byCfgVal || 16 == byCfgVal || 17==byCfgVal )
			{
				//15 16 17
				m_byTotalCallYm = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "CHANGEYX=", 9 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[9] );
			if( 3 == byCfgVal || 1 == byCfgVal)
			{
				//15 16 17
				m_byChangeYx = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "CHANGEYC=", 9 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[9] );
			if( 11 == byCfgVal || 13 == byCfgVal || 21==byCfgVal || 9==byCfgVal )
			{
				//15 16 17
				m_byChangeYc = byCfgVal;
			}
		}
		else if( strncmp( szLineBuf, "SOEYX=", 6 ) == 0 )
		{
			BYTE byCfgVal = atoi( &szLineBuf[6] );
			if( 2 == byCfgVal || 4 == byCfgVal || 30==byCfgVal || 31==byCfgVal )
			{
				//15 16 17
				m_bySoeYx = byCfgVal;
			}
		}
	}

	fclose( fp );

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ReadCfgTemplate  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ReadCfgMap
 * Description:  ��ȡ�����Ϣ
 *       Input:	 void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ReadCfgMap ( void )
{/*{{{*/
	char szFileName[256] = "";
	sprintf( szFileName, "%s%s" , IEC101SPREFIXFILENAME, m_sTemplatePath);
	print( szFileName );

	//��ȡ��Ҫת�������ݵ���ģ��
	ReadMapConfig( szFileName );

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ReadCfgMap  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ReadCfgInfo
 * Description:  ��ȡ������Ϣ
 *       Input:	 void
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ReadCfgInfo ( void  )
{/*{{{*/
	if( !ReadCfgTemplate(  ) )
	{
		return FALSE;
	}

	if( !ReadCfgMap(  ) )
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ReadCfgInfo  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  InitProtocolState
 * Description:  ��ʼ��Э�����
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::InitProtocolState ( void )
{/*{{{*/
	m_bLinkStatus = FALSE;
	m_dwSendFlag |= IEC101S_SPECIAL_DATA;

	//ң�ز���
	m_byYKAsduType = 0;
	m_byYkCot = 0;
	m_wYkStn = 0;
	m_wYkPnt = 0;
	m_byYkStatus = 0;

	m_wDataIndex = 0; //���ݱ��  ��������
	m_bDataInit = 1; //ʼ��Ϊ1

	//�÷���״̬
	m_bSending = FALSE;
	m_bReSending = FALSE;

	//�ü���״̬
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_byResendCount = 0;

	//����ط�����
	memset( m_byResendBuf, 0, 256 );
	m_byResendLen = 0;

	//��FCB
	m_bFcb = FALSE;
}		/* -----  end of method CIEC101S_2002::InitProtocolState  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  InitProtocolTransTab
 * Description:  ��ʼ��ת����Ϣ
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::InitProtocolTransTab ( void )
{/*{{{*/
	//��ȡת�����
	CreateTransTab();
}		/* -----  end of method CIEC101S_2002::InitProtocolTransTab  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  InitProtocolData
 * Description:  ��ʼ��Э������
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::InitProtocolData ( void )
{/*{{{*/
	//���ڴ����ݿ���--��ȡת����Ĭ������
	m_pMethod->ReadAllYcData(&m_fYcBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	m_pMethod->ReadAllYxData( &m_byYxBuf[ 0 ] ) ;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  InitProtocol
 * Description:  ��ʼ��Э��״̬
 *       Input:	 void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::InitProtocol ( void )
{/*{{{*/
	//��ʼ������״̬����
	InitProtocolState(  );

	//��ʼ�������Ϣ
	InitProtocolTransTab(  );

	//��ʼ��Э������
	InitProtocolData(  );
}		/* -----  end of method CIEC101S_2002::InitProtocol  ----- *//*}}}*/

/* ====================  InitEnd     ======================================= */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIE101S_2002
 *      Method:  TimerProc
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC101S_2002::TimerProc ( void )
{/*{{{*/
	ReadChangData();
	// sprintf ( m_szPrintBuf, "sendcount=%d recvcout=%d m_bLinkStatus=%d yc=%d yx=%d" , m_bySendCount, m_byRecvCount,m_bLinkStatus,m_dwAIEQueue.size(  ) ,m_dwDIEQueue.size(  ) );
	// print( m_szPrintBuf );
	if( m_bySendCount > IEC101S_2002_MAX_SEND_COUNT )
	{
		print( "IEC101S m_bySendCount > 3" );
		InitProtocolState(  );
	}

	if( m_byResendCount > IEC101S_2002_MAX_RESEND_COUNT )
	{
		print( "IEC101S m_RCount > 3" );
		InitProtocolState(  );
	}
}		/* -----  end of method CIE101S_2002::TimerProc  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  ProcessProtocolBuf
 * Description:  ����Э�鱨��
 *       Input:  ������ָ�� ����
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::ProcessProtocolBuf ( BYTE *pBuf, int len )
{/*{{{*/
	//�жϱ��� ���ҳ�һ֡���ʱ���
	if( !WhetherBufValid( pBuf, len ) )
	{
		print( "WhetherBufValid err" );
		m_bReSending = TRUE;
		return FALSE;
	}

	//��������
	if( !ProcessRecvBuf( pBuf, len ) )
	{
		print( "IEC101S:ProcessRecvBuf err!!!" );
		return FALSE;
	}

	//���ý��ղ���
	SetRecvParam(  );

	return TRUE;
}		/* -----  end of method CIEC101S_2002::ProcessProtocolBuf  ----- *//*}}}*/



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�鱨��
 *       Input:  ������ ���� ��Ϣָ��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{/*{{{*/
	assert( buf != NULL );
	BOOL bRtn = TRUE;

	if( pBusMsg )
	{
		//������Ϣ
		if ( !DealBusMsg( pBusMsg ) )
		{
			printf( "IEC101S:��Ϣ��������\n" );
			// return FALSE;
		}
	}

	//��ȡ���ͱ���
	if( !GetSendBuf( buf, len ) )
	{
		bRtn = FALSE;
	}
	// sprintf (m_szPrintBuf,  "len=%d\n", len );
	// print( m_szPrintBuf );

	//���÷��Ͳ���
	if( m_bLinkStatus )
	{
		// print( "set" );
		SetSendParam(  );
		if( bRtn )
		{
			m_byRecvCount = 0;
		}
	}

	return bRtn;
}		/* -----  end of method CIEC101S_2002::GetProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC101S_2002
 *      Method:  Init
 * Description:  ��ʼ��Э��
 *       Input:	 ���ߺ�
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC101S_2002::Init ( BYTE byLineNo )
{/*{{{*/
	//�������ļ�
	if ( !ReadCfgInfo(  ) )
	{
		printf( "IEC101S_2002:ReadCfgInfo err!!!\n" );
		return FALSE;
	}

	//��ʼ��Э��
	InitProtocol(  );

	return TRUE;
}		/* -----  end of method CIEC101S_2002::Init  ----- *//*}}}*/
