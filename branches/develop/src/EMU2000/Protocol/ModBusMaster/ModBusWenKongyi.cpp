/*
 * =====================================================================================
 *
 *       Filename:  ModBusWenKongyi.cpp
 *
 *    Description:  杨鹏超从现场发过来的干式温控仪协议  百色项目
					（1）上位机数据格式：
					地址字节—命令字节—开始地址高字节—开始地址低字节—读数据个数高字节—读数据个数低字节—校验低字节—校验高字节
					0x01(地址可变)—0X03(仅支持读数据字命令,其他为非法命令)—0X00—0X00—0X00—0X05—
					CRCLO--CRCHI
					上面的命令是从零号地址字开始读五个数据字。
					支持方式可以是：
					开始地址字（大于等于0，小于等于4）与数据个数字（大于等于1，小于等于5）。
					开始地址字的内容为：要读出的起始数据字在下位机的存储地址。读数据个数字的内容为：从起始数据字开始，连续读出数据字的个数。具体的对应关系参见后面的表格，可根据需要来确定。
					数据高位字节—数据位低字节：是数据字的分离，采用了十六进制。
					数据字=高位字节×256＋低位字节，请根据具体要求发送正确的范围代码。

					（2）下位机返回的数据格式：
					命令正确返回的数据格式为
					地址字节—命令字节—返回字节数（字乘2）—数据高字节—数据低字节—高—低….—高—低—校验低字节—校验高字节
					错误命令返回的数据格式
					1）命令错（非0X03	命令）：地址字节—命令字节—0X01—校验低字节—校验高字节
					2）功能错（0X03命令但开始地址和要求的数据长度错,CRC校验错）：地址字节—命令字节—0X02—校验低字节—校验高字节

					3数据在下位机的存储地址
					地址（字）			内容					备注
					0	状态字		（仅看低字节）
					1	A相温度字	（单位为0.1摄氏度，下同）
					2	B相温度字	
					3	C相温度字	
					4	历史最大温度值字	
					状态字   （高位） XX  XX  XX  XX（低位）
					TZ  GZ  CW  FJ
					TZ：	若置位‘11’，表示下位机已经跳闸。
					GZ：	若置位‘11’，表示下位机发生故障。
					CW：	若置位‘11’，表示下位机已经超温。
					FJ：	若置位‘11’，表示下位机已经启动风机。
					4 举例说明
					上位机想要读取该仪表的所有信息，发送数据为：
					0X01—0X03—0X00—0X00—0X00—0X05—0X85—0XC9
					此时下位机返回数据为：
					0X01—0X03—0X0A—0X00—0X03—0X00—0XFD—0X01—0X02—0X01—0X00—0X01—
					0XC2—0X F1—0X95
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


#include "ModBusWenKongyi.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  CModbusWenKongYi
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CModbusWenKongYi::CModbusWenKongYi ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
}  /* -----  end of method CModbusWenKongYi::CModbusWenKongYi  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  ~CModbusWenKongYi
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CModbusWenKongYi::~CModbusWenKongYi ()
{
}  /* -----  end of method CModbusWenKongYi::~CModbusWenKongYi  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性 
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusWenKongYi::WhetherBufValue ( BYTE *buf, int &len )
{
	BYTE *pointer = buf;
	WORD wCrc;
	int pos = 0;

	while( len >= 4 )
	{
		//判断地址
		if ( *pointer != m_wDevAddr )
		{
			goto DEFAULT;
		}

		//判断功能码
		if ( *( pointer + 1 ) != 0x03 )
		{
			goto DEFAULT;
		}

		//判断校验
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
}		/* -----  end of method CModbusWenKongYi::WhetherBufValue  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusWenKongYi::ProcessRecvBuf ( BYTE *buf, int len )
{
	if( len < 15 )
	{
		return FALSE;
	}

	for ( int i=0; i<4; i++)
	{
		BYTE byYxValue = 0;
		BYTE byYxBit = 0;
		byYxBit = ( buf[4] >> ( 2 * i ) ) & 0x03 ;
		if ( byYxBit == 0x03 )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		m_pMethod->SetYxData( m_SerialNo, i, byYxValue );
		// printf ( "yx pnt=%d val=%d\n", i, byYxValue );
	}

	for ( int i=0; i<4; i++)
	{
		float fYcValue;	
		fYcValue = (float)( MAKEWORD( buf[6 + 2 * i], buf[ 5 + 2 * i ] ) );

		m_pMethod->SetYcData( m_SerialNo, i, fYcValue );
		// printf ( "yc pnt=%d val=%f\n", i, fYcValue );
	}

	return TRUE;
}		/* -----  end of method CModbusWenKongYi::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度 消息（无遥控等消息内容 始终为NULL）
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusWenKongYi::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
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
	buf[len++] = 0x05;
	
	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);

	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method CModbusWenKongYi::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusWenKongYi::ProcessProtocolBuf ( BYTE *buf, int len )
{
	if ( !WhetherBufValue( buf, len ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusWenKongyi recv buf err !!!\n" );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );

		m_byRecvCount ++;
		return FALSE;	
	}
	
	ProcessRecvBuf( buf, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method CModbusWenKongYi::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusWenKongYi::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method CModbusWenKongYi::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModbusWenKongYi::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			OutBusDebug( m_byLineNo, (BYTE *)"CModbusWenKongYi:unlink\n", 30, 2 );
		}
	}
}		/* -----  end of method CModbusWenKongYi::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusWenKongYi
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusWenKongYi::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModbusWenKongYi::GetDevCommState  ----- */

