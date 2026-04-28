/*
 * =====================================================================================
 *
 *       Filename:  CIEC103_ZN.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2014年10月09日 09时29分57秒
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
//		 SQ=0 寻址一个信息体内顺序的信息元素（用于被测值和被记录的扰动表）
//		 SQ=1 寻址单个信息元素或综合信息元素（由信息体地址寻址的单个信息元素或综合信息元素）
/* COT */
//		 <1>：=自发（突发）
//		 <2>：=循环
//		 <3>：=复位帧计数位（FCB）
//		 <4>：=复位通信单元（CU）
//		 <5>：=启动/重新启动
//		 <6>：=电源合上
//		 <7>：=测试模式
//		 <8>：=时间同步
//		 <9>：=总查询（总召唤）
//		 <10>：=总查询（总召唤）终止
//		 <11>：=当地操作
//		 <12>：=远方操作
//	     <20>：=命令的肯定认可
//		 <21>：=命令的否定认可
//		 <31>：=扰动数据的传送
//		 <40>：=通用写命令的肯定认可
//		 <1>：=通用写命令的否定认可
//		 <2>：=对通用读命令有效数据响应
//		 <3>：=对通用读命令无效数据响应
//		 <4>：=通用写确认
#include "IEC103_zhongneng.h"
#include "../../share/global.h"



#define	IEC103DEBUG		1	//[> 终端打印 <]
#define	IEC103BUSDEBUG			        /* 总线打印 */
#define	IEC103DISPLAYCOT			/* 显示传送原因  */


extern "C" void GetCurrentTime( REALTIME *pRealTime );
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  CIEC103_ZN
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CIEC103_ZN::CIEC103_ZN ()
{/*{{{*/
	InitProtocolStatus(  );
}  /* -----  end of method CIEC103_ZN::CIEC103_ZN  (constructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  ~CIEC103_ZN
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CIEC103_ZN::~CIEC103_ZN ()
{/*{{{*/
	
}  /* -----  end of method CIEC103_ZN::~CIEC103_ZN  (destructor)  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  print
 * Description:  打印
 *       Input:	 缓存区 长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC103_ZN::print ( char *buf, int len )
{/*{{{*/
#ifdef  IEC103DEBUG
	printf ( "%s\n", buf );
#endif     /* -----  not IEC103DEBUG  ----- */

#ifdef  IEC103BUSDEBUG
	OutBusDebug( m_byLineNo, (BYTE *)buf, strlen(buf), 2 );
#endif     /* -----  not IEC103BUSDEBUG  ----- */
}		/* -----  end of method CIEC103_ZN::print  ----- *//*}}}*/



/*
 * -------------------------------------------------------------------------------------------------
 * class:	CIEC103_ZN
 * funct:	M_GD_NA_3_Frame
 * descr:	通用分类数据
 * param:	para0:接收帧 para1:帧长
 * (帧从通用分类数据集数目字段后被截断，长度也相应变小!)
 * retur:	BOOL
 * -------------------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::M_GD_NA_3_Frame(BYTE *pbuf, int len)
{/*{{{*/
	
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
	BYTE byVSQ = pbuf[7]; //可变结构限定词
	BYTE byCOT = pbuf[8]; //传输原因
	BYTE byAddr = pbuf[9]; //公共地址
	BYTE byFunc = pbuf[10]; //功能类型
	BYTE byInfo = pbuf[11];  //信息序号
	BYTE byRII = pbuf[12]; //返回信息标识符
	BYTE byNOG = pbuf[13]; //通用分类标识数目
	BYTE byNumStruct = byNOG & 0x3f;

	printf("byNumStruct = %d \n", byNumStruct);
	BYTE offset = 10;
	BYTE byGNo = 0xFF;
	

	for (int i = 0; i < byNumStruct; i++)
	{
		//获取数据
		int iOffset = i * 10;
		BYTE byGinLo = pbuf[14 + iOffset]; //通用分类标识序号低 组号
		BYTE byGinHi = pbuf[15 + iOffset]; //通用分类标识序号高 条目号
		BYTE byKOD = pbuf[16 + iOffset]; //描述类型
		BYTE byDataType = pbuf[17 + iOffset]; //数据类型
		BYTE byDataSize = pbuf[18 + iOffset]; //数据宽度
		BYTE byNum = pbuf[19 + iOffset]; //数据数目
		printf("g = %d item = %d\n", byGinLo , byGinHi );
		//if (byGinLo == 13||byGinLo == 9 )
		//if (byGinLo == 1)//第一组为定值
		//{
		//	float fVal = 0.0;
		//	memcpy(&fVal, &pbuf[20 + iOffset], 4);
		//	m_pMethod->SetYcData(m_SerialNo, count, fVal);
		//	printf("yc%d = %f\n",count, fVal);
		//	count++;
		//}
		float fVal = 0.0;
		if (byGinLo == 2)
		{
			
			memcpy(&fVal, &pbuf[20 + iOffset], 4);
			m_pMethod->SetYcData(m_SerialNo, byGinHi-1, fVal);
			printf("yc %d = %f\n", byGinHi - 1, fVal);
			
		}
		else if ( byGinLo == 3 )
		{
			memcpy(&fVal, &pbuf[20 + iOffset], 4);
			m_pMethod->SetYcData(m_SerialNo, 120+byGinHi - 1, fVal);
			printf("yc %d = %f\n", 120 + byGinHi - 1, fVal);
			

		} 
		else if (byGinLo == 4)
		{
			memcpy(&fVal, &pbuf[20 + iOffset], 4);
			m_pMethod->SetYcData(m_SerialNo, 240 + byGinHi - 1, fVal);
			printf("yc %d = %f\n", 240+ byGinHi - 1, fVal);
			
		}
		else
		{
			InitProtocolStatus();
		}
		

	}
	
	return TRUE;
}/*}}}*/

float CIEC103_ZN::floatvalue(BYTE *buf)
{/*{{{*/
	BYTE buftemp[4];
	//buftemp[0] = buf[6];
	//buftemp[1] = buf[7];
	//buftemp[2] = buf[8];
	//buftemp[3] = buf[9];
	//float ptmp = *(float *)buftemp;
	//return ptmp;
	buftemp[0] = buf[9];			//大端,使用objdump -a可以知道大小端!
	buftemp[1] = buf[8];
	buftemp[2] = buf[7];
	buftemp[3] = buf[6];
	return *(float *)buftemp;
}/*}}}*/







/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  ProcessHead10Buf
 * Description:  处理开头时0x10的报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::ProcessHead10Buf ( BYTE *buf, int len )
{/*{{{*/
	//判断ACD 位
	if (m_bIsSendName)
	{
		m_SendStatus = C_NAME_2;//发送装置名称
		m_bIsSendName = FALSE;
		count = 0;

	}
	else if (buf[0] == 0x10 && buf[1] == 0x20 && buf[4] == 0x16 )//10 20 01 21 16 
	{
		m_SendStatus = C_PL2_NA_2;
	}
	else if ((buf[0] == 0x10 && buf[1] == 0x09 && buf[4] == 0x16))  //一个分组完成
	{
		m_SendStatus = C_GD_NA_2;
		m_bFcb=1;


	}
	else
	{
		InitProtocolStatus();
	}
	

	return TRUE;
}		/* -----  end of method CIEC103_ZN::ProcessHead10Buf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  ProcessHead68Buf
 * Description:  处理开头时68的报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::ProcessHead68Buf ( BYTE *buf, int len )
{/*{{{*/
	BOOL bRtn = TRUE;
	
	if (buf[13] == 0x94 || buf[13] == 0xD4)
	{

		m_SendStatus = C_PL2_NA_2;
		printf("er ji erji er ji erji");

	}
	else if (m_SendStatus = C_NAME_2)
	{
		m_SendStatus = C_GD_NA_2;

	}
	switch ( buf[6] )						//类型标识!
	{
		
		case 0x0A:
			printf("ASDU10\n");
			bRtn = M_GD_NA_3_Frame(buf , len );
			break;

		default:
			break;
	}				/* -----  end switch  ----- */

	printf("-----------m_SendStatus=%d---\n", m_SendStatus);

	return bRtn;
}		/* -----  end of method CIEC103_ZN::ProcessHead68Buf  ----- *//*}}}*/



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  ResetCommUnit
 * Description:  复位通信单元
 *       Input:  发送缓存区 发送长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::ResetCommUnit ( BYTE *buf, int &len  )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = 0x40;
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103_ZN::ResetCommUnit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  CallLevel1Data
 * Description:  召唤一级数据
 *       Input:  发送缓存区 发送长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::CallLevel1Data ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = ChangeFcb(0x5A, m_bFcb);
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103_ZN::ResetFrameCountBit  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  CallLevel1Data
 * Description:  召唤二级数据
 *       Input:  发送缓冲区 发送长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::CallLevel2Data ( BYTE *buf, int &len )
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = ChangeFcb(0x7A, m_bFcb);
	buf[2] = m_wDevAddr;
	buf[3] = GetCs( &buf[1], 2 );
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}		/* -----  end of method CIEC103_ZN::ResetFrameCountBit  ----- *//*}}}*/


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  GetSendbuf
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::GetSendBuf ( BYTE *buf, int &len )
{/*{{{*/
	BOOL bRtn = TRUE;
	switch ( m_SendStatus )
	{
		case C_RCU_NA_2:	//初始化
			print( (char *)"初始化" );
			ResetCommUnit( buf, len );
			m_bIsSendName = TRUE;
			count = 0;
			break;
		case C_NAME_2:
			print((char *)"装置名称");
			CallLevel2Data(buf, len);//装置名称
			count = 0;
			break;

		case C_PL1_NA_2:	//召唤一级数据------5a
			print( (char *)"召唤一级数据" );
			CallLevel1Data( buf, len );
			break;

		case C_PL2_NA_2:	//召唤二级用户数据---7a
			print( (char *)"er ji" );
			CallLevel2Data( buf, len );
			break;
		case C_GD_NA_2:		//通用分类数据--------------------特殊开发
			{
				printf("call GDdata\n");
				 if (bysendpos == 0)
				{
					CallGDData_02(buf, len);
				}
				else if (bysendpos == 1)
				{
					CallGDData_03(buf, len);
				}
				else if (bysendpos == 2)
				{
					CallGDData_04(buf, len);
				}
				bysendpos++;
				if (bysendpos > 2)
					bysendpos = 0;				
			}break;

		default:
			sprintf (DebugBuf,  "IEC103:GetProtocolBuf can't find m_SendStatus = %d\n", m_SendStatus );
			print( DebugBuf );
			break;
	}				/* -----  end switch  ----- */

	return bRtn;
}		/* -----  end of method CIEC103_ZN::GetSendbuf  ----- *//*}}}*/


BOOL CIEC103_ZN::CallGDData(BYTE *buf, int &len)
{
	printf("CallGDData msg\n");
	buf[0] = 0x68;
	buf[1] = 0x0D;
	buf[2] = 0x0D;
	buf[3] = 0x68;
	buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;


	buf[6] = 0x15;///(1)

	buf[7] = 0x81;//(2)
	buf[8] = 0x2A;

	buf[9] = 0x01;//(3)----
	buf[10] = 0xFE;
	buf[11] = 0xF1;

	buf[12] = 0x10; //(4)

	buf[13] = 0x01;//(5)

	buf[14] = 0x01;//(6)//---
	buf[15] = 0x00;

	buf[16] = 0x01;//(7)

	buf[17] = GetCs(buf + 4, 13);
	buf[18] = 0x16;

	len = 19;
	return TRUE;
}

BOOL CIEC103_ZN::CallGDData_02(BYTE *buf, int &len)
{
	printf("CallGDData msg\n");
	buf[0] = 0x68;
	buf[1] = 0x0D;
	buf[2] = 0x0D;
	buf[3] = 0x68;
	buf[4] = 0x53;
	//buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;


	buf[6] = 0x15;///(1)

	buf[7] = 0x81;//(2)
	buf[8] = 0x2A;

	buf[9] = 0x01;//(3)----
	buf[10] = 0xFE;
	buf[11] = 0xF1;

	buf[12] = 0x00; //(4)---

	buf[13] = 0x01;//(5)

	buf[14] = 0x02;//(6)//---
	buf[15] = 0x00;

	buf[16] = 0x01;//(7)

	buf[17] = GetCs(buf + 4, 13);
	buf[18] = 0x16;

	len = 19;
	return TRUE;
}

BOOL CIEC103_ZN::CallGDData_03(BYTE *buf, int &len)
{
	printf("CallGDData msg\n");
	buf[0] = 0x68;
	buf[1] = 0x0D;
	buf[2] = 0x0D;
	buf[3] = 0x68;
	//buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[4] = 0x53;
	buf[5] = m_wDevAddr;


	buf[6] = 0x15;///(1)

	buf[7] = 0x81;//(2)
	buf[8] = 0x2A;

	buf[9] = 0x01;//(3)----
	buf[10] = 0xFE;
	buf[11] = 0xF1;

	buf[12] = 0x00; //(4)---

	buf[13] = 0x01;//(5)

	buf[14] = 0x03;//(6)//---
	buf[15] = 0x00;

	buf[16] = 0x01;//(7)

	buf[17] = GetCs(buf + 4, 13);
	buf[18] = 0x16;

	len = 19;
	return TRUE;
}
BOOL CIEC103_ZN::CallGDData_04(BYTE *buf, int &len)
{
	printf("CallGDData msg\n");
	buf[0] = 0x68;
	buf[1] = 0x0D;
	buf[2] = 0x0D;
	buf[3] = 0x68;
	buf[4] =0x73;
	//buf[4] = ChangeFcb(0x53, m_bFcb);
	buf[5] = m_wDevAddr;


	buf[6] = 0x15;///(1)

	buf[7] = 0x81;//(2)
	buf[8] = 0x2A;

	buf[9] = 0x01;//(3)----ADDR
	buf[10] = 0xFE;
	buf[11] = 0xF1;

	buf[12] = 0x00; //(4)

	buf[13] = 0x01;//(5)//通用分类个数

	buf[14] = 0x04;//(6)//---
	buf[15] = 0x00;

	buf[16] = 0x01;//(7)

	buf[17] = GetCs(buf + 4, 13);
	buf[18] = 0x16;

	len = 19;
	return TRUE;
}




/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  InitProtocolStatus
 * Description:  初始化协议基本状态
 *       Input:
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::InitProtocolStatus (  )
{/*{{{*/
	m_bLinkStatus = FALSE;		//链接状态为断
	m_SendStatus = C_RCU_NA_2;	//设为复位通信单元
	m_dwLinkTimeOut = 0;		//链接超时为0
	
	
	m_byYkErrorCount = 0;		//遥控错误计数0
	m_byRecvErrorCount = 0;     //接收错误计数0
	m_bFcb = 0;					//FCB置0
	m_bIsReSend = FALSE;		//重发标识位0
	m_byResendCount = 0;		//重发次数清零
	m_bIsSending = FALSE;		//发送后置1 接收后值0
	m_bIsNeedResend = TRUE;		//是否需要重发
	
	
	m_bIsGDCall = FALSE;     //是否召唤通用分组数据
	m_bIsSendName = TRUE;

	m_wReSendLen = 0;
	m_byYkSendLen = 0;
	m_byRemoteBusNo = 0;
	m_byRemoteAddr = 0;
	bysendpos = 0;
	count = 0;
	memset( m_byReSendBuf, 0, IEC103_MAX_BUF_LEN );
	memset( DebugBuf, 0, sizeof( DebugBuf ) );

	return TRUE;
}		/* -----  end of method CIEC103_ZN::InitProtocolStatus  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  GetDevCommState
 * Description:  设置装置链接状态
 *       Input:
 *		Return:  BOOL 0 正常 1 不正常
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::GetDevCommState (  )
{/*{{{*/
	if( m_bLinkStatus )
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}		/* -----  end of method CIEC103_ZN::GetDevCommState  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  TimerProc
 * Description:  时间处理函数 主要处理一些超时 总召唤等与时间有关的
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CIEC103_ZN::TimerProc (  )
{/*{{{*/
	int Interval = 250;
	//接收错误次数
	if( m_byRecvErrorCount > IEC103_MAX_ERROR_COUNT  )
	{
		m_byResendCount = 0;
		InitProtocolStatus();
	}
	//通讯超时时间
	m_dwLinkTimeOut += Interval;
	if (m_dwLinkTimeOut >= IEC103_LINK_TIMEOUT)
	{
			InitProtocolStatus();
		
	}
	if (COM_DEV_ABNORMAL == GetDevCommState())
	{
		InitProtocolStatus();
	}
}		/* -----  end of method CIEC103_ZN::TimerProc  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  ProcessProtocolBuf
 * Description:	 处理收到的数据缓存
 *       Input:  接收到的数据缓存 缓存长度
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::ProcessProtocolBuf ( BYTE *buf, int len )
{/*{{{*/
	int pos=0;
	BOOL bRtn = TRUE;

	printf("-------------recv--\n");
	for (int i = 0; i < len; i++)
		printf("%02x  ",buf[i]);
	printf("\n");

	if( !WhetherBufValue( buf, len, pos ) )
	{
		print ( (char *)"CIEC103_ZN:ProcessProtocolBuf buf Recv err!!!\n" );
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
		sprintf (DebugBuf,  "CIEC103_ZN:ProcessProtocolBuf buf[0]=%x err!!!\n", buf[pos] );
		print( DebugBuf );
	}

	//此处只判断是否处理 不能因为子站传的正确报文而没有处理导致通讯异常
	if( !bRtn )
	{
		print( (char *)"处理报文发生错误或未处理" );
		
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

	return TRUE;
}		/* -----  end of method CIEC103_ZN::ProcessProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  GetProtocolBuf
 * Description:  获取协议数据缓存
 *       Input:  缓存区 缓存区数据长度 总线消息
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{/*{{{*/
	BOOL bRtn = TRUE;
	bRtn = GetSendBuf( buf, len );
	/*if( bRtn )
	{
			m_wReSendLen = len;
			memcpy( m_byReSendBuf, buf, m_wReSendLen );
			m_bIsSending = TRUE;
			if( !m_bIsNeedResend )
			{
				m_bIsSending = FALSE;
				m_bIsNeedResend = TRUE;
			}
	}*/
	printf("--------send----------\n");
	for (int i = 0; i < len; i++)
		printf("%02x  ",buf[i]);
	printf("\n");
	return bRtn;
}		/* -----  end of method CIEC103_ZN::GetProtocolBuf  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CIEC103_ZN
 *      Method:  Init
 * Description:	 初始化协议数据
 *       Input:  总线号
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CIEC103_ZN::Init ( BYTE byLineNo )
{/*{{{*/
	
	if( !InitProtocolStatus() )
	{
		print ( (char *)"CIEC103_ZN:InitProtocolStatus Err\n" );
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC103_ZN::Init  ----- *//*}}}*/
