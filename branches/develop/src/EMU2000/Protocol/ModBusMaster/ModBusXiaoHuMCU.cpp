/*
 * =====================================================================================
 *
 *       Filename:  ModBusXiaoHuMCU.cpp
 *
 *    Description:  王孟华从现场发过来的消弧控制器  厦门正新集美厂
					四、报文格式
						下行报文格式（监控向装置下发报文）

						0EBH				同步头
						90H	
						0EBH	
						90H	
						目的地址			装置地址
						源地址				上位机地址
						状态量（00H）		故障特征码
						LEN=02H				报文长度
						CODE=55H			命令码
						57H					校验码
						55H					结束码
						AAH	

						上行报文格式（装置向监控上发报文）
						（1）无故障报文
						0EBH				同步头
						90H	
						0EBH	
						90H	
						目的地址			上位机地址
						源地址				装置地址
						状态量				输入开关量状态
						相电压				A相
						相电压				B相
						相电压				C相
						线电压				Uab
						线电压				Uac
						线电压				Ubc
						LEN=02H				报文长度
						CODE=50H			命令码
						52H					校验码
						55H					结束码
						AAH	



						（2）故障报文
						0EBH				同步头
						90H	
						0EBH	
						90H	
						目的地址			上位机地址
						源地址				装置地址
						状态量				故障特征码
						开关量状态			输入开关量状态
						年					信息段
						月	
						日	
						时	
						分	
						秒	
						LEN=09H				报文长度
						CODE=50H			命令码
						低位				代码和
						高位	
						55H					结束符
						AAH	
				   注：（1）报文长度指故障特征码、故障母线段号、报文长度和报文内容长度之和。
						（2）代码和指故障特征码、故障母线段号、报文长度、报文内容长度和命令码之和的低16位值。
						（3）故障类型： 

						11H表示谐振
						22H表示过压
						33H表示过A熔丝断
						44H表示过B熔丝断
						55H表示过C熔丝断

						66H表示过A  PT断
						77H表示过B  PT断
						88H表示过C  PT断

						99H表示过A  金属接地
						AAH表示过B  金属接地
						BBH表示过C  金属接

						CCH表示过A  弧光接地
						DDH表示过B  弧光接地
						EEH表示过C  弧光接地

						(4)输入开关量状态


						0	1	C熔丝 状态	B熔丝 状态	A熔丝 状态	C接触器状态	B接触器状态	A接触器 状态
						0	0						

						最高两位：01投运
						00 停运
							
					 （5）校验码是指故障特征码、报文长度和命令码之和的低8位。
 *
 *        Version:  1.0
 *        Created:  2014年11月19日 13时15分10秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */


#include "ModBusXiaoHuMCU.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  CModBusXHMCU
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CModBusXHMCU::CModBusXHMCU ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_bySrcAddr = 1;
	m_byDataType = 0;
}  /* -----  end of method CModBusXHMCU::CModBusXHMCU  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  ~CModBusXHMCU
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CModBusXHMCU::~CModBusXHMCU ()
{
}  /* -----  end of method CModBusXHMCU::~CModBusXHMCU  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  print
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CModBusXHMCU::print ( char *buf ) const
{

#ifdef  XHMCU_PRINT
	OutBusDebug( m_byLineNo, (BYTE *)buf, strlen( buf ), 2 );
#endif     /* -----  not XHMCU_PRINT  ----- */
}		/* -----  end of method CModBusXHMCU::print  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  GetWordSumCheck
 * Description:	 计算校验和  
 *       Input:  pBuf  缓冲区 注：pBuf的缓冲区大小 需 > len
 *				 len   需要校验的长度
 *		Return:  校验和
 *--------------------------------------------------------------------------------------
 */
WORD CModBusXHMCU::GetWordSumCheck ( BYTE *pBuf, int len  )
{
	WORD sum = 0;

	for ( int i=0; i<len; i++ )
	{
		sum += pBuf[i];
	}

	return sum;
}		/* -----  end of method CModBusXHMCU::GetWordSumCheck  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  GetSumCheck
 * Description:	 计算校验和  
 *       Input:  pBuf  缓冲区 注：pBuf的缓冲区大小 需 > len
 *				 len   需要校验的长度
 *		Return:  校验和
 *--------------------------------------------------------------------------------------
 */
BYTE CModBusXHMCU::GetSumCheck ( BYTE *pBuf, int len )
{
	return LOBYTE( GetWordSumCheck( pBuf, len ) );
}		/* -----  end of method CModBusXHMCU::GetSumCheck  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性 
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::WhetherBufValue ( BYTE *buf, int &len, int &pos )
{
	BYTE *pointer = buf;
	pos = 0;

	while( len >= 18 )
	{
		//判断同步字
		if( 0xEB != *pointer
				|| 0x90 != *( pointer + 1 )
				|| 0xEB != *( pointer + 2)
				|| 0x90 != *( pointer + 3 ) )
		{
			print( (char *)"同步字错误" );
			goto DEFAULT;
		}
		//判断地址
		if ( *( pointer + 4 ) != m_bySrcAddr
				|| *( pointer + 5 ) != m_wDevAddr)
		{
			print( (char *)"地址错误" );
			goto DEFAULT;
		}

		//判断结束码
		if( 0x55 == *( pointer + 16 ) && 0xAA == *( pointer + 17 ))
		{
			//判断报文长度
			if( 2 != *( pointer + 13 ) )
			{
				print( (char *)"报文长度错误" );
				goto DEFAULT;
			}
			//判断命令码
			if( 0x50 != *( pointer + 14 ) )
			{
				print( (char *)"命令码错误" );
				goto DEFAULT;
			}

			//判断校验码
			if( GetSumCheck( pointer + 13, 2 ) != *( pointer + 15 ) )
			{
				print( (char *)"校验码错误" );
				goto DEFAULT;
			}

			buf = buf + pos;
			len = 18;
			m_byDataType = XHMCU_NONE_TROUBLE_DATATYPE;
			return TRUE;
			
		}
		else if(  0x55 == *( pointer + 18 ) && 0xAA == *( pointer + 19 ) )
		{
			WORD crc;
			//判断报文长度
			if( 9 != *( pointer + 14 ) )
			{
				print( (char *)"报文长度错误" );
				goto DEFAULT;
			}
			//判断命令码
			if( 0x50 != *( pointer + 15 ) )
			{
				print( (char *)"命令码错误" );
				goto DEFAULT;
			}

			crc = GetWordSumCheck( pointer + 6, 10 );
			if( *( pointer + 16) != LOBYTE( crc ) 
					|| *( pointer + 17) != HIBYTE( crc ))
			{
				print( (char *)"代码和错误" );
				goto DEFAULT;
			}
			buf = buf + pos;
			len = 20;
			m_byDataType = XHMCU_TROUBLE_DATATYPE;
			return TRUE;
			
		}
		else
		{
			m_byDataType = 0;
			print( (char *)"结束码错误" );
		}

DEFAULT:
		pointer ++;
		len --;
		pos ++;
	}

	print( (char *) "CModBusXHMCU WhetherBufValue not find right buf ");

	return FALSE ;
}		/* -----  end of method CModBusXHMCU::WhetherBufValue  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  ProcessNoneTroubleData
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::ProcessNoneTroubleData ( BYTE *buf, int len )
{
	BYTE byYxByte;
	BYTE byYxValue;
	float fYcValue;
	int i;
	char szBuf[256];
	if( len != 18 )
	{
		return FALSE;
	}

	//处理yx
	byYxByte = buf[6];

	//其它yx
	for( i=0; i<6; i++ )
	{
		byYxValue = ( byYxByte >> ( i ) ) & 0x01;
		m_pMethod->SetYxData( m_SerialNo, ( WORD )i, byYxValue );
		sprintf( szBuf, "yx pnt=%d val=%d", (WORD)i, byYxValue );
		print( szBuf );
	}

	//是否投运
	if( 0x01 == ( ( byYxByte >> 6 ) & 0x03 ) )
	{
		byYxValue = 1;
		m_pMethod->SetYxData( m_SerialNo, (WORD)i, byYxValue );
	}
	else if ( 0x00 == ( ( byYxByte >> 6 ) & 0x03 ) )
	{
		byYxValue = 0;
		m_pMethod->SetYxData( m_SerialNo, (WORD)i, byYxValue );
	}
	else 
	{
		print( (char *)"投运编码错误" );
		return FALSE;
	}

	sprintf( szBuf, "yx pnt=%d val=%d", i, byYxValue );
	print( szBuf );

	for( i=7; i<21; i++ )
	{
		byYxValue = 0;
		m_pMethod->SetYxData( m_SerialNo, (WORD)i, byYxValue );
		sprintf( szBuf, "yx pnt=%d val=%d", (WORD)i, byYxValue );
		print( szBuf );
	}


	//处理yc
	for ( i=0; i<6; i++ )
	{
		fYcValue = ( float )buf[7 + i];
		m_pMethod->SetYcData( m_SerialNo, ( WORD )i, fYcValue );
		sprintf( szBuf, "yc pnt=%d val=%f", ( WORD )i, fYcValue );
		print( szBuf );
	}

	return TRUE;
}		/* -----  end of method CModBusXHMCU::ProcessNoneTroubleData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  ProcessTroubleData
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::ProcessTroubleData ( BYTE *buf, int len )
{
	BYTE byYxByte;
	BYTE byYxValue;
	WORD wPnt = 0;
	char szBuf[256];
	TIMEDATA ptmData;
	int i;
	if( len != 20 )
	{
		return FALSE;
	}

	//处理yx
	byYxByte = buf[7];

	//其它yx
	for( i=0; i<6; i++ )
	{
		byYxValue = ( byYxByte >> ( i ) ) & 0x01;
		m_pMethod->SetYxData( m_SerialNo, ( WORD )i, byYxValue );
		sprintf( szBuf, "yx pnt=%d val=%d", i, byYxValue );
		print( szBuf );
	}

	//是否投运
	if( 0x01 == ( ( byYxByte>>6 ) & 0x03 ) )
	{
		byYxValue = 1;
		m_pMethod->SetYxData( m_SerialNo, ( WORD )i, byYxValue );
	}
	else if ( 0x00 == ( ( byYxByte >> 6 ) & 0x03 ) )
	{
		byYxValue = 0;
		m_pMethod->SetYxData( m_SerialNo, (WORD)i, byYxValue );
	}
	else 
	{
		print( (char *)"投运编码错误" );
		return FALSE;
	}

	sprintf( szBuf, "yx pnt=%d val=%d", i, byYxValue );
	print( szBuf );

	//故障信息
	switch ( buf[6] )
	{
		case 0x11:	
			wPnt = 7;
			break;

		case 0x22:	
			wPnt = 8;
			break;

		case 0x33:	
			wPnt = 9;
			break;

		case 0x44:	
			wPnt = 10;
			break;

		case 0x55:	
			wPnt = 11;
			break;

		case 0x66:	
			wPnt = 12;
			break;

		case 0x77:	
			wPnt = 13;
			break;

		case 0x88:	
			wPnt = 14;
			break;

		case 0x99:	
			wPnt = 15;
			break;

		case 0xAA:	
			wPnt = 16;
			break;

		case 0xBB:	
			wPnt = 17;
			break;

		case 0xCC:	
			wPnt = 18;
			break;

		case 0xDD:	
			wPnt = 19;
			break;

		case 0xEE:	
			wPnt = 20;
			break;

		default:	
			print ( (char *)"故障特征码错误" );
			return FALSE;
			break;
	}				/* -----  end switch  ----- */

	if( !m_pMethod->IsSoeTime( 0,
				buf[13],
				buf[12],
				buf[11],
				buf[10],
				buf[9],
				buf[8]+2000) )
	{
		sprintf( szBuf, "ERROR:soe time err!!!%d-%d-%d %d:%d:%d", 
			    buf[8]+2000, buf[9], buf[10] ,
				buf[11], buf[12], buf[13]);
		print( szBuf );
		return FALSE;
	}
	ptmData.Year = buf[8] + 100;
	ptmData.Month  = buf[9];
	ptmData.Day  = buf[10];
	ptmData.Hour = buf[11];
	ptmData.Minute = buf[12];
	ptmData.Second = buf[13];


	if( 0 != wPnt )
	{
		m_pMethod->SetYxDataWithTime ( m_SerialNo,  wPnt, (BYTE)1, &ptmData );
		sprintf( szBuf, "yx soe  pnt=%d val=%d time = %d-%d-%d %d:%d:%d", 
				wPnt, 1, ptmData.Year + 1900, ptmData.Month,ptmData.Day ,
				ptmData.Hour, ptmData.Minute, ptmData.Second);
		print( szBuf );
	}

	return TRUE;
}		/* -----  end of method CModBusXHMCU::ProcessTroubleData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::ProcessRecvBuf ( BYTE *buf, int len )
{
	BOOL bRtn = FALSE;

	switch ( m_byDataType )
	{
		case XHMCU_NONE_TROUBLE_DATATYPE:	
			bRtn = ProcessNoneTroubleData( buf, len );
			break;

		case XHMCU_TROUBLE_DATATYPE:	
			bRtn = ProcessTroubleData( buf, len );
			break;

		default:	
			break;
	}				/* -----  end switch  ----- */

	m_byDataType = 0;

	return bRtn;
}		/* -----  end of method CModBusXHMCU::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度 消息（无遥控等消息内容 始终为NULL）
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	len = 0;

	//组织同步头
	buf[len++] = 0xEB;
	buf[len++] = 0x90;
	buf[len++] = 0xEB;
	buf[len++] = 0x90;

	buf[len++] = m_wDevAddr; // 目的地址 
	buf[len++] = m_bySrcAddr; // 源地址	恒写为1

	buf[len++] = 0x00; // 状态量(00) 
	buf[len++] = 0x02; // 报文长度 
	buf[len++] = 0x55; // 命令码 
	buf[len++] = GetSumCheck( &buf[6], 3 ); // 校验码 
	
	//结束码
    buf[ len++ ] = 0x55;
    buf[ len++ ] = 0xAA;

	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method CModBusXHMCU::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::ProcessProtocolBuf ( BYTE *buf, int len )
{
	int pos = 0;
	if ( !WhetherBufValue( buf, len, pos ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusXiaoHuMCU recv buf err !!!\n" );
		print( szBuf );
		m_byRecvCount ++;
		return FALSE;	
	}
	
	ProcessRecvBuf( buf + pos, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method CModBusXHMCU::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method CModBusXHMCU::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModBusXHMCU::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			print( (char *) "CModBusXHMCU:unlink\n");
		}
	}
}		/* -----  end of method CModBusXHMCU::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXHMCU::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModBusXHMCU::GetDevCommState  ----- */

