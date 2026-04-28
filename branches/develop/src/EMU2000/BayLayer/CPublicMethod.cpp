/*
 * =====================================================================================
 *
 *       Filename:  cpublicmethod.cpp
 *
 *    Description:   ͨѶ�������ڴ湲���ռ�֮���ṩ��������
 *
 *        Version:  1.0
 *        Created:  2014��07��17�� 08ʱ46��00��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *        Company:  esdtek
 *
 * =====================================================================================
 */


#include	"CPublicMethod.h"

/*==================================�ⲿ���� =====================================*/
extern "C" time_t MakeSecond( unsigned short wYear, unsigned short wMonth, unsigned short wDay,
		unsigned short wHour, unsigned short wMinute, unsigned short wSecond );
extern "C" void GetCurrentTime( REALTIME *pRealTime );
/*==================================���� =========================================*/
//static void GetCurrentTime( REALTIME *pRealTime )
//{
//	time_t     lSecond;
//    struct tm  currTime;
//    struct timeval tv;
//	struct timezone tz;
//
//	gettimeofday(&tv, &tz);
//    lSecond = (time_t)(tv.tv_sec);
//    localtime_r( &lSecond, &currTime );
//    pRealTime->wMilliSec = tv.tv_usec/1000;
//    pRealTime->wSecond   = currTime.tm_sec;
//    pRealTime->wMinute   = currTime.tm_min;
//    pRealTime->wHour     = currTime.tm_hour;
//    pRealTime->wDay      = currTime.tm_mday;
//    pRealTime->wMonth    = 1+currTime.tm_mon;
//    pRealTime->wYear     = 1900+currTime.tm_year;
//}

BOOL CPublicMethod::m_bDDBBusLinkStatus[ MAX_LINE ];//����״̬
BOOL CPublicMethod::m_bDDBStnLinkStatus[ MAX_STN_SUM ];//װ��״̬
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  CPublicMethod
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CPublicMethod::CPublicMethod ()
{/*{{{*/
	m_wGatherDevCount = 0 ;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ~CPublicMethod
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CPublicMethod::~CPublicMethod ()
{/*{{{*/
	printf ( "publicmethod destructor\n" );
}  /* -----  end of publicmethod CPublicMethod::~CPublicMethod  (destructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  CPublicMethod
 * Description:  ͨ��wSerialNo ��ȡ�����ߺź���Ӧ��ַ
 *       Input:  ˳��� ����Ӧ������ ��ַ
 *		Return:  1:�ɹ� 0:ʧ��
 *--------------------------------------------------------------------------------------
 */
BOOL CPublicMethod::GetBusLineAndAddr ( WORD wSerialNum, BYTE &byBusNo, WORD &wDevAddr , char *pDevName /*= NULL*/ )
{/*{{{*/
	CProtocol *pMoudle = GetProtocolMoudle( wSerialNum );
	if( NULL == pMoudle )
		return FALSE;

	byBusNo = pMoudle->m_byLineNo;
	wDevAddr = pMoudle->m_wDevAddr;
	if (pDevName)
		strcpy(pDevName, pMoudle->m_sDevName);

	return TRUE;
}		/* -----  end of method CPublicMethod::GetBusLineNo  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  GetProtocolMoudle
 * Description:  ͨ����Ż����Ӧ��Э��ģ���
 *       Input:  wSerialNo װ�����
 *		Return:  ��ѯ��ܣ�����NULL
 *				 ��ѯ�ɹ���ģ��ָ��
 *--------------------------------------------------------------------------------------
 */
CProtocol * CPublicMethod::GetProtocolMoudle ( WORD wSerialNo )
{/*{{{*/
	if(wSerialNo >= m_wGatherDevCount)
		return NULL;
	int nCount = m_pBusManager->m_sbus.size(); //���߸���PAUSEҲ��
	PBUSMANAGER pBus = NULL;
	CProtocol *pProtObj, *pMoudle;
	int i;
	int nSerialNo = 0;

	for (i = 0; i < nCount; i++)				//nCount:��Ϊ22!
	{
		int nMoudleSize;
		pBus = m_pBusManager->m_sbus[i];
		if( pBus == NULL )
		{
			//printf ( "[GetBusLineAndAddr]:pBus==NULL\n" );
			return NULL;
		}

		if( pBus->byBusNo == 0xFF )
			//			return NULL ;				ֱ�ӷ���NULL�����ͨѶ������������п���(PAUSE)�������޷�ת����Modified by cyz 2017-04-24
			continue;

		pProtObj = pBus->m_Protocol;
		if ( pProtObj == NULL )
		{
			//printf ( "[GetBusLineAndAddr]:pProtObj==NULL\n" );
			return NULL;
		}

		//һ�����ߵ�ģ�����
		nMoudleSize = pProtObj->m_module.size();

		//ģ��������
		if( pProtObj->m_ProtoType == PROTOCO_GATHER )
			nSerialNo += nMoudleSize;

		if(nSerialNo <= wSerialNo)
			continue;
		else{
			//�ҵ���Ӧģ��
			pMoudle = pProtObj->m_module[wSerialNo - (nSerialNo - nMoudleSize)];
			if ( pMoudle == NULL )
			{
				//printf ( "[GetBusLineAndAddr]:pMoudle==NULL\n" );
				return NULL;
			}
			//����ģ����������������һ�� ���� ģ�����ͺ�Ϊ0 ��ͨ��
			if( wSerialNo != pMoudle->m_SerialNo || pMoudle->m_wModuleType == 0 )
			{
				printf ( "[CPUBLICMETHOD]:can't find wSerialNo=%d \n", wSerialNo );
				return NULL;
			}

			return pMoudle;
		}
	}

	if(i >= nCount)
	{
		printf("i=%d nCount:%d can't find the SerialNo=%d\n", i, nCount, wSerialNo );
		return NULL;
	}

	return NULL;
}		/* -----  end of method CPublicMethod::GetProtocolMoudle  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  CPublicMethod
 * Description:  ͨ�����ߺź�װ�õ�ַ�Ż�ȡ ������
 *       Input:  ���ߺ� ��ַ��
 *		Return:  -1:ʧ��  �����ɹ�
 *--------------------------------------------------------------------------------------
 */
int  CPublicMethod::GetSerialNo ( BYTE byBusNo, WORD wDevAddr )
{/*{{{*/
	PBUSMANAGER pBus = NULL;
	CProtocol *pProtObj = NULL, *pMoudle = NULL;
	int nMoudleSize = -1;

	//by zhanghg
	if( m_pBusManager == NULL )
		return -1 ;

	int size = m_pBusManager->m_sbus.size() ;
	if( byBusNo >= size )
		return -1 ;

	pBus = m_pBusManager->m_sbus[byBusNo];
	if(pBus == NULL)
	{
		return -1;
	}

	pProtObj = pBus->m_Protocol;
	if ( pProtObj == NULL )
	{
		return -1;
	}

	//����ģ��
	nMoudleSize = pProtObj->m_module.size();
	for (int k = 0; k < nMoudleSize; k++)
	{
		pMoudle = pProtObj->m_module[k];
		if ( pMoudle == NULL )
		{
			return -1;
		}

		if(wDevAddr == pMoudle->m_wDevAddr)
		{
			return pMoudle->m_SerialNo;
		}
	}

	return -1;
}		/* -----  end of method CPublicMethod::GetSerialNo  ----- *//*}}}*/




/********************************************�ɼ�**************************************/
/*==================================ң�⴦��=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  CPublicMethod :: YcUpdate δʹ��
 * Description:  ͨ����YCDATA  ����ң����Ϣ
 *       Input:  �������  Э��������õ�YC_DATA����  ң������
 *      Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::YcUpdate ( WORD SerialNo, YC_DATA YcData[], UINT YcNum )
{/*{{{*/
	//int nLen = 0;
	UINT i;
	//RTDBDATA dbData;
	// float fVal;

	//dbData.dwAddrID = 0;
	//dbData.byDevNum = SerialNo;

	for ( i = 0; i < YcNum; i++ )
	{
		switch(YcData[i].byYcType)
		{
		case 0:
			//				wVal = (DWORD)YcData[i].YcValue;
			//				dbData.byTypeID = 0x63;
			//		        *((WORD*)&dbData.byDataBuf[6*i])   = YcData[i].wPnt;
			//				*((DWORD*)&dbData.byDataBuf[6*i+2]) = wVal;
			//				nLen = 6 * ( i + 1 );
			SetYcData( SerialNo, YcData[i].wPnt, YcData[i].YcValue);
			break;
		case 1:
			// fVal = (float)YcData[i].YcValue;
			// dbData.byTypeID = 0x6e;
			// *((WORD*)&dbData.byDataBuf[6*i])   = YcData[i].wPnt;
			// *((float*)&dbData.byDataBuf[6*i+2]) = fVal;
			// nLen = 6 * ( i + 1 );
			break;
		case 2:
			//				wVal = (DWORD)YcData[i].YcValue;
			//				dbData.byTypeID = 0x63;
			//				dbData.byDataBuf[0] = 1;
			//				dbData.byDataBuf[1] = YcData[i].MilSecond & 0xff;
			//				dbData.byDataBuf[2] = (YcData[i].MilSecond >> 8) & 0xff;
			//				dbData.byDataBuf[3] = YcData[i].Minute;
			//				dbData.byDataBuf[4] = YcData[i].Hour;
			//				dbData.byDataBuf[5] = YcData[i].Day;
			//				dbData.byDataBuf[6] = YcData[i].Month;
			//				dbData.byDataBuf[7] = YcData[i].Year;
			//		        *((WORD*)&dbData.byDataBuf[6*i+8])   = YcData[i].wPnt;
			//				*((DWORD*)&dbData.byDataBuf[6*i+ 10]) = wVal;
			//				nLen = 6 * (i + 1) + 8;
			// SetYcDataWithTime(SerialNo, YcData[i].wPnt, YcData[i].YcValue, (char *) &YcData[i].MilSecond);
			break;
		case 3:
			// fVal = (float)YcData[i].YcValue;
			// dbData.byTypeID = 0x72;
			// dbData.byDataBuf[0] = 1;
			// dbData.byDataBuf[1] = YcData[i].MilSecond & 0xff;
			// dbData.byDataBuf[2] = (YcData[i].MilSecond >> 8) & 0xff;
			// dbData.byDataBuf[3] = YcData[i].Minute;
			// dbData.byDataBuf[4] = YcData[i].Hour;
			// dbData.byDataBuf[5] = YcData[i].Day;
			// dbData.byDataBuf[6] = YcData[i].Month;
			// dbData.byDataBuf[7] = YcData[i].Year;
			// *((WORD*)&dbData.byDataBuf[6*i+8])   = YcData[i].wPnt;
			// *((float*)&dbData.byDataBuf[6*i+ 10]) = fVal;
			// nLen = 6 * (i + 1) + 8;
			break;
		default:
			break;
		}

	}
	//dbData.wDataLen = nLen;

	// m_pRdbObj->WriteData((BYTE *)&dbData, nLen+8);
	return ;
}		/* -----  end of publicmethod CPublicMethod::YcUpdate  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYcData
 * Description:  ���ô���һ��ң��
 *		 Input:  ������� װ�õ�� ң��ֵ
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYcData ( WORD wSerialNo, WORD wPnt, float fVal )
{/*{{{*/

	YC_DATA ycData ;
	ycData.wSerialNo = wSerialNo ;
	ycData.wPnt = wPnt ;
	ycData.fYcValue = fVal ;
	ycData.byYcType = 0 ;


	m_pRdbObj->WriteVal( wSerialNo , YC_TYPE , &ycData ) ;

	return ;
}		/* -----  end of method CPublicMethod::SetYcData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYcDataWithTime
 * Description:  ���ô���һ����ʱ���ң��
 *		 Input:  ������� װ�õ�� ң��ֵ ʱ��ָ��
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYcDataWithTime ( WORD wSerialNo, WORD wPnt, float fVal, TIMEDATA *pTime  )
{/*{{{*/
	YC_DATA ycData ;
	ycData.wSerialNo = wSerialNo ;
	ycData.wPnt = wPnt ;
	ycData.fYcValue = fVal ;
	ycData.byYcType = 2 ;


	ycData.MilSecond = pTime->MiSec ;
	ycData.Second = pTime->Second ;
	ycData.Minute = pTime->Minute ;
	ycData.Hour = pTime->Hour ;
	ycData.Day = pTime->Day ;
	ycData.Month = pTime->Month ;
	ycData.Year = pTime->Year ;

	m_pRdbObj->WriteVal( wSerialNo , YC_TYPE , &ycData ) ;


	return ;
}		/* -----  end of method CPublicMethod::SetYcDataWithTime  ----- *//*}}}*/

/*==================================ң�Ŵ���=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  CPublicMethod :: YxUpdateδʹ��

 * Description:  ����ң����Ϣ
 *       Input:  �������  Э��������õ�YX_DATA����  ң������
 *      Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::YxUpdate (  WORD SerialNo, YX_DATA YxData[], UINT YxNum)
{/*{{{*/
	//	int nLen = 0;
	//	RTDBDATA dbData;
	UINT i;

	//	dbData.dwAddrID = 0;
	//	dbData.byDevNum = SerialNo;
	for ( i = 0; i < YxNum; i++ )
	{
		switch(YxData[i].byYxType)
		{
		case 0:
		case 1:
			//			dbData.byTypeID = 0x66;
			//	        *((WORD*)&dbData.byDataBuf[3*i])   = YxData[i].wPnt;
			//			if(YxData[i].byYxType == 1)
			//				dbData.byDataBuf[3*i+2] = YxData[i].YxValue - 1;
			//			else
			//				dbData.byDataBuf[3*i+2] = YxData[i].YxValue;
			//			nLen = 3*(i+1);
			//	SetYxData( SerialNo, YxData[i].wPnt, YxData[i].byYxType);
			break;
		case 2:
		case 3:
			//			dbData.byTypeID = 0x6a;
			//			dbData.byDataBuf[0+9*i] = YxData[i].MilSecond & 0xff;
			//			dbData.byDataBuf[1+9*i] = (YxData[i].MilSecond >> 8) & 0xff;
			//			dbData.byDataBuf[2+9*i] = YxData[i].Minute;
			//			dbData.byDataBuf[3+9*i] = YxData[i].Hour;
			//			dbData.byDataBuf[4+9*i] = YxData[i].Day;
			//			dbData.byDataBuf[5+9*i] = YxData[i].Month;
			//			dbData.byDataBuf[6+9*i] = YxData[i].Year;
			//	        *((WORD*)&dbData.byDataBuf[7+9*i])   = YxData[i].wPnt;
			//			if(YxData[i].byYxType == 3)
			//				*((DWORD*)&dbData.byDataBuf[9+9*i]) = YxData[i].YxValue - 1;
			//			else
			//				*((DWORD*)&dbData.byDataBuf[9+9*i]) = YxData[i].YxValue  ;
			//			nLen = 9*(i+1);

			//SetYxDataWithTime( SerialNo, YxData[i].wPnt, YxData[i].byYxType, (char*) &YxData[i].MilSecond);
			break;
		default:
			break;

		}

	}
	//	dbData.wDataLen = nLen;
	//	m_pRdbObj->WriteData((BYTE *)&dbData, nLen+8);

	return ;
}		/* -----  end of publicmethod CPublicMethod::YxUpdate  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYxData
 * Description:  ���ô���һ������ң��
 *		 Input:  ������� װ�õ�� ң��ֵ(0 1)
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYxData ( WORD wSerialNo, WORD wPnt, BYTE byVal )
{/*{{{*/
	if(byVal != 0 && byVal != 1)
	{
		printf ( "YX SerialNo = %d Ponit = %d value = %d \n", wSerialNo, wPnt, byVal );
		return;
	}

	YX_DATA yxData ;
	yxData.byYxType = 0 ;
	yxData.YxValue = byVal ;
	yxData.wSerialNo = wSerialNo ;
	yxData.wPnt = wPnt ;

	m_pRdbObj->WriteVal( wSerialNo , YX_TYPE , &yxData ) ;

	return ;
}		/* -----  end of method CPublicMethod::SetYxData  ----- *//*}}}*/


/*
*--------------------------------------------------------------------------------------
*       Class:  CPublicMethod
*      Method:  SetYxVariousData
* Description:  ���ô���һ������ң��
*		 Input:  ������� װ�õ�� ң��ֵ(0 1)
*		Return:  void
*--------------------------------------------------------------------------------------
*/
void CPublicMethod::SetYxVariousData(WORD wSerialNo, WORD wPnt, WORD byVal)
{/*{{{*/

	YX_DATA yxData;
	yxData.byYxType = 5;
	yxData.YxValue = byVal;
	yxData.wSerialNo = wSerialNo;
	yxData.wPnt = wPnt;

	m_pRdbObj->WriteVal(wSerialNo, YX_TYPE, &yxData);

	return;
}		/* -----  end of method CPublicMethod::SetYxData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYxDataWithTime
 * Description: ���ô���һ����ʱ�굥��ң��
 *		 Input: ������� װ�õ�� ң��ֵ ʱ��ָ��
 *		Return: void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYxDataWithTime ( WORD wSerialNo, WORD wPnt, BYTE byVal, TIMEDATA *pTime )
{/*{{{*/
	if(byVal != 0 && byVal != 1)
	{
		printf ( "YXSoe SerialNo = %d Ponit = %d value = %d err\n", wSerialNo, wPnt, byVal );
		return;
	}
	if( !IsSoeTime( pTime->MiSec,
				pTime->Second,
				pTime->Minute,
				pTime->Hour,
				pTime->Day,
				pTime->Month,
				pTime->Year) )
	{
		//�Ժ������־
		return;
	}


	YX_DATA yxData ;
	yxData.byYxType = 2 ;
	yxData.YxValue = byVal ;
	yxData.wSerialNo = wSerialNo ;
	yxData.wPnt = wPnt ;
	yxData.MilSecond = pTime->MiSec ;
	yxData.Second = pTime->Second ;
	yxData.Minute = pTime->Minute ;
	yxData.Hour = pTime->Hour ;
	yxData.Day = pTime->Day ;
	yxData.Month = pTime->Month ;
	yxData.Year = pTime->Year ;

	m_pRdbObj->WriteVal( wSerialNo , YX_TYPE , &yxData ) ;

	return ;
}		/* -----  end of method CPublicMethod::SetYxDataWithTime  ----- *//*}}}*/

/*==================================ң������=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  CPublicMethod :: YmUpDateδʹ��

 * Description:  ����ң����Ϣ
 *       Input:  �������  Э��������õ�YM_DATA����  ң������
 *      Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::YmUpDate ( WORD SerialNo, YM_DATA YmData[], UINT YmNum )
{/*{{{*/
	//	int nLen = 0;
	//	RTDBDATA dbData;
	UINT i;

	//	dbData.dwAddrID = 0;
	//	dbData.byDevNum = SerialNo;
	//	dbData.byTypeID = 0x6c;

	for(i = 0; i < YmNum; i++)
	{
		//		*((WORD*)&dbData.byDataBuf[i*6])    = YmData[i].wPnt;
		//		*((DWORD*)&dbData.byDataBuf[i*6+2]) = (DWORD)YmData[i].YmValue;
		//		nLen = (i + 1) *6;
		SetYmData( SerialNo, YmData[i].wPnt, YmData[i].YmValue);
	}
	//	dbData.wDataLen = nLen;
	//	m_pRdbObj->WriteData((BYTE *)&dbData, nLen+8);
	return ;
}		/* -----  end of publicmethod CPublicMethod::YmUpDate  ----- *//*}}}*/



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYmData
 * Description:	 ���ô���һ������ң��
 *		 Input:  ������� װ�õ�� ң��ֵ(float)
 *		Return:	 void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYmData ( WORD wSerialNO, WORD wPnt, float fVal )
{/*{{{*/
	YM_DATA ymData ;
	ymData.YmValue = fVal;
	ymData.wSerialNo = wSerialNO;
	ymData.wPnt = wPnt ;
	m_pRdbObj->WriteVal( wSerialNO , YM_TYPE , &ymData ) ;
	return ;
}		/* -----  end of method CPublicMethod::SetYmData  ----- *//*}}}*/
void CPublicMethod::SetYmData( WORD wSerialNO, WORD wPnt, double dVal )
{/*{{{*/
	YM_DATA ymData ;
	ymData.YmValue = dVal;
	ymData.wSerialNo = wSerialNO;
	ymData.wPnt = wPnt ;
	m_pRdbObj->WriteVal( wSerialNO ,  YM_TYPE  , &ymData ) ;
	return ;
}
void CPublicMethod::SetVarsListData ( WORD wSerialNo, VARSLIST varslist )
{
    varslist.wSerialNo = wSerialNo ;
    if (varslist.num > VARIBLE_MAX_NUM) {
        varslist.num = VARIBLE_MAX_NUM ;
    }
    m_pRdbObj->WriteVal( wSerialNo , VARSLIST_TYPE , &varslist ) ;
    return ;
}
void CPublicMethod::SetYmData( WORD wSerialNO, WORD wPnt, QWORD dVal )
{/*{{{*/
	YM_DATA ymData ;
	ymData.YmValue = dVal;
	ymData.wSerialNo = wSerialNO;
	ymData.wPnt = wPnt ;
	m_pRdbObj->WriteVal( wSerialNO ,  YM_TYPE  , &ymData ) ;
	return ;
}
BOOL CPublicMethod::GetYmData( WORD wSerialNo, WORD wPnt, QWORD &dwVal )
{/*{{{*/

	int iRet = m_pRdbObj->GetPulseVal( wSerialNo , wPnt , &dwVal);
	if( iRet == -1 )
		return FALSE ;

	return TRUE ;
}/*}}}*/

/*==================================ң�ش���=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkSelRtn
 * Description:	 ң��ѡ�񷵻�
 *		 Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkSelRtn (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal )
{/*{{{*/
	printf ( "YK_SEL_RTN\n" );
	if( !ProcessDDB ( pProtocol, byBusNo, wDevAddr, wPnt, byVal, YK_SEL_RTN ) )
	{
		SetYkDeal ( pProtocol,  byBusNo, wDevAddr,  wPnt,  byVal, YK_SEL_RTN );
	}
	return ;
}		/* -----  end of method CPublicMethod::SetYkSelRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkExeRtn
 * Description:  ң��ִ�з���
 *		 Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkExeRtn (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal )
{/*{{{*/
	printf ( "YK_EXCT_RTN\n" );
	if( !ProcessDDB ( pProtocol, byBusNo, wDevAddr, wPnt, byVal, YK_EXCT_RTN ) )
	{
		SetYkDeal ( pProtocol,  byBusNo, wDevAddr,  wPnt,  byVal, YK_EXCT_RTN );
	}
	return ;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkCancelRtn
 * Description:  ң��ȡ��
 *		 Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkCancelRtn (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal )
{/*{{{*/
	printf ( "YK_CANCEL_RTN\n" );
	if( !ProcessDDB ( pProtocol, byBusNo, wDevAddr, wPnt, byVal, YK_CANCEL_RTN ) )
	{
		SetYkDeal ( pProtocol,  byBusNo, wDevAddr,  wPnt,  byVal, YK_CANCEL_RTN );
	}
	return ;
}/*}}}*/





/********************************************ת��**************************************/
//����Ӧ����ֵ�������
/*==================================ң�⴦��=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ReadAllYcData
 * Description:  ��ȡ���е�ң��ֵ
 *		 Input:	 �洢���ݵ�pData
 *      Return:  void
 */
void CPublicMethod::ReadAllYcData ( float *pData )
{/*{{{*/
	m_pRtuObj->ReadAnalogData( pData );
	return ;
}		/* -----  end of method CPublicMethod::ReadAllYcData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ReadYcData
 * Description:  ��ȡ�����ڴ��е�һ��ң��Դ��ֵ
 *		 Input:  ������  ң����
 *      Return:  DWORD
 *--------------------------------------------------------------------------------------
 */
DWORD CPublicMethod::ReadYcData ( WORD wSerialNO, WORD wPnt )
{/*{{{*/
	const ANALOGITEM *pItem =
		m_pRdbObj->GetAnalogObj(wSerialNO, wPnt );
	return pItem->dwRawVal;
}		/* -----  end of method CPublicMethod::ReadYcData  ----- *//*}}}*/

/*==================================ң�Ŵ���=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ReadAllYxData
 * Description:  ��ȡ���е�ң��ֵ
 *		 Input:  �洢���ݵ�pData
 *      Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::ReadAllYxData ( BYTE *pData )
{/*{{{*/
	m_pRtuObj->ReadDigitalData( pData );
	return ;
}		/* -----  end of method CPublicMethod::ReadAllYxData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ReadYxData
 * Description:  ��ȡң��ֵ
 *		 Input:  �����ţ� ��ţ� ��ȡֵ
 *      Return:  0 �ɹ� -1 ʧ��
 *--------------------------------------------------------------------------------------
 */
int CPublicMethod::ReadYxData ( WORD wSerialNO, WORD wPnt, WORD *pwVal)
{/*{{{*/
	int Rtn = m_pRdbObj->GetDigitalVal( wSerialNO, wPnt, pwVal);
	return Rtn;
}		/* -----  end of method CPublicMethod::ReadYxData  ----- *//*}}}*/

/*==================================ң������=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ReadAllYmData
 * Description:  ��ȡ���е��ֵ
 *		 Input:  �洢���ݵ�pData
 *      Return:  void
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::ReadAllYmData ( QWORD *pdwData )
{/*{{{*/
	m_pRtuObj->ReadPulseData( pdwData );
	return ;
}		/* -----  end of method CPublicMethod::ReadAllYmData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  ReadYmData
 * Description:  ��ȡһ�����ֵ
 *		 Input:  �����ţ� ��ţ� ��ȡֵ
 *      Return:  0 �ɹ� -1 ʧ
 *--------------------------------------------------------------------------------------
 */
int CPublicMethod::ReadYmData ( WORD wSerialNO, WORD wPnt, QWORD *pdwVal )
{/*{{{*/
	int Rtn = m_pRdbObj->GetPulseVal( wSerialNO, wPnt, pdwVal );
	return Rtn;
}		/* -----  end of method CPublicMethod::ReadYmData  ----- *//*}}}*/

/*==================================ң�ش���=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkSel
 * Description:  ң��ѡ��
 *		 Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ
 *      Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkSel (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal )
{/*{{{*/
	printf ( "YK_SEL byBusNo =%d wDevAddr=%d   \
		wPnt=%d bVal=%d\n" , byBusNo , wDevAddr , wPnt , byVal );
	if( !ProcessDDB( pProtocol ,byBusNo, wDevAddr,  wPnt,  byVal, YK_SEL ) )
	{
		SetYkDeal ( pProtocol,  byBusNo, wDevAddr,  wPnt  ,  byVal, YK_SEL );
	}
	return ;
}		/* -----  end of method CPublicMethod::SetYkSel  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkExe
 * Description:  ң��ִ��
 *		 Input:   ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ
 *      Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkExe (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal )
{/*{{{*/
	printf ( "YK_EXCT\n" );
	if( !ProcessDDB( pProtocol ,byBusNo, wDevAddr,  wPnt,  byVal, YK_EXCT ) )
	{
		SetYkDeal ( pProtocol,  byBusNo, wDevAddr,  wPnt,  byVal, YK_EXCT );
	}

	return ;
}		/* -----  end of method CPublicMethod::SetYkExe  ----- *//*}}}*/

BOOL CPublicMethod::ProcessDDB( const CProtocol *pProtocol , BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal ,int iFlag )
{/*{{{*/
	if( YK_SEL_RTN == iFlag
			|| YK_EXCT_RTN == iFlag
			|| YK_CANCEL_RTN == iFlag )
	{
		//�ж�˫���������
		if( !CPublicMethod::IsHaveDDB() ||
				( CPublicMethod::GetDDBSyncState() != STATUS_SLAVE ) ||
				pProtocol->m_ProtoType != PROTOCO_TRANSPROT
		  )
		{
			return FALSE;
		}

		BYTE byDDBBusNo = 0xFF ;
		WORD wDDBAddr = 0xFF ;
		//�ж��ܷ�����Ӧ��DDBЭ�����ߺ�װ��
		if( !CPublicMethod::GetDDBBusAndAddr( byDDBBusNo , wDDBAddr ) )
		{
			return FALSE;
		}
		//�ж�Ŀ�����ߺ͵�ַ
		if ( byBusNo == byDDBBusNo
				&& wDevAddr == wDDBAddr)
		{
			return FALSE;
		}

		SetDDBYkRtnDeal( pProtocol, byBusNo, wDevAddr, wPnt, byVal, iFlag );

		printf ( "DDB YK_RTN\n" );
		return TRUE;

	}
	else if( YK_SEL == iFlag
			|| YK_EXCT == iFlag
			|| YK_CANCEL == iFlag)
	{
		//�ж�˫���������
		if( !CPublicMethod::IsHaveDDB() ||
				( CPublicMethod::GetDDBSyncState() != STATUS_SLAVE ) ||
				pProtocol->m_ProtoType == PROTOCO_GATHER
		  )
		{
			return FALSE ;
		}

		//�ж��ܷ�����Ӧ��DDBЭ�����ߺ�װ��
		BYTE byDDBBusNo = 0xFF ;
		WORD wDDBAddr = 0xFF ;
		if( !CPublicMethod::GetDDBBusAndAddr( byDDBBusNo , wDDBAddr ) )
		{
			return FALSE;
		}

		//�ж�Ŀ�����ߺ͵�ַ
		if ( byBusNo == byDDBBusNo
				&& wDevAddr == wDDBAddr)
		{
			return FALSE;
		}

		PDDBYK_DATA pYkData = new DDBYK_DATA ;
		pYkData->byDestBusNo = byBusNo ;
		pYkData->wDestAddr = wDevAddr ;
		pYkData->wPnt = wPnt ;
		pYkData->byVal = byVal ;
		pYkData->byType = 0 ;

		SetDDBYkDeal( pProtocol , byDDBBusNo , wDDBAddr , pYkData , iFlag ) ;

		printf ( "DDB YK\n" );
		return TRUE;

	}

	return FALSE;
}/*}}}*/

void  CPublicMethod::SetDDBYkDeal ( const CProtocol *pProtocol , BYTE byBusNo, WORD wDevAddr, void *pVoid , int iFlag )
{/*{{{*/
	if( pProtocol == NULL || pVoid == NULL )
		return ;

	int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ( ( byBusNo >= nCount ) )
		return;

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG ;
	busMsg->byMsgType = YK_PROTO ;

	busMsg->DstInfo.byBusNo = byBusNo ;
	busMsg->DstInfo.wDevNo =  wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo ;
	busMsg->SrcInfo.wDevNo =  pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag ;
	busMsg->DataNum = 1;

	PDDBYK_DATA pYk_Data = ( PDDBYK_DATA ) pVoid ;
	busMsg->pData = pYk_Data;
	busMsg->DataLen =sizeof(DDBYK_DATA);

	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDDBYkRtnDeal
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDDBYkRtnDeal ( const CProtocol *pProtocol , BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal ,int iFlag  )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ( ( byBusNo >= nCount ) )
		return;

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG ;
	busMsg->byMsgType = YK_PROTO ;

	busMsg->DstInfo.byBusNo = byBusNo ;
	busMsg->DstInfo.wDevNo =  wDevAddr;

	if( !CPublicMethod::GetDDBDevBusAndAddr( busMsg->SrcInfo.byBusNo , busMsg->SrcInfo.wDevNo) )
	{
		delete busMsg;
		return;
	}
	//printf ( "115 %d %d\n", busMsg->SrcInfo.byBusNo, busMsg->SrcInfo.wDevNo );


	busMsg->dwDataType = iFlag;
	busMsg->DataNum = 1;

	YK_DATA * pYk_Data = new YK_DATA ;
	pYk_Data->wPnt = wPnt;
	pYk_Data->byVal = byVal;
	pYk_Data->byType = 0;
	busMsg->pData = pYk_Data;

	busMsg->DataLen =sizeof(YK_DATA);
	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );

	return ;

}		/* -----  end of method CPublicMethod::SetDDBYkRtnDeal  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkCancel
 * Description:  ң��ȡ��
 *		 Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ
 *      Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkCancel (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal )
{/*{{{*/
	printf ( "YK_CANCEL\n" );
	if( !ProcessDDB( pProtocol ,byBusNo, wDevAddr,  wPnt,  byVal, YK_CANCEL ) )
	{
		SetYkDeal ( pProtocol,  byBusNo, wDevAddr,  wPnt,  byVal, YK_CANCEL );
	}

	return;
}		/* -----  end of method CPublicMethod::SetYkCancel  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetYkDeal
 * Description:  ң�ش���
 *		 Input:   ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ ��ʶλ
 *      Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetYkDeal ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal, int iFlag )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ( ( byBusNo >= nCount ) )
		return;

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG ;
	busMsg->byMsgType = YK_PROTO ;

	busMsg->DstInfo.byBusNo = byBusNo ;
	busMsg->DstInfo.wDevNo =  wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo ;
	busMsg->SrcInfo.wDevNo =  pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag;
	busMsg->DataNum = 1;

	YK_DATA * pYk_Data = new YK_DATA ;
	pYk_Data->wPnt = wPnt;
	pYk_Data->byVal = byVal;
	pYk_Data->byType = 0;
	busMsg->pData = pYk_Data;

	busMsg->DataLen =sizeof(YK_DATA);
	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );

	return ;
}/*}}}*/

/*==================================��ֵ����=========================================*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneCall
 * Description:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ�� ��ֵ��ţ�0�� ��ʶλ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneCall ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo  )
{/*{{{*/
	printf ( "DZZONE_CALL\n" );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_CALL );
}		/* -----  end of method CPublicMethod::SetDzZoneCall  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneCallRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneCallRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_CALL_RTN DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_CALL );
}		/* -----  end of method CPublicMethod::SetDzZoneCallRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneSwitchPreset
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneSwitchPreset ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_SWITCH_PRESET DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_PRESET );
}		/* -----  end of method CPublicMethod::SetDzZoneSwitchPreset  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneSwitchPresetRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneSwitchPresetRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo  )
{/*{{{*/
	printf ( "DZZONE_SWITCH_PRESET_RTN DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_PRESET_RTN );
}		/* -----  end of method CPublicMethod::SetDzZoneSwitchPresetRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneSwitchExct
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneSwitchExct (  const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_SWITCH_EXCT DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_EXCT );
}		/* -----  end of method CPublicMethod::SetDzZoneSwitchExct  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneSwitchExctRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneSwitchExctRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_SWITCH_EXCT_RTN DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_EXCT_RTN );
}		/* -----  end of method CPublicMethod::SetDzZoneSwitchExctRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneSwitchCancel
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneSwitchCancel ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_SWITCH_CANCEL DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_CANCEL );
}		/* -----  end of method CPublicMethod::SetDzZoneSwitchCancel  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneSwitchCancelRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneSwitchCancelRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_SWITCH_CANCEL_RTN DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_CANCEL_RTN );
}		/* -----  end of method CPublicMethod::SetDzZoneSwitchCancelRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneError
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneError ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo )
{/*{{{*/
	printf ( "DZZONE_ERROR DzZoneNo  = %d\n", byDzZoneNo );
	SetDzZoneDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DZZONE_SWITCH_CANCEL_RTN );
}		/* -----  end of method CPublicMethod::SetDzZoneError  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzCall
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzCall ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_CALL DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_CALL );
}		/* -----  end of method CPublicMethod::SetDzCall  ----- *//*}}}*/


/*
*--------------------------------------------------------------------------------------
*       Class:  CPublicMethod
*      Method:  SetDzCall_By_StartOrder
* Description:
*       Input:
*		Return:
*--------------------------------------------------------------------------------------
*/
void CPublicMethod::SetDzCall_By_StartOrder(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDZStartOrder, DZ_DATA DzData[], int iDzDataNum)
{/*{{{*/
	printf("DZ_CALL DzStartOrder  = %d DzDataNum=%d\n", byDZStartOrder, iDzDataNum);
	SetDzDeal_By_StartOrder(pProtocol, byBusNo, wDevAddr, byDZStartOrder, DzData, iDzDataNum, DZ_CALL);
}
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzCallRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzCallRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_CALL_RTN DzZoneNo  = %d DzDataNum = %d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_CALL_RTN );
}		/* -----  end of method CPublicMethod::SetDzCallRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzWritePreset
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzWritePreset ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_WRITE_PRESET DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_WRITE_PRESET );
}		/* -----  end of method CPublicMethod::SetDzWritePreset  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzWritePresetRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzWritePresetRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_WRITE_PRESET_RTN DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_WRITE_PRESET_RTN );
}		/* -----  end of method CPublicMethod::SetDzWritePresetRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzWriteExct
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzWriteExct ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_WRITE_EXCT DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_WRITE_EXCT );
}		/* -----  end of method CPublicMethod::SetDzWriteExct  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzWriteExctRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzWriteExctRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_WRITE_EXCT_RTN DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_WRITE_EXCT_RTN );
}		/* -----  end of method CPublicMethod::SetDzWriteExctRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzWriteCancel
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzWriteCancel ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_WRITE_CANCEL DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_WRITE_CANCEL );
}		/* -----  end of method CPublicMethod::SetDzWriteCancel  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzWriteCancelRtn
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzWriteCancelRtn ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_WRITE_CANCEL_RTN DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_WRITE_CANCEL_RTN );
}		/* -----  end of method CPublicMethod::SetDzWriteCancelRtn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzError
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzError ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum )
{/*{{{*/
	printf ( "DZ_ERROR DzZoneNo  = %d DzDataNum=%d\n", byDzZoneNo, iDzDataNum );
	SetDzDeal( pProtocol, byBusNo, wDevAddr, byDzZoneNo, DzData,iDzDataNum, DZ_ERROR );
}		/* -----  end of method CPublicMethod::SetDzError  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzDeal
 * Description:	 ��ֵ����
 *       Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ ��ʶλ
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzDeal (  const CProtocol *pProtocol,
		BYTE byBusNo,
		WORD wDevAddr,
		BYTE byDzZoneNo,
		DZ_DATA DzData[],
		int iDzDataNum,
		int iFlag )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ( ( byBusNo >= nCount ) )
	{
		return;
	}

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG ;
	busMsg->byMsgType = DZ_PROTO ;

	busMsg->DstInfo.byBusNo = byBusNo ;
	busMsg->DstInfo.wDevNo =  wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo ;
	busMsg->SrcInfo.wDevNo =  pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag & 0x7F;
	busMsg->DataNum = iDzDataNum + 1;

	DZ_DATA * pDzData = new DZ_DATA[iDzDataNum+1] ;		//������delete?
	DZ_DATA * pData = pDzData;
	pData->wPnt = 0xFFFF;			//��ֵ���޵��
	pData->byVal[0] = byDzZoneNo;	//��ֵ����
	pData->byType = 0;				//����

	pData++;

	if(DzData != NULL)
		memcpy( pData, DzData, sizeof( DZ_DATA ) * iDzDataNum );


	//busMsg->pData = (void *)DzData;		//- by cyz!
	busMsg->pData = (void *)pDzData;		//+ by cyz!
	busMsg->DataLen =sizeof(DZ_DATA) * (iDzDataNum + 1);

	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );
}		/* -----  end of method CPublicMethod::SetDzDeal  ----- *//*}}}*/

/*
*--------------------------------------------------------------------------------------
*       Class:  CPublicMethod
*      Method:  SetDzDeal_By_StartOrder
* Description:	 ��ֵ����
*       Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ ��ʶλ
*		Return:
*--------------------------------------------------------------------------------------
*/
void CPublicMethod::SetDzDeal_By_StartOrder(const CProtocol *pProtocol,
	BYTE byBusNo,
	WORD wDevAddr,
	BYTE byDZStartOrder,
	DZ_DATA DzData[],
	int iDzDataNum,
	int iFlag)
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	printf("byBusNo=%d  nCount=%d\n", byBusNo, nCount);
	PBUSMANAGER pBus;

	if ((byBusNo >= nCount))
	{
		return;
	}

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG;
	busMsg->byMsgType = DZ_PROTO;

	busMsg->DstInfo.byBusNo = byBusNo;
	busMsg->DstInfo.wDevNo = wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo;
	busMsg->SrcInfo.wDevNo = pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag & 0x7F;
	busMsg->DataNum = iDzDataNum + 1;

	DZ_DATA * pDzData = new DZ_DATA[iDzDataNum + 1];		//������delete?
	DZ_DATA * pData = pDzData;
	//pData->wPnt = 0xFFFF;			//��ֵ���޵��
	//pData->byVal[0] = byDzZoneNo;	//��ֵ����
	//pData->byType = 0;				//����
	printf("--------iDzDataNum=%d------\n", iDzDataNum);
	pData->wPnt = byDZStartOrder;			//��ֵ����ʼ���
	pData->byVal[0] = 0;	//��ֵ����
	pData->byType = 0;				//����

	pData++;

	if (DzData != NULL)
		memcpy(pData, DzData, sizeof(DZ_DATA)* iDzDataNum);

	//busMsg->pData = (void *)DzData;		//- by cyz!
	busMsg->pData = (void *)pDzData;		//+ by cyz!
	busMsg->DataLen =sizeof(DZ_DATA) * (iDzDataNum + 1);

	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );
}		/* -----  end of method CPublicMethod::SetDzDeal  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  SetDzZoneDeal
 * Description:  ��ֵ������
 *       Input:  ԴЭ��ָ�� Ŀ�����ߺ� Ŀ�ĵ�ַ��  װ�õ�� ֵ ��ʶλ
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::SetDzZoneDeal ( const CProtocol *pProtocol,
		BYTE byBusNo,
		WORD wDevAddr,
		BYTE byDzZoneNo,
		int iFlag )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ( ( byBusNo >= nCount ) )
	{
		return;
	}

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG ;
	busMsg->byMsgType = DZ_PROTO ;

	busMsg->DstInfo.byBusNo = byBusNo ;
	busMsg->DstInfo.wDevNo =  wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo ;
	busMsg->SrcInfo.wDevNo =  pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag;
	busMsg->DataNum = 1;

	DZ_DATA * pData = new DZ_DATA ;
	pData->wPnt = 0xFFFF;			//��ֵ���޵��
	pData->byVal[0] = byDzZoneNo;	//��ֵ����
	pData->byType = 0;				//����

	busMsg->pData = (void *)pData;
	busMsg->DataLen =sizeof(DZ_DATA);

	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );
}		/* -----  end of method CPublicMethod::SetDzZoneDeal  ----- *//*}}}*/


void CPublicMethod::Unvarnished( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr , char * pCmd ,int len , int iFlag)
{
    int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ( ( byBusNo >= nCount ) )
	{
		return;
	}

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG ;
	busMsg->byMsgType = UNVARNISH_PROTO ;

	busMsg->DstInfo.byBusNo = byBusNo ;
	busMsg->DstInfo.wDevNo =  wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo ;
	busMsg->SrcInfo.wDevNo =  pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag ;

	char * pInfo = new char[len];
	memcpy(pInfo, pCmd, len);

	busMsg->pData = (void *)pInfo;
	busMsg->DataLen = len;

	LMSG msg ;
	msg.pVoid = busMsg ;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo] ;
	pBus->SendMsg( &msg );
}

void CPublicMethod::UnvarnishedRtn( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr , char * pCmd ,int len , int iFlag)
{
	int nCount = m_pBusManager->m_sbus.size();
	PBUSMANAGER pBus;

	if ((byBusNo >= nCount))
	{
		return;
	}

	/* ��֯��Ϣ���� */
	PBUSMSG busMsg = new BUSMSG;
	busMsg->byMsgType = UNVARNISH_PROTO;

	busMsg->DstInfo.byBusNo = byBusNo;
	busMsg->DstInfo.wDevNo = wDevAddr;

	busMsg->SrcInfo.byBusNo = pProtocol->m_byLineNo;
	busMsg->SrcInfo.wDevNo = pProtocol->m_wDevAddr;

	busMsg->dwDataType = iFlag;

	char * pInfo = new char[len];
	memcpy(pInfo, pCmd, len);

	busMsg->pData = (void *)pInfo;
	busMsg->DataLen = len;

	LMSG msg;
	msg.pVoid = busMsg;

	/* ������Ϣ */
	pBus = m_pBusManager->m_sbus[byBusNo];
	pBus->SendMsg(&msg);
}



void CPublicMethod::CloseSocket( BYTE byBusLine )
{/*{{{*/
	if( m_pRtuObj )
		if( m_pRtuObj->m_byLineNo != byBusLine )
			return ;

	if( m_pPort )
	{
		if( m_pPort->IsPortValid() )
			m_pPort->ClosePort() ;
	}
}/*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  OpenSocket
 * Description:  ��TCP�ͻ���
 *       Input:  ���ߺ�
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CPublicMethod::OpenSocket ( BYTE byBusLine )
{/*{{{*/
	char szBuf[256];
	if( m_pRtuObj )
		if( m_pRtuObj->m_byLineNo != byBusLine )
		{
			return ;
		}

	if( m_pPort )
	{
		int iReturn = strcmp( m_pPort->ClassName() , "CTcpClientShort" ) ;
		if( !m_pPort->IsPortValid()  && ( iReturn == 0 ) )
			m_pPort->OpenPort(szBuf) ;
	}
}		/* -----  end of method CPublicMethod::OpenSocket  ----- *//*}}}*/

BOOL CPublicMethod::IsPortValid( )
{/*{{{*/
	return m_pPort->IsPortValid() ;
}/*}}}*/

BOOL CPublicMethod::GetDevCommState( BYTE byLineNo , WORD wDevNo )
{/*{{{*/
	WORD wSerialNo = GetSerialNo( byLineNo, wDevNo );
	if (wSerialNo >= m_wGatherDevCount)
	{
		return COM_ABNORMAL;
	}

	return GetDevCommState( wSerialNo );
}/*}}}*/

BOOL CPublicMethod::GetDevCommState( WORD wSerialNo )
{/*{{{*/
	if (wSerialNo == 0xFFFF)
	{
		return COM_ABNORMAL;
	}

	// BYTE byLineNo = 0xFF ;
	// WORD wDevNo = 0xFFFF ;
	// BOOL bDevState = FALSE ;
	CProtocol *pMoudle = GetProtocolMoudle( wSerialNo );
	if( NULL == pMoudle )
	{
		return COM_ABNORMAL ;
	}

	//�ж�˫���������
	BOOL bStatus = COM_DEV_ABNORMAL ;
	if( CPublicMethod::IsHaveDDB() &&
			( CPublicMethod::GetDDBSyncState() == STATUS_SLAVE ) &&
			pMoudle->m_ProtoType == PROTOCO_GATHER
	  )
	{
		if( !CPublicMethod::GetDDBStnLinkStatus( wSerialNo , bStatus ) )
			bStatus = COM_DEV_ABNORMAL ;
		return bStatus ;
	}
	else
	{
		return pMoudle->GetDevCommState(  );
	}

	return COM_ABNORMAL;

	// if( GetBusLineAndAddr( wSerialNo , byLineNo , wDevNo ) )
	// bDevState = GetDevCommState( byLineNo , wDevNo ) ;

	// return bDevState ;
}/*}}}*/

BOOL CPublicMethod::GetCommState( BYTE byLineNo )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	if (byLineNo >= nCount)
	{
		return COM_ABNORMAL;
	}

	//װ�õ�ַΪ0����ʹ��
	//�������
	PBUSMANAGER pBus = m_pBusManager->m_sbus[ byLineNo ] ;
	if (pBus == NULL || NULL == pBus->m_pMethod)
	{
		return COM_ABNORMAL;
	}

	CProtocol * pProtocol = pBus->m_Protocol ;
	if (!pProtocol)
	{
		return COM_ABNORMAL;
	}

	BOOL bStatus = COM_ABNORMAL ;
	if( CPublicMethod::IsHaveDDB() &&
			( CPublicMethod::GetDDBSyncState() == STATUS_SLAVE ) &&
			pProtocol->m_ProtoType == PROTOCO_GATHER
	  )
	{
		if (!CPublicMethod::GetDDBBusLinkStatus(byLineNo, bStatus))
		{
			bStatus = COM_ABNORMAL;
		}

		return bStatus;
	}
	else
	{
		if (pBus->m_pMethod->IsPortValid())
		{
			return COM_NORMAL;
		}
		else
		{
			return COM_ABNORMAL;
		}

	}

}/*}}}*/

BYTE CPublicMethod::GetToTalBusNum( )
{/*{{{*/
	return m_pBusManager->m_sbus.size( ) ;
}/*}}}*/

BYTE CPublicMethod::GetDevNum( BYTE byBusNo )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	if ( byBusNo >= nCount )
		return 0 ;

	//װ�õ�ַΪ0����ʹ��
	//�������
	PBUSMANAGER pBus = m_pBusManager->m_sbus[ byBusNo ] ;
	if( pBus == NULL )
		return 0 ;

	if(pBus->m_Protocol->m_ProtoType == PROTOCO_TRANSPROT)		//by cyz! 2017-06-27	������ת��ͨ������ģ����Ŀ����Ĵ���!
		return 0;

	return pBus->m_Protocol->m_module.size() ;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  GetAGetAddrByLineNoAndModuleNo
 * Description:  ͨ�����ߺź�ģ��ŵõ�װ�õ�ַ
 *       Input:  byLineNo : ���ߺ�
 *				 wModuleNo: ģ���
 *		Return:  װ�õ�ַ
 *				 ʧ�ܷ���0
 *--------------------------------------------------------------------------------------
 */
WORD CPublicMethod::GetAddrByLineNoAndModuleNo ( BYTE byLineNo, WORD wModuleNo )
{/*{{{*/
	PBUSMANAGER pBus = NULL;
	CProtocol *pProtObj = NULL, *pMoudle = NULL;
	int nMoudleSize = -1;

	//by zhanghg
	if( m_pBusManager == NULL )
		return 0 ;

	int size = m_pBusManager->m_sbus.size() ;
	if( byLineNo >= size )
		return 0 ;

	pBus = m_pBusManager->m_sbus[byLineNo];
	if(pBus == NULL)
	{
		return 0;
	}

	pProtObj = pBus->m_Protocol;
	if ( pProtObj == NULL )
	{
		return 0;
	}

	//����ģ��
	nMoudleSize = pProtObj->m_module.size();
	if( wModuleNo > nMoudleSize )
	{
		return 0;
	}

	pMoudle = pProtObj->m_module[wModuleNo];
	if ( pMoudle == NULL )
	{
		return 0;
	}

	return pMoudle->m_wDevAddr;
}		/* -----  end of method CPublicMethod::GetAddrByLineNoAndModuleNo   ----- *//*}}}*/

// --------------------------------------------------------
/// \��Ҫ:	ͨ�����ߺź�ģ��ŵõ�װ�ÿ����ַ(����)
///
/// \����:	byLineNo
/// \����:	wModuleNo
///
/// \����:	WORD
// --------------------------------------------------------
char* CPublicMethod::GetDevNameByLineNoAndModuleNo ( BYTE byLineNo, WORD wModuleNo )
{/*{{{*/
	PBUSMANAGER pBus = NULL;
	CProtocol *pProtObj = NULL, *pMoudle = NULL;
	int nMoudleSize = -1;

	//by zhanghg
	if( m_pBusManager == NULL )
		return 0 ;

	int size = m_pBusManager->m_sbus.size() ;
	if( byLineNo >= size )
		return 0 ;

	pBus = m_pBusManager->m_sbus[byLineNo];
	if(pBus == NULL)
	{
		return 0;
	}

	pProtObj = pBus->m_Protocol;
	if ( pProtObj == NULL )
	{
		return 0;
	}

	//����ģ��
	nMoudleSize = pProtObj->m_module.size();
	if( wModuleNo > nMoudleSize )
	{
		return 0;
	}

	pMoudle = pProtObj->m_module[wModuleNo];
	if ( pMoudle == NULL )
	{
		return 0;
	}

	return pMoudle->m_sDevName;
}		/* -----  end of method CPublicMethod::GetAddrByLineNoAndModuleNo   ----- *//*}}}*/

//��ȡ����Э�����ͣ���ת��Э�黹�ǲɼ�Э��
BYTE CPublicMethod::GetBusLineProtocolType( BYTE byLineNo )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	if ( byLineNo >= nCount )
		return 0xFF ;

	PBUSMANAGER pBus = m_pBusManager->m_sbus[ byLineNo ] ;
	if( pBus == NULL )
		return 0xFF ;

	if( pBus->byBusNo == 0xFF || !pBus->m_Protocol )
		return  0xFF ;

	return (BYTE)(pBus->m_Protocol->m_ProtoType) ;
}/*}}}*/

BOOL CPublicMethod::m_IshaveDDB = FALSE ; //û��˫������Э��
BYTE CPublicMethod::m_DDBState = STATUS_SLAVE ; //Ϊ��վ
BYTE CPublicMethod::m_DDBBusNo = 0xFF ;//���ߺ�
WORD CPublicMethod::m_DDBwAddr = 0xFFFF ;//��ַ��
BYTE CPublicMethod::m_DDBDevBusNo = 0xFF ;//���ߺ�
WORD CPublicMethod::m_DDBDevwAddr = 0xFFFF ;//��ַ��

BOOL CPublicMethod::IsHaveDDB( )
{/*{{{*/
	return CPublicMethod::m_IshaveDDB ;
}/*}}}*/

BYTE CPublicMethod::GetDDBSyncState( )
{/*{{{*/
	return CPublicMethod::m_DDBState ;
}/*}}}*/

void CPublicMethod::SetDDBProtocol( )
{/*{{{*/
	CPublicMethod::m_IshaveDDB = TRUE ;
}/*}}}*/

void CPublicMethod::SetDDBSyncState( BYTE bySyncState )
{/*{{{*/
	CPublicMethod::m_DDBState = bySyncState ;
}/*}}}*/

void CPublicMethod::SetDDBBusAndAddr( BYTE byBusNo , WORD wAddr )
{/*{{{*/
	CPublicMethod::m_DDBBusNo = byBusNo ;
	CPublicMethod::m_DDBwAddr = wAddr ;
}/*}}}*/

BOOL CPublicMethod::GetDDBBusAndAddr( BYTE &byBusNo , WORD &wAddr )
{/*{{{*/
	if( !IsHaveDDB( ) )
		return FALSE ;

	byBusNo = CPublicMethod::m_DDBBusNo ;
	wAddr = CPublicMethod::m_DDBwAddr ;
	return TRUE ;
}/*}}}*/

void CPublicMethod::SetDDBDevBusAndAddr( BYTE byBusNo , WORD wAddr )
{/*{{{*/
	CPublicMethod::m_DDBDevBusNo = byBusNo ;
	CPublicMethod::m_DDBDevwAddr = wAddr ;
}/*}}}*/

BOOL CPublicMethod::GetDDBDevBusAndAddr( BYTE &byBusNo , WORD &wAddr )
{/*{{{*/
	if( !IsHaveDDB( ) )
		return FALSE ;

	byBusNo = CPublicMethod::m_DDBDevBusNo ;
	wAddr = CPublicMethod::m_DDBDevwAddr ;
	return TRUE ;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  IsLeapYear
 * Description:  �ж��Ƿ�������
 *       Input:  Year:��
 *		Return:  BOOL��������
 *				 FALSE:��������
 *--------------------------------------------------------------------------------------
 */
BOOL CPublicMethod::IsLeapYear ( UINT uiYear ) const
{/*{{{*/
	if ((uiYear % 100)==0)
	{
		if ((uiYear % 400)==0)
		{
			return TRUE;

		}

	}
	else if ((uiYear % 4)==0)
	{
		return TRUE;

	}

	return FALSE;
}		/* -----  end of method CPublicMethod::IsLeapYear  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CPublicMethod
 *      Method:  IsSoeTime
 * Description:  �Ƿ�����ȷ��SOETime
 *       Input:  ����<1000
 *				 ��<=60
 *				 �֡�=60
 *				 �� ��=60
 *				 ��
 *				 �� 1900---3000
 *		Return:  TRUE:����ȷ��soeʱ��
 *				 FALSE��������ȷ��soeʱ��
 *--------------------------------------------------------------------------------------
 */
BOOL CPublicMethod::IsSoeTime ( UINT uiMilSec,
		UINT uiSec,
		UINT uiMin,
		UINT uiHour,
		UINT uiDay,
		UINT uiMonth,
		UINT uiYear) const
{/*{{{*/
	if( uiMilSec > 999
			|| uiSec > 59
			|| uiMin > 59
			|| uiHour > 23
			|| uiMonth > 12)
	{
		return FALSE;
	}

	if( uiYear < 1900 && uiYear > 3000 )
	{
		return FALSE;
	}

	switch ( uiMonth )
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		if( uiDay > 31 )
		{
			return FALSE;
		}
		break;

	case 4:
	case 6:
	case 9:
	case 11:
		if( uiDay > 30 )
		{
			return FALSE;
		}
		break;

	case 2:
		if( IsLeapYear( uiYear ) )
		{
			if( uiDay > 29 )
			{
				return FALSE;
			}
		}
		else
		{
			if( uiDay > 28 )
			{
				return FALSE;
			}
		}
		break;

	default:
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
}/*}}}*/

WORD CPublicMethod::GetGatherDevCount( )
{/*{{{*/
	return m_wGatherDevCount ;
}/*}}}*/

BOOL CPublicMethod::SetGatherDevCount( WORD wCount )
{/*{{{*/
	m_wGatherDevCount = wCount ;
	return TRUE ;
}/*}}}*/

BYTE CPublicMethod::GetSingleGatherDevCount( BYTE byBusNo , BYTE byDevIndex , WORD * pAddr )
{/*{{{*/
	int nCount = m_pBusManager->m_sbus.size();
	if ( byBusNo >= nCount )
		return 0 ;

	//װ�õ�ַΪ0����ʹ��
	//�������
	PBUSMANAGER pBus = m_pBusManager->m_sbus[ byBusNo ] ;
	if( pBus == NULL )
		return 0 ;

	if( pBus->m_Protocol == NULL || !pBus->m_Protocol->m_module.size()
			|| pBus->m_Protocol->m_ProtoType != PROTOCO_GATHER )
		return 0 ;

	int size = pBus->m_Protocol->m_module.size() ;
	if( pAddr == NULL )
		return size ;
	else
	{
		if( byDevIndex >= size )
		{
			*pAddr = 0 ;
			return 0 ;
		}

		CProtocol * pProtocol = pBus->m_Protocol->m_module[ byDevIndex ] ;
		*pAddr = pProtocol->m_wDevAddr ;
		return  1 ;
	}
}/*}}}*/

PBUSMANAGER CPublicMethod::GetBus( BYTE byIndex )
{/*{{{*/
	if( byIndex > MAX_LINE )
		return NULL ;

	int size = m_pBusManager->m_sbus.size() ;
	if( byIndex >= size )
		return NULL ;

	PBUSMANAGER pBusManager = m_pBusManager->m_sbus[ byIndex ] ;
	return pBusManager ;
}/*}}}*/

/*******************************************************************************
 * ��:CPublicMethod
 * ������:SetDDBBusLinkStatus
 * ��������:��������״̬
 * ����:BYTE byBusNo �������ߺ�0 - ( MAX_LINE-1 )
 * ����:BOOL bStatus 1Ϊͨ����Ϊ��
 * ����ֵ:BOOL ��ByBusNo ̫�󷵻�FALSE
 ******************************************************************************/
BOOL CPublicMethod::SetDDBBusLinkStatus(BYTE byBusNo, BOOL bStatus)
{/*{{{*/
	if( MAX_LINE <= byBusNo )
	{
		printf("SetDDBBusLinkStatus byBusNo:%d is bigger than %d\n",
				byBusNo,MAX_LINE);
		return FALSE;
	}

	CPublicMethod::m_bDDBBusLinkStatus[ byBusNo ] = bStatus;

	return TRUE;
}   /*-------- end class CPublicMethod method SetDDBBusLinkStatus -------- *//*}}}*/

/*******************************************************************************
 * ��:CPublicMethod
 * ������:GetDDBBusLinkStatus( BYTE byBusNo, BOOL &bStatus )
 * ��������:�������ͨѶ״̬
 * ����:BYTE byBusNo �������ߺ�0 - ( MAX_LINE-1 )
 * ����:BOOL bStatus 1Ϊͨ����Ϊ��
 * ����ֵ:BOOL ��ByBusNo ̫�󷵻�FALSE
 ******************************************************************************/
BOOL CPublicMethod::GetDDBBusLinkStatus( BYTE byBusNo, BOOL &bStatus )
{/*{{{*/
	if( MAX_LINE <= byBusNo )
	{
		printf("GetDDBBusLinkStatus byBusNo:%d is bigger than %d\n",
				byBusNo,MAX_LINE);
		return FALSE;
	}

	bStatus = CPublicMethod::m_bDDBBusLinkStatus[ byBusNo ];

	return TRUE;
}   /*-------- end class CPublicMethod method GetDDBBusLinkStatus( BYTE byBusNo, BOOL &bStatus ) -------- *//*}}}*/

/*******************************************************************************
 * ��:CPublicMethod
 * ������:SetDDBStnLinkStatus( WORD wSerialNo, BOOL bStatus )
 * ��������:����װ��״̬
 * ����:wSerialNo װ����� 0 - (MAX_STN_SUM-1)
 * ����:BOOL bStatus 1Ϊͨ����Ϊ��
 * ����ֵ:BOOL �����̫�󷵻�FALSE
 ******************************************************************************/
BOOL CPublicMethod::SetDDBStnLinkStatus( WORD wSerialNo, BOOL bStatus )
{/*{{{*/
	if (MAX_STN_SUM <= wSerialNo )
	{
		printf("SetDDBStnLinkStatus wSerialNo:%d is bigger than %d\n",
				wSerialNo, MAX_STN_SUM);
		return FALSE;
	}

	CPublicMethod::m_bDDBStnLinkStatus[ wSerialNo ] = bStatus;

	return TRUE;

}   /*-------- end class CPublicMethod method SetDDBStnLinkStatus( WORD wSerialNo, BOOL bStatus ) -------- *//*}}}*/

/*******************************************************************************
 * ��:CPublicMethod
 * ������:GetDDBStnLinkStatus( BYTE byBusNo, BOOL &bStatus )
 * ��������:���װ��״̬
 * ����:wSerialNo װ����� 0 - (MAX_STN_SUM-1)
 * ����:BOOL bStatus 0Ϊͨ��1Ϊ��
 * ����ֵ:BOOL �����̫�󷵻�FALSE
 ******************************************************************************/
BOOL CPublicMethod::GetDDBStnLinkStatus( WORD wSerialNo, BOOL &bStatus )
{/*{{{*/
	if (MAX_STN_SUM <= wSerialNo )
	{
		printf("GetDDBStnLinkStatus wSerialNo:%d is bigger than %d\n",
				wSerialNo, MAX_STN_SUM);
		return FALSE;
	}

	bStatus = CPublicMethod::m_bDDBStnLinkStatus[ wSerialNo ];

	return TRUE;
}   /*-------- end class CPublicMethod method GetDDBStnLinkStatus( BYTE byBusNo, BOOL &bStatus ) -------- *//*}}}*/
