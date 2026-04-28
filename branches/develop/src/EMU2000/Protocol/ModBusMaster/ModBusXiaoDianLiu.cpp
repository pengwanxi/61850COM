/*
 * =====================================================================================
 *
 *       Filename:  ModBusXiaoDianLiu.cpp
 *
 *    Description:  王孟华从现场发过来的小电流接地选线保护装置  厦门正新集美厂
					二、数据交换界面
						1、RTU帧格式
							地址	功能码	数据	效验
							8-BITS	8-BITS	N*8-BITS	16-BITS
						2．CRC-16校验   X16+X15+X2+1
						3．数据请求帧（下行）
							按遥测量上送接地数据
							装 置 地 址	 1字节	功能码 1字节	数据起 始地址 （高位）	数据起 始地址 （低位）	请求数 据字数 （高位）	请求数 据字数 （低位）	CRC （高位）	CRC （低位）
							1~0FEH					04				00					00						00						36							CRC校验范围

							注释：其中数据起始地址和数据字数均表示取得的数据是字为单位而不是字节为单位。
						4．数据响应帧（上行）
						装置地址，1字节	功能码 1字节	数据字节数，1字节	数据0….	数据N	CRC （高位）	CRC （低位）
						1~0FEH			04				DataLen				Data					CRC校验范围

						三、规约的使用
						1、本规约只提供故障事件的查询，查询故障事件帧的类型为04。
							本规约提供一个事件缓冲区，如下：
							序号	事件列表
							1	母线1下的故障事件，包括（接地故障、母线故障、电压告警）
							2	母线2下的故障事件，同上
							3	母线3下的故障事件，同上
							4	母线4下的故障事件，同上
							5	历史故障事件1
							6	历史故障事件2
							……	……
							36	历史故障事件32
							1-4事件默认为4段母线下的当前实时故障事件，5-36为32个历史故障事件。（每个事件帧占内存 18个字节，即9个字）
						事件帧的内存格式说明，如下：（每个事件帧占内存 18个字节，即9个字）
						ID		1	2	3	4	5	6	7	8
						字节数	2	2	1	1	1	1	1	1
						位描述	故障线路号	故障时刻母线电压值	故障母线号	故障类型	SSec	SMin	SHour	SDay

						9	10	11	12	13	14	15	16
						1	1	1	1	1	1	1	1
						Smon	SYear	ESec	EMin	EHour	EDay	EMon	Eyear

						事件帧详细说明：
						*故障线路号：4-51只是线路序号（占内存2个字节），低位在前，高位在后
						*故障时刻母线电压值： （占内存2个字节）低位在前，高位在后
						*故障母线号：0-3代表母线的序号
						*故障类型：0-无故障,1-母线故障，2-零序电压过高报警，3-接地故障
						*其中带S的时间为故障起始时间，E的时间为结束时间。1-4段母线下的当前故障没有结束时间。
						2、 数据字节数不含本身及CRC字节。
						3、 CRC对除本身以外的所有字节从“装置地址”起校验。
						4、 本规约为问答式，无论装置有无接地事件被检测到，只要收到数据请求，均上送数据。
 *
 *        Version:  1.0
 *        Created:  2015年04月22日 11时13分29秒 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */


#include "ModBusXiaoDianLiu.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  CModBusXDL
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CModBusXDL::CModBusXDL ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
}  /* -----  end of method CModBusXDL::CModBusXDL  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  ~CModBusXDL
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CModBusXDL::~CModBusXDL ()
{
}  /* -----  end of method CModBusXDL::~CModBusXDL  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  print
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CModBusXDL::print ( char *buf )
{
#ifdef  XDL_PRINT
	OutBusDebug( m_byLineNo, (BYTE *)buf, strlen( buf ), 2 );
#endif     /* -----  not XDL_PRINT  ----- */
}		/* -----  end of method CModBusXDL::print  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性 
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXDL::WhetherBufValue ( BYTE *buf, int &len ,int &pos)
{
	BYTE *pointer = buf;
	WORD wCrc;
	pos = 0;

	while( len >= 77 )
	{
		//判断地址
		if ( *pointer != m_wDevAddr )
		{
			print( (char *)"CModBusXDL WhetherBufValue addr error" );
			goto DEFAULT;
		}

		//判断功能码
		if ( *( pointer + 1 ) != 0x04 )
		{
			print( (char *)"CModBusXDL WhetherBufValue code error" );
			goto DEFAULT;
		}

		//判断校验
		wCrc = GetCrc( pointer, ( *( pointer + 2 ) + 3 ) );
		if( *( pointer + ( *( pointer + 2 ) + 3 ) ) !=  HIBYTE(wCrc)
		 || *( pointer + ( *( pointer + 2 ) + 4 ) ) !=  LOBYTE(wCrc))
		{
			print( (char *)"CModBusXDL WhetherBufValue crc error" );
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

	print( (char *) "CModBusXDL WhetherBufValue not find right buf ");

	return FALSE ;
}		/* -----  end of method CModBusXDL::WhetherBufValue  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXDL::ProcessRecvBuf ( BYTE *buf, int len )
{
	char szBuf[256];
	if( len < 77 )
	{
		return FALSE;
	}

	for ( int i=0; i<4; i++)
	{
		BYTE byYxValue = 0;
		BYTE byYxByte = 0;
		BYTE byYxPnt;

		byYxPnt = i*3;
		byYxByte = buf[2 + i * 18 + 6];

		if( 0 == byYxByte )
		{
			byYxValue = 0;
			m_pMethod->SetYxData( m_SerialNo, byYxPnt, byYxValue );
			sprintf( szBuf,"yx %d value=%d", byYxPnt , byYxValue   );
			print( szBuf );
			m_pMethod->SetYxData( m_SerialNo, byYxPnt + 1, byYxValue );
			sprintf( szBuf,"yx %d value=%d", byYxPnt + 1, byYxValue   );
			print( szBuf );
			m_pMethod->SetYxData( m_SerialNo, byYxPnt + 2, byYxValue );
			sprintf( szBuf,"yx %d value=%d", byYxPnt + 2, byYxValue   );
			print( szBuf );
		}
		else if( byYxByte > 0 && byYxByte < 4 )
		{
			byYxPnt = i * 3 + byYxByte - 1 ;
			byYxValue = 1;

			m_pMethod->SetYxData( m_SerialNo,  byYxPnt, byYxValue );
			sprintf( szBuf,"yx %d value=%d", byYxPnt, byYxValue   );
			print( szBuf );
		}
		else
		{
			char szBuf[256] = "";
			sprintf( szBuf, "ModBusXiaoDianLiu ProcessRecvBuf err !!! errtype > %d \n", byYxByte );
			print( szBuf );
			return FALSE;
		}

		// printf ( "yx pnt=%d val=%d\n", i, byYxValue );
	}

	return TRUE;
}		/* -----  end of method CModBusXDL::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度 消息（无遥控等消息内容 始终为NULL）
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXDL::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	len = 0;

	buf[len++] = m_wDevAddr;
	buf[len++] = 0x04;

	buf[len++] = 0x00;
	buf[len++] = 0x00;
	buf[len++] = 0x00;
	buf[len++] = 0x36;
	
	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);

	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method CModBusXDL::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXDL::ProcessProtocolBuf ( BYTE *buf, int len )
{
	int pos = 0;
	if ( !WhetherBufValue( buf, len, pos ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusXiaoDianLiu recv buf err !!!\n" );
		print( szBuf );

		m_byRecvCount ++;
		return FALSE;	
	}
	
	ProcessRecvBuf( buf+pos, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method CModBusXDL::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXDL::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method CModBusXDL::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModBusXDL::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			print( ( char * ) "CModBusXDL:unlink\n");
		}
	}
}		/* -----  end of method CModBusXDL::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXDL
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModBusXDL::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModBusXDL::GetDevCommState  ----- */

