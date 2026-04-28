/*
 * =====================================================================================
 *
 *       Filename:  CIEC103.cpp
 *
 *    Description:  ����ڱ�׼103���д���
 *
 *        Version:  1.0
 *        Created:  2014��10��09�� 09ʱ29��57��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *   Organization:  esdtek
 *		  history:
 *
 * =====================================================================================
 */
/* SQ */
//		 SQ=0 Ѱַһ����Ϣ����˳�����ϢԪ�أ����ڱ���ֵ�ͱ���¼���Ŷ�����
//		 SQ=1 Ѱַ������ϢԪ�ػ��ۺ���ϢԪ�أ�����Ϣ���ַѰַ�ĵ�����ϢԪ�ػ��ۺ���ϢԪ�أ�
/* COT */
//		 <1>��=�Է���ͻ����
//		 <2>��=ѭ��
//		 <3>��=��λ֡����λ��FCB��
//		 <4>��=��λͨ�ŵ�Ԫ��CU��
//		 <5>��=����/��������
//		 <6>��=��Դ����
//		 <7>��=����ģʽ
//		 <8>��=ʱ��ͬ��
//		 <9>��=�ܲ�ѯ�����ٻ���
//		 <10>��=�ܲ�ѯ�����ٻ�����ֹ
//		 <11>��=���ز���
//		 <12>��=Զ������
//	     <20>��=����Ŀ϶��Ͽ�
//		 <21>��=����ķ��Ͽ�
//		 <31>��=�Ŷ����ݵĴ���
//		 <40>��=ͨ��д����Ŀ϶��Ͽ�
//		 <1>��=ͨ��д����ķ��Ͽ�
//		 <2>��=��ͨ�ö�������Ч������Ӧ
//		 <3>��=��ͨ�ö�������Ч������Ӧ
//		 <4>��=ͨ��дȷ��
#include "IEC103.h"
#include "../../share/global.h"



#define	IEC103DEBUG		1	//[> �ն˴�ӡ <]
#define	IEC103BUSDEBUG			        /* ���ߴ�ӡ */
#define	IEC103DISPLAYCOT			/* ��ʾ����ԭ��  */


extern "C" void GetCurrentTime( REALTIME *pRealTime );
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  CIEC103
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CIEC103::CIEC103 ()
{/*{{{*/
	InitProtocolStatus(  );
}  /* -----  end of method CIEC103::CIEC103  (constructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ~CIEC103
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CIEC103::~CIEC103 ()
{/*{{{*/
	m_IEC103_CfgInfo.clear();
}  /* -----  end of method CIEC103::~CIEC103  (destructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  print
 * Description:  ��ӡ
 *       Input:	 ������ ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC103::print ( char *buf, int len )
{/*{{{*/
#ifdef  IEC103DEBUG
	printf ( "%s\n", buf );
#endif     /* -----  not IEC103DEBUG  ----- */

#ifdef  IEC103BUSDEBUG
	OutBusDebug( m_byLineNo, (BYTE *)buf, strlen(buf), 2 );
#endif     /* -----  not IEC103BUSDEBUG  ----- */
}		/* -----  end of method CIEC103::print  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  DisplayCot
 * Description:  ��ʾ����ԭ�� ����鿴
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC103::DisplayCot ( BYTE byCot )
{/*{{{*/

#ifdef  IEC103DISPLAYCOT

	//����ԭ��   //����δд
	switch ( byCot )
	{
		case 1:
			print( (char *)"�Է�(ͻ��)" );
			break;

		case 2:
			print( (char *)"ѭ��" );
			break;

		case 3:
			print( (char *)"��λ֡����" );
			break;

		case 4:
			print( (char *)"��λͨ�ŵ�Ԫ" );
			break;

		case 5:
			print( (char *)"����/��������" );
			break;

		case 6:
			print( (char *)"��Դ����" );
			break;

		case 7:
			print( (char *)"����ģʽ" );
			break;

		case 8:
			print( (char *)"ʱ��ͬ��" );
			break;

		case 9:
			print( (char *)"�ܲ�ѯ" );
			break;

		case 10:
			print( (char *)"�ܲ�ѯ��ֹ" );
			break;

		case 11:
			print( (char *)"���ز���" );
			break;

		case 12:
			print( (char *)"Զ������" );
			break;

		case 20:
			print( (char *)"����϶��Ͽ�" );
			break;

		case 21:
			print( (char *)"������Ͽ�" );
			break;

		case 31:
			print( (char *)"�Ŷ����ݴ���" );
			break;

		case 40:
			print( (char *)"ͨ��д����϶��Ͽ�" );
			break;

		case 41:
			print( (char *)"ͨ��д������Ͽ�" );
			break;

		case 42:
			print( (char *)"ͨ�ö�������Ч������Ӧ" );
			break;

		case 43:
			print( (char *)"ͨ�ö�������Ч������Ӧ" );
			break;

		case 44:
			print( (char *)"ͨ������дȷ��" );
			break;

		default:
			break;
	}				/* -----  end switch  ----- */

#endif     /* -----  not IEC103DISPLAYCOT  ----- */
}		/* -----  end of method CIEC103::DisplayCot  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  GetModulePnt
 * Description:	 ��ȡ�������͵��
 *       Input:	 �������� ������ ��Ϣ��� ���
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::GetModulePnt( BYTE byDataType, BYTE byFunType, BYTE byInfoIndex, WORD &wPnt )
{/*{{{*/
	int i;
	for ( i=0; i<(int)m_IEC103_CfgInfo.size(); i++ )
	{
		//printf ("1 = %d, i= %d\n",m_IEC103_CfgInfo[i].DataType, i);
		if( byDataType == m_IEC103_CfgInfo[i].DataType )
		{
			//printf("2 = %d %d %d\n",m_IEC103_CfgInfo[i].FunType, m_IEC103_CfgInfo[i].InfoIndex, m_IEC103_CfgInfo[i].DataNum  );
			//printf( "3 = %d\n", m_IEC103_CfgInfo[i].AddInfo );
			if( (byFunType == m_IEC103_CfgInfo[i].FunType)
				&& (byInfoIndex >= m_IEC103_CfgInfo[i].InfoIndex)
				&& (byInfoIndex < (m_IEC103_CfgInfo[i].InfoIndex + m_IEC103_CfgInfo[i].DataNum)))
			{
				if ( m_IEC103_CfgInfo[i].AddInfo == 0  )
				{
					wPnt = byInfoIndex - m_IEC103_CfgInfo[i].InfoIndex + m_IEC103_CfgInfo[i].StartIndex;
					return TRUE;
				}
			}
		}
	}

	if ( i >= (int)m_IEC103_CfgInfo.size() )
	{
		//sprintf( DebugBuf, "DataType=%d FunType=%d InfoIndex=%d not found",byDataType, byFunType, byInfoIndex );
		//print( DebugBuf );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC103::GetModulePnt  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  GetModuleInfo
 * Description:	 yk�ü�!
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::GetModuleInfo ( BYTE byDataType,  WORD wPnt, CfgInfo &tCfgInfo, BYTE &byFunType, BYTE &byInfoIndex )
{/*{{{*/
	int i;
	for ( i=0; i<(int)m_IEC103_CfgInfo.size(  ); i++ )
	{
		if( byDataType == m_IEC103_CfgInfo[i].DataType )
		{
			if(wPnt >= m_IEC103_CfgInfo[i].StartIndex
				&& wPnt < m_IEC103_CfgInfo[i].StartIndex + m_IEC103_CfgInfo[i].DataNum )
			{
				byFunType = m_IEC103_CfgInfo[i].FunType;
				byInfoIndex = wPnt - m_IEC103_CfgInfo[i].StartIndex + m_IEC103_CfgInfo[i].InfoIndex;
				memcpy( &tCfgInfo, &m_IEC103_CfgInfo[i], sizeof(CfgInfo));
				return TRUE;
			}
		}
	}

	if ( i >= (int)m_IEC103_CfgInfo.size() )
	{
		sprintf( DebugBuf, "yk wPnt = %d not found", wPnt);
		print( DebugBuf );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC103::GetModuleInfo  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_IRCFS_TA_3_Frame
 * Description:  ��ʶ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_IRCFS_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE Cot = buf[8];

	switch ( Cot )
	{
		case 0x03://��λ֡����λ(FCB)
			break;

		case 0x04://��λͨ�ŵ�Ԫ(CU)
			print( (char *)"��λͨ�ŵ�Ԫ(CU)" );
			m_SendStatus = C_SYN_Ta_3;//ʱ��ͬ��
			break;

		case 0x05://����/��������
			print( (char *)"����/�������� �л�ʱ��ͬ��" );
			m_SendStatus = C_SYN_Ta_3;//ʱ��ͬ��
			break;

		case 0x06://��Դ����
			break;

		default:
			return FALSE;
			break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CIEC103::M_IRCFS_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  C_SYN_TA_3_Frame
 * Description:	 ʱ��ͬ�� ASDU6
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::C_SYN_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	return TRUE;
}		/* -----  end of method CIEC103::C_SYN_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  C_TGI_NA_3_Frame
 * Description:  //�ܲ�ѯ���� ASDU8
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_TGI_NA_3_Frame ( BYTE *buf, int len  )
{/*{{{*/
	return TRUE;
}		/* -----  end of method CIEC103::C_TGI_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_TTM_TA_3_Frame
 * Description:	 ��ʱ��ı���	ASDU1
 *       Input:
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_TTM_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byFunType = 0;
	BYTE byInfoIndex = 0;
	BYTE byYxValue;
	WORD wPnt;
	TIMEDATA tTimeData;
	TIMEDATA *pTime = &tTimeData;
	WORD wMiSec;

	//�ж�֡������ĳ��ȺͿɱ�ṹ�޶���
	if ( len != 20 || ((buf[7] & 0x7f) != 1) )
		return FALSE;

	DisplayCot( buf[8] );

	byFunType = buf[10];
	byInfoIndex = buf[11];

	//��ȡң�ŵ��
	if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex, wPnt ) )
	{
		return FALSE;
	}

	//����ң��ֵ
	byYxValue = buf[12] & 0x03;
	if( byYxValue != 0x02 && byYxValue != 0x01)
	{
		return FALSE;
	}

	//����ң��
	m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue-1 );

	//����ʱ��
	wMiSec = buf[13] |  buf[14] << 8;
	pTime->MiSec = wMiSec % 1000;
	pTime->Second = wMiSec/1000;
	pTime->Minute = buf[15] & 0x3f;
	pTime->Hour = buf[16] & 0x1f;

	REALTIME curTime;
	GetCurrentTime( &curTime );
	pTime->Day = curTime.wDay;
	pTime->Month = curTime.wMonth;
	pTime->Year = curTime.wYear - 1900;

	//����soe
	// m_pMethod->SetYxDataWithTime( m_SerialNo, wPnt, byYxValue-1, pTime);
// 	sprintf( DebugBuf, "YxUpdate:dev%d pnt%d=%d time=%d-%d-%d %d:%d:%d", m_wDevAddr, wPnt, byYxValue-1,
// 			pTime->Year+1900,
// 			pTime->Month,
// 			pTime->Day,
// 			pTime->Hour,
// 			pTime->Minute,
// 			pTime->Second);
// 	print( DebugBuf );

	return TRUE ;
}		/* -----  end of method CIEC103::M_TTM_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_TMR_TA_3_Frame
 * Description:  �����ʱ���ʱ�걨�� ASDU2
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_TMR_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byFunType = 0;
	BYTE byInfoIndex = 0;
	BYTE byYxValue;
	WORD wPnt;
	TIMEDATA tTimeData;
	TIMEDATA *pTime = &tTimeData;
	WORD wMiSec;

	//�ж�֡������ĳ��ȺͿɱ�ṹ�޶���
	if ( len != 24 || ((buf[7] & 0x7f) != 1) )
		return FALSE;

	DisplayCot( buf[8] );

	byFunType = buf[10];
	byInfoIndex = buf[11];

	//��ȡң�ŵ��
	if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex, wPnt ) )
	{
		return FALSE;
	}

	//����ң��ֵ
	byYxValue = buf[12] & 0x03;
	if( byYxValue != 0x02 && byYxValue != 0x01)
	{
		return FALSE;
	}

	//����ң��
	m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue-1 );

	//����ʱ��
	wMiSec = buf[17] |  buf[18] << 8;
	pTime->MiSec = wMiSec % 1000;
	pTime->Second = wMiSec/1000;
	pTime->Minute = buf[19] & 0x3f;
	pTime->Hour = buf[20] & 0x1f;

	REALTIME curTime;
	GetCurrentTime( &curTime );

	pTime->Day = curTime.wDay;
	pTime->Month = curTime.wMonth;
	pTime->Year = curTime.wYear - 1900;

	//����soe
	// m_pMethod->SetYxDataWithTime( m_SerialNo, wPnt, byYxValue-1, pTime);
	sprintf( DebugBuf, "YxUpdate:dev%d pnt%d=%d time=%d-%d-%d %d:%d:%d", m_wDevAddr, wPnt, byYxValue-1,
			pTime->Year+1900,
			pTime->Month,
			pTime->Day,
			pTime->Hour,
			pTime->Minute,
			pTime->Second);
	print( DebugBuf );

	return TRUE;
}		/* -----  end of method CIEC103::M_TMR_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_SP_NA_3_Frame
 * Description:  ���ٻ�ʱ����ĵ�����Ϣ״̬֡ ASDU40
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_SP_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wPnt;
	int i=0;

	byYxNum = buf[7] & 0x7f;

	DisplayCot( buf[8] );
	//�ж�SQ
	if( 0x80 & buf[7] )
	{
		//SQ = 1 ���ݸ�ʽΪ fun0 inf0 val0 fun1 inf1 val1
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+yxnum*3+sin:1+cs:1+0x16:1
		if ( ( 13 + 3 * byYxNum ) != len )
			return FALSE;

		for ( i=0; i<byYxNum; i++ )
		{
			byFunType = buf[10 + 3 * i];
			byInfoIndex = buf[11 + 3 * i];

			//��ȡң�ŵ��
			if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex, wPnt ) )
			{
				continue;
			}

			//����ң��ֵ
			byYxValue = buf[12 + 3 * i] & 0x01;

			//����ң��
			m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
		}
	}
	else
	{
		//SQ = 0 ���ݸ�ʽΪ fun0 inf0 val0 val1
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+yxnum+sin:1+cs:1+0x16:1
		if ( ( 15 + byYxNum ) != len )
			return FALSE;

		byFunType = buf[10];
		byInfoIndex = buf[11];
		for ( i=0; i<byYxNum; i++ )
		{
			//��ȡң�ŵ��
			if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
			{
				continue;
			}

			//����ң��ֵ
			byYxValue = buf[12 + i] & 0x01;

			//����ң��
			m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
		}
	}


	return TRUE;
}		/* -----  end of method CIEC103::M_SP_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_DP_NA_3_Frame
 * Description:  ˫����Ϣ״̬֡ ASDU42
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_DP_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wPnt;
	int i=0;

	byYxNum = buf[7] & 0x7f;
	DisplayCot( buf[8] );
	//�ж�SQ
	if( 0x80 & buf[7] )
	{
		//SQ = 1 ���ݸ�ʽΪ fun0 inf0 val0 fun1 inf1 val1
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+yxnum*3+sin:1+cs:1+0x16:1
		if ( ( 13 + 3 * byYxNum ) != len )
			return FALSE;

		for ( i=0; i<byYxNum; i++ )
		{
			byFunType = buf[10 + 3 * i];
			byInfoIndex = buf[11 + 3 * i];

			//��ȡң�ŵ��
			if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex, wPnt ) )
			{
				continue;
			}

			//����ң��ֵ
			byYxValue = buf[12 + 3 * i] & 0x03;
			if ( byYxValue != 0x01 && byYxValue != 0x02 )
			{
				continue;
			}

			//����ң��
			m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue-1 );

		}
	}
	else
	{
		//SQ = 0 ���ݸ�ʽΪ fun0 inf0 val0 val1
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+yxnum+sin:1+cs:1+0x16:1
		if ( ( 15 + byYxNum ) != len )
			return FALSE;

		byFunType = buf[10];
		byInfoIndex = buf[11];
		for ( i=0; i<byYxNum; i++ )
		{
			//��ȡң�ŵ��
			if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
			{
				//return FALSE;
				////���һ֡�����м����ȡ��ϳ�������Ͽ������״̬���ϲ�����!
				continue;
			}

			//����ң��ֵ
			//byYxValue = buf[12 + 3 * i] & 0x03;			//ΪʲôҪ����3?
			byYxValue = buf[12 + i] & 0x03;
			if ( byYxValue != 0x01 && byYxValue != 0x02 )
			{
				continue;
			}

			//����ң��
			m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue-1 );

		}
	}
	return TRUE;
}		/* -----  end of method CIEC103::M_DP_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_SP_TA_3_Frame
 * Description:	 ��ʱ��ĵ�����Ϣ״̬�仯֡ ASDU41
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_SP_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wMiSec;
	TIMEDATA tTimeData;
	TIMEDATA *pTime = &tTimeData;
	REALTIME curTime;
	WORD wPnt;
	int i=0;

	byYxNum = buf[7] & 0x7f;
	DisplayCot( buf[8] );

	//fun0 inf0 val0 misecL0 misecH0 min0 hour0
	//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+yxnum*7+sin:1+cs:1+0x16:1
	if ( ( 13 + 7 * byYxNum ) != len )
		return FALSE;

	GetCurrentTime( &curTime );

	for ( i=0; i<byYxNum; i++)
	{
		byFunType = buf[10 + 7 * i];
		byInfoIndex = buf[11 + 7 * i];

		//��ȡң�ŵ��
		if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex, wPnt ) )
		{
			continue;
		}

		//����ң��ֵ
		byYxValue = buf[12 + 7 * i] & 0x01;

		//����ң��
		m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );

		//����ʱ��
		wMiSec = buf[13] |  buf[14] << 8;
		pTime->MiSec = wMiSec % 1000;
		pTime->Second = wMiSec/1000;
		pTime->Minute = buf[15] & 0x3f;
		pTime->Hour = buf[16] & 0x1f;

		pTime->Day = curTime.wDay;
		pTime->Month = curTime.wMonth;
		pTime->Year = curTime.wYear - 1900;

		//����soe
		m_pMethod->SetYxDataWithTime( m_SerialNo, wPnt, byYxValue, pTime);
	}
	return TRUE ;
}		/* -----  end of method CIEC103::M_SP_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_DP_TA_3_Frame
 * Description:  ��ʱ���˫����Ϣ״̬�仯֡ ASDU43
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_DP_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wPnt;
	TIMEDATA tTimeData;
	TIMEDATA *pTime = &tTimeData;
	REALTIME curTime;
	WORD wMiSec;
	int i=0;

	byYxNum = buf[7] & 0x7f;

	//fun0 inf0 val0 misecL0 misecH0 min0 hour0
	//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+yxnum*7+sin:1+cs:1+0x16:1
	if ( ( 13 + 7 * byYxNum ) != len )
		return FALSE;
	DisplayCot( buf[8] );

	GetCurrentTime( &curTime );

	for ( i=0; i<byYxNum; i++)
	{
		byFunType = buf[10 + 7 * i];
		byInfoIndex = buf[11 + 7 * i];

		//��ȡң�ŵ��
		if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex, wPnt ) )
		{
			continue;
		}

		//����ң��ֵ
		byYxValue = buf[12 + 7 * i] & 0x03;
		if ( byYxValue != 0x01 && byYxValue != 0x02 )
		{
			continue;
		}

		//����ң��
		m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue-1 );

		//����ʱ��
		wMiSec = buf[13] |  buf[14] << 8;
		pTime->MiSec = wMiSec % 1000;
		pTime->Second = wMiSec/1000;
		pTime->Minute = buf[15] & 0x3f;
		pTime->Hour = buf[16] & 0x1f;

		pTime->Day = curTime.wDay;
		pTime->Month = curTime.wMonth;
		pTime->Year = curTime.wYear - 1900;

		//����soe
		m_pMethod->SetYxDataWithTime( m_SerialNo, wPnt, byYxValue-1, pTime);

	}
	return TRUE ;
}		/* -----  end of method CIEC103::M_DP_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_SS_NA_3_Frame
 * Description:  ���ٻ�ʱ����ĵ���״̬��״̬�仯��Ϣ֡ ASDU44
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_SS_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wSt;
	WORD wPnt;
	int i=0, j=0;

	byYxNum = buf[7] & 0x3f;
	DisplayCot( buf[8] );

	if ( buf[7] & 0x80 )
	{
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+yxnum*7+sin:1+cs:1+0x16:1
		//7 =fun:1+inf:1+SCD:4 + QDS:1
		if( 13 + 7 * byYxNum != len )
			return FALSE;

		for ( i=0; i<byYxNum; i++)
		{
			byFunType = buf[10 + 7 * i];
			byInfoIndex = buf[11 + 7 * i];
			wSt = buf[12 + 7 * i] | buf[13 + 7 * i];

			for ( j=0; j<16; j++)
			{
				//��ȡң�ŵ��
				if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+j+i*16, wPnt ) )
				{
					continue;
				}

				if( wSt & (1 << j) )
				{
					byYxValue = 1;
				}
				else
				{
					byYxValue = 0;
				}

				//����ң��
				m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
			}
		}

	}
	else
	{
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+yxnum*5+sin:1+cs:1+0x16:1
		//5 = SCD:4 + QDS:1
		if( 15 + 5 * byYxNum != len )
			return FALSE;

		byFunType = buf[10 + 5 * i];
		byInfoIndex = buf[11 + 5 * i];

		for ( i=0; i<byYxNum; i++)
		{
			wSt = buf[12 + 5 * i] | buf[13 + 5 * i];

			for ( j=0; j<16; j++)
			{
				//��ȡң�ŵ��
				if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+j+i*16, wPnt ) )
				{
					continue;
				}

				if( wSt & (1 << j) )
				{
					byYxValue = 1;
				}
				else
				{
					byYxValue = 0;
				}

				//����ң��
				m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
			}
		}

	}
	return TRUE;
}		/* -----  end of method CIEC103::M_SS_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_DS_NA_3_Frame
 * Description:  ���ٻ�ʱ����ĵ���״̬��״̬�仯��Ϣ֡  ASDU46
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_DS_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wSt;
	WORD wPnt;
	int i=0, j=0;

	byYxNum = buf[7] & 0x3f;
	DisplayCot( buf[8] );

	if ( buf[7] & 0x80 )
	{
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+yxnum*7+sin:1+cs:1+0x16:1
		//7 =fun:1+inf:1+SCD:4 + QDS:1
		if( 13 + 7 * byYxNum != len )
			return FALSE;

		for ( i=0; i<byYxNum; i++)
		{
			byFunType = buf[10 + 7 * i];
			byInfoIndex = buf[11 + 7 * i];
			wSt = buf[12 + 7 * i] | buf[13 + 7 * i];

			for ( j=0; j<8; j++)
			{
				//��ȡң�ŵ��
				if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+j+i*8, wPnt ) )
				{
					continue;
				}

				//0 3 �м�״̬��ȷ��״̬ 1��2��
				if( ( wSt & ( 0x03 << (2*j) ) ) >> (2*j) ==0x02)
				{
					byYxValue=1;
				}
				else if((wSt & ( 0x03 << (2*j) ) ) >> (2*j) == 0x01)
				{
					byYxValue=0;
				}
				else
				{
					continue;
				}

				//����ң��
				m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
			}
		}

	}
	else
	{
		//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+yxnum*5+sin:1+cs:1+0x16:1
		//5 = SCD:4 + QDS:1
		if( 15 + 5 * byYxNum != len )
			return FALSE;

		byFunType = buf[10 + 5 * i];
		byInfoIndex = buf[11 + 5 * i];
		for ( i=0; i<byYxNum; i++)
		{
			wSt = buf[12 + 5 * i] | buf[13 + 5 * i];

			for ( j=0; j<16; j++)
			{
				//��ȡң�ŵ��
				if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+j+i*16, wPnt ) )
				{
					continue;
				}

				//0 3 �м�״̬��ȷ��״̬ 1��2��
				if( ( wSt & ( 0x03 << (2*j) ) ) >> (2*j) ==0x02)
				{
					byYxValue=1;
				}
				else if((wSt & ( 0x03 << (2*j) ) ) >> (2*j) == 0x01)
				{
					byYxValue=0;
				}
				else
				{
					continue;
				}

				//����ң��
				m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
			}
		}

	}
	return TRUE;
}		/* -----  end of method CIEC103::M_DS_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_SS_TA_3
 * Description:  ״̬�仯ʱ����ĵ���״̬��״̬�仯��Ϣ ASDU45
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_SS_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wSt;
	WORD wPnt;
	TIMEDATA tTimeData;
	TIMEDATA *pTime = &tTimeData;
	REALTIME curTime;
	WORD wMiSec;
	int i=0, j=0;

	byYxNum = buf[7] & 0x7f;
	//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+yxnum*10+sin:1+cs:1+0x16:1
	//10 = inf:1 + SCD:4 + QDS:1 + 4
	if( 14 + 10 * byYxNum != len )
		return FALSE;
	DisplayCot( buf[8] );

	//��ȡʱ��
	GetCurrentTime( &curTime );

	byFunType = buf[10];

	for ( i=0; i<byYxNum; i++)
	{
		byFunType = buf[10];
		byInfoIndex = buf[11 + 10 * i];
		wSt = buf[12 + 10 * i] | buf[13 + 10 * i];

		//ʱ�丳ֵ
		wMiSec = buf[17] |  buf[18] << 8;
		pTime->MiSec = wMiSec % 1000;
		pTime->Second = wMiSec/1000;
		pTime->Minute = buf[19] & 0x3f;
		pTime->Hour = buf[20] & 0x1f;
		pTime->Day = curTime.wDay;
		pTime->Month = curTime.wMonth;
		pTime->Year = curTime.wYear - 1900;

		for ( j=0; j<16; j++)
		{
			//��ȡң�ŵ��
			if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+j + i*16, wPnt ) )
			{
				continue;
			}

			if( wSt & (1 << j) )
			{
				byYxValue = 1;
			}
			else
			{
				byYxValue = 0;
			}

			//����ң��
			m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
			//����soe
			m_pMethod->SetYxDataWithTime( m_SerialNo, wPnt, byYxValue, pTime);
		}
	}

	return TRUE;
}		/* -----  end of method CIEC103::M_SS_TA_3  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_DS_TA_3_Frame
 * Description:  ״̬�仯ʱ�����˫��״̬��״̬�仯��Ϣ ASDU47
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_DS_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYxNum;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byYxValue;
	WORD wSt;
	WORD wPnt;
	TIMEDATA tTimeData;
	TIMEDATA *pTime = &tTimeData;
	REALTIME curTime;
	WORD wMiSec;
	int i=0, j=0;

	byYxNum = buf[7] & 0x7f;
	//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+yxnum*10+sin:1+cs:1+0x16:1
	//10 = inf:1 + SCD:4 + QDS:1 + 4
	if( 14 + 10 * byYxNum != len )
		return FALSE;
	DisplayCot( buf[8] );

	//��ȡʱ��
	GetCurrentTime( &curTime );

	byFunType = buf[10];
	for ( i=0; i<byYxNum; i++)
	{
		byInfoIndex = buf[11 + 10 * i];
		wSt = buf[12 + 10 * i] | buf[13 + 10 * i];

		//ʱ�丳ֵ
		wMiSec = buf[17] |  buf[18] << 8;
		pTime->MiSec = wMiSec % 1000;
		pTime->Second = wMiSec/1000;
		pTime->Minute = buf[19] & 0x3f;
		pTime->Hour = buf[20] & 0x1f;
		pTime->Day = curTime.wDay;
		pTime->Month = curTime.wMonth;
		pTime->Year = curTime.wYear - 1900;

		for ( j=0; j<8; j++)
		{
			//��ȡң�ŵ��
			if( !GetModulePnt( IEC103_YX_DATATYPE, byFunType, byInfoIndex+j+i+8, wPnt ) )
			{
				continue;
			}

			//0 3 �м�״̬��ȷ��״̬ 1��2��
			if( ( wSt & ( 0x03 << (2*j) ) ) >> (2*j) ==0x02)
			{
				byYxValue=1;
			}
			else if((wSt & ( 0x03 << (2*j) ) ) >> (2*j) == 0x01)
			{
				byYxValue=0;
			}
			else
			{
				continue;
			}

			//����ң��
			m_pMethod->SetYxData( m_SerialNo, wPnt,	byYxValue );
			//����soe
			m_pMethod->SetYxDataWithTime( m_SerialNo, wPnt, byYxValue, pTime);
		}
	}


	return TRUE;
}		/* -----  end of method CIEC103::M_DS_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEI_NA_3_Frame
 * Description:  ����ֵ ASDU3
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEI_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYcNum = buf[7] & 0x7f;
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wYcValue;
	WORD wPnt;
	float fYcValue;
	int i=0;

	//val0 misecL0 misecH0 min0 hour0
	//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+ycnum*2+cs:1+0x16:1
	if( ( 14 + byYcNum * 2) != len )
		return FALSE;
	DisplayCot( buf[8] );

	byFunType = buf[10];
	byInfoIndex = buf[11];

	for (i=0; i<byYcNum; i++)
	{
		//��ȡң����
		if( !GetModulePnt( IEC103_YC_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
		{
			continue;
		}

		//�����Ǵ�MMI�Ͽ��������޸ĵ�
		//--start
		wYcValue = buf[12+2*i] | ( buf[13+2*i] << 8 );
		wYcValue=(wYcValue>>3)&0x1FFF;
		if(wYcValue&0x1000)
		{
			wYcValue=((~wYcValue)+1)&0x1FFF;
			fYcValue=wYcValue;
			fYcValue=-fYcValue;
		}
		else
		{
			fYcValue=wYcValue;
		}

		fYcValue/=4096.0;
		//--end

		//����ң��
		m_pMethod->SetYcData( m_SerialNo, wPnt, fYcValue );
		sprintf( DebugBuf, "YcUpdate:dev%d pnt%d=%f line:%d", m_wDevAddr, wPnt, fYcValue, __LINE__);
		print( DebugBuf );
	}

	return TRUE;
}		/* -----  end of method CIEC103::M_MEI_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_TME_TA_3_Frame
 * Description:  �����ʱ���ʱ�걻��ֵ ASDU4 ������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_TME_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	// BYTE byYcNum = buf[7] & 0x7f;
	// BYTE byFunType;
	// BYTE byInfoIndex;
	// WORD wYcValue;
	// WORD wPnt;
	// float fYcValue;

	if ( len != 26 )
		return FALSE;
	DisplayCot( buf[8] );



	return TRUE;
}		/* -----  end of method CIEC103::M_TME_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEII_NA_3_Frame
 * Description:  ����ֵII ASDU9
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEII_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	return M_MEI_NA_3_Frame( buf, len );
}		/* -----  end of method CIEC103::M_MEII_NA_3_Frame  ----- *//*}}}*/

/*
 * -------------------------------------------------------------------------------------------------
 * class:	CIEC103
 * funct:	M_GD_NA_3_Frame
 * descr:	ͨ�÷�������
 * param:	para0:����֡ para1:֡��
 * (֡��ͨ�÷������ݼ���Ŀ�ֶκ󱻽ضϣ�����Ҳ��Ӧ��С!)
 * retur:	BOOL
 * -------------------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_GD_NA_3_Frame(BYTE *pbuf, int len)
{/*{{{*/
	//Ϊ���������⿪��
// 	float value = 0;
// 	for(int i = 0; i < 18; ++i){
// 		value = floatvalue(buf + i*10);
// 		m_pMethod->SetYcData(m_SerialNo, i, value);
// 	}

	if (pbuf == NULL)
		return FALSE;

	int recv_len = pbuf[1];
	if (len != recv_len + 6)
		return FALSE;

	BYTE byctl = pbuf[4];
	BYTE byAdd = pbuf[5];
	if (byAdd != m_wDevAddr)
		return FALSE;

	BYTE byType = pbuf[6];
	BYTE byVSQ = pbuf[7]; //�ɱ�ṹ�޶���
	BYTE byCOT = pbuf[8]; //����ԭ��
	BYTE byAddr = pbuf[9]; //������ַ
	BYTE byFunc = pbuf[10]; //��������
	BYTE byInfo = pbuf[11];  //��Ϣ���
	BYTE byRII = pbuf[12]; //������Ϣ��ʶ��
	BYTE byNOG = pbuf[13]; //ͨ�÷����ʶ��Ŀ
	BYTE byNumStruct = byNOG & 0x3f;

	printf("byNumStruct = %d \n", byNumStruct);
	BYTE offset = 10;
	BYTE byGNo = 0xFF;

	for (int i = 0; i < byNumStruct; i++)
	{
		//��ȡ����
		int iOffset = i * 10;
		BYTE byGinLo = pbuf[14 + iOffset]; //ͨ�÷����ʶ��ŵ� ���
		BYTE byGinHi = pbuf[15 + iOffset]; //ͨ�÷����ʶ��Ÿ� ��Ŀ��
		BYTE byKOD = pbuf[16 + iOffset]; //��������
		BYTE byDataType = pbuf[17 + iOffset]; //��������
		BYTE byDataSize = pbuf[18 + iOffset]; //���ݿ���
		BYTE byNum = pbuf[19 + iOffset]; //������Ŀ
		printf("g = %d item = %d\n", byGinLo , byGinHi );
		if (byGinLo == 13||byGinLo == 9 )
		{
			float fVal = 0.0;
			memcpy(&fVal, &pbuf[20 + iOffset], 4);
			m_pMethod->SetYcData(m_SerialNo, i, fVal);
			printf("yc%d = %f\n", i, fVal);
		}
	}

	return TRUE;
}/*}}}*/

float CIEC103::floatvalue(BYTE *buf)
{/*{{{*/
	BYTE buftemp[4];
	//buftemp[0] = buf[6];
	//buftemp[1] = buf[7];
	//buftemp[2] = buf[8];
	//buftemp[3] = buf[9];
	//float ptmp = *(float *)buftemp;
	//return ptmp;
	buftemp[0] = buf[9];			//���,ʹ��objdump -a����֪����С��!
	buftemp[1] = buf[8];
	buftemp[2] = buf[7];
	buftemp[3] = buf[6];
	return *(float *)buftemp;
}/*}}}*/
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEIII_NA_3_Frame
 * Description:  ����ֵIII ASDU15
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEIII_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	//����
	return TRUE;
}		/* -----  end of method CIEC103::M_MEIII_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEIII_TA_3_Frame
 * Description:
		//����ֵIII ASDU32 ֻ��ÿ��ң��ֵֻռ�����ֽ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEIII_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	//����
	BYTE byYcNum = buf[7] & 0x7f;
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wYcValue;
	WORD wPnt;
	float fYcValue;
	int i=0;

	//val0 misecL0 misecH0 min0 hour0
	//�жϱ��ĳ��� head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+ycnum*2+time:4+cs:1+0x16:1
	if( ( 18 + byYcNum * 2) != len )
		return FALSE;
	DisplayCot( buf[8] );

	DisplayCot( buf[8] );

	byFunType = buf[10];
	byInfoIndex = buf[11];

	for (i=0; i<byYcNum; i++)
	{
		//��ȡң����
		if( !GetModulePnt( IEC103_YC_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
		{
			continue;
		}

		//�����Ǵ�MMI�Ͽ��������޸ĵ�
		//--start
		wYcValue = buf[12+2*i] | ( buf[13+2*i] << 8 );
		wYcValue=(wYcValue>>3)&0x1FFF;
		if(wYcValue&0x1000)
		{
			wYcValue=((~wYcValue)+1)&0x1FFF;
			fYcValue=wYcValue;
			fYcValue=-fYcValue;
		}
		else
		{
			fYcValue=wYcValue;
		}

		fYcValue/=4096.0;
		//--end

		//����ң��
		m_pMethod->SetYcData( m_SerialNo, wPnt, fYcValue );
		sprintf( DebugBuf, "YcUpdate:dev%d pnt%d=%f line:%d", m_wDevAddr, wPnt, fYcValue, __LINE__);
		print( DebugBuf );
	}

	return TRUE;
	//MMI�ϴ�����ʽ  ���ǲ���
	// return M_MEI_NA_3_Frame( buf, len );
	// return TRUE;
}		/* -----  end of method CIEC103::M_MEIII_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEIV_TA_3_Frame
 * Description:
		//����ֵIV ASDU33 ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEIV_TA_3_Frame ( BYTE *buf , int len )
{/*{{{*/
	return M_MEIII_TA_3_Frame( buf, len );
	//MMI�ϴ�����ʽ  ���ǲ���
	// return M_MEI_NA_3_Frame( buf, len );
}		/* -----  end of method CIEC103::M_MEIV_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEV_TA_3_Frame
 * Description:
		//����ֵV ASDU34 ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEV_TA_3_Frame ( BYTE *buf, int len  )
{/*{{{*/
	return TRUE;
}		/* -----  end of method CIEC103::M_MEV_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEVI_TA_3_Frame
 * Description:  ��ʱ��ı���ֵVI ASDU35 ����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEVI_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	return TRUE;
}		/* -----  end of method CIEC103::M_MEVI_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_MEVII_NA_3_Frame
 * Description:  ��Ӧ���ٻ��ı���ֵVII ��������ֵ�ñ���ֵII ASDU50
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_MEVII_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYcNum = buf[7] & 0x7f;
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wYcValue;
	WORD wPnt;
	float fYcValue;
	int i=0;

	DisplayCot( buf[8] );
	//��������ֵ�ñ���ֵII
	if( buf[7] & 0x80 )
	{
		// inf0 val0
		//head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+ycnum*3+cs:1+0x16:1
		if( ( 13 + byYcNum * 3) != len )
			return FALSE;

		byFunType = buf[10];
		for (i=0; i<byYcNum; i++)
		{
			byInfoIndex = buf[11 + 3 * i];
			//��ȡ���
			if( !GetModulePnt( IEC103_YC_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
			{
				continue;
			}

			//--start
			wYcValue = buf[12+3*i] | ( buf[13+3*i] << 8 );
			wYcValue=(wYcValue>>3)&0x1FFF;
			if(wYcValue&0x1000)
			{
				wYcValue=((~wYcValue)+1)&0x1FFF;
				fYcValue=wYcValue;
				fYcValue=-fYcValue;
			}
			else
			{
				fYcValue=wYcValue;
			}
			fYcValue/=4096.0;
			//--end

			//����ң��
			m_pMethod->SetYcData( m_SerialNo, wPnt, fYcValue );
		sprintf( DebugBuf, "YcUpdate:dev%d pnt%d=%f line:%d", m_wDevAddr, wPnt, fYcValue, __LINE__);
		print( DebugBuf );
		}
	}
	//��Ӧ���ٻ��ı���ֵVII
	else
	{
		byFunType = buf[10];
		byInfoIndex = buf[11 ];

		for (i=0; i<byYcNum; i++)
		{
			//��ȡң����
			if( !GetModulePnt( IEC103_YC_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
			{
				continue;
			}

			//�����Ǵ�MMI�Ͽ��������޸ĵ�
			//--start
			wYcValue = buf[12+3*i] | ( buf[13+3*i] << 8 );
			wYcValue=(wYcValue>>3)&0x1FFF;
			if(wYcValue&0x1000)
			{
				wYcValue=((~wYcValue)+1)&0x1FFF;
				fYcValue=wYcValue;
				fYcValue=-fYcValue;
			}
			else
			{
				fYcValue=wYcValue;
			}

			fYcValue/=4096.0;
			//--end

			//����ң��
			m_pMethod->SetYcData( m_SerialNo, wPnt, fYcValue );
		sprintf( DebugBuf, "YcUpdate:dev%d pnt%d=%f line:%d", m_wDevAddr, wPnt, fYcValue, __LINE__);
		print( DebugBuf );
		}

		return  M_MEI_NA_3_Frame( buf, len );
	}
	return TRUE;
}		/* -----  end of method CIEC103::M_MEVII_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_IT_NA_3_Frame
 * Description:  �������������֡ ASDU36
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_IT_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYmNum = buf[7] & 0x7f;
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wPnt;
	DWORD dwYmValue;
	QWORD qYmValue;
	BYTE byYmBuf[4];
	int i=0;
	// inf0 val0
	// head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+ycnum*5+rii:1+cs:1+0x16:1
	if( ( 15 + byYmNum * 5) != len )
		return FALSE;

	DisplayCot( buf[8] );
	byFunType = buf[10];
	byInfoIndex = buf[11];

	for ( i=0; i<byYmNum; i++)
	{
		if( !GetModulePnt( IEC103_YM_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
		{
			continue;
		}

    GlobalCopyByEndian( byYmBuf, &buf[12 + 5*i ], 4);
		// memcpy( &dwYmValue, &buf[12 + 5 * i], 4 );
		// byYmBuf[3] = buf[12 + 5 * i];
		// byYmBuf[2] = buf[13 + 5 * i];
		// byYmBuf[1] = buf[14 + 5 * i];
		// byYmBuf[0] = buf[15 + 5 * i];

		memcpy( &dwYmValue, byYmBuf, 4 );
		qYmValue = (QWORD)dwYmValue;

		m_pMethod->SetYmData( m_SerialNo, wPnt, qYmValue );
	}
	return TRUE;
}		/* -----  end of method CIEC103::M_IT_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_IT_TA_3_Frame
 * Description:
		//��ʱ��ĵ������������֡  ASDU37
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_IT_TA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYmNum = buf[7] & 0x7f;
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wPnt;
	DWORD dwYmValue;
	QWORD qYmValue;
	BYTE byYmBuf[4];
	int i = 0;
	// inf0 val0
	// head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+ycnum*5+rii:1+cs:1+0x16:1
	if( ( 15 + byYmNum * 9) != len )
		return FALSE;

	DisplayCot( buf[8] );
	byFunType = buf[10];
	byInfoIndex = buf[11];

	for ( i=0; i<byYmNum; i++)
	{
		if( !GetModulePnt( IEC103_YM_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
		{
			continue;
		}

		// memcpy( &dwYmValue, &buf[12 + 9 * i], 4 );
    GlobalCopyByEndian(byYmBuf, &buf[12 + 9 * i], 4);
		// byYmBuf[3] = buf[12 + 9 * i];
		// byYmBuf[2] = buf[13 + 9 * i];
		// byYmBuf[1] = buf[14 + 9 * i];
		// byYmBuf[0] = buf[15 + 9 * i];

		memcpy( &dwYmValue, byYmBuf, 4 );
		qYmValue = (QWORD)dwYmValue;

		m_pMethod->SetYmData( m_SerialNo, wPnt, qYmValue );
	}
	return TRUE;
}		/* -----  end of method CIEC103::M_IT_TA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  M_IT_TA_3_SIPROTEC_Frame
 * Description:  �����������103ң�����ͷ�ʽ��205������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::M_IT_TA_3_SIPROTEC_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byYmNum = buf[7] & 0x7f;
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wPnt;
	DWORD dwYmValue;
	QWORD qYmValue;
	BYTE byYmBuf[4];
	int i = 0;
    bool s = false;  // ����λ
	// inf0 val0
	// head:4+ctl:1+add:1+asdu:1+vsq:1+cot:1+addr:1+fun:1+inf:1+ymnum*8+cs:1+0x16:1
	if( ( 14 + byYmNum * 8) != len )
		return FALSE;

	DisplayCot( buf[8] );
	byFunType = buf[10];
	byInfoIndex = buf[11];

	for ( i=0; i<byYmNum; i++)
	{
		if( !GetModulePnt( IEC103_YM_DATATYPE, byFunType, byInfoIndex+i, wPnt ) )
		{
			continue;
		}

		// memcpy( &dwYmValue, &buf[12 + 9 * i], 4 );
		// byYmBuf[3] = buf[12 + 8 * i]  ;
		// byYmBuf[2] = buf[13 + 8 * i];
		// byYmBuf[1] = buf[14 + 8 * i];
		// byYmBuf[0] = buf[15 + 8 * i] & 0x0f;
        if ( 0 != (buf[15+8*i] & 0x10) )
        {
            // ����λ
            s = TRUE;
        }
        GlobalCopyByEndian(byYmBuf,&buf[12 + 8 * i], 4);

		memcpy( &dwYmValue, byYmBuf, 4 );
        // ����λ1��ʱ��ȡ����
        if ( s )
        {
            dwYmValue = ~dwYmValue + 1;
        }
        qYmValue = (QWORD)dwYmValue;

        m_pMethod->SetYmData( m_SerialNo, wPnt, qYmValue );
		sprintf( DebugBuf, "YmUpdate:dev%d pnt%d=%lu ", m_wDevAddr, wPnt, dwYmValue);
		print( DebugBuf );
	}

	return TRUE;
}		/* -----  end of method CIEC103::M_IT_TA_3_SIPROTEC_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  C_DC_NA_3_Frame
 * Description:  ң�ط���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::C_DC_NA_3_Frame ( BYTE *buf, int len )
{/*{{{*/
	BYTE byFunType;
	BYTE byInfoIndex;
	WORD wPnt;
	BYTE byYkValue;
	BYTE byDataType;

	DisplayCot( buf[8] );
	//Զ�̲��� 12 һ������ 20 cot
	if( m_wDevAddr != buf[5] &&  0x0c != buf[8] && 0x14 != buf[8]  )
		return FALSE;

	byYkValue = buf[12] & 0x03;
	if( byYkValue != 0x01 && byYkValue != 0x02 )
		return FALSE;

	byFunType = buf[10];
	byInfoIndex = buf[11];
	if( !GetModulePnt( IEC103_YK_DATATYPE, byFunType, byInfoIndex, wPnt ) )
		return FALSE;

	//0x80ң��ѡ�� 0xc0ң��ȡ�� 0x00ң��ִ��
	byDataType = buf[12] & 0xc0;
	if( byDataType == 0x80 )
		m_pMethod->SetYkSelRtn( this, m_byRemoteBusNo, m_byRemoteAddr, wPnt, byYkValue - 1 );
	else if ( byDataType == 0 )
		m_pMethod->SetYkExeRtn( this, m_byRemoteBusNo, m_byRemoteAddr, wPnt, byYkValue - 1 );
	else
		m_pMethod->SetYkCancelRtn(this, m_byRemoteBusNo, m_byRemoteAddr, wPnt, byYkValue - 1);
	return TRUE;
}		/* -----  end of method CIEC103::C_DC_NA_3_Frame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ProcessHead10Buf
 * Description:  ������ͷʱ0x10�ı���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ProcessHead10Buf ( BYTE *buf, int len )
{/*{{{*/
	// //�жϵ�ַ�Ƿ���ȷ
	// if ( buf[2] != m_wDevAddr )
		// return FALSE;

	//�����������·����״̬
	// if( m_SendStatus == C_RLK_NA_3 )
		// m_SendStatus = C_RCU_NA_3;
	//�ж�ACD λ
	// else if( buf[1] & 0x20 )
	//�ж�ACD λ
	if( m_bIsTotalCall )
	{
		m_SendStatus = C_IGI_NA_3;
		m_bIsTotalCall = FALSE;
	}
	else if( m_bIsYmCall )
	{
		m_SendStatus = C_PL1_NA_3;
		m_bIsYmCall = FALSE;
	}
	else if (m_bIsGDCall)
	{
		m_SendStatus = C_PL1_NA_3;
		m_bIsGDCall = FALSE;
	}
	else if( buf[1] & 0x20 )
	{
		m_SendStatus = C_PL1_NA_3;
	}
	else
	{
		m_SendStatus = C_PL2_NA_3;
	}

	switch ( buf[1] & 0x0f )
	{
		case 0x00: //ȷ��֡ ȷ��
			print( (char *)"ȷ��֡" );
			break;

		case 0x08: //��������Ӧ����֡
			print( (char *)"��������֡" );
			break;

		case 0x09:	//�����ٻ�����
			print( (char *)"�����ٻ�����" );
			break;

		case 0x0b://����·״̬���������ش�����֡
			print( (char *)"��·״̬���������ش�����֡" );
			break;


		// case 0x01://ȷ��֡ ��·æ δ�յ�����
			// break;

		// case 0x02:
		// case 0x03:
		// case 0x04:
		// case 0x05: //����
			// break;

		// case 0x06:
		// case 0x07:
		// case 0x0d://�����̺��û�Э�̶���
			// break;

		// case 0x0d://��·����δ����
			// break;

		// case 0x0f://��·����δ���
			// break;

		default://Ĭ�ϴ���
			print( (char *)"default" );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */


	return TRUE;
}		/* -----  end of method CIEC103::ProcessHead10Buf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ProcessHead68Buf
 * Description:  ������ͷʱ68�ı���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ProcessHead68Buf ( BYTE *buf, int len )
{/*{{{*/
	BOOL bRtn = TRUE;
	// //�жϵ�ַ�Ƿ���ȷ
	// if ( buf[5] != m_wDevAddr )
		// return FALSE;

	//�ж�ACD λ
	if( m_bIsTotalCall )
	{
		m_SendStatus = C_IGI_NA_3;
		m_bIsTotalCall = FALSE;
	}
	else if( buf[4] & 0x20 )
		m_SendStatus = C_PL1_NA_3;
	else
		m_SendStatus = C_PL2_NA_3;

	switch ( buf[6] )						//���ͱ�ʶ!
	{
		case 0x05://��ʶ���� ASDU5
			print( (char *)"��ʶ���� ASDU5" );
			bRtn = M_IRCFS_TA_3_Frame( buf, len );
			break;

		case 0x06://ʱ��ͬ��
			print( (char *)"ʱ��ͬ�� ASDU6 �л��ܲ�ѯ" );
			bRtn =  C_SYN_TA_3_Frame( buf, len );
			m_SendStatus = C_IGI_NA_3; //�ܲ�ѯ
			m_bIsTotalCall = TRUE;
			break;

		case 0x08://�ܲ�ѯ����
			print( (char *)"�ܲ�ѯ���� ASDU8 �ٻ�ң��" );
			m_bIsYmCall = TRUE;
			bRtn = M_TGI_NA_3_Frame( buf, len );
			// if( m_pMethod->m_pRdbObj->m_p )
			m_SendStatus = C_CI_NA_3;//�ٻ�ң��
			break;
		
		case 0x58://ң��ȷ�� ASDU88
			print((char *)"ң��ȷ�� ASUD88 �ٻ�ͨ�÷�������");
			m_bIsGDCall = TRUE;
			m_SendStatus = C_GD_NA_3;//�ٻ�ͨ�÷�������
			printf("process GDdata");
			break;

		case 0x0A:
			printf("ASDU10\n");
			bRtn = M_GD_NA_3_Frame(buf , len );
			break;

		case 0x01://��ʱ��ı���
			print( (char *)"��ʱ��ı��� ASDU1" );
			bRtn = M_TTM_TA_3_Frame( buf, len );
			break;

		case 0x02://�����ʱ���ʱ�걨��
			print( (char *)"�����ʱ���ʱ�걨�� ASDU2" );
			bRtn = M_TMR_TA_3_Frame( buf, len );
			break;

		case 0x28://���ٻ�ʱ����ĵ�����Ϣ״̬֡ ASDU40
			print( (char *)"���ٻ�ʱ����ĵ�����Ϣ״̬֡ ASDU40" );
			bRtn = M_SP_NA_3_Frame(buf, len);
			break;

		case 0x2a:	//���ٻ�ʱ�����˫����Ϣ״̬֡ ASDU42
			print( (char *)"���ٻ�ʱ�����˫����Ϣ״̬֡ ASDU42" );
			bRtn = M_DP_NA_3_Frame(buf, len);
			break;

		case 0x29:	//��ʱ��ĵ�����Ϣ״̬�仯֡ ASDU41
			print( (char *)"��ʱ��ĵ�����Ϣ״̬�仯֡ ASDU41" );
			bRtn = M_SP_TA_3_Frame(buf, len);
			break;

		case 0x2b:	//��ʱ���˫����Ϣ״̬�仯֡ ASDU43
			print( (char *)"��ʱ���˫����Ϣ״̬�仯֡ ASDU43" );
			bRtn = M_DP_TA_3_Frame(buf, len);
			break;

		case 0x2c:	//���ٻ�ʱ����ĵ���״̬��״̬�仯��Ϣ֡ ASDU44
			print( (char *)"���ٻ�ʱ����ĵ���״̬��״̬�仯��Ϣ֡ ASDU44" );
			bRtn = M_SS_NA_3_Frame(buf, len);
			break;

		case 0x2e:	//���ٻ�ʱ�����˫��״̬��״̬�仯��Ϣ֡ ASDU46
			print( (char *)"���ٻ�ʱ�����˫��״̬��״̬�仯��Ϣ֡ ASDU46" );
			bRtn = M_DS_NA_3_Frame(buf, len);
			break;

		case 0x2d:	//״̬�仯ʱ����ĵ���״̬��״̬�仯��Ϣ ASDU45
			print( (char *)"״̬�仯ʱ����ĵ���״̬��״̬�仯��Ϣ ASDU45" );
			bRtn = M_SS_TA_3_Frame(buf, len);
			break;

		case 0x2f:	//״̬�仯ʱ�����˫��״̬��״̬�仯��Ϣ ASDU47
			print( (char *)"״̬�仯ʱ�����˫��״̬��״̬�仯��Ϣ ASDU47" );
			bRtn = M_DS_TA_3_Frame(buf, len);
			break;

		case 0x03://����ֵI
			print( (char *)"����ֵI ASDU3" );
			bRtn = M_MEI_NA_3_Frame( buf, len  );
			break;

		case 0x04:	//�����ʱ���ʱ�걻��ֵ ASDU4
			print( (char *)"�����ʱ���ʱ�걻��ֵ ASDU4" );
			M_TME_TA_3_Frame(buf, len);
			break;

		case 0x09://����ֵII
			print( (char *)"����ֵII ASUD9" );
			bRtn = M_MEII_NA_3_Frame( buf, len );
			break;

		case 0x0f:	//����ֵIII ASDU15
			print( (char *)"����ֵIII ASDU15" );
			bRtn = M_MEIII_NA_3_Frame( buf, len );
			break;

		case 0x20:	//����ֵIII ASDU32
			print( (char *)"����ֵIII ASDU32" );
			bRtn = M_MEIII_TA_3_Frame( buf, len );
			break;

		case 0x21:	//����ֵIV ASDU33
			print( (char *)"����ֵIV ASDU33 " );
			bRtn = M_MEIV_TA_3_Frame( buf, len );
			break;

		case 0x22:	//����ֵ����V ASDU34
			print( (char *)"����ֵ����V ASDU34 " );
			bRtn = M_MEV_TA_3_Frame( buf, len );
			break;

		case 0x23:	//��ʱ��ı���ֵ����VI ASDU35
			print( (char *)"��ʱ��ı���ֵ����VI ASDU35" );
			bRtn = M_MEVI_TA_3_Frame( buf, len );
			break;

		case 0x32:	//��Ӧ���ٻ��ı���ֵVII ��������ֵ�ñ���ֵII ASDU50
			print( (char *)"��Ӧ���ٻ��ı���ֵVII ��������ֵ�ñ���ֵII ASDU50" );
			bRtn = M_MEVII_NA_3_Frame( buf, len );
			break;

		case 0x24://�������������֡ ASDU36
			print( (char *)"�������������֡ ASUD36" );
			M_IT_NA_3_Frame( buf, len );
			break;

		case 0x25://��ʱ��ĵ������������֡ ASDU37
			print( (char *)"��ʱ��ĵ������������֡ ASUD37" );
			M_IT_TA_3_Frame( buf, len );
			break;

		case 0xcd:// ������ң��
			print( (char *)"��ʱ��ĵ������������֡ ASUD205(������)" );
			M_IT_TA_3_SIPROTEC_Frame( buf, len );
			break;

		case 0x14://һ������
			//����MMI   ��׼Э����
			// C_DC_NA_3_Frame( buf, len );
			// break;

		case 0x40://��·��	ASDU64
			// break;

		case 0x41://���� ASDU65
			print( (char *)"һ������ ��·�� ���� ASUD20 ASDU64 ASDU41" );
			if ( !C_DC_NA_3_Frame( buf, len  ) )
			{
				print( (char *)"ң�ؽ�������" );
			}
			m_byYkErrorCount = 0;
			break;

		// case 0x42://�������� ASDU67
			// break;

		default:
			break;
	}				/* -----  end switch  ----- */

	return bRtn;
}		/* -----  end of method CIEC103::ProcessHead68Buf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ResetFrameCountBit
 * Description:  ��λ֡����λ
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ResetFrameCountBit ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = 0x47;
	buf[2] = m_wDevAddr;
	buf[3] = GetCs(&buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ResetCommUnit
 * Description:  ��λͨ�ŵ�Ԫ
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ResetCommUnit ( BYTE *buf, int &len  )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = 0x40;
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103::ResetCommUnit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  CallLevel1Data
 * Description:  �ٻ�һ������
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::CallLevel1Data ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = ChangeFcb(0x5A, m_bFcb);
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  CallLevel1Data
 * Description:  �ٻ���������
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::CallLevel2Data ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = ChangeFcb(0x5B, m_bFcb);
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  RequestLinkState
 * Description:  ������·״̬
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::RequestLinkState ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = 0x49;
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  TimeSyn
 * Description:  ��ʱ
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::TimeSyn( BYTE *buf, int &len )
{/*{{{*/
	time_t lSecond;
	struct tm currTime;
	struct timeval tv;
	struct timezone tz;

	gettimeofday( &tv, &tz );
    lSecond = (time_t)(tv.tv_sec);
    localtime_r( &lSecond, &currTime );

	buf[0] = 0x68;
	buf[1] = 0x0F;
	buf[2] = 0x0F;
	buf[3] = 0x68;
	buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;
	buf[6] = 0x06;
	buf[7] = 0x81;
	buf[8] = 0x08;
	buf[9] = m_wDevAddr;
	buf[10] = 0xff;
	buf[11] = 0x00;

	BYTE byMin = currTime.tm_min;
	BYTE bywDay = currTime.tm_wday ;
	BYTE bymDay = currTime.tm_mday;
	buf[12] = ( tv.tv_usec/1000 ) & 0xff;
	buf[13] = ( ( tv.tv_usec/1000 ) >> 8) & 0xff;
	buf[14] = byMin & 0x3f;
	buf[15] = ( (BYTE)currTime.tm_hour )  & 0x1f;
	buf[16] =( ( ( bywDay << 5 )  &0xE0 ) | ( bymDay & 0x1f ) )  ;
	buf[17] = (currTime.tm_mon + 1)& 0x0f;
	buf[18] = currTime.tm_year % 100;

	buf[19] = GetCs(buf+4, 15);
	buf[20] = 0x16;

	len = 21;
	return TRUE;
}		/* -----  end of method CIEC103::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  TotalCallData
 * Description:  ���ٻ�
 *       Input:  ���ͻ����� ���ͳ���
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::TotalCallData( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x68;
	buf[1] = 0x09;
	buf[2] = 0x09;
	buf[3] = 0x68;
	buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;
	buf[6] = 0x07;
	buf[7] = 0x81;
	buf[8] = 0x09;
	buf[9] = m_wDevAddr;
	buf[10] = 0xff;
	buf[11] = 0x00;
	buf[12] = 0x00;

	buf[13] = GetCs(buf+4, 9);
	buf[14] = 0x16;

	len = 15;
	return TRUE;
}		/* -----  end of method CIEC103::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  CallYmData
 * Description:  �������������֡�ٻ����� ASDU88
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::CallYmData ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x68;
	buf[1] = 0x0A;
	buf[2] = 0x0A;
	buf[3] = 0x68;
	buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;
	buf[6] = 0x58;
	buf[7] = 0x81;
	buf[8] = 0x02;
	buf[9] = m_wDevAddr;
	buf[10] = 0x01;
	buf[11] = 0x00;
	buf[12] = 0x05;
	buf[13] = 0x00;

	buf[14] = GetCs(buf+4, 10);
	buf[15] = 0x16;

	len = 16;
	return TRUE;
}		/* -----  end of method CIEC103::CallYmData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  GetSendbuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::GetSendBuf ( BYTE *buf, int &len )
{/*{{{*/
	BOOL bRtn = TRUE;
	switch ( m_SendStatus )
	{
		case C_RFB_NA_3:	//��λ֡������λ
			ResetFrameCountBit( buf, len );
			break;

		case C_RCU_NA_3:	//��λͨ�ŵ�Ԫ
			print( (char *)"��λͨ�ŵ�Ԫ" );
			ResetCommUnit( buf, len );
			break;

		case C_PL1_NA_3:	//�ٻ�һ������
			print( (char *)"�ٻ�һ������" );
			CallLevel1Data( buf, len );
			break;

		case C_PL2_NA_3:	//�ٻ������û�����
			print( (char *)"�ٻ������û�����" );
			CallLevel2Data( buf, len );
			break;

		case C_RLK_NA_3:	//������·״̬
			print( (char *)"������·״̬" );
			RequestLinkState( buf, len );
			break;

		case C_SYN_Ta_3:	//ʱ��ͬ�� ASDU6				//ASDU������������ͱ�ʶ!
			print( (char *)"ʱ��ͬ��" );
			TimeSyn( buf, len );
			m_SendStatus = C_IGI_NA_3; //�ܲ�ѯ
			m_bIsTotalCall = TRUE;
			m_bIsNeedResend = FALSE;
			break;

		case C_IGI_NA_3:	//�ܲ�ѯ
			print( (char *)"�ܲ�ѯ" );
			TotalCallData( buf, len );
			m_bIsTotalCall = FALSE;
			break;

		case C_CI_NA_3:	    //�ٻ�ң��
			print( (char *)"�ٻ�ң��" );
			CallYmData( buf, len );
			break;

		case C_GD_NA_3:		//ͨ�÷�������--------------------
			printf("call GDdata\n");
			CallGDData(buf, len);
			break;

		case C_GRC_NA_3:	//һ������
			break;

		case C_GC_NA_3:		//ͨ������
			break;

		case C_ODT_NA_3:	//�Ŷ���������
			break;

		case C_ADT_NA_3:	//�Ŷ������Ͽ�
			break;

		default:
			sprintf (DebugBuf,  "IEC103:GetProtocolBuf can't find m_SendStatus = %d\n", m_SendStatus );
			print( DebugBuf );
			break;
	}				/* -----  end switch  ----- */

	return bRtn;
}		/* -----  end of method CIEC103::GetSendbuf  ----- *//*}}}*/


BOOL CIEC103::CallGDData(BYTE *buf, int &len)
{
	printf("CallGDData msg\n");
	buf[0] = 0x68;
	buf[1] = 0x0D;
	buf[2] = 0x0D;
	buf[3] = 0x68;
	buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;

	buf[6] = 0x15;
	buf[7] = 0x81;
	buf[8] = 0x2A;
	buf[9] = 0x01;
	buf[10] = 0xFE;
	buf[11] = 0xF1;
	buf[12] = 0x10;
	buf[13] = 0x01;
	buf[14] = 0x09; 
	buf[15] = 0x01;
	buf[16] = 0x01;

	buf[17] = GetCs(buf + 4, 13);
	buf[18] = 0x16;

	len = 19;
	return TRUE;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  YkSel
 * Description:  ң��ѡ��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::YkSel ( YK_DATA *pYkData, BYTE *buf, int &len )
{/*{{{*/
	CfgInfo tCfgInfo;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byDCC = 0;

	if ( !GetModuleInfo( IEC103_YK_DATATYPE, pYkData->wPnt, tCfgInfo, byFunType, byInfoIndex) )
		return FALSE;

	// printf ( "FunType=%d %d\n", byFunType, byInfoIndex );

	if( pYkData->byVal == 0 )
	{
		byDCC = 0x81;
	}
	else if( pYkData->byVal == 1 )
	{
		byDCC = 0x82;
	}

	buf[0] =0x68;
	buf[1] =0x0A;
	buf[2] =0x0A;
	buf[3] =0x68;
	buf[4] =ChangeFcb(0x53,m_bFcb);
	buf[5] =m_wDevAddr;


	switch ( tCfgInfo.DataFormat )
	{
		case 0:	// ASDU64 ��·��
			buf[6] =64;
			buf[8] =12;
			break;

		case 1: //ASDU20 һ������
			buf[6] =20;
			buf[8] =20;
			break;

		case 2: //ASDU65 ����
			buf[6] =65;
			buf[8] =12;
			break;

		// case 3://ASDU67	��������
			// buf[6] =67;
			// buf[8] =12;
			// break;

		default:
			break;
	}				/* -----  end switch  ----- */

	buf[7] =0x81;
	buf[9] =m_wDevAddr;
	buf[10]=byFunType;
	buf[11]=byInfoIndex;
	buf[12]=byDCC;
	buf[13]=0x00;

	buf[14]=GetCs((buf+4),10);
	buf[15]=0x16;

	len = 16;
	return TRUE;
}		/* -----  end of method CIEC103::YkSel  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  YkExct
 * Description:  ң��ִ��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::YkExct ( YK_DATA *pYkData, BYTE *buf, int &len )
{/*{{{*/
	CfgInfo tCfgInfo;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byDCC = 0;

	if ( !GetModuleInfo( IEC103_YK_DATATYPE, pYkData->wPnt, tCfgInfo, byFunType, byInfoIndex) )
		return FALSE;

	if( pYkData->byVal == 0 )
	{
		byDCC = 0x01;
	}
	else if( pYkData->byVal == 1 )
	{
		byDCC = 0x02;
	}

	buf[0] =0x68;
	buf[1] =0x0A;
	buf[2] =0x0A;
	buf[3] =0x68;
	buf[4] =ChangeFcb(0x53,m_bFcb);
	buf[5] =m_wDevAddr;


	switch ( tCfgInfo.DataFormat )
	{
		case 0:	// ASDU64 ��·��
			buf[6] =64;
			buf[8] =12;
			break;

		case 1: //ASDU20 һ������
			buf[6] =20;
			buf[8] =20;
			break;

		case 2: //ASDU65 ����
			buf[6] =65;
			buf[8] =12;
			break;

		// case 3://ASDU67	��������
			// buf[6] =67;
			// buf[8] =12;
			// break;

		default:
			break;
	}				/* -----  end switch  ----- */

	buf[7] =0x81;
	buf[9] =m_wDevAddr;
	buf[10]=byFunType;
	buf[11]=byInfoIndex;
	buf[12]=byDCC;
	buf[13]=0x00;

	buf[14]=GetCs((buf+4),10);
	buf[15]=0x16;

	len = 16;
	return TRUE;
}		/* -----  end of method CIEC103::YkExct  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  YkCancel
 * Description:  ң��ȡ��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::YkCancel ( YK_DATA *pYkData, BYTE *buf, int &len )
{/*{{{*/
	CfgInfo tCfgInfo;
	BYTE byFunType;
	BYTE byInfoIndex;
	BYTE byDCC = 0;

	if ( !GetModuleInfo( IEC103_YK_DATATYPE, pYkData->wPnt, tCfgInfo, byFunType, byInfoIndex) )
		return FALSE;

	if( pYkData->byVal == 0 )
	{
		byDCC = 0xc1;
	}
	else if( pYkData->byVal == 1 )
	{
		byDCC = 0xc2;
	}

	buf[0] =0x68;
	buf[1] =0x0A;
	buf[2] =0x0A;
	buf[3] =0x68;
	buf[4] =ChangeFcb(0x53,m_bFcb);
	buf[5] =m_wDevAddr;


	switch ( tCfgInfo.DataFormat )
	{
		case 0:	// ASDU64 ��·��
			buf[6] =64;
			buf[8] =12;
			break;

		case 1: //ASDU20
			buf[6] =20;
			buf[8] =20;
			break;

		case 2: //ASDU65 ����
			buf[6] =65;
			buf[8] =12;
			break;

		// case 3://ASDU67	����
			// buf[6] =67;
			// buf[8] =12;
			// break;

		default:
			break;
	}				/* -----  end switch  ----- */

	buf[7] =0x81;
	buf[9] =m_wDevAddr;
	buf[10]=byFunType;
	buf[11]=byInfoIndex;
	buf[12]=byDCC;
	buf[13]=0x00;

	buf[14]=GetCs((buf+4),10);
	buf[15]=0x16;

	len = 16;
	return TRUE;
}		/* -----  end of method CIEC103::YkCancel  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ProcessBusMsg
 * Description:  ����������Ϣbuf
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ProcessBusMsg ( PBUSMSG pBusMsg, BYTE *buf, int &len )
{/*{{{*/
	BOOL bRtn = TRUE;
	switch ( pBusMsg->byMsgType )
	{
		case YK_PROTO:	//ң����Ϣ
			{
				m_byRemoteBusNo = pBusMsg->SrcInfo.byBusNo;
				m_byRemoteAddr = pBusMsg->SrcInfo.wDevNo;
				YK_DATA *pYkData = (YK_DATA *)pBusMsg->pData;
				switch ( pBusMsg->dwDataType )
				{
					case YK_SEL:
						print( (char *)"ң��ѡ��" );
						bRtn = YkSel( pYkData, buf, len );
						m_byYkErrorCount = 1;
						break;

					case YK_EXCT:
						print( (char *)"ң��ִ��" );
						bRtn = YkExct( pYkData, buf, len );
						m_byYkErrorCount = 1;
						break;

					case YK_CANCEL:
						print( (char *)"ң��ȡ��" );
						bRtn = YkCancel( pYkData, buf, len );
						m_byYkErrorCount = 0;
						break;

					default:
						break;
				}				/* -----  end switch  ----- */

				m_byYkSendLen = len;
				memcpy(m_byYkSendBuf, buf, m_byYkSendLen );
				m_dwYkTimeOut = 0;

			}
			break;

		default:
			sprintf( DebugBuf, "IEC103:ProcessBusMsg can't find msgtype = %d\n", pBusMsg->byMsgType );
			print ( DebugBuf );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */
	return bRtn;
}		/* -----  end of method CIEC103::ProcessBusMsg  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  CIEC103
 * Description:  ��ȡ������Ϣ
 *       Input:
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ReadCfgInfo (  )
{/*{{{*/
	FILE *fp = NULL;
	char szLineBuf[256];
	CfgInfo tCfgInfo;
	char *p = NULL;
	BYTE i = 0;
	int iNum;
	char szFileName[256] = "";

	sprintf( szFileName, "%s%s", IEC103PREFIXFILENAME, m_sTemplatePath);
	fp = fopen( szFileName, "r" );
	if( fp == NULL )
	{/*{{{*/
		sprintf(DebugBuf,  "CIEC103:ReadCfgInfo fopen %s err!!!\n", szFileName );
		print( DebugBuf );
		printf ( "%s", DebugBuf );
		return FALSE;
	}/*}}}*/
	else
	{/*{{{*/
		sprintf(DebugBuf,  "CIEC103:ReadCfgInfo fopen %s Ok!!!\n", szFileName );
		print( DebugBuf );
		printf ( "%s", DebugBuf );
	}/*}}}*/

	while( fgets( szLineBuf, sizeof(szLineBuf), fp ) != NULL )
	{/*{{{*/
		i = 0;
		rtrim( szLineBuf );
		if( szLineBuf[0] == '#' || szLineBuf[0] == ';'
			|| (szLineBuf[0]-'0') < 0 || (szLineBuf[0] - '0') > 9)
		{
			continue;
		}

		p = strtok( szLineBuf, "," );
		if( p == NULL )
		{
			continue;
		}
		else
		{
			tCfgInfo.FunType = atoi( p );
		}

		while( ( p = strtok( NULL, "," ) ) )
		{/*{{{*/
			++i;
			iNum = atoi(p);
			if( iNum > 255 || iNum < 0 )
			{
				sprintf( DebugBuf, "CIEC103:ReadCfgInfo file: %s line:%d byte:%d is err!!! \n", m_sTemplatePath,(int)m_IEC103_CfgInfo.size(), i);
				print ( DebugBuf );
				continue;
			}
			switch ( i  )
			{/*{{{*/
				case 1:
					tCfgInfo.InfoIndex = atoi( p );
					break;

				case 2:
					tCfgInfo.AddInfo = atoi( p );
					break;

				case 3:
					tCfgInfo.DataType = atoi( p );
					break;

				case 4:
					tCfgInfo.StartIndex = atoi( p );
					break;

				case 5:
					tCfgInfo.DataNum = atoi( p );
					break;

				case 6:
					tCfgInfo.DataFormat = atoi( p );
					break;

				// case 7:
					// break;

				default:
					break;
			}				/* -----  end switch  ----- *//*}}}*/
		}/*}}}*/

		m_IEC103_CfgInfo.push_back( tCfgInfo );
	}/*}}}*/

	fclose( fp );
	return TRUE;
}		/* -----  end of method CIEC103::ReadCfgInfo  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  InitProtocolStatus
 * Description:  ��ʼ��Э�����״̬
 *       Input:
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::InitProtocolStatus (  )
{/*{{{*/
	m_bLinkStatus = FALSE;		//����״̬Ϊ��
	m_SendStatus = C_RCU_NA_3;	//��Ϊ��λͨ�ŵ�Ԫ
	m_dwLinkTimeOut = 0;		//���ӳ�ʱΪ0
	m_dwYkTimeOut = 0;			//ң�س�ʱΪ0
	m_dwTotalCallTime = 0;		//�ܲ�ѯʱ��Ϊ0
	m_byYkErrorCount = 0;		//ң�ش������0
	m_byRecvErrorCount = 0;     //���մ������0
	m_bFcb = 0;					//FCB��0
	m_bIsReSend = FALSE;		//�ط���ʶλ0
	m_byResendCount = 0;		//�ط���������
	m_bIsSending = FALSE;		//���ͺ���1 ���պ�ֵ0
	m_bIsNeedResend = TRUE;		//�Ƿ���Ҫ�ط�
	m_bIsYking = FALSE;			//�Ƿ�ң��״̬
	m_bIsTotalCall = FALSE;     //�Ƿ�����
	m_bIsYmCall = FALSE;		//�Ƿ��ٻ�YM
	m_bIsGDCall = FALSE;     //�Ƿ��ٻ�ͨ�÷�������

	m_wReSendLen = 0;
	m_byYkSendLen = 0;
	m_byRemoteBusNo = 0;
	m_byRemoteAddr = 0;
	memset( m_byReSendBuf, 0, IEC103_MAX_BUF_LEN );
	memset( m_byYkSendBuf, 0, sizeof( m_byYkSendBuf ) );
	memset( DebugBuf, 0, sizeof( DebugBuf ) );




	return TRUE;
}		/* -----  end of method CIEC103::InitProtocolStatus  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  GetDevCommState
 * Description:  ����װ������״̬
 *       Input:
 *		Return:  BOOL 0 ���� 1 ������
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::GetDevCommState (  )
{/*{{{*/
	if( m_bLinkStatus )
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}		/* -----  end of method CIEC103::GetDevCommState  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  TimerProc
 * Description:  ʱ�䴦������ ��Ҫ����һЩ��ʱ ���ٻ�����ʱ���йص�
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC103::TimerProc (  )
{/*{{{*/
	// if ( !m_bLinkStatus )
		// return;

	int Interval = 250;
	//���ٻ�ʱ��
	m_dwTotalCallTime += Interval;
	if( m_dwTotalCallTime >= IEC103_TOTAL_CALL )
	{
		m_SendStatus = C_IGI_NA_3;
		m_bIsTotalCall = TRUE;
		m_dwTotalCallTime = 0;
	}

	//ͨѶ��ʱʱ��
	m_dwLinkTimeOut += Interval;
	if(m_dwLinkTimeOut >= IEC103_LINK_TIMEOUT)
	{
		if( m_bLinkStatus == TRUE )
		{
			InitProtocolStatus();
		}
	}

	//ң�س�ʱ �ٴ���
	if( m_byYkErrorCount > 0 )
	{
		m_dwYkTimeOut += Interval;
		if( m_dwYkTimeOut >= IEC103_YK_TIMEOUT )
		{
			m_dwYkTimeOut = 0;
			m_byYkErrorCount ++;
			m_bIsYking = TRUE;
			if( m_byYkErrorCount > 3 )
			{
				m_byYkErrorCount = 0;
			}
		}

	}

	//���մ������
	if( m_byRecvErrorCount > IEC103_MAX_ERROR_COUNT  )
	{
		m_byResendCount = 0;
		InitProtocolStatus();
	}

	//�ط�����
	if( m_byResendCount >= IEC103_MAX_RESEND_COUNT )
	{
		m_byResendCount = 0;
		InitProtocolStatus(  );
	}
}		/* -----  end of method CIEC103::TimerProc  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  ProcessProtocolBuf
 * Description:	 �����յ������ݻ���
 *       Input:  ���յ������ݻ��� ���泤��
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::ProcessProtocolBuf ( BYTE *buf, int len )
{/*{{{*/
	int pos=0;
	BOOL bRtn = TRUE;
	printf("----------recv---------\n");
	for (int i = 0; i < len; i++)
	{
		printf("%02x ", buf[i]);
	}
	printf("\n");
	if( !WhetherBufValue( buf, len, pos ) )
	{
		print ( (char *)"CIEC103:ProcessProtocolBuf buf Recv err!!!\n" );
		m_byRecvErrorCount ++;
		m_bIsReSend = TRUE;
		return FALSE;
	}

	if( buf[pos] == 0x10 )
	{
		bRtn = ProcessHead10Buf( &buf[pos], len );
	}
	else if( buf[pos] == 0x68)
	{
		bRtn = ProcessHead68Buf( &buf[pos], len );
	}
	else
	{
		sprintf (DebugBuf,  "CIEC103:ProcessProtocolBuf buf[0]=%x err!!!\n", buf[pos] );
		print( DebugBuf );
	}

	//�˴�ֻ�ж��Ƿ��� ������Ϊ��վ������ȷ���Ķ�û�д�������ͨѶ�쳣
	if( !bRtn )
	{
		print( (char *)"�������ķ��������δ����" );
		// m_byRecvErrorCount ++;
		// m_bIsReSend = TRUE;
	}
	// else
	// {
		m_byRecvErrorCount = 0;
		m_bLinkStatus = TRUE;
		m_dwLinkTimeOut = 0;
		m_bIsReSend = FALSE;
		m_byResendCount = 0;
		m_bIsSending = FALSE;
	// }

	return bRtn;
}		/* -----  end of method CIEC103::ProcessProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�����ݻ���
 *       Input:  ������ ���������ݳ��� ������Ϣ
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{/*{{{*/
	BOOL bRtn = TRUE;
	if ( m_bIsYking )
	{
		print( (char *)"ң���ط�" );
		memcpy( buf, m_byYkSendBuf, m_byYkSendLen );
		len = m_byYkSendLen;
		buf[4] =ChangeFcb(0x53,m_bFcb);
		buf[14]=GetCs((buf+4),10);
		m_bIsYking = FALSE;
	}
	else if ( m_bIsReSend || m_bIsSending && m_SendStatus != C_RCU_NA_3)
	{
		len = m_wReSendLen;
		memcpy( buf, m_byReSendBuf, len );
		m_byResendCount ++;
		sprintf( DebugBuf, "�ط� %d ��", m_byResendCount  );
		print( DebugBuf );
	}
	else if( pBusMsg != NULL && m_bLinkStatus)
	{
		print( (char *)"������Ϣ" );
		if( !ProcessBusMsg( pBusMsg, buf, len ) )
		{
			print( (char *)"������Ϣ����ʧ��" );
			return FALSE;
		}
	}else if(time(NULL) % 800 == 0){		//+ by cyz!
		//֮ǰ�����Ϊ�յ�ASDU5֮���ʱ�������е��豸Ҫ���ʱ�����ֲ��ظ�ASDU5,����ϵͳ��������������ʱ�䲻��ȷ����սҲ�ᱻ��.���ڴ����Ӷ�ʱ���ã�800s��ʱһ��!
		TimeSyn(buf, len);
	}
	else
	{
		bRtn = GetSendBuf( buf, len );
		if( bRtn )
		{
			m_wReSendLen = len;
			memcpy( m_byReSendBuf, buf, m_wReSendLen );
			m_bIsSending = TRUE;
			if( !m_bIsNeedResend )
			{
				m_bIsSending = FALSE;
				m_bIsNeedResend = TRUE;
			}
		}
	}
	printf("----------send---------\n");
	for (int i = 0; i < len; i++)
	{
		printf("%02x ",buf[i]);
	}
	printf("\n");
	return bRtn;
}		/* -----  end of method CIEC103::GetProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103
 *      Method:  Init
 * Description:	 ��ʼ��Э������
 *       Input:  ���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103::Init ( BYTE byLineNo )
{/*{{{*/
	if( !ReadCfgInfo() )
	{
		print ( (char *)"CIEC103:ReadCfgInfo Err!!!\n" );
		return FALSE;
	}

	if( !InitProtocolStatus() )
	{
		print ( (char *)"CIEC103:InitProtocolStatus Err\n" );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC103::Init  ----- *//*}}}*/
