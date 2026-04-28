/*
 * =====================================================================================
 *
 *       Filename:  ModBusDSE7320FDJ.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年12月17日 09时55分14秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */
#include "ModBusDSE7320FDJ.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

static void print( BYTE byBusNo, char *buf )
{
	OutBusDebug( byBusNo, (BYTE *)buf, strlen(buf) , 2);		
}
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  CModbusDSE7320FDJ
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CModbusDSE7320FDJ::CModbusDSE7320FDJ ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_bySendPos = 0;
	m_byDataType = 0;
	m_byYcDealFlag = 0;
	m_byYxDealFlag = 0;
}  /* -----  end of method CModbusDSE7320FDJ::CModbusDSE7320FDJ  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  ~CModbusDSE7320FDJ
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CModbusDSE7320FDJ::~CModbusDSE7320FDJ ()
{
}  /* -----  end of method CModbusDSE7320FDJ::~CModbusDSE7320FDJ  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  GetSendBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::GetSendBuf ( BYTE *buf, int &len )
{
	len =0;
	BYTE byRegisterNum = 0;
	WORD wStartReguster = 0;
	m_byYcDealFlag = 0;
	m_byYxDealFlag = 0;

	buf[len++] = m_wDevAddr;
	buf[len++] = 0x03;

	switch ( m_bySendPos )
	{
	case 0:	//yc0-6 油压 水温 备用 燃油油位 充电电压 电池电压 转速
		wStartReguster = 1024;
		byRegisterNum = 7;
		m_byYcDealFlag = 1;
		break;

	case 1:	//yc7-19 L1-N 电压 L2-N 电压 L3-N 电压 L1-L2电压 L2-L3电压 L3-L1电压 L1相电流 L2相电流 L3相电流 接地电流 L1相功率 L2相功率 L3相功率
		wStartReguster = 1032;
		byRegisterNum = 26;
		m_byYcDealFlag = 2;
		break;

	case 2: //yc20 市电频率	
		wStartReguster = 1059;
		byRegisterNum = 1;
		m_byYcDealFlag = 3;
		break;

	case 3:	//yc21-26 市电L1-N 电压 市电L2-N 电压 市电L3-N 电压 市电L1-L2电压 市电L2-L3电压 市电L3-L1电压
		wStartReguster = 1060;
		byRegisterNum = 12;
		m_byYcDealFlag = 4;
		break;

	case 4:	//yc27-35	三相总功率 L1相视在功率 L2相视在功率 L3相视在功率 总视在功率 L1相无功功率 L2相无功功率 L3相无功功率 三相总无功功率
		wStartReguster = 1536;
		byRegisterNum = 18;
		m_byYcDealFlag = 5;
		break;

	case 5:	 //yc36-39 L1相功率因数 L2相功率因数 L3相功率因数 平均功率因数
		wStartReguster = 1554;
		byRegisterNum = 4;
		m_byYcDealFlag = 6;
		break;

	case 6: //yx 0-8 急停（无 普通 停机）低油压（无 普通 停机） 高水温（无 普通 停机）
		wStartReguster = 39425;
		byRegisterNum = 1;
		m_byYxDealFlag = 1;
		break;

	case 7:	//yx9-12 停止 手动 测试 自动
		wStartReguster = 48648;
		byRegisterNum = 4;
		m_byYxDealFlag = 2;
		break;

	default:	
		print( m_byLineNo, (char *)"sendpos err!!!" );
		break;
	}				/* -----  end switch  ----- */
	buf[len++] = HIBYTE( wStartReguster );
	buf[len++] = LOBYTE( wStartReguster );

	buf[len++] = 0x00;
	buf[len++] = byRegisterNum;

	WORD wCRC = GetCrc( buf, len );
	buf[ len++ ] = HIBYTE(wCRC);
	buf[ len++ ] = LOBYTE(wCRC);

	return TRUE;
}		/* -----  end of method CModbusDSE7320FDJ::GetSendBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性 
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::WhetherBufValue ( BYTE *buf, int &len )
{
	BYTE *pointer = buf;
	WORD wCrc;
	char szBuf[256];
	int pos = 0;
	// printf ( "444\n" );
	while( len >= 4 )
	{
		//判断地址
		if ( *pointer != m_wDevAddr )
		{
			sprintf( szBuf, "ModBusDSE7320FDJ recv addr err recvAddr=%d LocalAddr=%d\n", *pointer, m_wDevAddr );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );
			goto DEFAULT;
		}

		//判断功能码
		if ( *( pointer + 1 ) != 0x03 )
		{
			sprintf( szBuf, "ModBusDSE7320FDJ recv funcode err recv fuccode=%d\n", *( pointer + 1 ) );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );
			goto DEFAULT;
		}

		//判断长度
		if( *( pointer + 2 ) > len - 3 )
		{
			sprintf( szBuf, "ModBusDSE7320FDJ recv len err recv len=%d\n", *( pointer + 2 ) );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );
			goto DEFAULT;
		}

		//判断校验
		wCrc = GetCrc( pointer, ( *( pointer + 2 ) + 3 ) );
		if( *( pointer + ( *( pointer + 2 ) + 3 ) ) !=  HIBYTE(wCrc)
				|| *( pointer + ( *( pointer + 2 ) + 4 ) ) !=  LOBYTE(wCrc))
		{
			sprintf( szBuf, "ModBusDSE7320FDJ recv crc err recvcrc=%.2x%.2x, localcrc=%.2x%.2x\n",
					*( pointer + ( *( pointer + 2 ) + 3 ) ), *( pointer + ( *( pointer + 2 ) + 4 ) ), 
					HIBYTE(wCrc), LOBYTE(wCrc)) ;
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );
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
}		/* -----  end of method CModbusDSE7320FDJ::WhetherBufValue  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  ProcessYxBuf
 * Description:  特殊处理遥信数据
 *       Input:	 接受缓冲区 长度
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::ProcessYxBuf ( BYTE *buf, int len )
{
	switch ( m_byYxDealFlag )
	{
	case 1:	
		{
			if( len != 7 )
			{
				OutBusDebug( m_byLineNo, (BYTE *)"yx len err", 20, 2 );
				return FALSE;
			}

			WORD wYxVal = MAKEWORD( buf[4], buf[3] );
			BYTE byTmp;
			for( int i=2; i>-1; i-- )
			{
				byTmp = (wYxVal >> ((i+1) * 4)) & 0x0f;
				BYTE byYxflag = 0;
				char szBuf[256];
				// printf ( "555\n" );
				switch ( byTmp )
				{
				case 0:	
					byYxflag = 1;
					break;

				case 2:	
					byYxflag = 2;
					break;

				case 3:	
					byYxflag = 3;
					break;

				case 4:	
					byYxflag = 4;
					break;

				default:	
					byYxflag = 1;
					break;
				}				/* -----  end switch  ----- */

				for( int k=0; k<4;k++ )
				{
					if( k == byYxflag - 1 )
					{
						m_pMethod->SetYxData( m_SerialNo, i*4+k, 1 );
						sprintf( szBuf, "CModbusDSE7320FDJ:yx%d = 1", i*4+k );
					}
					else
					{
						m_pMethod->SetYxData( m_SerialNo, i*4+k, 0 );
						sprintf( szBuf, "CModbusDSE7320FDJ:yx%d = 0", i*4+k );	
					}
					print( m_byLineNo, szBuf );
				}
			}
		}
		break;

	case 2:	
		{
			if( len != 13 )
			{
				OutBusDebug( m_byLineNo, (BYTE *)"yx len err", 20, 2 );
				return FALSE;
			}
			WORD wPnt = 12;
			for ( int i=0; i<4; i++)
			{
				char szBuf[256];
				WORD wTmp = MAKEWORD( buf[4+2*i], buf[3+2*i]);
				if( 1 == wTmp )
				{
					m_pMethod->SetYxData( m_SerialNo, i+wPnt, 1 );
					sprintf( szBuf, "CModbusDSE7320FDJ:yx%d = 1", i+wPnt );	
				}
				else
				{
					m_pMethod->SetYxData( m_SerialNo, i+wPnt, 0 );
					sprintf( szBuf, "CModbusDSE7320FDJ:yx%d = 0", i+wPnt );	
				}
				print( m_byLineNo, szBuf );
			}
		}
		break;

	default:	
		print( m_byLineNo, (char *)"can't find ycdatatype" );
		break;
	}				/* -----  end switch  ----- */

	return TRUE;
} /* -----  end of method CModbusDSE7320FDJ::ProcessYxBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  ProcessYcBuf
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::ProcessYcBuf ( BYTE *buf, int len )
{
	WORD wPnt;
	BYTE byDataNum;
	switch ( m_byYcDealFlag )
	{
	case 1:	
		if( 19 != len )
		{
			OutBusDebug( m_byLineNo, (BYTE *)"yc len err!!!", 20, 2 );
			return FALSE;
		}
		wPnt = 0;
		byDataNum = 7;
		for( int i=0; i<byDataNum; i++ )
		{
			WORD wYcVal;
			float fYcVal;
			wYcVal = MAKEWORD( buf[4+i*2] , buf[3+i*2] );
			fYcVal = (float)wYcVal;
			m_pMethod->SetYcData( m_SerialNo, wPnt+i, fYcVal );
			char szBuf[256];
			sprintf( szBuf, "CModbusDSE7320FDJ:yc%d = %f", wPnt+i, fYcVal );	
			print( m_byLineNo, szBuf );
		}
		break;

	case 2:	
		if( 57 != len )
		{
			OutBusDebug( m_byLineNo, (BYTE *)"yc len err!!!", 20, 2 );
			return FALSE;
		}
		wPnt = 7;
		byDataNum = 13;
		for( int i=0; i<byDataNum; i++ )
		{
			WORD wHiYcVal;
			WORD wLoYcVal;
			DWORD dwYcVal;
			float fYcVal;
			wHiYcVal = MAKEWORD( buf[4+i*4] , buf[3+i*4] );
			wLoYcVal = MAKEWORD( buf[6+i*4] , buf[5+i*4] );
			dwYcVal = MAKELONG( wLoYcVal, wHiYcVal );
			fYcVal = (float)dwYcVal;

			m_pMethod->SetYcData( m_SerialNo, wPnt+i, fYcVal );
			char szBuf[256];
			sprintf( szBuf, "CModbusDSE7320FDJ:yc%d = %f", wPnt+i, fYcVal );	
			print( m_byLineNo, szBuf );
		}
		break;

	case 3:	
		if( 7 != len )
		{
			OutBusDebug( m_byLineNo, (BYTE *)"yc len err!!!", 20, 2 );
			return FALSE;
		}
		wPnt = 20;
		byDataNum = 1;
		for( int i=0; i<byDataNum; i++ )
		{
			WORD wYcVal;
			float fYcVal;
			wYcVal = MAKEWORD( buf[4+i*2] , buf[3+i*2] );
			fYcVal = (float)wYcVal;
			m_pMethod->SetYcData( m_SerialNo, wPnt+i, fYcVal );
			char szBuf[256];
			sprintf( szBuf, "CModbusDSE7320FDJ:yc%d = %f", wPnt+i, fYcVal );	
			print( m_byLineNo, szBuf );
		}
		break;

	case 4:	
		if( 29 != len )
		{
			OutBusDebug( m_byLineNo, (BYTE *)"yc len err!!!", 20, 2 );
			return FALSE;
		}
		wPnt = 21;
		byDataNum = 6;
		for( int i=0; i<byDataNum; i++ )
		{
			WORD wHiYcVal;
			WORD wLoYcVal;
			DWORD dwYcVal;
			float fYcVal;
			wHiYcVal = MAKEWORD( buf[4+i*4] , buf[3+i*4] );
			wLoYcVal = MAKEWORD( buf[6+i*4] , buf[5+i*4] );
			dwYcVal = MAKELONG( wLoYcVal, wHiYcVal );
			fYcVal = (float)dwYcVal;

			m_pMethod->SetYcData( m_SerialNo, wPnt+i, fYcVal );
			char szBuf[256];
			sprintf( szBuf, "CModbusDSE7320FDJ:yc%d = %f", wPnt+i, fYcVal );	
			print( m_byLineNo, szBuf );
		}
		break;

	case 5:	
		if( 41 != len )
		{
			OutBusDebug( m_byLineNo, (BYTE *)"yc len err!!!", 20, 2 );
			return FALSE;
		}
		wPnt = 27;
		byDataNum = 9;
		for( int i=0; i<byDataNum; i++ )
		{
			WORD wHiYcVal;
			WORD wLoYcVal;
			DWORD dwYcVal;
			float fYcVal;
			wHiYcVal = MAKEWORD( buf[4+i*4] , buf[3+i*4] );
			wLoYcVal = MAKEWORD( buf[6+i*4] , buf[5+i*4] );
			dwYcVal = MAKELONG( wLoYcVal, wHiYcVal );
			if( dwYcVal & 0x80 )
			{
				int iYcVal = (int)dwYcVal;
				fYcVal = ~iYcVal + 1;
				fYcVal *= -1.0;
			}
			else
			{
				fYcVal = (float)dwYcVal;
			}

			m_pMethod->SetYcData( m_SerialNo, wPnt+i, fYcVal );
			char szBuf[256];
			sprintf( szBuf, "CModbusDSE7320FDJ:yc%d = %f", wPnt+i, fYcVal );	
			print( m_byLineNo, szBuf );
		}
		break;

	case 6:	
		if( 13 != len )
		{
			OutBusDebug( m_byLineNo, (BYTE *)"yc len err!!!", 20, 2 );
			return FALSE;
		}
		wPnt = 36;
		byDataNum = 4;
		for( int i=0; i<byDataNum; i++ )
		{
			WORD wYcVal;
			float fYcVal;
			wYcVal = MAKEWORD( buf[4+i*2] , buf[3+i*2] );
			fYcVal = (float)wYcVal;
			m_pMethod->SetYcData( m_SerialNo, wPnt+i, fYcVal );
			char szBuf[256];
			sprintf( szBuf, "CModbusDSE7320FDJ:yc%d = %f", wPnt+i, fYcVal );	
			print( m_byLineNo, szBuf );
		}
		break;

	default:	
		print( m_byLineNo, (char *)"can't find ycdatatype" );
		break;
	}				/* -----  end switch  ----- */
	return TRUE;
}		/* -----  end of method CModbusDSE7320FDJ::ProcessYcBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::ProcessRecvBuf ( BYTE *buf, int len )
{
	//BYTE byPos;

	//byPos = GetSendPos(  );
	GetSendPos(  );
	// printf ( "%d %d \n", m_byYxDealFlag, m_byYcDealFlag );
	//位置是0 为遥信， 其他为遥测
	if( m_byYxDealFlag > 0)
	{
		return ProcessYxBuf( buf, len );
	}
	else if( m_byYcDealFlag > 0 )
	{
		return ProcessYcBuf( buf, len );
	}
	
	return FALSE;
}		/* -----  end of method CModbusDSE7320FDJ::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  GetSendPos
 * Description:  获取发送位置
 *       Input:  void 
 *		Return:  BYTE 发送位置
 *--------------------------------------------------------------------------------------
 */
BYTE CModbusDSE7320FDJ::GetSendPos ( void )
{
	return ( m_bySendPos % MODBUSMASTER_DSE7320FDJ_MAX_POS );
}		/* -----  end of method CModbusDSE7320FDJ::GetSendPos  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  ChangeSendPos
 * Description:  改变发送位置
 *       Input:	 void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModbusDSE7320FDJ::ChangeSendPos ( void )
{
	m_bySendPos = m_bySendPos++ % MODBUSMASTER_DSE7320FDJ_MAX_POS;

	if( m_bySendPos >= ( MODBUSMASTER_DSE7320FDJ_MAX_POS ) )
	{
		m_bySendPos = 0;
	}

}		/* -----  end of method CModbusDSE7320FDJ::ChangeSendPos  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度 消息（无遥控等消息内容 始终为NULL）
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	ChangeSendPos(  );

	GetSendBuf( buf, len );
	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method CModbusDSE7320FDJ::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::ProcessProtocolBuf ( BYTE *buf, int len )
{
	// printf ( "000\n" );
	if ( !WhetherBufValue( buf, len ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusDSE7320FDJ recv buf err !!!\n" );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );

		m_byRecvCount ++;
		return FALSE;	
	}
	// printf ( "111\n" );
	ProcessRecvBuf( buf, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method CModbusDSE7320FDJ::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method CModbusDSE7320FDJ::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModbusDSE7320FDJ::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		// m_bySendPos = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			OutBusDebug( m_byLineNo, (BYTE *)"CModbusDSE7320FDJ:unlink\n", 30, 2 );
		}
	}
}		/* -----  end of method CModbusDSE7320FDJ::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusDSE7320FDJ
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusDSE7320FDJ::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModbusDSE7320FDJ::GetDevCommState  ----- */