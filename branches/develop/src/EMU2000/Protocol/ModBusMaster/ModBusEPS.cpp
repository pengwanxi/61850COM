/*
 * =====================================================================================
 *
 *       Filename:  ModBusEPS.cpp
 *
 *    Description:百色现场， 从厂家要过来的协议， 具体如下
 ///////////////////////////////////////////////////////////////////////////////////////
// 上位机读12864屏数据协议
 //上位机查询设备状态的命令解析
// 上位机发送以下命令查询状态：
// AAN 03 AAH AAL 00 04 CRCL CRCH //AAH、AAL代表需要查询的内容，解释如下。编码格式见最后

// AAN：设备号，即当多个设备同时连接到一个串口上时，使用。
// 例如：对设备5进行操作，则AAN=0x05。
// 设备号是出厂时对产品的设置，本协议内无指令可以改变。


// AAH AAL：代表地址码，如下表
  // 0	//系统状态    //=0的时候比较特别，回复的数据中每一位有特定含义，具体看下一小结
  // 1	//市电电压A 
  // 2	//市电电压B 
  // 3	//市电电压C 
  // 4	//输出电压A  
  // 5	//输出电压B 
  // 6	//输出电压C  
  // 7	//输出电流A  
  // 8	//输出电流B 
  // 9	//输出电流C 
  // 10	//电池电压  
  // 11	//电池1电压
  // 12	//电池2电压
  // 13	//电池3电压
  // 14	//电池4电压
  // 15	//电池5电压
  // 16	//电池6电压
  // 17	//电池7电压
  // 18	//电池8电压
  // 19	//电池9电压
  // 20	//电池10电压
  // 21	//电池11电压
  // 22	//电池12电压
  // 23	//电池13电压
  // 24	//电池14电压
  // 25	//电池15电压
  // 26	//电池16电压
  // 27	//充电电流
  // 28	//输出功率
  // 29	//输出频率	
        // 30 //命令帧数据 以下命令是上位机为设备设置参数会用到的		 
	// 31 //市电电压A系数：1.000  
	// 32 //市电电压A偏差：0.00
	// 33 //市电电压B系数：1.000
	// 34 //市电电压B偏差：0.00
	// 35 //市电电压C系数：1.000
	// 36 //市电电压C偏差：0.00

	// 37 //输出电压A系数：1.000  
	// 38 //输出电压A偏差：0.00
	// 39 //输出电压B系数：1.000
	// 40 //输出电压B偏差：0.00
	// 41 //输出电压C系数：1.000
	// 42 //输出电压C偏差：0.00

	// 43 //输出电流A系数：1.000  
	// 44 //输出电流A偏差：0.00
	// 45 //输出电流B系数：1.000
	// 46 //输出电流B偏差：0.00
	// 47 //输出电流C系数：1.000
	// 48 //输出电流C偏差：0.00
	
	// 49 //总电池电压系数：1.000
	// 50 //总电池电压偏差：0.00

	// 51 //1号系数：1.000
	// 52 //1号偏差：0.00
	// 53 //2号系数：1.000
	// 54 //2号偏差：0.00
	// 55 //3号系数：1.000
	// 56 //3号偏差：0.00
	// 57 //4号系数：1.000
	// 58 //4号偏差：0.00


	// 59 //电池安时：100A
	// 60 //充电电流系数：1.000
	// 61 //充电电流偏差：0.00

	// 62 //电池数量4
	// 63 //系统功率：1.00
	// 64 //开关机指令：1.00

	// 67 //开机1  关机0   手动1 自动0    不联动0 联动1  不强启0 强启1


// *************************以下为查询回复命令解析**************************************

// 设备回复以下命令，返回被查询状态：
// AAN 03 04 DA3 DA4 DA1 DA2 CRCL CRCH //这里面包含状态数据，注意！这里的数据顺序是3412

// DA1 DA2 DA3 DA4:四个字节为返回的数据
// CRCH CRCL：16位CRC检验

// 对于返回数据DA1~DA4，这里分为两部分解释说明。
/////////////////////////AAH AAL为0时的解析///////////////////////////////////////////
// 当AAH\AAL为0时，即查询系统状态，返回数据格式如下：

// DA1    
// bit0—bit3    系统状态  
              // 0 关机 
              // 1	市电
              // 2	逆变
              // 3	应急
              // 4	手动
              // 5	过载
              // 6	过流
              // 7	过温
              // 8	联动
              // 9	强启
              // 10	错误
              // 11	月检
              // 12	年检
              // 13	充电
// Bit4—bit5   	市电状态
              // 0	正常
              // 1	欠压
              // 2	过压
              // 3	开路
// Bit6—bit7   电池状态
              // 0	正常
              // 1	欠压
              // 2	过压
              // 3	开路
            
// DA2       
// bit0—bit1   输出状态
            // 0	正常
            // 1	过载
            // 2	开路
            // 3	错误
// Bit2—bit3    巡检状态
              // 0	正常
              // 1	欠压
              // 2	过压
              // 3	开路
// Bit4—bit5   充电状态
            // 0	正常
            // 1	开路
// Bit6---bit7   月检状态
               // 0 正常
               // 1 失败
               // 2 未完成
// DA3
// bit0---bit1   年检状态
              // 0 正常
              // 1 失败
              // 2 未完成
// Bit2---bit3   报警音 
              // 0	无报警音
              // 1	报警音
// DA4
             // Bit4          逆变灯     0  关闭 ，1 打开LED3
             // Bit5          市电灯     0  关闭   1 打开LED1
             // Bit6          充电灯     0  关闭   1 打开LED2
             // Bit7          故障灯     0  关闭   1 打开LED4
/////////////////////////AAH AAL为非0时的解析///////////////////////////////////////////

// 非0时候查询的是电压、电流等数值，因此计算公式如下：
// 首先将四位数据按照 DA1 DA2 DA3 DA4 组成一个U32的数值，设为X

// 实际电压值=X/1000

// 这个公式适用于所有数值查询的情况，使用的时候注意收到的数据顺序，
// 例如，收到数据为01 03 04 0x11 0x22 0x33 0x44 CRCL CRCH
// 则实际电压=0x22331122/1000 伏


///////////以 上 是上位机 查询 设备状态及状态回复的说明///////////////////////
//////////////以 下 是上位机 写入 设备状态及状态回复的说明///////////////////
// 上位机回数据到12864协议

// 上位机发：AAN 10 AAH AAL 00 02 04 DA3 DA4 DA1 DA2 CRCL CRCH
// 12864回发：AAN 10 AAH AAL 00 02 04 DA3 DA4 DA1 DA2 CRCL CRCH

// AAH AAL：地址码，见上面列表30-63
// DA1 DA2 DA3 DA4:四个字节为返回的数据
// CRCH CRCL：16位CRC检验
// 同样，写入的时候需要注意DA的顺序，

// 例如，你要设置某电压为16797V，则首先将16797*1000=16797000，
// 再将20000转换为U32的数据=0x01004D48，则上位机应该发送以下命令给设备：
// AAN 10 AAH AAL 00 02 04 4D 48 01 00 CRCL CRCH

 *
 *        Version:  1.0
 *        Created:  2014年11月24日 09时40分06秒
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

#include "ModBusEPS.h"
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  CModbusEPS
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CModbusEPS::CModbusEPS ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_bySendPos = 0;
}  /* -----  end of method CModbusEPS::CModbusEPS  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  ~CModbusEPS
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CModbusEPS::~CModbusEPS ()
{
}  /* -----  end of method CModbusEPS::~CModbusEPS  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  WhetherBufValue
 * Description:  查看接收报文有效性 
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::WhetherBufValue ( BYTE *buf, int &len )
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
}		/* -----  end of method CModbusEPS::WhetherBufValue  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  ProcessYxBuf
 * Description:  特殊处理遥信数据
 *       Input:	 接受缓冲区 长度
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::ProcessYxBuf ( BYTE *buf, int len )
{
	WORD wPnt = 0;
	BYTE byYxValue = 0;
	BYTE byTemp;
	int i;
	char szBuf[256];
	/* 处理DA1 */
	// 处理DA1 bit0-bit3  0-13
	byTemp = buf[5] & 0x0f;	
	for( i = 0; i < 14; i++)
	{
		if( byTemp >= 14)
		{
			sprintf( szBuf, "yx update point=0 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 0, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );

		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}
	//处理 DA1 bit4-bit5 14-17
	byTemp = ( buf[5] >> 4 ) & 0x03;
	for ( i=0; i<4; i++)
	{
		if( byTemp >= 4)
		{
			sprintf( szBuf, "yx update point=14 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 14, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}
		
		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );

		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}
	
	//处理 DA1 bit6-bit7 18-21
	byTemp = ( buf[5] >> 6 ) & 0x03;
	for ( i=0; i<4; i++)
	{
		if( byTemp >= 4)
		{
			sprintf( szBuf, "yx update point=18 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 18, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );

		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	/* 处理数据DA2 buf[6] */
	//处理DA2 bit0-bit1 22-25
	byTemp =  buf[6]  & 0x03;
	for ( i=0; i<4; i++)
	{
		if( byTemp >= 4)
		{
			sprintf( szBuf, "yx update point=22 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 22, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	//处理DA2 bit2-bit3 26-29
	byTemp = (buf[6] >> 2)  & 0x03;
	for ( i=0; i<4; i++)
	{
		if( byTemp >= 4)
		{
			sprintf( szBuf, "yx update point=26 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 26, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	//处理DA2 bit4-bit5 30-31
	byTemp = (buf[6] >> 4)  & 0x03;
	for ( i=0; i<2; i++)
	{
		if( byTemp >= 2)
		{
			sprintf( szBuf, "yx update point=30 val=1"  );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 30, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	//处理DA2 bit6-bit7 32-34
	byTemp = (buf[6] >> 6)  & 0x03;
	for ( i=0; i<3; i++)
	{
		if( byTemp >= 3)
		{
			sprintf( szBuf, "yx update point=32 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 32, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	/* 处理DA3 buf[3] */
	//处理DA3 bit0-bit1 35-37
	byTemp = ( buf[3] )  & 0x03;
	for ( i=0; i<3; i++)
	{
		if( byTemp >= 3)
		{
			sprintf( szBuf, "yx update point=35 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 35, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	//处理DA3 bit2-bit3 38-39
	byTemp = (buf[3] >> 2)  & 0x03;
	for ( i=0; i<2; i++)
	{
		if( byTemp >= 2)
		{
			sprintf( szBuf, "yx update point=38 val=%d",  byYxValue );
			OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
			m_pMethod->SetYxData( m_SerialNo, 38, 1 );
			wPnt ++;
			continue;
		}
		if( byTemp == i )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	/* 处理DA4 buf[4] */
	//处理DA4 bit4 bit5 bit6 bit7 40-43
	for ( i=0; i<4; i++)
	{
		byTemp = ( buf[4] >> (4 + i) ) ;
		if( byTemp & 0x01 )
		{
			byYxValue = 1;
		}
		else
		{
			byYxValue = 0;
		}

		sprintf( szBuf, "yx update point=%d val=%d", wPnt, byYxValue );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
		m_pMethod->SetYxData( m_SerialNo, wPnt, byYxValue );
		wPnt ++;
	}

	return TRUE;
}		/* -----  end of method CModbusEPS::ProcessYxBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  ProcessRecvBuf
 * Description:  处理报文  更新数据
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::ProcessRecvBuf ( BYTE *buf, int len )
{
	BYTE byPos;
	//判断长度
	if( len != 9 )
	{
		return FALSE;
	}

	byPos = GetSendPos(  );

	//位置是0 为遥信， 其他为遥测
	if( 0 == byPos)
	{
			
		return ProcessYxBuf( buf, len );
	}
	else
	{
		WORD wLWord;
		WORD wHWord;
		DWORD dwYcVal;
		float fYcVal;

		wHWord = MAKEWORD( buf[6], buf[5] );
		wLWord = MAKEWORD( buf[4], buf[3] );
		dwYcVal = MAKELONG( wLWord, wHWord );
		fYcVal = (float)dwYcVal;
		fYcVal = fYcVal * 0.001;

		if( fYcVal > 100000 )
		{
			return TRUE;
		}

		char szBuf[256];
		sprintf( szBuf, "yc update point=%d val=%f", byPos-1, fYcVal );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );

		m_pMethod->SetYcData( m_SerialNo, byPos-1, fYcVal );
	}

	return TRUE;
}		/* -----  end of method CModbusEPS::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  GetSendPos
 * Description:  获取发送位置
 *       Input:  void 
 *		Return:  BYTE 发送位置
 *--------------------------------------------------------------------------------------
 */
BYTE CModbusEPS::GetSendPos ( void )
{
	return ( m_bySendPos % MODBUSMASTER_EPS_MAX_POS );
}		/* -----  end of method CModbusEPS::GetSendPos  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  ChangeSendPos
 * Description:  改变发送位置
 *       Input:	 void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModbusEPS::ChangeSendPos ( void )
{
	m_bySendPos = m_bySendPos++ % MODBUSMASTER_EPS_MAX_POS;

	if( m_bySendPos >= ( MODBUSMASTER_EPS_MAX_POS ) )
	{
		m_bySendPos = 0;
	}

}		/* -----  end of method CModbusEPS::ChangeSendPos  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  GetProtocolBuf
 * Description:  获取发送报文  
 *       Input:  缓冲区 长度 消息（无遥控等消息内容 始终为NULL）
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	if( pBusMsg != NULL )
	{
		return FALSE;	
	}

	ChangeSendPos(  );

	len = 0;

	buf[len++] = m_wDevAddr;
	buf[len++] = 0x03;

	buf[len++] = 0x00;
	buf[len++] = m_bySendPos;
	buf[len++] = 0x00;
	buf[len++] = 0x04;
	
	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);

	m_bySendCount ++;

	return TRUE;
}		/* -----  end of method CModbusEPS::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  ProcessProtocolBuf
 * Description:  处理接收报文
 *       Input:  缓冲区 长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::ProcessProtocolBuf ( BYTE *buf, int len )
{
	if ( !WhetherBufValue( buf, len ) )
	{
		char szBuf[256] = "";
		sprintf( szBuf, "%s",  "ModBusEPS recv buf err !!!\n" );
		OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen( szBuf ), 2 );

		m_byRecvCount ++;
		return FALSE;	
	}
	
	ProcessRecvBuf( buf, len );

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;

	return TRUE;
}		/* -----  end of method CModbusEPS::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  Init
 * Description:  初始化协议
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::Init ( BYTE byLineNo )
{
	return TRUE;
}		/* -----  end of method CModbusEPS::Init  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  TimerProc
 * Description:  时钟处理
 *       Input:  void
 *		Return:  void
 *--------------------------------------------------------------------------------------
 */
void CModbusEPS::TimerProc ( void )
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		// m_bySendPos = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			OutBusDebug( m_byLineNo, (BYTE *)"CModbusEPS:unlink\n", 30, 2 );
		}
	}
}		/* -----  end of method CModbusEPS::TimerProc  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CModbusEPS
 *      Method:  GetDevCommState
 * Description:  获取装置状态
 *       Input:  void
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CModbusEPS::GetDevCommState ( void )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method CModbusEPS::GetDevCommState  ----- */



