/*
 * =====================================================================================
 *
 *       Filename:  DDB.cpp
 *
 *    Description:  双机冗余协议
 *
 *        Version:  1.0
 *        Created:  2014年10月15日 13时35分00秒
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
#include <unistd.h>
#include "../../BayLayer/CPublicMethod.h"
#include "DDB.h"


#define	DDBDEBUG	1				/* 如果定义此语名，则会在调试端口上打印 */
#define	DDBPRINT	1				/* 如果定义此语名，则会在UDP端口上打印 */

/* in librtdb.so*/
extern "C" int  SetCurrentTime( REALTIME *pRealTime  );
extern "C" void GetCurrentTime( REALTIME *pRealTime  );
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
struct tm *tmp = NULL;
char time_buf[64] = {0};
char *_time()
{
	time_t timp = time(NULL);
	tmp = gmtime(&timp);
	memset(time_buf, 0, sizeof(time_buf));
	sprintf(time_buf, "%02d:%02d:%02d", tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	return time_buf;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  CDDB
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDDB::CDDB ()
{/*{{{*/
	m_byLocalStatus = STATUS_SLAVE;//初始化设为从
	m_byRemoteStatus = STATUS_MASTER;//远程初始化设为主


	m_bSwitchState = FALSE;
	m_bTimeProcCount = 0;

	m_byMachineId = IDENTITY_SINGLE;

	m_wAllYcNum = 0;
	m_wAllYxNum = 0;
	m_wAllYmNum = 0;

	m_pYcHeadAddr = 0;
	m_pYxHeadAddr = 0;
	m_pYmHeadAddr = 0;

	m_iDelayedSwitchMinute = 0;
	m_iDelayedSynSecond = 0;
	m_tmLastSwitchTime = 0;
	m_tmNowSwitchTime = 0;
	m_byQuickSwitchNum = 0;

	m_bRecvResponseSwitch = FALSE;

	// 初始化协议状态
	InitProtocolStatus(  );
}  /* -----  end of method CDDB::CDDB  (constructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ~CDDB
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDDB::~CDDB ()
{
}  /* -----  end of method CDDB::~CDDB  (destructor)  ----- */


/* ********************** 其他 *********************************************************/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  DDBYxUpdate
 * Description:  //获取共享内存数据类型的起始位置和数量
 *       Input:	 数据类型1：遥信 2：遥测 3：遥脉  共享内存站指针 起始位置 数量
 *		Return:	 BOOL 当类型不对时返回FALSE
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::GetStnStartPosAndNum ( BYTE byDataType, STNPARAM *pStn, DWORD &dwStartPos, WORD &wCount )
{/*{{{*/
	switch ( byDataType )
	{
	case 1://yx
		wCount = pStn->wDigitalSum;
		dwStartPos = pStn->dwDigitalPos;
		break;

	case 2://yc
		wCount = pStn->wAnalogSum;
		dwStartPos = pStn->dwAnalogPos;
		break;

	case 3://ym
		wCount = pStn->wPulseSum;
		dwStartPos = pStn->dwPulsePos;
		break;

	default:
		return FALSE;
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CDDB::DDBYxUpdate  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetSerialNoAndPnt
 * Description:  通过当前的数据类型和数据点号位置，来获得序号和点号
 *       Input:  byDataType:数据类型
 *       Input:  wPos:数据位置
 *       Input:  wSerialNo:要获得的装置序号
 *       Input:  wPnt:要获得的装置点号
 *       Input:  pStn:中间的共享内存装置指针
 *		Return: 　当相应类型（byDataType）的位置（wPos）不对时返回false
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::GetSerialNoAndPnt (BYTE byDataType, WORD wPos, WORD &wSerialNo, WORD &wPnt, STNPARAM *pStn)
{/*{{{*/
	int i;
	WORD wStnNum;
	WORD wCount;
	DWORD dwStartPos;

	// 根据相应的数据类型，确定数量的合理性
	switch (byDataType)
	{
	case DDB_YX_DATATYPE: // yx
		{
			if( wPos > MAX_DIGITAL_SUM || wPos > m_pMethod->m_pRdbObj->m_nDigitalSum )
			{
				print ( (char *)"DDB:GetSerialNoAndPnt yx wPos is too big!!!" );
				return FALSE;
			}
		}
		break;

	case DDB_YC_DATATYPE://yc
		{
			if( wPos > MAX_ANALOG_SUM || wPos > m_pMethod->m_pRdbObj->m_nAnalogSum )
			{
				print ( (char *)"DDB:GetSerialNoAndPnt yc wPos is too big!!!" );
				return FALSE;
			}
		}
		break;

	case DDB_YM_DATATYPE://ym
		{
			if( wPos > MAX_PULSE_SUM || wPos > m_pMethod->m_pRdbObj->m_nPulseSum )
			{
				print ( (char *)"DDB:GetSerialNoAndPnt ym wPos is too big!!!" );
				return FALSE;
			}
		}
		break;
	default:
		return FALSE;
		break;
	}

	//第一次进入 寻找符合条件的
	if( pStn == NULL )
	{
		pStn = &m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.StnUnit[0];
		wStnNum = m_pMethod->m_pRdbObj->m_wStnSum;

		for ( i=0; i<wStnNum; i++, pStn++)
		{
			if ( !GetStnStartPosAndNum( byDataType, pStn, dwStartPos, wCount ) )
			{
				print ( (char *)"DDB:GetSerialNoAndPnt get stn startpos and num error!!!" );
				continue;
			}

			//大于起始位置 小于起始位置+数量
			if ( wPos >= dwStartPos && wPos < ( wCount + dwStartPos ) )
			{
				wSerialNo = pStn->wStnNum;
				wPnt = wPos - dwStartPos;
				break;
			}
		}
	}
	else//以后在第一次的pStn基础上依次增加
	{
		if ( !GetStnStartPosAndNum( byDataType, pStn, dwStartPos, wCount ) )
		{
			print ( (char *)"DDB:GetSerialNoAndPnt get stn startpos and num error!!!" );
		}

		if( wPos >= wCount + dwStartPos)
		{
			pStn ++;
		}

		wSerialNo = pStn->wStnNum;
		wPnt = wPos - dwStartPos;
	}

	return TRUE;
}		/* -----  end of method CDDB::GetSerialNoAndPnt  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  SwitchStatus
 * Description:  切换主从 1为从换主 0为主换从
 *       Input:	 BOOL bStatus 为1时设置本地为主机模式
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::SwitchStatus ( BOOL bStatus )
{/*{{{*/
	if( bStatus )
	{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		// 本机已经是主机则返回
		if( m_byLocalStatus == STATUS_MASTER )
		{
			return FALSE;
		}

		// 本地为主机 对端为从
		m_byLocalStatus = STATUS_MASTER;
		m_byRemoteStatus = STATUS_SLAVE;

		print( (char *)"Local is Master, Remote is slave!\n"  );


		// if( m_bSyn )
		// {
		// m_SendStatus = NONESTATUS;
		// }
	}
	else
	{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		// 本机已经是从机则返回
		if( m_byLocalStatus == STATUS_SLAVE )
		{
			return FALSE;
		}

			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		// 本地为从机 对端为主
		m_byLocalStatus = STATUS_SLAVE;
		m_byRemoteStatus = STATUS_MASTER;

		//
		m_bRecvResponseSwitch = TRUE;

		print( (char *)"Local is slave, Remote is master!\n" );

		// if( m_bSyn )
		// {
		// m_SendStatus = REQUEST_DATA;
		// m_byDataType = DDB_YX_DATATYPE;
		// m_wDataPos = 0;
		// }
	}
	//更新本地状态
	CPublicMethod::SetDDBSyncState( m_byLocalStatus );
	return TRUE;
}		/* -----  end of method CDDB::SwitchStatus  ----- *//*}}}*/

BOOL CDDB::DevStateSwitchStatus ( BOOL bStatus )
{/*{{{*/
	m_bTimeProcCount = 0;

	SwitchStatus( bStatus );

	time(&m_tmNowSwitchTime);
	time_t TimeDifference = abs(m_tmNowSwitchTime - m_tmLastSwitchTime);
	printf("%lu = %lu - %lu  \n",TimeDifference,m_tmNowSwitchTime,m_tmLastSwitchTime);
	if( TimeDifference < m_iDelayedSynSecond+10 || m_byQuickSwitchNum == 0 )
	{
		m_tmLastSwitchTime = m_tmNowSwitchTime;
		m_byQuickSwitchNum++;
		printf("m_byQuickSwitchNum = %d\n",m_byQuickSwitchNum);
		if( m_byQuickSwitchNum > 6)
		{
			m_byQuickSwitchNum = 6;
		}
	}
	else
	{
		m_tmNowSwitchTime = 0;
		m_tmLastSwitchTime = 0;
		m_byQuickSwitchNum = 0;
	}

	InitProtocolStatus(  );

	// m_byDataType = DDB_YX_DATATYPE;//初始化设置成YX
	// m_bRemoteSyn = FALSE;
	// m_wDataPos = 0;
	// m_bIsReSend = FALSE;		//重发标识位0
	// m_byResendCount = 0;		//重发次数清零
	// m_bIsSending = FALSE;		//发送后置1 接收后值0
	print((char *)"~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	return TRUE;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetDataNum
 * Description:  获取共享内存中数据数量
 *       Input:	 1:遥信 2遥测 3遥脉
 *		Return:  相应数据的数量
 *--------------------------------------------------------------------------------------
 */
WORD CDDB::GetDataNum ( BYTE byDataType )
{/*{{{*/
	WORD wDataSum = 0;
	switch ( byDataType )
	{
	case 1://遥信
		wDataSum = m_pMethod->m_pRdbObj->m_nDigitalSum;
		break;

	case 2://遥测
		wDataSum = m_pMethod->m_pRdbObj->m_nAnalogSum;
		break;

	case 3://遥脉
		wDataSum = m_pMethod->m_pRdbObj->m_nPulseSum;
		break;

	default:
		break;
	}				/* -----  end switch  ----- */
	return wDataSum;
}		/* -----  end of method CDDB::GetDataNum  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetDataHeadAddr
 * Description:	 获取数据首地址
 *       Input:  1:遥信首地址 2：遥测首地址 3：遥脉首地址
 *		Return:	 指针首地址
 *--------------------------------------------------------------------------------------
 */
DWORD CDDB::GetDataHeadAddr ( BYTE byDataType )
{/*{{{*/
	DWORD dwHeadAddr = 0;

	switch ( byDataType )
	{
	case 1://遥信
		dwHeadAddr = (DWORD)&m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.DigitalTable[0] ;
		break;

	case 2://遥测
		dwHeadAddr = (DWORD)&m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.AnalogTable[0] ;			//将pointer转为无符号整型!
		break;

	case 3://遥脉
		dwHeadAddr = (DWORD)&m_pMethod->m_pRdbObj->m_pRTDBSpace->RTDBase.PulseTable[0] ;
		break;

	default:
		break;
	}				/* -----  end switch  ----- */

	return dwHeadAddr;
}		/* -----  end of method CDDB::GetDataHeadAddr  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetCommonFrame
 * Description:  获取通用的数据帧buf[0]=68 buf[1]=16 buf[3]=funcode|id|status
 *       Input:	 缓存区 功能码
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::GetCommonFrame ( BYTE *buf, BYTE byFuncCode )
{/*{{{*/
	BYTE byFrameType;

	buf[0]=0x68;
	buf[1]=0x16;

	byFrameType = byFuncCode & 0x0F;

	if (m_byMachineId == IDENTITY_B)
		byFrameType |= 0x10;   /* B->1*/

	if(m_byLocalStatus == STATUS_SLAVE)
		byFrameType |= 0x20;   /* 1*/

	buf[3] = byFrameType;
}		/* -----  end of method CDDB::GetCommonFrame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetCrc
 * Description:	 计算校验码 校验方式采用MMI上crc校验
 *       Input:
 *		Return:	 WORD
 *--------------------------------------------------------------------------------------
 */
WORD CDDB::GetCrc ( BYTE *pBuf, int nLen )
{/*{{{*/

	WORD Genpoly=0xA001;
	WORD CRC=0xFFFF;
	WORD index;
	while(nLen--)
	{
		CRC=CRC^(WORD)*pBuf++;
		for(index=0;index<8;index++)
		{
			if((CRC & 0x0001)==1)
				CRC=(CRC>>1)^Genpoly;
			else
				CRC=CRC>>1;
		}
	}

	return CRC;
}		/* -----  end of method CDDB::GetCrc  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  print
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::print ( char *szBuf )
{/*{{{*/

#ifdef  DDBDEBUG
	printf ( "%s", szBuf );
#endif     /* -----  not DDBDEBUG  ----- */

#ifdef DDBPRINT
	OutBusDebug(m_byLineNo, (BYTE *)szBuf,strlen(szBuf) , 2);
#endif


}		/* -----  end of method CDDB::print  ----- *//*}}}*/

/* **********************处理部分 ***************************************/
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  YcDataDeal
 * Description:  遥测处理方法
 *       Input:  缓存区 长度
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::YcDataDeal ( BYTE *buf, int len )
{/*{{{*/
	WORD wDataNum;
	WORD wDataPos;
	//DWORD dwYcVal;				//- by cyz! 如果是负数则解释错误!
	int32 dwYcVal;					//+ by cyz!
	STNPARAM *pStn = NULL;
	BYTE *pointer = buf;
	WORD wSerialNo, wPnt;
	BYTE tempbuf[8] = {0};							//+ by cyz!

	wDataNum = ( pointer[5] << 8 ) | pointer[6];
	wDataPos = ( pointer[7] << 8 ) | pointer[8];

	pointer += 9;
	while( wDataNum > 0 )
	{
		memcpy( &dwYcVal, pointer, 4 );
		memcpy(tempbuf, pointer, 4);				//+ by cyz!
		//if(tempbuf[0] || tempbuf[1] || tempbuf[2] || tempbuf[3]){
			//printf(":::: YcDataDeal ::::%02x %02x %02x %02x\n", tempbuf[0], tempbuf[1], tempbuf[2], tempbuf[3]);
			//float *p = (float *)tempbuf;
			//printf(":::: YcDataDeal::fval:%f  dwYcVal:%f ::::\n", *p, (float)dwYcVal);
		//}

		printf("line = %d\n", __LINE__);

		if ( GetSerialNoAndPnt( 2, wDataPos, wSerialNo, wPnt, pStn  ) )
		{
			printf("line =%d %d\n", __LINE__, dwYcVal );
			m_pMethod->SetYcData( wSerialNo, wPnt, (float)dwYcVal );
		}

		printf("line =%d\n", __LINE__);

		pointer += 4;
		wDataPos++;
		wDataNum--;
	}

	m_wDataPos = wDataPos;
	return TRUE;
}		/* -----  end of method CDDB::YcDataDeal  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  YxDataDeal
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::YxDataDeal ( BYTE *buf, int len )
{/*{{{*/
	WORD wDataNum;
	WORD wDataPos;
	BYTE byTmp, i;
	BYTE *pointer = buf;
	STNPARAM *pStn = NULL;
	WORD wSerialNo, wPnt;
	BYTE byYxVal;

	wDataNum = ( pointer[5] << 8 ) | pointer[6];
	wDataPos = ( pointer[7] << 8 ) | pointer[8];

	pointer += 9;

	while( wDataNum > 0 )
	{
		byTmp = *pointer;
		for(i=0; i<8; i++)
		{
			if( byTmp & ( 0x01 << i ) )
			{
				byYxVal = 1;
			}
			else
			{
				byYxVal = 0;
			}

			if ( GetSerialNoAndPnt (1,wDataPos, wSerialNo, wPnt, pStn ) )
			{
				m_pMethod->SetYxData( wSerialNo, wPnt, byYxVal );
			}

			wDataNum -- ;
			wDataPos ++;

			if( wDataNum == 0 )
				break;
		}

		pointer ++;
	}

	m_wDataPos = wDataPos;
	return TRUE;
}		/* -----  end of method CDDB::YxDataDeal  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  YmDataDeal
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::YmDataDeal( BYTE *buf, int len)
{/*{{{*/
	WORD wDataNum;
	WORD wDataPos;
	DWORD dwYmVal;
	BYTE *pointer = buf;
	WORD wSerialNo, wPnt;
	STNPARAM *pStn = NULL;

	wDataNum = ( pointer[5] << 8 ) | pointer[6];
	wDataPos = ( pointer[7] << 8 ) | pointer[8];

	pointer += 9;

	while( wDataNum > 0 )
	{
		memcpy( &dwYmVal, pointer, 4 );

		if ( GetSerialNoAndPnt( 3, wDataPos, wSerialNo, wPnt, pStn  ) )
		{
			m_pMethod->SetYmData( wSerialNo, wPnt, (QWORD)dwYmVal );
		}
		pointer += 4;
		wDataPos++;
		wDataNum--;
	}

	m_wDataPos = wDataPos;
	return TRUE;
}		/* -----  end of method CDDB::YmDataDeal  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  TimeSyncDeal
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL  CDDB::TimeSyncDeal ( BYTE *buf, int len )
{/*{{{*/
	REALTIME CurrTime;

	CurrTime.wYear = ( buf[5] << 8 ) | buf[6];
	CurrTime.wMonth = buf[7];
	CurrTime.wDay = buf[8];
	CurrTime.wHour = buf[9];
	CurrTime.wMinute = buf[10];
	CurrTime.wSecond = buf[11];
	CurrTime.wMilliSec = ( buf[12] << 8 ) | buf[13];

	SetCurrentTime( &CurrTime );
	//char tempbuf[64] = {0};		//+3 by cyz!
	//sprintf(tempbuf, "date -s \"%d-%d-%d %d:%d:%d\"", CurrTime.wYear, CurrTime.wMonth, CurrTime.wDay, CurrTime.wHour, CurrTime.wMinute, CurrTime.wSecond);
	//system(tempbuf);
	return TRUE;
}		/* -----  end of method CDDB::TimeSyncDeal  ----- *//*}}}*/

/*******************************************************************************
 * 类:CDDB
 * 函数名:LinkBusStatusDela
 * 功能描述:总线状态处理
 * 参数:BYTE *buf, int len
 * 返回值:BOOL
 ******************************************************************************/
BOOL CDDB::LinkBusStatusDeal(BYTE *buf, int len)
{/*{{{*/
	WORD wDataNum;
	WORD wDataPos;
	BYTE *pointer = buf;

	// 数据数量和位置
	wDataNum = ( pointer[5] << 8 ) | pointer[6];
	wDataPos = ( pointer[7] << 8 ) | pointer[8];

	pointer += 8;

	while( wDataNum > 0 )
	{
		pointer ++;
		for (int i = 0; i < 8; i++)
		{
			BOOL bStatus = ( *pointer >> i ) & 0x01;
			CPublicMethod::SetDDBBusLinkStatus(wDataPos, bStatus);
			// printf("deal bus%d status=%d\n", wDataPos, bStatus);

			wDataPos++;
			wDataNum--;
			if( 0 == wDataNum )
			{
				break;
			}
		}
	}

	m_wDataPos = wDataPos;
	return TRUE;
}   /*-------- end class CDDB method LinkBusStatusDela -------- *//*}}}*/

/*******************************************************************************
 * 类:CDDB
 * 函数名:LinkStnStatusDeal
 * 功能描述:装置状态处理
 * 参数:BYTE *buf, int len
 * 返回值:BOOL
 ******************************************************************************/
BOOL CDDB::LinkStnStatusDeal(BYTE *buf, int len)
{/*{{{*/
	WORD wDataNum;
	WORD wDataPos;
	BYTE *pointer = buf;

	// 数据数量和位置
	wDataNum = ( pointer[5] << 8 ) | pointer[6];
	wDataPos = ( pointer[7] << 8 ) | pointer[8];

	pointer += 8;

	while( wDataNum > 0 )
	{
		pointer ++;
		for (int i = 0; i < 8; i++)
		{
			BOOL bStatus = ( *pointer >> i ) & 0x01;
			CPublicMethod::SetDDBStnLinkStatus(wDataPos, bStatus);
			printf("deal stn%d status=%d\n", wDataPos, bStatus);
			wDataPos++;
			wDataNum--;
			if( 0 == wDataNum )
			{
				break;
			}
		}
	}

	m_wDataPos = wDataPos;
	return TRUE;

}   /*-------- end class CDDB method LinkStnStatusDeal -------- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessRecvDataBuf
 * Description:  处理数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessRecvDataBuf ( BYTE *buf, int len )
{/*{{{*/
	BOOL bRtn = FALSE;

	switch ( buf[4] )
	{
	case DDB_YC_DATATYPE:
		bRtn = YcDataDeal( buf, len );
		if( buf[3] & 0x80 )
		{
			print( (char *)"yc end\n" );
			m_byDataType = DDB_YM_DATATYPE;
			m_wDataPos = 0;
		}
		break;

	case DDB_YX_DATATYPE:
		bRtn = YxDataDeal( buf, len );
		if( buf[3] & 0x80 )
		{
			print( (char *)"yx end\n" );
			m_byDataType = DDB_YC_DATATYPE;
			m_wDataPos = 0;
		}
		break;

	case DDB_YM_DATATYPE:
		bRtn = YmDataDeal( buf, len );
		if( buf[3] & 0x80 )
		{
			print( (char *)"ym end\n" );
			m_byDataType = DDB_LINKBUSSTATUS_DATATYPE;
			m_wDataPos = 0;
		}
		break;

	case DDB_TIME_DATATYPE:
		bRtn = TimeSyncDeal( buf, len );
		print( (char *)"timesync end\n" );
		m_byDataType = DDB_YX_DATATYPE;
		m_wDataPos = 0;
		break;


	case DDB_LINKBUSSTATUS_DATATYPE://总线状态处理
		{
			print( (char *)"bus begin\n" );
			LinkBusStatusDeal( buf,len);
			if( buf[3] & 0x80 )
			{
				print( (char *)"bus end\n" );
				m_byDataType = DDB_LINKSTNSTATUS_DATATYPE;
				m_wDataPos = 0;
			}
		}
		break;

	case DDB_LINKSTNSTATUS_DATATYPE://装置状态处理
		{
			LinkStnStatusDeal( buf, len );
			if( buf[3] & 0x80 )
			{
				print( (char *)"stn end\n" );
				m_byDataType = DDB_TIME_DATATYPE;
				m_wDataPos = 0;
			}
		}
		break;

	case DDB_YK_DATATYPE:
		print( (char *)"yk \n" );
		if( m_bIsYking )
		{
			m_bIsYking = FALSE;
			m_byDataType = m_bySaveDataType;
			// m_byDataType = DDB_YX_DATATYPE;
		}
		else
		{
			PackSendYkMsg( &buf[5] );
		}
		break;

	default:
		break;
	}				/* -----  end switch  ----- */

	return bRtn;
}		/* -----  end of method CDDB::ProcessRecvDataBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessRequeStSyn
 * Description:  请求同步数据处理
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessRequestSyn ( BYTE *buf, int len )
{/*{{{*/
	m_SendStatus = RESPONSE_SYN;
	return TRUE;
}		/* -----  end of method CDDB::ProcessRequeStSyn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessResponseSyn
 * Description:	 响应同步报文处理
//如果远程未同步 自己也未同步，则按照A主B从 如果对方已同步 本地未同步 则对方主则本地从 对方从则本地主的原则
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessResponseSyn ( BYTE *buf, int len )
{/*{{{*/
	/*	if( ( buf[3] >> 5 ) & 0x01 )
		m_byRemoteStatus = STATUS_SLAVE;
		else
		m_byRemoteStatus = STATUS_MASTER;
		*/
	m_bRemoteSyn = buf[4];
	//未同步
	if( !m_bSyn )
	{
		// //对方已同步
		// if( m_bRemoteSyn )
		// {
		// //远程主则本地从
		// if( m_byRemoteStatus == STATUS_MASTER )
		// {
		// //本地设为从
		// m_byLocalStatus = STATUS_SLAVE;
		// }
		// else if( m_byRemoteStatus == STATUS_SLAVE )
		// {
		// //本地设为主
		// m_byLocalStatus = STATUS_MASTER;
		// }
		// m_SendStatus = RESPONSE_SYN;
		// }
		// else
		// {
		// //双方均未同步 则按照A 主B从的原则
		// if( m_byMachineId == IDENTITY_A )
		// {
		// m_byLocalStatus = STATUS_MASTER;
		// m_byRemoteStatus = STATUS_SLAVE;
		// }
		// else
		// {
		// m_byRemoteStatus = STATUS_MASTER;
		// m_byLocalStatus = STATUS_SLAVE;
		// }
		// }

		print ( (char *)( "Local status is %d\n" ));
		if( m_byLocalStatus == STATUS_SLAVE )
		{
			//本地设为从 请求数据
			m_SendStatus = REQUEST_DATA;
		}
		else
		{
			// //本地为主 等待询问数据
			m_SendStatus = NONESTATUS;
		}
		//类型为遥信
		m_byDataType = DDB_YX_DATATYPE;

		//更新同步状态
		m_bSyn = TRUE;
		// CPublicMethod::SetDDBSyncState( m_bSyn );
	}
	else
	{
		// if( !m_bRemoteSyn )
		// {
		// m_SendStatus = RESPONSE_SYN;
		// }
		// else
		// {
		if( m_byLocalStatus == STATUS_SLAVE )
		{
			//本地设为从 请求数据
			m_SendStatus = REQUEST_DATA;
		}
		else
		{
			//设置响应
			m_SendStatus = RESPONSE_SYN;
			// //本地为主 等待询问数据
			// m_SendStatus = NONESTATUS;
		}
		//类型为遥信
		m_byDataType = DDB_YX_DATATYPE;
		// }
	}

	return TRUE;
}		/* -----  end of method CDDB::ProcessResponseSyn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessRequestData
 * Description:	 收到请求数据 设为发送数据模式
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessRequestData ( BYTE *buf, int len )
{/*{{{*/
	m_byDataType = buf[4];
	m_SendStatus = RESPONSE_DATA;

	if( DDB_YK_DATATYPE == m_byDataType
			&& STATUS_MASTER == m_byLocalStatus )
	{
		print( (char *)"PackSendYkMsg\n" );
		PackSendYkMsg( &buf[5] );
	}
	return TRUE;
}		/* -----  end of method CDDB::ProcessRequestData  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessResponseData
 * Description:  处理收到的数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessResponseData ( BYTE *buf, int len )
{/*{{{*/
	// SwitchStatus( 1 );
	return ProcessRecvDataBuf( buf, len );
}		/* -----  end of method CDDB::ProcessResponseData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessRequestSwitch
 * Description:  主从转化模式请求
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessRequestSwitch ( BYTE *buf, int len )
{/*{{{*/
	/*从转换主*/
	DevStateSwitchStatus(1);

	m_SendStatus = RESPONSE_SWITCH;

	return TRUE;
}		/* -----  end of method CDDB::ProcessRequestSwitch  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessResponseSwitch
 * Description:  主从转化相应
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessResponseSwitch ( BYTE *buf, int len )
{/*{{{*/
	DevStateSwitchStatus(0);

	m_SendStatus = REQUEST_DATA;

	return TRUE;
}		/* -----  end of method CDDB::ProcessResponseSwitch  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  PackSendYkMsg
 * Description:  组织和发送yk命令
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::PackSendYkMsg ( BYTE *buf )
{/*{{{*/
	m_YkStatus = (DDBYKSTATUS)buf[0];

	m_SaveDestYkData.byDestBusNo = buf[1];
	m_SaveDestYkData.wDestAddr = MAKEWORD( buf[3], buf[2] );
	m_SaveDestYkData.wPnt = MAKEWORD( buf[5] , buf[4]);
	m_SaveDestYkData.byVal = buf[6];

	if( STATUS_SLAVE == m_byLocalStatus )
	{
		CPublicMethod::SetDDBDevBusAndAddr( m_SaveDestYkData.byDestBusNo, m_SaveDestYkData.wDestAddr );
	}

	print((char *)"CDDB::PackSendYkMsg\n");
	switch ( m_YkStatus )
	{
	case DDB_YK_SEL:
		print((char *)"CDDB::YkSel\n");
		m_pMethod->SetYkSel( this,
				m_SaveDestYkData.byDestBusNo,
				m_SaveDestYkData.wDestAddr,
				m_SaveDestYkData.wPnt,
				m_SaveDestYkData.byVal);
		m_YkStatus = DDB_YK_SEL_CONFIRM;
		break;

	case DDB_YK_EXE:
		print((char *)"CDDB::YkExe\n");
		m_pMethod->SetYkExe( this,
				m_SaveDestYkData.byDestBusNo,
				m_SaveDestYkData.wDestAddr,
				m_SaveDestYkData.wPnt,
				m_SaveDestYkData.byVal);
		m_YkStatus = DDB_YK_EXE_CONFIRM;
		break;

	case DDB_YK_CANCEL:
		print((char *)"CDDB::YkCancel\n");
		m_pMethod->SetYkCancel( this,
				m_SaveDestYkData.byDestBusNo,
				m_SaveDestYkData.wDestAddr,
				m_SaveDestYkData.wPnt,
				m_SaveDestYkData.byVal);
		m_YkStatus = DDB_YK_CANCEL_CONFIRM;
		break;

	case DDB_YK_SEL_RTN:
		print((char *)"CDDB::YkSelRtn\n");
		m_pMethod->SetYkSelRtn( this,
				m_bySaveSrcBusNo,
				m_wSaveSrcDevAddr,
				m_SaveDestYkData.wPnt,
				m_SaveDestYkData.byVal);
		m_YkStatus = DDB_YK_SEL_RTN_CONFIRM;
		break;

	case DDB_YK_EXE_RTN:
		print((char *)"CDDB::YkExeRtn\n");
		m_pMethod->SetYkExeRtn( this,
				m_bySaveSrcBusNo,
				m_wSaveSrcDevAddr,
				m_SaveDestYkData.wPnt,
				m_SaveDestYkData.byVal);
		m_YkStatus = DDB_YK_EXE_RTN_CONFIRM;
		break;

	case DDB_YK_CANCEL_RTN:
		print((char *)"CDDB::YkCancelRtn\n");
		m_pMethod->SetYkCancelRtn( this,
				m_bySaveSrcBusNo,
				m_wSaveSrcDevAddr,
				m_SaveDestYkData.wPnt,
				m_SaveDestYkData.byVal);
		m_YkStatus = DDB_YK_CANCEL_RTN_CONFIRM;
		break;

	default:
		print((char *)"CDDB::YkError\n");
		InitProtocolStatus( );
		break;
	}				/* -----  end switch  ----- */

	if( DDB_YK_DATATYPE != m_byDataType )
		m_bySaveDataType = m_byDataType;
	m_byDataType = DDB_YK_DATATYPE;

	return TRUE;
}		/* -----  end of method CDDB::PackSendYkMsg  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  WhetherBufValue
 * Description:  判断103报文的有效性 基本判断
 *       Input:  收到的缓存区buf 收到的数据长度len 缓存区有效位置pos
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::WhetherBufValue (BYTE *buf, int &len, int &pos )
{/*{{{*/
	char szBuf[256];
	BYTE *pointer = buf;
	WORD wCrc;
	BYTE byHCrc, byLCrc;
	BYTE byDataLen;
	pos = 0;

	if( buf == NULL || len <= 0 )
		return FALSE;

	while ( len > 0 )
	{
		if( 0x68 == *pointer && 0x16 == *(pointer+1))		//?
		{
			byDataLen = *(pointer+2);
			if(byDataLen+5 > len)
			{
				len--;
				pointer++;
				pos ++;
				sprintf (szBuf, "len err len=%d byDataLen=%d\n", len, byDataLen );
				print( szBuf );
				continue;
			}
			wCrc = GetCrc( pointer+3, byDataLen);
			byHCrc = (wCrc >> 8) & 0xff;
			byLCrc = wCrc & 0xff;
			if ( byHCrc != *(pointer + 3 + byDataLen) || byLCrc != *( pointer + 4 + byDataLen ))
			{
				len--;
				pointer++;
				pos ++;
				print ( (char *)"crc err\n" );
				continue;
			}

			return TRUE;
		}
		else
		{
			len--;
			pointer++;
			pos ++;
		}
	}

	return FALSE;
}		/* -----  end of method CDDB::WhetherBufValue  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  JudgeStatus
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::JudgeStatus ( void )
{/*{{{*/
	if( m_byRemoteStatus == m_byLocalStatus )
	{
		if( m_byMachineId == IDENTITY_A )
		{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
			SwitchStatus( 1 );
		}
		if( m_byMachineId == IDENTITY_B )
		{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
			SwitchStatus( 0 );
		}
	}
}		/* -----  end of method CDDB::JudgeStatus  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  JudgeStatus
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::JudgeStatus ( BYTE byRemoteByte )
{/*{{{*/
	if( ( byRemoteByte >> 5 ) & 0x01 )
	{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		m_byRemoteStatus = STATUS_SLAVE;
	}
	else
	{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		m_byRemoteStatus = STATUS_MASTER;
	}

	JudgeStatus(  );
}		/* -----  end of method CDDB::JudgeStatus  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessRecvTypeBuf
 * Description:  处理接收数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessRecvTypeBuf ( BYTE *buf, int len  )
{/*{{{*/
	//printf ( "buf[3]&0xf=%d\n", buf[3]&0xf );
	JudgeStatus( buf[3] );							//m_byRemoteStatus = ((para >> 5) & 0x01) ? STATUS_SLAVE : STATUS_MASTER;

	switch ( buf[3]&0x0f )
	{
	case REQUEST_SYN:								//0
		print ( (char *)"DDB RECV REQUEST_SYN\n" );
		ProcessRequestSyn( buf, len );
		break;

	case RESPONSE_SYN:
		print ( (char *)"DDB RECV RESPONSE_SYN\n" );
		ProcessResponseSyn( buf, len );
		break;

	case REQUEST_DATA:
		print ( (char *)"DDB RECV REQUEST_DATA" );
		if( buf[4] == DDB_YK_DATATYPE && m_bIsYking)
		{
			print( (char *)"yk rtn\n" );
			m_bIsYking = FALSE;
			m_byDataType = m_bySaveDataType;
		}
		else
		{
			ProcessRequestData( buf, len );
		}
		break;

	case RESPONSE_DATA:
		// print ( (char *)"DDB RECV RESPONSE_DATA" );
		ProcessResponseData( buf, len );
		break;

	case REQUEST_SWITCH:
		ProcessRequestSwitch( buf, len );
		break;

	case RESPONSE_SWITCH:
		ProcessResponseSwitch( buf, len );
		break;

	default:
		break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CDDB::ProcessRecvTypeBuf  ----- *//*}}}*/

/* **********************发送部分 ***************************************/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  YcDataSend
 * Description:	 从内存数据库中获取收据发送
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::YcDataSend ( BYTE *buf, int &len )
{/*{{{*/
	WORD wYcDataNum = m_wAllYcNum;
	ANALOGITEM *pYcData = m_pYcHeadAddr;
	WORD wMaxByte;
	WORD wLeaveYcNum;
	WORD wSendNum = 0;

	GetCommonFrame(buf, RESPONSE_DATA);
	buf[4] = DDB_YC_DATATYPE;
	buf[7] = (m_wDataPos >> 8) & 0xff;
	buf[8] = m_wDataPos & 0xff;

	pYcData += m_wDataPos;
	wMaxByte = DDB_MAX_BUF_LEN - 11;
	wLeaveYcNum = wYcDataNum - m_wDataPos;
	len = 9;

	while(wMaxByte >= 4 && wLeaveYcNum > 0)
	{
		memcpy(buf+len, &(pYcData->dwRawVal), 4);

		//if(buf[len] || buf[len+1] || buf[len+2] || buf[len+3])
			//printf("DDB:%02x %02x %02x %02x\n", buf[len], buf[len + 1], buf[len + 2], buf[len + 3]);
		len += 4;
		wMaxByte -= 4;

		pYcData ++;
		wLeaveYcNum --;
		wSendNum ++;
	}

	buf[5] = (wSendNum >> 8) & 0xff;
	buf[6] = wSendNum & 0xff;
	buf[2] = len-3;

	m_wDataPos += wSendNum;

	if( wLeaveYcNum == 0 )
	{
		buf[3] |= 0x80;
		m_wDataPos = 0;
	}

	return TRUE;
}		/* -----  end of method CDDB::YcDataSend  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  YxDataSend
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::YxDataSend ( BYTE *buf, int &len )
{/*{{{*/
	WORD wYxDataNum = m_wAllYxNum;
	DIGITALITEM *pYxData = m_pYxHeadAddr;
	WORD wMaxByte;
	WORD wLeaveYxNum;
	BYTE byTmp, i;
	WORD wSendNum = 0;

	GetCommonFrame(buf, RESPONSE_DATA);
	buf[4] = DDB_YX_DATATYPE;
	buf[7] = (m_wDataPos >> 8) & 0xff;
	buf[8] = m_wDataPos & 0xff;

	pYxData += m_wDataPos;
	wMaxByte = DDB_MAX_BUF_LEN - 11;
	wLeaveYxNum = wYxDataNum - m_wDataPos;
	len = 9;

	while(wMaxByte > 0 && wLeaveYxNum > 0)
	{
		byTmp = 0x00;

		for ( i=0; i<8; i++)
		{
			if ( ( pYxData->wStatus & 0x01 ) == 1 )
			{
				byTmp |= (0x01 << i);
			}
			pYxData++;
			wLeaveYxNum--;
			wSendNum++;
			if(wLeaveYxNum == 0 )
				break;
		}

		buf[len++] = byTmp;
		wMaxByte -- ;
	}


	buf[5] = (wSendNum >> 8) & 0xff;
	buf[6] = wSendNum & 0xff;
	buf[2] = len-3;

	m_wDataPos += wSendNum ;

	if( wLeaveYxNum == 0 )
	{
		buf[3] |= 0x80;
		m_wDataPos = 0;
	}

	return TRUE;
}		/* -----  end of method CDDB::YxDataSend  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  YmDataSend
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::YmDataSend ( BYTE *buf, int &len )
{/*{{{*/
	WORD wYmDataNum = m_wAllYmNum;
	PULSEITEM *pYmData = m_pYmHeadAddr;
	WORD wMaxByte;
	WORD wLeaveYmNum;
	WORD wSendNum = 0;

	GetCommonFrame(buf, RESPONSE_DATA);
	buf[4] = DDB_YM_DATATYPE;
	buf[7] = (m_wDataPos >> 8) & 0xff;
	buf[8] = m_wDataPos & 0xff;

	pYmData += m_wDataPos;
	wMaxByte = DDB_MAX_BUF_LEN - 11;
	wLeaveYmNum = wYmDataNum - m_wDataPos;
	len = 9;

	while(wMaxByte >= 4 && wLeaveYmNum > 0)
	{
		memcpy(buf+len, &(pYmData->dwRawVal), 4);

		len += 4;
		wMaxByte -= 4;
		pYmData ++;
		wLeaveYmNum --;
		wSendNum ++;
	}

	m_wDataPos += wSendNum;

	buf[5] = (wSendNum >> 8) & 0xff;
	buf[6] = wSendNum & 0xff;
	buf[2] = len-3;

	if ( wLeaveYmNum == 0 )
	{
		buf[3] |= 0x80;
		m_wDataPos = 0;
	}
	return TRUE;
}		/* -----  end of method CDDB::YmDataSend  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  TimeSyncSend
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::TimeSyncSend ( BYTE *buf, int &len  )
{/*{{{*/
	REALTIME CurrTime;

	GetCurrentTime( &CurrTime );
	GetCommonFrame( buf, RESPONSE_DATA );

	buf[4] = DDB_TIME_DATATYPE;
	buf[5] = ( CurrTime.wYear >> 8) & 0xff;
	buf[6] = CurrTime.wYear & 0xff;
	buf[7] = CurrTime.wMonth;
	buf[8] = CurrTime.wDay;
	buf[9] = CurrTime.wHour;
	buf[10] = CurrTime.wMinute;
	buf[11] = CurrTime.wSecond;
	buf[12] = ( CurrTime.wMilliSec >> 8) & 0xff;
	buf[13] = ( CurrTime.wMilliSec ) & 0xff;

	len = 14;
	buf[2] = len-3;

	return TRUE;
}		/* -----  end of method CDDB::TimeSyncSend  ----- *//*}}}*/

/*******************************************************************************
 * 类:CDDB
 * 函数名:LinkBusStatusSend
 * 功能描述:总线状态发送
 * 参数: BYTE *buf 发送缓冲
 * 参数: int &len　发送长度
 * 返回值:BOOL
 ******************************************************************************/
BOOL CDDB::LinkBusStatusSend( BYTE *buf, int &len )
{/*{{{*/
	BYTE byBusNum = m_pMethod->GetToTalBusNum();
	int i = 0;
	BYTE byStatus = 0xff;

	GetCommonFrame( buf, RESPONSE_DATA );

	buf[4] = DDB_LINKBUSSTATUS_DATATYPE;//数据类型　
	// 数据数量，可以在一帧传完
	buf[5] = ( ( byBusNum ) >> 8) & 0xff;
	buf[6] = ( byBusNum ) & 0xff;

	// 数据起始位置　此处只能是０
	buf[7] = (m_wDataPos >> 8) & 0xff;
	buf[8] = m_wDataPos & 0xff;

	// 判断总线
	if( MAX_LINE < byBusNum || 0 == byBusNum)
	{
		return FALSE;
	}

	len = 9;
	for (i = 0; i < byBusNum; i++)
	{
		// 每个字节有８个通讯状态　共
		BOOL bStatus = m_pMethod->GetCommState(i);
		CPublicMethod::SetDDBBusLinkStatus(i, bStatus);
		// printf("bus%d status=%d\n", i, bStatus);
		if( bStatus )
		{
			byStatus |= ( ( bStatus << ( i % 8)) );
		}
		else
		{
			byStatus &= ( ( bStatus << ( i % 8)) | ( 0xff >> (8 - ( i % 8))));
		}

		if ( 0 == ( ( i+1 )%8 ) )
		{
			buf[len++] = byStatus;
			byStatus = 0xff;
		}
	}

	if( 0 != ( byBusNum % 8 ))
	{
		buf[len++] = byStatus;
	}

	if( i != byBusNum)
	{
		return FALSE;
	}

	buf[2] = len - 3;
	buf[3] |= 0x80;
	m_wDataPos = 0;

	return TRUE;
}   /*-------- end class CDDB method LinkBusStatusSend -------- *//*}}}*/

/*******************************************************************************
 * 类:CDDB
 * 函数名:LinkStnStatusSend
 * 功能描述:装置状态发送
 * 参数: BYTE *buf 发送缓冲
 * 参数: int &len　发送长度
 * 返回值:BOOL
 ******************************************************************************/
BOOL CDDB::LinkStnStatusSend(BYTE *buf, int &len)
{/*{{{*/
	WORD wSerialNum = m_pMethod->m_pRdbObj->m_wStnSum;
	//int i = 0;
	BYTE byStatus = 0xff;
	WORD wMaxByte;
	WORD wLeaveStatusNum;
	WORD wSendNum = 0;

	GetCommonFrame( buf, RESPONSE_DATA );

	buf[4] = DDB_LINKSTNSTATUS_DATATYPE;
	// 数据起始位置　此处只能是０
	buf[7] = (m_wDataPos >> 8) & 0xff;
	buf[8] = m_wDataPos & 0xff;

	wMaxByte = DDB_MAX_BUF_LEN - 11;
	wLeaveStatusNum = ( wSerialNum - m_wDataPos );
	len = 9;

	while(wMaxByte > 1 && wLeaveStatusNum > 0)
	{
		BOOL bStatus = m_pMethod->GetDevCommState( m_wDataPos );
		// printf("stn%d status=%d\n", m_wDataPos, bStatus);
		CPublicMethod::SetDDBStnLinkStatus(m_wDataPos, bStatus);
		if( bStatus )
		{
			byStatus |= (bStatus << ( wSendNum % 8) );
		}
		else
		{
			byStatus &= ( ( bStatus << ( wSendNum % 8)) | ( 0xff >> ( 8 - (wSendNum % 8 ))));
		}

		if ( 0 == ( ( wSendNum+1 )%8 ) )
		{
			buf[len++] = byStatus;
			byStatus = 0xff;
			wMaxByte -= 1;
		}

		m_wDataPos++;
		wSendNum ++;
		wLeaveStatusNum --;
	}

	if( 0 != ( wSendNum % 8 ))
	{
		buf[len++] = byStatus;
	}

	buf[5] = (wSendNum >> 8) & 0xff;
	buf[6] = wSendNum & 0xff;
	buf[2] = len-3;

	if ( wLeaveStatusNum == 0 )
	{
		buf[3] |= 0x80;
		m_wDataPos = 0;
	}

	return TRUE;
}   /*-------- end class CDDB method LinkStnStatusSend -------- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetSendDataBuf
 * Description:  获取数据报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::GetSendDataBuf ( BYTE *buf, int &len )
{/*{{{*/
	switch ( m_byDataType )
	{
	case DDB_YX_DATATYPE:
		YxDataSend( buf, len );
		break;

	case DDB_YC_DATATYPE:
		YcDataSend( buf, len );
		break;

	case DDB_YM_DATATYPE:
		YmDataSend( buf, len );
		break;

	case DDB_LINKBUSSTATUS_DATATYPE://总线状态发送
		{
			LinkBusStatusSend( buf, len);
		}
		break;

	case DDB_LINKSTNSTATUS_DATATYPE://装置状态
		{
			LinkStnStatusSend( buf, len);
		}
		break;

	case DDB_TIME_DATATYPE:
		TimeSyncSend( buf, len );
		m_byDataType = DDB_YX_DATATYPE;
		break;

	case DDB_YK_DATATYPE:
		print( (char *)"ResponseYkData" );
		ResponseYkData( buf, len );
		break;


	default:
		break;
	}				/* -----  end switch  ----- */

	//组织发送结束后置待数据状态
	m_SendStatus = NONESTATUS;
	return TRUE;
}		/* -----  end of method CDDB::GetSendDataBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  RequestSyn
 * Description:	 请求同步报文
 *       Input:	 发送缓冲区 缓冲区长度
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::RequestSyn ( BYTE *buf, int &len )
{/*{{{*/
	//数据长度
	buf[2] = 1;
	GetCommonFrame( buf, REQUEST_SYN );

	len = 4;

	return TRUE;
}		/* -----  end of method CDDB::RequestSyn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ResponseSyn
 * Description:	 响应同步报文
 *       Input:	 发送缓冲区 缓冲区长度
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ResponseSyn ( BYTE *buf, int &len )
{/*{{{*/
	GetCommonFrame( buf, RESPONSE_SYN );

	//是否同步
	if(m_bSyn)
		buf[4] = 1;
	else
		buf[4] = 0;

	buf[2] = 2;
	len = 5;

	return TRUE;
}		/* -----  end of method CDDB::ResponseSyn  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  RequestData
 * Description:  请求数据报文j
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::RequestData ( BYTE *buf, int &len )
{/*{{{*/
	GetCommonFrame( buf, REQUEST_DATA );

	//数据类型
	buf[4] = m_byDataType;

	//收到数据数量
	buf[5] = (m_wDataPos >> 8) & 0xff;
	buf[6] = (m_wDataPos) & 0xff;

	buf[2] = 4;
	len = 7;

	return TRUE;
}		/* -----  end of method CDDB::RequestData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ResponseData
 * Description:  响应数据报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ResponseData ( BYTE *buf, int &len )
{/*{{{*/
	return GetSendDataBuf(buf, len);
}		/* -----  end of method CDDB::ResponseData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  RequestSwitch
 * Description:	 请求转换报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::RequestSwitch ( BYTE *buf, int &len )
{/*{{{*/
	GetCommonFrame( buf, REQUEST_SWITCH );
	buf[2] = 1;
	len = 4;
	//m_bSwitchSending = TRUE;
	//m_bSwitchState = TRUE;
	m_SendStatus = NONESTATUS;

	return TRUE;
}		/* -----  end of method CDDB::RequestSwitch  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ResponseSwitch
 * Description:  响应报文转换
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ResponseSwitch ( BYTE *buf, int &len )
{/*{{{*/
	GetCommonFrame( buf, RESPONSE_SWITCH );
	buf[2] = 1;
	len = 4;
	m_SendStatus = NONESTATUS;

	return TRUE;
}		/* -----  end of method CDDB::ResponseSwitch  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  RequestYkData
 * Description:  请求yk数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::RequestYkData ( BYTE *buf, int &len )
{/*{{{*/
	//请求数据
	GetCommonFrame( buf, m_SendStatus );

	//数据类型
	buf[4] = m_byDataType;

	//yk状态
	buf[5] = m_YkStatus;

	if( STATUS_MASTER == m_byLocalStatus )
	{
		//装置总线号
		buf[6] = m_bySaveSrcBusNo;

		//装置地址
		buf[7] = HIBYTE( m_wSaveSrcDevAddr );
		buf[8] = LOBYTE(m_wSaveSrcDevAddr );
	}
	else
	{
		//装置总线号
		buf[6] = m_SaveDestYkData.byDestBusNo;

		//装置地址
		buf[7] = HIBYTE(m_SaveDestYkData.wDestAddr);
		buf[8] = LOBYTE(m_SaveDestYkData.wDestAddr);
	}

	//装置点号
	buf[9] = HIBYTE(m_SaveDestYkData.wPnt);
	buf[10] = LOBYTE(m_SaveDestYkData.wPnt);

	//遥控值
	buf[11] = m_SaveDestYkData.byVal;

	//数据长度
	buf[2] = 9;

	len = 12;

	m_bIsYking = FALSE;					//by cyz!	添加该语句解决了备机端m_bIsYking始终为true而导致备机无法请求遥测遥信遥脉数据的问题!
	return TRUE;
}		/* -----  end of method CDDB::RequestYkData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ResponseYkData
 * Description:  回复yk报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ResponseYkData ( BYTE *buf, int &len  )
{/*{{{*/
	GetCommonFrame(buf, m_SendStatus);
	//数据类型
	buf[4] = m_byDataType;

	//yk状态
	buf[5] = m_YkStatus;

	//数据长度
	buf[2] = 3;
	len = 6;

	//发送后重新判定状态
	switch ( m_YkStatus )
	{
	case DDB_YK_SEL_CONFIRM:
		m_YkStatus = DDB_YK_SEL_RTN;
		break;

	case DDB_YK_EXE_CONFIRM:
		m_YkStatus = DDB_YK_EXE_RTN;
		break;

	case DDB_YK_SEL:
		m_YkStatus = DDB_YK_EXE;
		break;

	default:
		break;
	}				/* -----  end switch  ----- */

	m_byDataType = m_bySaveDataType;
	return TRUE;
}		/* -----  end of method CDDB::ResponseYkData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ErgodicDevState
 * Description:  遍历底层一条总线上是否全部装置状态都未连通
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ErgodicDevState()
{/*{{{*/
	BYTE eBusNo,eDevNo;
	BYTE eBusNum = m_pMethod->GetToTalBusNum();
	for( eBusNo=0; eBusNo<eBusNum; eBusNo++ )
	{
		BYTE ProtocolType = m_pMethod->GetBusLineProtocolType( eBusNo );
		if( ProtocolType == PROTOCO_GATHER )
		{
			BYTE eDevNum = m_pMethod->GetDevNum( eBusNo );
			BYTE eAbnormalDevnum = 0;
			for( eDevNo=0; eDevNo<eDevNum; eDevNo++ )
			{
				WORD eDevaddr = m_pMethod->GetAddrByLineNoAndModuleNo ( eBusNo , eDevNo );

				if( FALSE == m_pMethod->GetDevCommState( eBusNo, eDevaddr ) )
				{
					break;
				}
				else
				{
					eAbnormalDevnum++;
				}
			}

			if( eAbnormalDevnum == eDevNum )
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  AddSendCrc
 * Description:  添加发送报文校验
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::AddSendCrc ( BYTE *buf, int &len )
{/*{{{*/
	WORD wCrc;
	if ( buf[2] != len -3 )
		return FALSE;

	wCrc = GetCrc(buf+3, buf[2]);
	buf[len++] = ( wCrc >> 8 ) & 0xff;
	buf[len++] = (wCrc) &0xff;

	return TRUE;
}		/* -----  end of method CDDB::AddSendCrc  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetSendTypeBuf
 * Description:	 获取发送类型数据
 *       Input:	 发送缓冲区 缓冲区长度
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::GetSendTypeBuf ( BYTE *buf, int &len )
{/*{{{*/
	BOOL bRtn = FALSE;
	memset( buf, 0, len );
	//printf ( "m_bSwitchState=%d m_byLocalStatus=%d\n", m_bSwitchState, m_byLocalStatus );
	if( m_bSwitchState == TRUE && m_byLocalStatus == STATUS_MASTER )
	{
		m_SendStatus = REQUEST_SWITCH;
		m_bSwitchState = FALSE;
	}

	//printf ( "m_SendStatus=%d m_byDataType=%d\n", m_SendStatus, m_byDataType );
	switch ( m_SendStatus )
	{
	case REQUEST_SYN:
		print ( (char *)"DDB SEND REQUEST_SYN\n" );
		bRtn = RequestSyn(buf, len);
		break;

	case RESPONSE_SYN:
		print ( (char *)"DDB SEND RESPONSE_SYN\n" );
		bRtn = ResponseSyn(buf, len);
		break;

	case REQUEST_DATA:
		print ( (char *)"DDB SEND REQUEST_DATA" );
		if( m_bIsYking )
		{
			if( DDB_YK_DATATYPE != m_byDataType )
				m_bySaveDataType = m_byDataType;
			m_byDataType = DDB_YK_DATATYPE;
			print( (char *)"RequestYkData\n" );
			bRtn = RequestYkData( buf, len );
		}
		else if( DDB_YK_DATATYPE == m_byDataType )
		{
			bRtn = ResponseYkData( buf, len );
		}
		else
		{
			bRtn = RequestData(buf,len);
		}
		break;

	case RESPONSE_DATA:
		print ( (char *)"DDB SEND RESPONSE_DATA" );
		if( m_bIsYking )
		{
			if( DDB_YK_DATATYPE != m_byDataType )
				m_bySaveDataType = m_byDataType;
			m_byDataType = DDB_YK_DATATYPE;
			print( (char *)"ResponseYkData\n" );
			bRtn = RequestYkData( buf, len );
		}
		else
		{
			bRtn = ResponseData(buf,len);
		}
		break;

	case REQUEST_SWITCH:
		print ( (char *)"DDB SEND REQUEST_SWITCH\n" );
		bRtn = RequestSwitch(buf,len);
		break;

	case RESPONSE_SWITCH:
		print ( (char *)"DDB SEND RESPONSE_SWITCH\n" );
		bRtn = ResponseSwitch(buf, len);
		break;

	case NONESTATUS:
		bRtn = TRUE;
		len = 0;
		break;

	default:
		char szBuf[256];
		len = 0;
		sprintf ( szBuf, "DDB:GetProtocolBuf can't find m_SendStatus = %d\n", m_SendStatus );
		print( szBuf );
		break;
	}				/* -----  end switch  ----- */

	if( bRtn )
	{
		if( len > 0 )
			AddSendCrc(buf, len);
	}
	else
	{
		if( m_SendStatus != NONESTATUS )
		{
			char szBuf[256];
			sprintf (szBuf, "get message error!!!len=%d %.2x %.2x %.2x %.2x %.2x %.2x", len,buf[0], buf[1], buf[2],buf[3],buf[4],buf[5] );
			print( szBuf );
		}
		else
		{
			print ( (char *)"m_SendStatus = NONESTATUS" );
		}
	}

	if( m_SendStatus == NONESTATUS )
	{
		bRtn = TRUE;
	}

	return bRtn;
}		/* -----  end of method CDDB::GetSendTypeBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessBusMsg
 * Description:  处理消息报文
 *       Input:  pBusMsg:消息指针
 *				 buf：发送缓冲
 *				 len：长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessBusMsg ( PBUSMSG pBusMsg, BYTE *buf, int &len )
{/*{{{*/
	switch ( pBusMsg->byMsgType )
	{
	case YK_PROTO:	//处理遥控
		{
			print( (char *)"Recv YkMsg\n" );
			ProcessYK( pBusMsg ) ;
		}
		break;

	default:
		printf ( "DDB:ProcessBusMsg can't find msgtype = %d\n", pBusMsg->byMsgType );
		return FALSE;
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CDDB::ProcessBusMsg  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessBusMsg
 * Description:  处理遥控消息
 *       Input:  消息指针
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessYK( PBUSMSG pBusMsg )
{/*{{{*/
	if( !JudgeYkMsg( pBusMsg ) )
	{
		print( (char *)"CDDB::JudgeYkMsg error\n" );
		return FALSE;
	}

	SaveYkMsgInfo( pBusMsg );
	SetYkDataStatus( pBusMsg );
	print( (char *)"CDDB::ProcwssYk Ok\n" );

	return FALSE ;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  JudgeYkMsg
 * Description:  判断YK消息是否准确
 *       Input:  消息指针
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::JudgeYkMsg ( PBUSMSG pBusMsg )
{/*{{{*/
	DWORD dwYkType = pBusMsg->dwDataType ;

	if( STATUS_MASTER == m_byLocalStatus  )  //主站只接收Rtn消息
	{
		if( YK_SEL_RTN != dwYkType
				&& YK_EXCT_RTN != dwYkType
				&& YK_CANCEL_RTN != dwYkType
				&& YK_ERROR != dwYkType)
		{
			printf ( (char *)"CDDB:JudgeYkMsg ykmsg yktype error\n" );
			return FALSE;
		}

		//没有yk选择返回确认的不能进行yk返回
		if( YK_SEL_RTN == dwYkType)
		{
			if( DDB_YK_SEL_RTN != m_YkStatus )
			{
				return FALSE;
			}
		}

		//没有yk执行返回确认的不能进行yk返回
		if( YK_EXCT_RTN == dwYkType )
		{
			if( DDB_YK_EXE_RTN != m_YkStatus )
			{
				return FALSE;
			}
		}
		return TRUE;
	}
	else if( STATUS_SLAVE == m_byLocalStatus ) //主站只接收下发消息
	{
		if( YK_SEL != dwYkType
				&& YK_EXCT != dwYkType
				&& YK_CANCEL != dwYkType)
		{
			printf ( (char *)"CDDB:JudgeYkMsg ykmsg yktype error\n" );
			return FALSE;
		}

		//没有经过yk选择返回确认的不能进行yk执行
		// if( YK_EXCT == dwYkType )
		// {
		// if( DDB_YK_SEL_RTN_CONFIRM != m_YkStatus  )
		// {
		// return FALSE;
		// }
		// }
		return TRUE;
	}

	printf ( "DDB::JudgeYkMsg not DDB\n" );

	return FALSE;
}		/* -----  end of method CDDB::JudgeYkMsg  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  SaveYkMsgInfo
 * Description:  保存yk消息信息
 *       Input:  消息指针
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::SaveYkMsgInfo ( PBUSMSG pBusMsg )
{/*{{{*/
	PDDBYK_DATA pYkData = ( PDDBYK_DATA )pBusMsg->pData ;
	//保存转发协议的总线号和地址号
	m_bySaveSrcBusNo = pBusMsg->SrcInfo.byBusNo;
	m_wSaveSrcDevAddr = pBusMsg->SrcInfo.wDevNo;

	if( STATUS_MASTER == m_byLocalStatus )
	{
		print ( (char *)"SaveYkMsgInfo master not save\n" );
		return ;
	}

	// CPublicMethod::SetDDBDevBusAndAddr( m_bySaveSrcBusNo, m_wSaveSrcDevAddr );
	//保存装置消息
	memcpy( &m_SaveDestYkData, pYkData, sizeof( DDBYK_DATA ) );
}		/* -----  end of method CDDB::SaveYkMsgInfo  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  SetYkDataStatus
 * Description:  设置Yk数据状态
 *       Input:  消息指针
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::SetYkDataStatus ( PBUSMSG pBusMsg )
{/*{{{*/
	//保存现有的发送状态，方便发完yk数据后切回
	// m_bySaveDataType = m_byDataType;
	// m_byDataType = DDB_YK_DATATYPE;
	m_bIsYking = TRUE;

	DWORD dwYkType = pBusMsg->dwDataType ;

	switch ( dwYkType )
	{
	case YK_SEL:
		m_YkStatus = DDB_YK_SEL;
		break;

	case YK_EXCT:
		m_YkStatus = DDB_YK_EXE;
		break;

	case YK_CANCEL:
		m_YkStatus = DDB_YK_CANCEL;
		break;

	case YK_SEL_RTN:
		m_YkStatus = DDB_YK_SEL_RTN;
		break;

	case YK_EXCT_RTN:
		m_YkStatus = DDB_YK_EXE_RTN;
		break;

	case YK_CANCEL_RTN:
		m_YkStatus = DDB_YK_CANCEL_RTN;
		break;

	case YK_ERROR:
		m_YkStatus = DDB_YK_ERROR;
		break;

	default:
		print( (char *)"DDB:SetYkDataStatus none dwYkType" );
		m_YkStatus = DDB_YK_NONE_STATUS;
		break;
	}				/* -----  end switch  ----- */

}		/* -----  end of method CDDB::SetYkDataStatus  ----- *//*}}}*/

/* **********************初始化协议 ***************************************/
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  CDDB
 * Description:  读取配置信息
 *       Input:
 *		Return:  BOOL
 *		配置项：
 *		身份：machineid=A（或B）
 *		本地地址：localip=192.168.1.200（此处也可写为 localNetcard=eth0 那个好？//无必要 可以通过传进参数判断）
 *		远程地址：localip=192.168.1.211
 *		端口    ：1111（主备设备必须是一样的端口号 方便通讯）
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ReadCfgInfo (  )
{/*{{{*/
	//BYTE bRtn;
	char szMachineBuf[256];
	char szFilename[256] = "";

	sprintf( szFilename, "%s%s", DDBPREFIXFILENAME, m_sTemplatePath );
	CProfile profile( szFilename ) ;
	if( !profile.IsValid() )
	{
		printf( "CDDB:ReadCfgInfo Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DDB" ;
	char sKey[ 20 ][ 100 ]={ "machineid" , "localip" , "renoteip" , "port",\
		"DelayedSwitchMinute" , "DelayedSynSecond"  } ;

	//bRtn = profile.GetProfileString( sSect , sKey[ 0 ] , "NULL" , szMachineBuf , sizeof( szMachineBuf ) ) ;
	profile.GetProfileString( sSect , sKey[ 0 ] , (char *)"NULL" , szMachineBuf , sizeof( szMachineBuf ) ) ;
	if ( szMachineBuf[0] == 'A' )
	{
		m_byMachineId = IDENTITY_A;
	}
	else if( szMachineBuf[0] == 'B' )
	{
		m_byMachineId = IDENTITY_B;
	}
	else
	{
		m_byMachineId = IDENTITY_SINGLE; ;
		printf ( "CDDB:ReadCfgInfo Read m_byMachineId err !!! \n" );
		return FALSE;
	}

	m_iDelayedSwitchMinute = profile.GetProfileInt( sSect , sKey[ 4 ] , 0 ) ;
	if( m_iDelayedSwitchMinute < 10 )
	{
		m_iDelayedSwitchMinute = 10;
	}
	else if( m_iDelayedSwitchMinute > 120 )
	{
		m_iDelayedSwitchMinute = 120 ;
	}

	m_iDelayedSynSecond = profile.GetProfileInt( sSect , sKey[ 5 ] , 0 ) ;
	if( m_iDelayedSynSecond < 10 )
	{
		m_iDelayedSynSecond = 10;
	}
	else if( m_iDelayedSynSecond > 120 )
	{
		m_iDelayedSynSecond = 120 ;
	}

	printf ( "DelayedSynSecond=%ds DelayedSwitchMinute=%dm\n", m_iDelayedSynSecond, m_iDelayedSwitchMinute );

	// bRtn = profile.GetProfileString( sSect , sKey[ 1 ] , "NULL"  , m_szLocalIp , sizeof( m_szLocalIp ) ) ;
	// if( bRtn == 4 )
	// {
	// printf ( "CDDB:ReadCfgInfo Read m_szLocalIp err !!! \n" );
	// return FALSE;
	// }
	// bRtn = profile.GetProfileString( sSect , sKey[ 2 ] , "NULL"  , m_szRemoteIp , sizeof( m_szRemoteIp ) ) ;
	// if( bRtn == 4 )
	// {
	// printf ( "CDDB:ReadCfgInfo Read m_szRemoteIp err !!! \n" );
	// return FALSE;
	// }
	// m_dwPort = profile.GetProfileInt( sSect , sKey[ 3 ] , 0 ) ;
	// if( m_dwPort <= 1000 )
	// {
	// printf ( "CDDB:ReadCfgInfo Read port err !!! \n" );
	// return FALSE;
	// }

	printf( "localid=%d\n", m_byMachineId );
	return TRUE;
}		/* -----  end of method CDDB::ReadCfgInfo  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  InitProtocolStatus
 * Description:  初始化协议基本状态
 *       Input:
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::InitProtocolStatus ( )
{/*{{{*/
	if ( m_byMachineId == IDENTITY_SINGLE )
		return FALSE;

	m_bSyn = TRUE;				//本地同步
	m_bRemoteSyn = TRUE;		//远程同步
	// m_byLocalStatus = STATUS_SLAVE;//初始化设为从
	// m_byRemoteStatus = STATUS_MASTER;//远程初始化设为主
	m_byDataType = DDB_YX_DATATYPE;//初始化设置成YX
	m_bLinkStatus = FALSE;		//链接状态为断
	m_SendStatus = REQUEST_SYN;	//设为复位通信单元
	m_dwLinkTimeOut = 0;		//链接超时为0
	m_byRecvErrorCount = 0;     //接收错误计数0
	m_bIsReSend = FALSE;		//重发标识位0
	m_byResendCount = 0;		//重发次数清零
	m_bIsSending = FALSE;		//发送后置1 接收后值0
	m_bSwitchState = FALSE;     //是否要切换

	m_wDataPos = 0;				//数据起始位设为0

	//清空重发缓存区
	memset(m_byReSendBuf, 0 , DDB_MAX_BUF_LEN);
	m_wReSendLen = 0;


	//设置 同步状态为未同步
	// CPublicMethod::SetDDBSyncState( m_bSyn );

			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");


	return TRUE;
}		/* -----  end of method CDDB::InitProtocolStatus  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  InitProtocolData
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::InitProtocolData ( BOOL bStatus )
{/*{{{*/
	//获取数据数量及首地址
	m_wAllYcNum = GetDataNum( 2 );
	m_wAllYxNum = GetDataNum( 1 );
	m_wAllYmNum = GetDataNum( 3 );
	// printf ( "ycnum=%d yxnum=%d ymnum=%d\n", m_wAllYcNum, m_wAllYxNum, m_wAllYmNum );

	m_pYcHeadAddr = ( ANALOGITEM * )GetDataHeadAddr( 2 );
	m_pYxHeadAddr = ( DIGITALITEM * )GetDataHeadAddr( 1 );
	m_pYmHeadAddr = ( PULSEITEM * )GetDataHeadAddr( 3 );

	SwitchStatus( bStatus );


	// printf ( "pyc=%p yxnum=%p ymnum=%p\n", m_pYcHeadAddr, m_pYxHeadAddr, m_pYmHeadAddr );
}		/* -----  end of method CDDB::InitProtocolData  ----- *//*}}}*/

/* ********************** 主干或虚函数部分 ***************************************/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  TimerProc
 * Description:  时间处理函数 主要处理一些超时 总召唤等与时间有关的
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDDB::TimerProc (  )
{/*{{{*/
	// if ( !m_bLinkStatus )
	// return;
	if ( m_bSyn )
	{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		JudgeStatus(  );
		// if ( m_byMachineId == IDENTITY_B && m_byLocalStatus == STATUS_MASTER && m_byRemoteStatus == STATUS_MASTER )
		// {
		// print ((char *)"\nDDB m_bSyn=TRUE m_id=B m_byLocalStatus m_byRemoteStatus = STATUS_MASTER\n" );
		// m_byLocalStatus = STATUS_SLAVE;
		// }
		// if ( m_byMachineId == IDENTITY_A && m_byLocalStatus == STATUS_SLAVE && m_byRemoteStatus == STATUS_SLAVE)
		// {
		// print ( (char *)"\nDDB m_bSyn=TRUE m_id=A m_byLocalStatus m_byRemoteStatus = STATUS_SLAVE\n" );
		// m_byLocalStatus = STATUS_MASTER;
		// }

		// CPublicMethod::SetDDBSyncState( m_byLocalStatus );
	}

	//设置通讯通
	// int Interval = 250;

	//通讯超时时间
	// m_dwLinkTimeOut += Interval;
	// if(m_dwLinkTimeOut >= DDB_LINK_TIMEOUT)
	// {
	// m_dwLinkTimeOut = 0;
	// if( m_bLinkStatus == TRUE )
	// {
	// if ( m_byLocalStatus == STATUS_MASTER )
	// {
	// m_SendStatus = NONESTATUS;
	// }
	// else if( m_byLocalStatus == STATUS_SLAVE )
	// {
	// SwitchStatus( 1 );
	// }
	// }
	// }


	//collect state

	if( m_byQuickSwitchNum < 6 )
	{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		if( m_byLocalStatus == STATUS_MASTER )//&& m_bSwitchSending == FALSE )
		{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
			if( m_bTimeProcCount == 255)
			{
				m_bTimeProcCount = 11;
			}
			else
			{
				m_bTimeProcCount++;
			}

			if( m_bTimeProcCount >= 10)
			{
				if( FALSE == ErgodicDevState() )
				{
					m_bSwitchState = TRUE;
					// m_bIsReSend = FALSE;
					//m_byResendCount = 0;
					// m_bIsSending = FALSE;
					return ;
				}
			}
		}
	}
	else
	{
		time_t TempNowTime,TempTimeDifference;
		time(&TempNowTime);
		TempTimeDifference = abs( TempNowTime - m_tmLastSwitchTime );
		if( TempTimeDifference > m_iDelayedSwitchMinute*60 )
		{
			m_byQuickSwitchNum = 0;
		}
	}

	//接收错误次数
	if( m_byRecvErrorCount > DDB_MAX_ERROR_COUNT  )
	{
		char szBuf[256];
		sprintf ( szBuf, "DDB Recv err > %d reversion link\n", DDB_MAX_ERROR_COUNT );
		print( szBuf );
		InitProtocolStatus(  );

		if ( m_byLocalStatus == STATUS_MASTER )
		{
			print ( (char *)"This is Master, wait for data! \n" );
			m_SendStatus = NONESTATUS;
		}
		else
		{
			print ( (char *)"This is slave, ask for data again! \n" );
			m_SendStatus = REQUEST_SYN;
		}
	}

	//重发计数
	if( m_byResendCount >= DDB_MAX_RESEND_COUNT )
	{
		char szBuf[256];
		sprintf (szBuf, "DDB ReSend > %d", DDB_MAX_RESEND_COUNT );
		print( szBuf );

			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		InitProtocolStatus(  );

		if ( m_byLocalStatus == STATUS_MASTER )
		{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
		}
		else
		{
			//printf("func:%s line:%d time:%s stat:%s\n", __func__, __LINE__, _time(), m_byLocalStatus ? "SLAVE" : "MASTER");
			// //未初始化时A主B从
			// if( !m_bSyn )
			// {
			// if( m_byMachineId == IDENTITY_A )
			// {
			// SwitchStatus( 1 );
			// m_SendStatus = NONESTATUS;
			// }
			// else
			// {
			// m_SendStatus = REQUEST_SYN;
			// }
			// }
			// else
			// {
			// SwitchStatus( 1 );
			// m_SendStatus = NONESTATUS;
			// }
			SwitchStatus( 1 );
			// m_SendStatus = NONESTATUS;
		}

	}

}		/* -----  end of method CDDB::TimerProc  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  ProcessProtocolBuf
 * Description:	 处理收到的数据缓存
 *       Input:  接收到的数据缓存 缓存长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::ProcessProtocolBuf ( BYTE *buf, int len )
{/*{{{*/
	//printf("\n******************************DDB******************************\n");
	//for(int i = 0; i < len; i++)
		//printf("%02X ", buf[i]);
	//printf("\n******************************END******************************\n");
	int pos=0;
	BOOL bRtn = FALSE;
	if( !WhetherBufValue( buf, len, pos ) )
	{
		print ( (char *)"CDDB:ProcessProtocolBuf buf Recv err!!!\n" );
		m_byRecvErrorCount ++;
		m_bIsReSend = TRUE;
		usleep(100 * 1000);
		return FALSE;
	}

	bRtn = ProcessRecvTypeBuf( &buf[pos], len );
	if( !bRtn )
	{
		print ( (char *)"DDB ProcessRecv err\n" );
		m_byRecvErrorCount ++;
		m_bIsReSend = TRUE;
	}
	else
	{
		m_byRecvErrorCount = 0;
		// m_bLinkStatus = TRUE;
		// m_dwLinkTimeOut = 0;
		m_bIsReSend = FALSE;
		m_byResendCount = 0;
		m_bIsSending = FALSE;
	}

	if(buf[2] + 5 != len)							//+2 by cyz!考虑到粘包的简易处理!
		ProcessProtocolBuf(buf + buf[2] + 5, len - buf[2] - 5);
	return bRtn;
}		/* -----  end of method CDDB::ProcessProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  GetProtocolBuf
 * Description:  获取协议数据缓存
 *       Input:  缓存区 缓存区数据长度 总线消息
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{/*{{{*/
	BOOL bRtn = TRUE;
	// printf ( "m_SendStatus=%d m_byDataType=%d\n", m_SendStatus, m_byDataType );
	// if ( ( m_bIsReSend || m_bIsSending ) && ( m_SendStatus == REQUEST_SYN ))
	if( m_bRecvResponseSwitch == TRUE )
	{
		sleep(m_iDelayedSynSecond);
	}
	m_bRecvResponseSwitch = FALSE;

	if ( ( m_bIsReSend || m_bIsSending ) && (m_wReSendLen > 0) )
	{
		if( m_SendStatus == REQUEST_SYN || m_SendStatus == RESPONSE_SYN )
		{
			usleep( 200 * 1000 );
		}
		else
		{
			usleep( 20 * 1000 );
		}
		print ( (char *)"DDB Get Resend Buf\n" );
		len = m_wReSendLen;
		memcpy( buf, m_byReSendBuf, len );
		m_byResendCount ++;
	}
	// else if( pBusMsg != NULL && m_bLinkStatus)
	else if( pBusMsg != NULL )
	{
		if( !ProcessBusMsg( pBusMsg, buf, len ) )
			return FALSE;
	}
	else
	{
		bRtn =  GetSendTypeBuf( buf, len );

		if( bRtn )
		{
			m_byResendCount = 0;

			if ( len > DDB_MAX_BUF_LEN )
			{
				char szBuf[256];
				sprintf (szBuf, "len=%d is too big for %d", len, DDB_MAX_BUF_LEN );
				print( szBuf );
				return FALSE;
			}

			m_wReSendLen = len;
			memcpy( m_byReSendBuf, buf, m_wReSendLen );
			m_bIsSending = TRUE;
		}
	}

	//printf("\n------------------------SND----------------------------\n");
	//for(int i = 0; i < len; i++)
		//printf("%02X ", buf[i]);
	//printf("\n------------------------END-----------------------------\n\n");
	return bRtn;
}		/* -----  end of method CDDB::GetProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDDB
 *      Method:  Init
 * Description:	 初始化协议数据
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDDB::Init ( BYTE byLineNo )
{/*{{{*/
	if ( CPublicMethod::IsHaveDDB() )
		return FALSE;

	//读配置文件
	if( !ReadCfgInfo() )
	{
		// printf ( "CDDB:ReadCfgInfo Err!!!\n" );
		return FALSE;
	}

	//初始化状态
	if( !InitProtocolStatus(  ) )
	{
		printf ( "CDDB:InitProtocolStatus Err\n" );
		return FALSE;
	}

	InitProtocolData( 0 );

	printf ( "DDB Init Success\n" );
	//更新协议
	CPublicMethod::SetDDBProtocol();
	CPublicMethod::SetDDBBusAndAddr( m_byLineNo , m_wDevAddr ) ;
	return TRUE;
}		/* -----  end of method CDDB::Init  ----- *//*}}}*/
