/*
 * =====================================================================================
 *
 *       Filename:  CDTWithTime.cpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2015年06月09日 18时28分18秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp
 *   Organization:
 *
 *		  history:	Time								Author			version			Description
 *					2015年06月30日 09时10分34秒			mengqp			1.0				created
 * =====================================================================================
 */

#include "CDTWithTime.h"
#include <stdio.h>
#include <dirent.h>
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  CDTWithTime
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDTWithTime::CDTWithTime ()
{
	memset( m_fYcBuf, 0, sizeof( m_fYcBuf ) );
	memset( m_byYxBuf, 0, sizeof( m_byYxBuf ) );
	memset( m_dwYmBuf, 0, sizeof( m_dwYmBuf ) );

	//默认是15分钟
	m_wAllDataInterval = 15;
	m_LocalHeartbeatTime = 60 * 1000;

	//三遥的起始位置
	m_dwFileYcBeginPos = 0;
	m_dwFileYxBeginPos = 0;
	m_dwFileYmBeginPos = 0;

	//文件相关
	memset( m_szPathDir, 0, sizeof( m_szPathDir ) );
	memset( m_szRecentFileName , 0, sizeof( m_szRecentFileName ) );

	//初始化基本状态参数
	InitProtocolState(  );
	printf ( "CDTWithTime construtor\n" );


}  /* -----  end of method CDTWithTime::CDTWithTime  (constructor)  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ~CDTWithTime
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDTWithTime::~CDTWithTime ()
{
	printf ( "CDTWithTime destrutor\n" );
	/* 析构变量 */
}  /* -----  end of method CDTWithTime::~CDTWithTime  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  TimerProc
 * Description:  时间处理
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::TimerProc ( void )
{
	/* 	更新变化数据 */
	ReadChangData();

	/* 时间协议处理 */
	TimeToProtocol(  );

	/* 	协议超时或超次数处理 */
	ProtocolErrorProc(  );
}		/* -----  end of method CDTWithTime::TimerProc  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ProcessProtocolBuf
 * Description:  处理协议报文
 *       Input:	 pBuf缓冲区指针
 *				 len:长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::ProcessProtocolBuf ( BYTE *pBuf, int len )
{
	int pos =0;
	/* 判断报文合理性  并找出一帧合理报文 */
	if( !WhetherBufValid( pBuf, len, pos ) )
	{
		print( ( char * )"CDTWithTime can't find right recv buf" );
		SetState( DATATRANS_RESEND_STATE );
		return FALSE;
	}

	/* 处理报文 */
	if( !ProcessRecvBuf( &pBuf[pos], len ) )
	{
		return FALSE;
	}

	/* 接收状态切换 */
	SetRecvParam(  );
	return TRUE;
}		/* -----  end of method CDTWithTime::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetProtocolBuf
 * Description:  获取协议报文
 *       Input:  buf:组织报文缓冲区
 *				 len:缓冲区长度
 *				 pBusMsg:消息指针  在此协议中无用
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetProtocolBuf ( BYTE *buf,
		int &len,
		PBUSMSG pBusMsg)
{
	BOOL bRtn = FALSE;
/* 	判断输入参数有效性 */
	//判断buf
	if( NULL == buf  )
	{
		print( ( char * )" CDTWithTime GetProtocolBuf buf = NULL" );
		return FALSE;
	}
	//不判断pBusMsg

/* 	获取发送报文 */
	bRtn = GetSendBuf( buf, len );

/* 	发送状态参数切换 */
	SetSendParam( bRtn );

	return bRtn;
}		/* -----  end of method CDTWithTime::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  byLineNo:总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::Init ( BYTE byLineNo )
{
	//读配置文件
	if( !ReadCfgInfo(  ) )
	{
		return FALSE;
	}

	//初始化数据
	InitProtocol(  );

	sprintf ( m_szPathDir, "%sBus%.2dlog/", DATATRANSSPREFIXFILENAME, m_byLineNo + 1);
	m_DirFile.CreateDir( m_szPathDir );

	CloseLink(  );

	printf ( "CDTWithTime Init Ok\n" );
	return TRUE;
}		/* -----  end of method CDTWithTime::Init  ----- */


/* #####   time 时间部分   ################################################### */
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  TimeToProtocol
 * Description:  时间协议处理
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::TimeToProtocol ( void )
{
	if( TimeToWriteFile(  ) )
	{
		SetState( DATATRANS_WRITEFILE_STATE );
		print( (char *)"CDTWithTime TimeToWriteFile" );
		if( SaveDataToFile( &m_SaveFileTime ) )
		{
		}
		UnsetState( DATATRANS_WRITEFILE_STATE );

	}
	//是否到时间发送全数据 若数据没有发完 不经过此处
	if( TimeToAll(  ) )
	{
		print( (char *)"CDTWithTime timetoall" );
		DWORD dwAll = DATATRANS_YC_STATE | DATATRANS_YX_STATE | DATATRANS_YM_STATE ;
		SetState( dwAll );
		OpenLink(  );
	}

	// if( TimeToHeartbeat(  ) )
	// {
		// print( (char *)"CDTWithTime timetoheart" );
		// SetState( DATATRANS_HEARTBEAT_STATE );
		// OpenLink(  );
	// }

}		/* -----  end of method CDTWithTime::TimeToProtocol  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  TimeToAll
 * Description:  是否到时间发送全部数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::TimeToAll ( void )
{
	if( IsHaveState( DATATRANS_WRITEFILE_STATE ) )
	{
		return FALSE;
	}

	if( 0 != strlen( m_szReadFile ) )
	{
		return FALSE;
	}

	char *pReadFile = m_DirFile.GetDirFile( m_szPathDir ) ;
	if( NULL != pReadFile )
	{
		sprintf( m_szReadFile, "%s%s", m_szPathDir, pReadFile );
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::TimeToAll  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  TimeToWriteFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::TimeToWriteFile ( void  )
{
	time_t nowTime;
	struct tm *pRecordTime;

	nowTime = time( NULL );
	pRecordTime = localtime( &nowTime );
	struct tm *pTm = pRecordTime;

	DeleteOldestFile(  );
	//间隔大于1小时
	if( m_wAllDataInterval >= 60 )
	{
		DWORD tmp = m_wAllDataInterval % 60;
		DWORD tmpTime = 60 / tmp;

		if( 0 == ( pRecordTime->tm_min % tmp ) )
		{
			if( m_LocalAddHour >= m_LocalSumTime )
			{
				m_LocalAddHour =0;
				m_LocalAddTime = 0;
				memcpy( &m_SaveFileTime , pRecordTime, sizeof( struct tm ));

				char szTmpTimeBuf[64] ;
				sprintf( szTmpTimeBuf, "%.4d%.2d%.2d%.2d%.2d.log",
						pTm->tm_year + 1900,
						pTm->tm_mon,
						pTm->tm_mday,
						pTm->tm_hour,
						pTm->tm_min);

				if( 0 != ( strcmp( szTmpTimeBuf, m_szRecentFileName ) ) )
				{
					return TRUE;
				}


			}

			m_LocalAddTime ++;
			if( m_LocalAddTime >= tmpTime )
			{
				m_LocalAddHour ++;
			}
		}
	}
	//时间间隔小于1小时 切分为 2 3 5 10 15 20 30
	else
	{
		if( 0 == ( pRecordTime->tm_min % m_wAllDataInterval ) )
		{
			memcpy( &m_SaveFileTime , pRecordTime, sizeof( struct tm ));
			char szTmpTimeBuf[64] ;
			sprintf( szTmpTimeBuf, "%.4d%.2d%.2d%.2d%.2d.log",
					pTm->tm_year + 1900,
					pTm->tm_mon,
					pTm->tm_mday,
					pTm->tm_hour,
					pTm->tm_min);

			if( 0 != ( strcmp( szTmpTimeBuf, m_szRecentFileName ) ) )
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::TimeToWrite  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  TimeToHeartbeat
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::TimeToHeartbeat ( void )
{
	m_LocalHeartbeatAddTime += 200;
	if( m_LocalHeartbeatAddTime  >= m_LocalHeartbeatTime )
	{
		//如果还有其它操作
		if( m_ProtocolState )
		{
			return FALSE;
		}

		//重置心跳
		m_LocalHeartbeatAddTime = 0;
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::TimeToHeartbeat  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  SaveDataToFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::SaveDataToFile ( struct tm *pTm )
{
	char szTmpTimeBuf[64] ;
	sprintf( szTmpTimeBuf, "%.4d%.2d%.2d%.2d%.2d.log",
			pTm->tm_year + 1900,
			pTm->tm_mon,
			pTm->tm_mday,
			pTm->tm_hour,
			pTm->tm_min);
	if( 0 == ( strcmp( szTmpTimeBuf, m_szRecentFileName ) ) )
	{
		return FALSE;
	}


	//将数据写入文件
	if( WriteDataToFile(szTmpTimeBuf) )
	{
		print( (char *)"write ok"  );
		strcpy( m_szRecentFileName, szTmpTimeBuf );
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::SaveDataToFile  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  DeleteOldestFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::DeleteOldestFile ( void )
{
	DWORD dwDirSize = m_DirFile.GetDirSize( (char *)DATATRANSSPREFIXFILENAME );
	if( dwDirSize >= CDTWITHTIME_MAX_SAVE_SIZE)
	{
		m_DirFile.DeleteOldestFile( m_szPathDir );
	}
}		/* -----  end of method CDTWithTime::DeleteOldestFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  WriteDataToFile
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::WriteDataToFile ( char *pszFileName )
{
	char szFile[128];
	BYTE szBuf[1024];
	int len;

	sprintf( szFile, "%s%s", m_szPathDir, pszFileName );
	//是否有该文件
	if( m_DirFile.IsFile ( szFile ) )
	{
		return FALSE;
	}

	DWORD dwState = DATATRANS_WRITEFILE_YC_STATE
		| DATATRANS_WRITEFILE_YX_STATE
		| DATATRANS_WRITEFILE_YM_STATE
		| DATATRANS_WRITEFILE_HEAD_STATE;
	SetState( dwState );

	print( (char *)"begin to write buf" );
	//组织数据buf
	while( PackFileDataBuf( szBuf, len ) )
	{
		if( !m_DirFile.WriteToFile( szFile, szBuf, len ) )
		{
			m_DirFile.DeleteFile( szFile );
			return FALSE;
		}
	}

	//将数据写入文件
	return TRUE;
}		/* -----  end of method CDTWithTime::WriteDataToFile  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackFileDataBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackFileDataBuf ( BYTE *buf, int &len )
{
	if( IsHaveState( DATATRANS_WRITEFILE_HEAD_STATE	 ) )
	{
		print( (char *) "write head file" );
		return PackFileHeadBuf( buf, len );
	}

	if( IsHaveState( DATATRANS_WRITEFILE_YX_STATE	 ) )
	{
		print( (char *) "write yx file" );
		return PackFileYxBuf( buf, len );
	}

	if( IsHaveState( DATATRANS_WRITEFILE_YC_STATE	 ) )
	{
		print( (char *) "write yc file" );
		return PackFileYcBuf( buf, len );
	}

	if( IsHaveState( DATATRANS_WRITEFILE_YM_STATE	 ) )
	{
		print( (char *) "write ym file" );
		return PackFileYmBuf( buf, len );
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::PackFileDataBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackFileHeadBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackFileHeadBuf ( BYTE *buf, int &len )
{
	len = 0;

	buf[len++] = 0x68;
	//地址
	buf[len++] = HIBYTE( m_wDevAddr );
	buf[len++] = LOBYTE( m_wDevAddr );
	//功能码
	buf[len++] = 0;

	buf[len++] = HIBYTE( m_SaveFileTime.tm_year + 1900 );
	buf[len++] = LOBYTE( m_SaveFileTime.tm_year +1900 );
	buf[len++] = ( BYTE )m_SaveFileTime.tm_mon;
	buf[len++] = ( BYTE )m_SaveFileTime.tm_mday;
	buf[len++] = ( BYTE )m_SaveFileTime.tm_hour;
	buf[len++] = ( BYTE )m_SaveFileTime.tm_min;

	UnsetState( DATATRANS_WRITEFILE_HEAD_STATE );

	return TRUE;
}		/* -----  end of method CDTWithTime::PackFileHeadBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackFileYcBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackFileYcBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
    len = 0;

	for( int i=m_wFileDataPos; i<m_wAISum; i++ )
	{
		float fVal = m_fYcBuf[i];
		BYTE szTmp[4];
		//获取装置状态， 如果装置不通， 则设为无效数据非当前值
		WORD wSerialNo = GetSerialNoFromTrans( YC_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		//有效性
		buf[len++] = (BYTE)bDevState;

		//值
		memcpy( szTmp, &fVal, 4 );
		buf[len++] = szTmp[3];
		buf[len++] = szTmp[2];
		buf[len++] = szTmp[1];
		buf[len++] = szTmp[0];

		//最大范围200
		byCount ++;
		if( byCount >= 200 )
		{
			break;
		}
	}

	//数量
	m_wFileDataPos += byCount;

	if( m_wFileDataPos >= m_wAISum )
	{
		UnsetState( DATATRANS_WRITEFILE_YC_STATE );
		m_wFileDataPos = 0;
	}

	return TRUE;
}		/* -----  end of method CDTWithTime::PackFileYcBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackFileYxBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackFileYxBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
    len = 0;

	for( int i=m_wFileDataPos; i<m_wDISum; i += 4 )
	{

		for ( int j=0; j<4; j++)
		{
			if( i + j >= m_wDISum )
			{
				break;
			}
			WORD wVal = m_byYxBuf[i+j];
			//获取装置状态， 如果装置不通， 则设为无效数据非当前值
			WORD wSerialNo = GetSerialNoFromTrans( YX_TRANSTOSERIALNO , i+j ) ;
			BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;

			//值
			BYTE byVal =  ( wVal & 0x01 );
			buf[len++]  = bDevState ;
			buf[len++]  = byVal ;
			byCount ++;
		}

		//最大范围240
		if( byCount >= 250 )
		{
			break;
		}
	}

	m_wFileDataPos += byCount;

	if( m_wFileDataPos >= m_wDISum )
	{
		UnsetState( DATATRANS_WRITEFILE_YX_STATE );
		m_wFileDataPos = 0;
	}
	return TRUE;
}		/* -----  end of method CDTWithTime::PackFileYxBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackFileYmBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackFileYmBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
    len = 0;

	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);

	for( int i=m_wFileDataPos; i<m_wPISum; i++ )
	{
		char szTmp[4];
		DWORD dwVal = (DWORD)(m_dwYmBuf[i]);
		//获取装置状态， 如果装置不通， 则设为无效数据非当前值
		WORD wSerialNo = GetSerialNoFromTrans( DD_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		//有效性
		buf[len++] = (BYTE)bDevState;

		float fVal = ( float )dwVal;
		memcpy( szTmp, &fVal, 4 );
		buf[len++] = szTmp[3];
		buf[len++] = szTmp[2];
		buf[len++] = szTmp[1];
		buf[len++] = szTmp[0];
		//值
		// buf[len++] = HIBYTE(HIWORD(dwVal));
		// buf[len++] = LOBYTE(HIWORD(dwVal));
		// buf[len++] = HIBYTE(LOWORD(dwVal));
		// buf[len++]  = LOBYTE(LOWORD(dwVal));

		//最大范围50
		byCount ++;
		if( byCount >= 200 )
		{
			break;
		}
	}

	//数量
	m_wFileDataPos += byCount;
	if( m_wFileDataPos >= m_wPISum )
	{
		UnsetState( DATATRANS_WRITEFILE_YM_STATE );
		m_wFileDataPos = 0;
	}

	return TRUE;
}		/* -----  end of method CDTWithTime::PackFileYmBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ProtocolErrorProc
 * Description:  协议错误处理
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::ProtocolErrorProc ( void )
{
	m_byTimerCount ++;

	if( m_byTimerCount > 20 )
	{
		m_byTimerCount = 0;
		if( ! IsHaveState(DATATRANS_LINK_STATE) )
		print( "CDTWithTime is runing please wait" );
	}

	if( m_bySendCount > DATATRANS_MAX_SEND_COUNT )
	{
		sprintf( m_szPrintBuf, "sendcount=%d > %d init protocol",m_bySendCount,  DATATRANS_MAX_SEND_COUNT );
		print(m_szPrintBuf  );
		InitProtocolState(  );
	}
}		/* -----  end of method CDTWithTime::ProtocolErrorProc  ----- */

/* #####   recv 接收部分   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ProcessRecvBuf
 * Description:  处理接收报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::ProcessRecvBuf ( BYTE *pBuf, int len )
{
	if( !m_bSending )
	{
		return FALSE;
	}

	switch ( pBuf[1] )
	{
		case 0xF1:
			if( !IsHaveState( DATATRANS_YC_STATE ) && !IsHaveState( DATATRANS_YM_STATE )  )
			{
				return FALSE;
			}
			if( IsHaveState( DATATRANS_YC_OVER_STATE ) )
			{
				UnsetState( DATATRANS_YC_OVER_STATE );
				UnsetState( DATATRANS_YC_STATE );
			}

			if( IsHaveState( DATATRANS_YM_OVER_STATE ) )
			{
				UnsetState( DATATRANS_YM_OVER_STATE );
				UnsetState( DATATRANS_YM_STATE );
				char szBuf[128];
				if( m_DirFile.IsFile(  m_szReadFile ) )
				{
					print( (char *)"CDTWithTime delete file" );
					m_DirFile.DeleteFile( m_szReadFile );
					memset( m_szReadFile, 0, sizeof( m_szReadFile ) );
				}
				print( szBuf );
			}

			print( (char *)"CDTWithTime pocess recv yc/ym" );
			break;

		case 0xF3:
			if( !IsHaveState( DATATRANS_YX_STATE )
					&& !IsHaveState( DATATRANS_CHANGE_YX_STATE ) )
			{
				return FALSE;
			}

			if( IsHaveState( DATATRANS_CHANGE_YX_STATE ) )
			{
				print( (char *)"CDTWithTime pocess recv changeyx" );
				UnsetState( DATATRANS_CHANGE_YX_STATE );
			}

			if( IsHaveState( DATATRANS_YX_OVER_STATE ) )
			{
				UnsetState( DATATRANS_YX_OVER_STATE );
				UnsetState( DATATRANS_YX_STATE );
			}
			print( (char *)"CDTWithTime pocess recv yx" );
			break;

		case 0xF5:
			if( !IsHaveState ( DATATRANS_CHANGE_YX_STATE ) )
			{
				return FALSE;
			}

			print( (char *)"CDTWithTime pocess recv changeyx" );
			UnsetState( DATATRANS_CHANGE_YX_STATE );
			break;

		// case 0xF7:
			// if( !IsHaveHeart(  ) )
			// {
				// return FALSE;
			// }

			// UnsetState( DATATRANS_HEARTBEAT_STATE );
			// print( (char *)"CDTWithTime pocess recv heart" );

			// break;


		default:
			print( (char *)"CDTWithTime can't pocess recv buf" );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CDTWithTime::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  SetRecvParam
 * Description:  设置接收参数
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::SetRecvParam ( void  )
{
	m_bSending = FALSE;
	m_bySendCount = 0;
	UnsetState( DATATRANS_RESEND_STATE );
}		/* -----  end of method CDTWithTime::SetRecvParam  ----- */


/* #####   send 发送部分   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetSendBuf
 * Description:  获取发送报文
 *       Input:  buf 缓冲区
 *				 len 长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetSendBuf ( BYTE *buf, int &len )
{
	BOOL bRtn = TRUE;
	//获取发送状态
	if( !GetProtocolState(  ) )
	{
		return FALSE;
	}

	//根据格式组织相应发送报文
	bRtn = GetSendTypeBuf( buf, len );

	//储存重发报文
	SaveResendBuf( buf, len, bRtn );

	return bRtn;
}		/* -----  end of method CDTWithTime::GetSendBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetProtocolState
 * Description:  根椐条件得到协议状态
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetProtocolState ( void )
{
	//重发格式判断
	if( IsResend(  ) )
	{
		// print( (char *)"CDTWithTime resend" );
		SetState( DATATRANS_RESEND_STATE );
		OpenLink(  );
		return TRUE;
	}

	//变化Yx数据
	if( IsHaveChangeYX(  ) )
	{
		// print( (char *)"CDTWithTime changeyx" );
		// SetState( DATATRANS_CHANGE_YX_STATE );
		// OpenLink(  );
		// return TRUE;
	}

	//全数据
	if( IsHaveAll(  ) )
	{
		print( (char *)"CDTWithTime alldata" );
		OpenLink(  );
		return TRUE;
	}

	//心跳
	if( IsHaveHeart(  ) )
	{
		// print( (char *)"CDTWithTime heart" );
		// OpenLink(  );
		// return TRUE;
	}

	CloseLink(  );
	return FALSE;
}		/* -----  end of method CDTWithTime::GetProtocolState  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  IsResend
 * Description:  是否需要重发
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::IsResend ( void ) const
{
	return IsHaveState( DATATRANS_RESEND_STATE );
}		/* -----  end of method CDTWithTime::IsResend  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  IsHaveChangeYX
 * Description:  是否有变化YX
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::IsHaveChangeYX ( void  ) const
{
	if ( m_dwDIEQueue.size( ) > 0 )
	{
		return TRUE;
	}

	return FALSE;
}		/*  -----  end of method CDTWithTime::IsHaveChangeYXData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  IsHaveAll
 * Description:  是否有全部数据上送
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::IsHaveAll ( void  ) const
{
	DWORD dwAll = DATATRANS_YC_STATE | DATATRANS_YX_STATE | DATATRANS_YM_STATE
		| DATATRANS_YC_OVER_STATE | DATATRANS_YX_OVER_STATE | DATATRANS_YM_OVER_STATE;
	if( dwAll & m_ProtocolState )
	{
		return TRUE;
	}

	return FALSE;
}		/*  -----  end of method CDTWithTime::IsHaveAll  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  IsHaveHeart
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::IsHaveHeart ( void ) const
{
	return IsHaveState( DATATRANS_HEARTBEAT_STATE );
}		/* -----  end of method CDTWithTime::IsHaveHeart  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetSendTypeBuf
 * Description:  获取相应类型的数据
 *       Input:  buf 缓冲区
 *				 len 长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetSendTypeBuf ( BYTE *buf, int &len)
{
	/* 获取重发数据 */
	if( IsHaveState( DATATRANS_RESEND_STATE ) )
	{
		GetResendBuf( buf, len );
		return TRUE;
	}

	/* 获取变化遥信数据 */
	if( IsHaveState( DATATRANS_CHANGE_YX_STATE ) )
	{
		// return GetChangeYXBuf( buf, len  );
	}

	/* 获取全部数据 */
	if( GetAllDataBuf( buf, len ) )
	{
		return TRUE;
	}

	/*  获取心跳数据*/
	if( IsHaveHeart(  ) )
	{
		// return GetHeartBuf( buf, len );
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::GetSendTypeBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetChangeYXBuf
 * Description:  获取变化YX数据
 *       Input:  buf 缓冲区
 *				 len 长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetChangeYXBuf ( BYTE *buf, int &len )
{
	/* 组织变化YX报文 */
	BOOL bRtn =  PackChangeYXBuf( buf, len );
	//状态设置
	// UnsetState( DATATRANS_CHANGE_YX_STATE );
	print( (char *)"CDTWithTime get changeyx" );

	return bRtn;
}		/* -----  end of method CDTWithTime::GetChangeYXBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackChangeYXBuf
 * Description:  组织YX报文
 *       Input:  buf 缓冲区
 *				 len 长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackChangeYXBuf ( BYTE *buf, int &len )
{
	WORD wSerialNo;
	WORD wPnt;
	WORD wVal;
	BOOL bDevState;
	//BYTE byVal;
	struct tm tmStruct;
	WORD wMiSecond;
	WORD wTime;

	//获取变化遥信的序号、点号和值
	if( !GetSOEInfo( wSerialNo, &wPnt, &wVal, &tmStruct, &wMiSecond ) )
	{
		return FALSE;
	}
	//获取装置状态
	bDevState = m_pMethod->GetDevCommState( wSerialNo );
	//bDevState:0正常 1不正常 wVal:0分 1合 10 11无效 00 01有效
	//byVal = ( bDevState << 1 ) | ( wVal & 0x01 );

    len = 0;
	//报文头
	buf[len++] = 0x68;
	//地址
	buf[len++] = HIBYTE( m_wDevAddr );
	buf[len++] = LOBYTE( m_wDevAddr );
	//功能码
	buf[len++] = 0xF4;
	buf[len++] = HIBYTE( tmStruct.tm_year + 1900 );
	buf[len++] = LOBYTE( tmStruct.tm_year + 1900 );
	buf[len++] = ( BYTE )tmStruct.tm_mon;
	buf[len++] = ( BYTE )tmStruct.tm_mday;
	buf[len++] = ( BYTE )tmStruct.tm_hour;
	buf[len++] = ( BYTE )tmStruct.tm_min;
	wTime = tmStruct.tm_sec * 1000 + wMiSecond;
	buf[len++] = HIBYTE( wTime ) ;
	buf[len++] = LOBYTE( wTime ) ;
	//起始点号
	buf[len++] = HIBYTE( wPnt + m_wAISum );
	buf[len++] = LOBYTE( wPnt + m_wAISum );
	//数量
	buf[len++] = 0x01;
	//值
	buf[len++] = (BYTE)( bDevState );
	buf[len++] = ( BYTE )( wVal & 0x01 );

	return TRUE;
}		/* -----  end of method CDTWithTime::PackChangeYXBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetAllDataBuf
 * Description:  获取全部数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetAllDataBuf ( BYTE *buf, int &len )
{
	print( (char *)"CDTWithTime alldata get" );
	/* YC数据 */
	if( IsHaveState( DATATRANS_YC_STATE ) )
	{
		print( (char *)"CDTWithTime get yc" );
		return GetYCDataBuf( buf, len );
	}

	/* YX数据 */
	if( IsHaveState( DATATRANS_YX_STATE ) )
	{
		// print( (char *)"CDTWithTime get yx" );
		// return GetYXDataBuf( buf, len );
	}

	/* YM数据 */
	if ( IsHaveState( DATATRANS_YM_STATE ) )
	{
		print( (char *)"CDTWithTime get ym" );
		return GetYMDataBuf( buf, len );
	}

	return FALSE;
}		/* -----  end of method CDTWithTime::GetAllDataBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetYcDataBuf
 * Description:  获取yc数据包
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetYCDataBuf ( BYTE *buf, int &len )
{
	/* 组织YC报文 */
	BOOL bRtn = PackYCBuf( buf, len );
	//状态设置
	if( m_wAllDataPos >= m_wAISum )
	{
		// UnsetState( DATATRANS_YC_STATE );
		SetState ( DATATRANS_YC_OVER_STATE );
		m_wAllDataPos = 0;
	}
	return bRtn;
}		/* -----  end of method CDTWithTime::GetYcDataBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackYCBuf
 * Description:  组织YX报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackYCBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
	BYTE byReadNum;
    len = 0;
	if( m_wAllDataPos >= m_wAISum )
	{
		return FALSE;
	}

	if( !PackHeadBuf( buf, len, 0xF0, ESD_DATATRANS_YC_DATATYPE ) )
	{
		return FALSE;
	}

	if( m_wAllDataPos + 45 <= m_wAISum )
	{
		byCount = 45;
	}
	else
	{
		byCount = m_wAISum - m_wAllDataPos;
	}

	byReadNum = m_DirFile.ReadFromFile( m_szReadFile,
			&buf[len],
			byCount * 5 ,
			m_wAllDataPos*5 + m_dwFileYcBeginPos);

	if( byReadNum != ( byCount * 5  ))
	{
		return FALSE;
	}

	len += byReadNum;
	//数量
	buf[12] = byCount;

	m_wAllDataPos += byCount;
	return TRUE;
}		/* -----  end of method CDTWithTime::PackYCBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetYcDataBuf
 * Description:  获取yc数据包
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetYXDataBuf ( BYTE *buf, int &len )
{
	/* 组织YX报文 */
	BOOL bRtn = PackYXBuf( buf, len );
	//状态设置
	if( m_wAllDataPos >= m_wDISum )
	{
		// UnsetState( DATATRANS_YX_STATE );
		SetState ( DATATRANS_YX_OVER_STATE );
		m_wAllDataPos = 0;
	}
	return bRtn;
}		/* -----  end of method CDTWithTime::GetYcDataBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackYXBuf
 * Description:  组织YX报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackYXBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
	BYTE byReadNum;
    len = 0;
	if( m_wAllDataPos >= m_wDISum )
	{
		return FALSE;
	}

	if( !PackHeadBuf( buf, len, 0xF2, ESD_DATATRANS_YX_DATATYPE ) )
	{
		return FALSE;
	}

	if( m_wAllDataPos + 120 <= m_wDISum )
	{
		byCount = 120;
	}
	else
	{
		byCount = m_wDISum - m_wAllDataPos;
	}

	byReadNum = m_DirFile.ReadFromFile( m_szReadFile,
			&buf[len],
			byCount * 2 ,
			m_wAllDataPos*2 + m_dwFileYxBeginPos);

	if( byReadNum != ( byCount * 2  ))
	{
		return FALSE;
	}

	//数量
	len += byReadNum;
	buf[12] = byCount;

	m_wAllDataPos += byCount;
	return TRUE;
}		/* -----  end of method CDTWithTime::PackYXBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetYMDataBuf
 * Description:  获取yc数据包
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetYMDataBuf ( BYTE *buf, int &len )
{
	/* 组织YM报文 */
	BOOL bRtn = PackYMBuf( buf, len );
	//状态设置
	if( m_wAllDataPos >= m_wPISum )
	{
		print( (char *) "ym end");
		// UnsetState( DATATRANS_YM_STATE );
		SetState ( DATATRANS_YM_OVER_STATE );
		m_wAllDataPos = 0;
	}
	return bRtn;
}		/* -----  end of method CDTWithTime::GetYMDataBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackYMBuf
 * Description:  组织YX报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackYMBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
	BYTE byReadNum;
    len = 0;
	if( m_wAllDataPos >= m_wPISum )
	{
		return FALSE;
	}

	if( !PackHeadBuf( buf, len, 0xF0, ESD_DATATRANS_YM_DATATYPE ) )
	{
		return FALSE;
	}

	if( m_wAllDataPos + 45 <= m_wPISum )
	{
		byCount = 45;
	}
	else
	{
		byCount = m_wPISum - m_wAllDataPos;
	}

	byReadNum = m_DirFile.ReadFromFile( m_szReadFile,
			&buf[len],
			byCount * 5 ,
			m_wAllDataPos*5 + m_dwFileYmBeginPos);

	if( byReadNum != ( byCount * 5  ))
	{
		return FALSE;
	}

	//数量
	len += byReadNum;
	buf[12] = byCount;

	m_wAllDataPos += byCount;
	return TRUE;
}		/* -----  end of method CDTWithTime::PackYMBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  PackHeadBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::PackHeadBuf ( BYTE *buf, int &len, BYTE byFuncCode, BYTE byType )
{
	len = 0;
	len = m_DirFile.ReadFromFile( m_szReadFile, buf, 10, 0 );
	if( 10 != len )
	{
		return FALSE;
	}

	buf[3] = byFuncCode;

	//起始点号
	if( ESD_DATATRANS_YM_DATATYPE == byType)
	{
		buf[len++] = HIBYTE( m_wAllDataPos + DATATRANS_MAX_YC_NUM);
		buf[len++] = LOBYTE( m_wAllDataPos + DATATRANS_MAX_YC_NUM);
	}
	else
	{
		buf[len++] = HIBYTE( m_wAllDataPos );
		buf[len++] = LOBYTE( m_wAllDataPos );
	}
	//数量
	buf[len++] = 0x00;

	return TRUE;
}		/* -----  end of method CDTWithTime::PackHeadBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetHeartBuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::GetHeartBuf ( BYTE *buf, int &len )
{
	len = 0;
	buf[len++] = 0x68;
	buf[len++] = 0xF6;

	return TRUE;
}		/* -----  end of method CDTWithTime::GetHeartBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  GetResendBuf
 * Description:  获取重发数据
 *       Input:  buf:缓冲区
 *				 len:长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::GetResendBuf ( BYTE *buf, int &len )
{
	len = m_iResendLen;
	memcpy( buf, m_byResendBuf, len );

	m_byResendCount ++;
}		/* -----  end of method CDTWithTime::GetResendBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  SaveResendBuf
 * Description:  保存重发数据
 *       Input:  buf:缓冲区
 *				 len:长度
 *				 byValid:是否保存
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::SaveResendBuf ( BYTE *buf, int len, BOOL byValid )
{
	if( byValid )
	{
		m_iResendLen = len;
		memcpy( m_byResendBuf, buf, m_iResendLen );
	}
}		/* -----  end of method CDTWithTime::SaveResendBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  SetSendParam
 * Description:  设置发送参数
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::SetSendParam ( BOOL bIsSendValid )
{
	if( bIsSendValid  )
	{
		m_bSending = TRUE;
		m_bySendCount ++;
		SetState( DATATRANS_RESEND_STATE );
	}
}		/* -----  end of method CDTWithTime::SetSendParam  ----- */

/* #####   Init 初始化部分   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ReadCfgInfo
 * Description:  读取配置模板信息
 *       Input:  void
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::ReadCfgInfo ( void )
{
	char szPath[256] = "";
	sprintf( szPath, "%s%s" ,DATATRANSSPREFIXFILENAME, m_sTemplatePath );
	print( szPath );

	//读配置模板的点表信息
    ReadCfgMapInfo ( szPath );

	//读配置模板的特殊配置信息
	// sprintf( szPath, "%sBus%.2dOtherCfg.txt",DATATRANSSPREFIXFILENAME,  m_byLineNo+1  );
	if( !ReadCfgOtherInfo( szPath ) )
	{
		// return FALSE;
	}

	print( (char *)"CDTWithTime ReadCfgInfo OK" );
	return TRUE;
}		/* -----  end of method CDTWithTime::ReadCfgInfo  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ReadCfgMapInfo
 * Description:  读取点表信息
 *       Input:  szPath 模板路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::ReadCfgMapInfo ( char *szPath )
{
	//调用Rtu.cpp读取
	ReadMapConfig( szPath );
}		/* -----  end of method CDTWithTime::ReadCfgMapInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  ReadCfgOtherInfo
 * Description:  读配置模板的特殊配置信息
 *       Input:  szPath 模板路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDTWithTime::ReadCfgOtherInfo ( char *szPath )
{
	FILE *fp = NULL;
	char szLineBuf[256];
	int iLineNum = 0;
	int iOtherInfoNum = 0;
	int iOtherInfoAllNum = 2;

	fp = fopen( szPath , "r");
	if ( NULL == fp )
	{
		printf ( "open file %s err!!!\n", szPath );
		return FALSE;
	}

	while ( NULL != fgets(szLineBuf, sizeof(szLineBuf), fp)
			&& 30 > iLineNum )
	{
		iLineNum ++;
		if( 0 == strncmp ( szLineBuf, "SENDINTERVAL=", 13 ) )
		{
			WORD wCfgVal = (WORD)( atoi( &szLineBuf[13] ) );
			//判断是否是整分
			if( 0 != ( wCfgVal % 60 ) )
			{
				printf ( "CDTWithTime wCfgVal is not minutes %d\n", wCfgVal );
			}

			//转化成分
			wCfgVal = wCfgVal / 60;

			if( wCfgVal > 1 )
			{
				if( wCfgVal >= 60 )
				{
					m_LocalSumTime = wCfgVal / 60;
					wCfgVal = wCfgVal % 60;
				}

				if( 0 == ( 60 %  wCfgVal  ) )
				{
					m_wAllDataInterval =m_LocalSumTime * 60 + wCfgVal;
					iOtherInfoNum ++;
					continue;
				}
				else
				{
					printf ( "CDTWithTime SENDINTERVAL/3600=%d is error!!! not a right sec  default is used\n", wCfgVal );
					m_wAllDataInterval = 15;
				}
			}
			else
			{
				printf ( "CDTWithTime SENDINTERVAL=%d error!!! default is used\n", wCfgVal );
				m_wAllDataInterval = 15;
			}

			iOtherInfoNum ++;
		}
		if( 0 ==strncmp( szLineBuf, "HEARTTIME=", 10 ) )
		{
			WORD wCfgVal = (WORD)( atoi( &szLineBuf[10] ) );
			if( wCfgVal >= 1 )
			{
				m_LocalHeartbeatTime = wCfgVal * 1000;
			}
			else
			{
				printf ( "CDTWithTime HEARTTIME=%d error!!! default is used\n", wCfgVal );
				m_LocalHeartbeatTime = 60 * 1000;
			}

			iOtherInfoNum ++;
		}

		if( iOtherInfoAllNum <= iOtherInfoNum )
		{
			printf ( "CDTWithTime alldata interval=%dmin, heat interval=%lums\n", m_wAllDataInterval, m_LocalHeartbeatTime );
			break;
		}
	}

	fclose( fp );
	return TRUE;
}		/* -----  end of method CDTWithTime::ReadCfgOtherInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  InitProtocol
 * Description:  初始化协议状态
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::InitProtocol ( void )
{
	//初始化基本状态参数
	InitProtocolState(  );

	//初始化点表信息
	InitProtocolTransTab(  );

	//初始化协议数据
	InitProtocolData(  );

	print( (char *)"CDTWithTime InitProtocol OK" );
}		/* -----  end of method CDTWithTime::InitProtocol  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  InitProtocolState
 * Description:  初始化协议参数
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::InitProtocolState ( void )
{
	/* 初始化信息 */
	//初始化协议状态
	if( IsHaveState( DATATRANS_LINK_STATE ) )
	{
		CloseLink(  );
	}
	m_ProtocolState = 0;
	//初始化重发状态
	m_byResendCount = 0;
	m_iResendLen = 0;
	memset( m_byResendBuf, 0, sizeof( m_byResendBuf ) );
	//全数据位置
	m_wAllDataPos = 0;
	//状态相关
	m_bSending=FALSE;;
	m_bySendCount = 0;

	//时间相关
	m_LocalAddTime = 0;
	m_LocalHeartbeatAddTime = 0;
	m_byTimerCount = 0;
	m_LocalAddHour =0;

	//文件相关
	memset( &m_SaveFileTime , 0, sizeof( struct tm ) );
	m_wFileDataPos = 0;
	memset( m_szReadFile, 0, sizeof( m_szReadFile ) );

	print( (char *)"CDTWithTime InitProtocolState" );

}		/* -----  end of method CDTWithTime::InitProtocolState  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  InitProtocolTransTab
 * Description:  初始化转发信息
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::InitProtocolTransTab ( void )
{
	/* 获取点表信息的转发序号 */
    CreateTransTab();
}		/* -----  end of method CDTWithTime::InitProtocolTransTab  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDTWithTime
 *      Method:  InitProtocolData
 * Description:  初始化协议数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDTWithTime::InitProtocolData ( void )
{
	/* 从内存数据库中--获取转发表默认数据 */
	m_pMethod->ReadAllYcData(&m_fYcBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	m_pMethod->ReadAllYxData( &m_byYxBuf[ 0 ] ) ;

	m_dwFileYxBeginPos = 10;
	m_dwFileYcBeginPos = 10 + m_wDISum * 2;
	m_dwFileYmBeginPos = 10 + m_wDISum * 2 +m_wAISum * 5;
}		/* -----  end of method CDTWithTime::InitProtocolData  ----- */

/* #####   other 其它部分   ################################################### */
int CDTWithTime::GetRealVal(BYTE byType, WORD wPnt, void *v)
{
    WORD  wValue = 0 ;
    switch(byType)
    {
    case 0:
        if(wPnt>=DATATRANS_MAX_YC_NUM) return -2;
        memcpy(v, &m_fYcBuf[wPnt], sizeof(WORD));
        break;
    case 1:
        {
			if(wPnt>=DATATRANS_MAX_YX_NUM)
				return -2;

			if( m_byYxBuf[ wPnt ] ==0 )
				wValue = 0;
			else
				wValue = 1;

			memcpy(v, &wValue, sizeof(WORD));
		}
        break;
    case 2:
        if(wPnt>=DATATRANS_MAX_YM_NUM) return -2;
        memcpy(v, &m_dwYmBuf[wPnt], sizeof(QWORD));
        break;
    default:
        return -1;
    }
    return 0;
}

BOOL CDTWithTime::WriteAIVal(WORD wSerialNo ,WORD wPnt, float fVal)
{
    if(m_pwAITrans==NULL) return FALSE;
    WORD wNum = m_pwAITrans[wPnt];
    if(wNum>m_wAISum) return FALSE;
    if(wNum<DATATRANS_MAX_YC_NUM)//mengqp 将<=改为< 否则m_wAIBuf[4096]越界
    {
        float fDelt = fVal - m_fYcBuf[wNum];
        if(abs((int)fDelt)>=m_wDeadVal)
        {
            m_fYcBuf[wNum] = fVal;
			// if(m_bDataInit)
			// {
                AddAnalogEvt( wSerialNo , wNum, fVal);
			// }
        }
    }
    return TRUE ;
}

BOOL CDTWithTime::WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal)
{
    if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>m_wDISum) return FALSE;
    if( wNum<DATATRANS_MAX_YX_NUM)//mengqp 将<= 改为<
    {
        if( m_byYxBuf[ wNum ] != wVal )
        {
            m_byYxBuf[ wNum ] = wVal ;
            // if(m_bDataInit)
			// {
                AddDigitalEvt( wSerialNo , wNum, wVal);
			// }
        }
    }
    return TRUE ;
}
BOOL CDTWithTime::WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal)
{
    if(m_pwPITrans==NULL) return FALSE;
    WORD wNum = m_pwPITrans[wPnt];
    if(wNum>m_wPISum) return FALSE;
    if(wNum<DATATRANS_MAX_YM_NUM)//mengqp 将<= 改为<
    {
        m_dwYmBuf[wNum] = dwVal;
    }
    return TRUE ;
}

BOOL CDTWithTime::WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{
    if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>=m_wDISum) return FALSE;
    if(wNum<DATATRANS_MAX_YX_NUM)
    {
        AddSOEInfo(wSerialNo , wNum, wVal, lTime, wMiSecond);
    }
    return TRUE ;
}
/* ====================  OtherEnd    ======================================= */
