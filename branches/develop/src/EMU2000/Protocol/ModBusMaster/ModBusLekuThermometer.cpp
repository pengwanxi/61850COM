/*
 * =====================================================================================
 *
 *       Filename:  ModBusLekuThermometer.cpp
 *
 *    Description:  肖青云拜耳医药现场，上海乐库变压器用电子温度计
				
				1.  通讯设置说明
				    本协议采用 MODBUS 通讯协议的 RTU 方式；
				    ●  采用 RS485 异步通讯的数据传送格式；
				    ●  设置波特率 9600bps或4800bps、2400bps，无奇偶校验，8位数据位，1位停止位；
				    ●  帧校验采用 CRC16 校验方式；
				    ●  通讯距离＜1200m/双绞线（与温控器数量及通讯线路有关）；

				2.  数据格式
					a.  上位机下传数据格式
						发送次序	说明	赋值	备注
							1	仪器地址	01H－C7H	
							2	功能代码	03H	为固定格式
							3	起始地址字  高8位	00H	
							4	起始地址字  低8位	00H	
							5	读数据的个数字  高8位	00H	
							6	读数据的个数字  低8位	06H	
							7	CRC 校验  低8位	××H	
							8	CRC 校验  高8位	××H	

					b.  下位机上传数据格式
						发送次序	说明	赋值	备注
							1	仪器地址	01H－C7H	
							2	功能代码	03H	
							3	返回数据的字节数	0CH	返回数据的字节×2
							4	仪器状态字  高8位	00H	
							5	仪器状态字  低8位	××H	状态字定义见下表
							6	A相温度数据  高8位	00H	单位1℃，所得数据减23H即为实测温度
							7	A相温度数据  低8位	××H	
							8	B相温度数据  高8位	00H	
							9	B相温度数据  低8位	××H	
							10	C相温度数据  高8位	00H	
							11	C相温度数据  低8位	××H	
							12	风机定时启动时间 高8位	00H	为“0”，表示已取消风机定时启动时间
							13	风机定时启动时间 低8位	××H	
							14	D相温度数据  高8位	00H	无该相温度检测功能时，此数据无效
							15	D相温度数据  低8位	××H	
							16	CRC 校验  低8位	××H	
							17	CRC 校验  高8位	××H	

					c.仪器状态字低8位定义
					第0位＝“0”，表示A相传感器工作正常；第0位＝“1”，表示A相传感器有故障
					第1位＝“0”，表示B相传感器工作正常；第1位＝“1”，表示B相传感器有故障
					第2位＝“0”，表示C相传感器工作正常；第2位＝“1”，表示C相传感器有故障
					第3位＝“0”，表示风机未启动        ；第3位＝“1”，表示风机已启动
					第4位＝“0”，表示变压器绕组未超温  ；第4位＝“1”，表示变压器绕组已超温
					第5位＝“0”，表示未输出跳闸信号    ；第5位＝“1”，表示已输出跳闸信号
					第6位＝“0”，表示D相传感器工作正常；第6位＝“1”，表示D相传感器有故障
					第7位＝“0”，不使用

					d.总线静止时间要求
						发送数据前要求数据总线静止时间即无数据发送时间大于（5ms），近似9600波特率发送5个字节数据的时间。

					e.举例说明
						当前有一台下位机，所设地址为“1”， A相温度为64℃，B相温度为54℃，C相温度为68℃，风机定时启动时间为“24”小时，D相温度为65℃，Pt100传感器无故障，风机未启动，没有输出超温报警和跳闸信号。
						上位机发送的数据为：
							01H－03H－00H－00H－00H－06H－C5H－C8H
				         	此时下位机回复的数据为：
				          		01－03H－0CH－00H－00H－00H－63H－00H－59H－00H－67H－00H－18H－00H－64H－52H－A3H

				3.  通讯地址的设置
				      每台仪器须设置通讯地址，如果用户的一台PC机同时监控多台仪器，每台仪器须设置不同的通讯地址。
 *        Version:  1.0
 *        Created:  2016年04月21日 09时45分00秒
 *       Revision:  none
 *       Compiler:  
 *
 *         Author:  ykk
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */


#include "ModBusLekuThermometer.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  CModBusXHMCU
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
ModBusLekuThermometer::ModBusLekuThermometer ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
}  /* -----  end of method CModBusXHMCU::CModBusXHMCU  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModBusXHMCU
 *      Method:  ~CModBusXHMCU
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
ModBusLekuThermometer::~ModBusLekuThermometer ()
{
}  /* -----  end of method CModBusXHMCU::~CModBusXHMCU  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  print
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void ModBusLekuThermometer::print ( char *buf ) const
{

#ifdef  XHMCU_PRINT
	OutBusDebug( m_byLineNo, (BYTE *)buf, strlen( buf ), 2 );
#endif     /* -----  not XHMCU_PRINT  ----- */
}		/* -----  end of method CModBusXHMCU::print  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性 
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLekuThermometer::WhetherBufValue ( BYTE *buf, int &len, int &pos )
{
	BYTE *pointer = buf;
	pos = 0;

	while( len >= 17 )
	{
		//判断同步字
		if( m_wDevAddr != *pointer
				|| 0x03 != *( pointer + 1 )
				|| 0x0c != *( pointer + 2 ) )
		{
			print( (char *)"报文头" );
			pointer ++;
			len --;
			pos ++;
			continue;
		}
		//判断地址
		WORD wCRC = GetCrc( buf, buf[2] + 3 );
		if( ( HIBYTE(wCRC) == buf[ buf[2] + 3 ] ) && ( LOBYTE(wCRC) == buf[ buf[2] + 4 ] ) )
		{
			return TRUE;
		}
		else
		{
			print( (char *)"校验未通过" );
			pointer ++;
			len --;
			pos ++;
			continue;
		}
	}

	print( (char *) "CModBusXHMCU WhetherBufValue not find right buf ");

	return FALSE ;
}		/* -----  end of method ModBusLekuThermometer::WhetherBufValue  ----- */

	
/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLekuThermometer::ProcessRecvBuf ( BYTE *buf, int len )
{
	BYTE i;
	BYTE Temp_Yx = buf[4];
	WORD wVal;
	for( i=0;i<8;i++ )
	{
		wVal = Temp_Yx%2;
		m_pMethod->SetYxData ( m_SerialNo , i , wVal );
		Temp_Yx /= 2;
	}
	for( i=0;i<5;i++ )
	{
		wVal = (buf[5+(i*2)]<<8 | buf[6+(i*2)]);
		m_pMethod->SetYcData( m_SerialNo , i , (float)wVal );
	}
	
	return TRUE;
}		/* -----  end of method ModBusLekuThermometer::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度 消息（无遥控等消息内容 始终为NULL）
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLekuThermometer::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	len = 0;

	//组织同步头
	buf[len++] = m_wDevAddr;
	buf[len++] = 0x03;
	buf[len++] = 0x00;
	buf[len++] = 0x00;// 寄存器地址 
	buf[len++] = 0x00; 
	buf[len++] = 0x06; // 寄存器个数

	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);
	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method ModBusLekuThermometer::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLekuThermometer::ProcessProtocolBuf ( BYTE *buf, int len )
{
	int pos = 0;
	if ( !WhetherBufValue( buf, len, pos ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusLekuThermometer recv buf err !!!\n" );
		print( szBuf );
		m_byRecvCount ++;
		return FALSE;	
	}
	
	ProcessRecvBuf( buf + pos, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method ModBusLekuThermometer::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLekuThermometer::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method ModBusLekuThermometer::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void ModBusLekuThermometer::TimerProc ( void )
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
}		/* -----  end of method ModBusLekuThermometer::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  ModBusLekuThermometer
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL ModBusLekuThermometer::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method ModBusLekuThermometer::GetDevCommState  ----- */

