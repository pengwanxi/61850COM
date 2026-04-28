/*
* =====================================================================================
*
*       Filename:  CIEC103.cpp
*
*    Description:  针对于标准103进行处理
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
#include "iec103_nanzi_pst645.h"
#include "../../share/global.h"



#define	IEC103DEBUG		1	//[> 终端打印 <]
#define	IEC103BUSDEBUG			        /* 总线打印 */
#define	IEC103DISPLAYCOT			/* 显示传送原因  */


extern "C" void GetCurrentTime(REALTIME *pRealTime);
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

CIEC103_Nanzi_PST645::CIEC103_Nanzi_PST645()
{/*{{{*/
	InitProtocolStatus();
}  /* -----  end of method CIEC103::CIEC103  (constructor)  ----- *//*}}}*/

CIEC103_Nanzi_PST645::~CIEC103_Nanzi_PST645()
{/*{{{*/
}

void CIEC103_Nanzi_PST645::print(char *buf, int len)
{/*{{{*/
#ifdef  IEC103DEBUG
	printf("%s\n", buf);
#endif     /* -----  not IEC103DEBUG  ----- */

#ifdef  IEC103BUSDEBUG
	OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf), 2);
#endif     /* -----  not IEC103BUSDEBUG  ----- */
}		/* -----  end of method CIEC103::print  ----- *//*}}}*/

BOOL CIEC103_Nanzi_PST645::ProcessHead10Buf(BYTE *buf, int len)
{/*{{{*/
 // //判断地址是否正确
 // if ( buf[2] != m_wDevAddr )
 // return FALSE;

 //如果是请求链路设置状态
 // if( m_SendStatus == C_RLK_NA_3 )
 // m_SendStatus = C_RCU_NA_3;
 //判断ACD 位
 // else if( buf[1] & 0x20 )
 //判断ACD 位


	if (buf[1] & 0x20)
	{
		m_SendStatus = C_PL1_NA;
	}

	switch (buf[1] & 0x0f)
	{
	case 0x00: //确认帧 确认
		print((char *)"确认帧");
		break;

	case 0x08: //以数据相应请求帧
		print((char *)"数据请求帧");
		break;

	case 0x09:	//无所召唤数据
		print((char *)"无所召唤数据");
		break;

	case 0x0b://以链路状态或访问请求回答请求帧
		print((char *)"链路状态或访问请求回答请求帧");
		break;

	default://默认处理
		print((char *)"default");
		return FALSE;
		break;
	}				/* -----  end switch  ----- */


	return TRUE;
}		/* -----  end of method CIEC103::ProcessHead10Buf  ----- *//*}}}*/


BOOL CIEC103_Nanzi_PST645::ProcessHead68Buf(BYTE *buf, int len)
{/*{{{*/
	BOOL bRtn = TRUE;
	//判断ACD 位
	if (buf[4] & 0x20)
		m_SendStatus = C_PL1_NA;

	switch (buf[6])						//类型标识!
	{
	case 0x0A://一般命令
			bRtn = prcess0x0A(buf, len); 
			 break;
	default:
		break;
	}				/* -----  end switch  ----- */

	return bRtn;
}	

BOOL CIEC103_Nanzi_PST645::prcess0x0A(BYTE *pbuf, int &len)
{
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
	BYTE byGNo = 0xFF ;

	for ( int i = 0 ; i < byNumStruct ; i++ )
	{
		if( byGNo == 0xFF )
				byGNo = pbuf[14 ];

		//获取数据
		if (byGNo == 1)  //组号1为遥测信息
		{
			int iOffset = i * 10;
			BYTE byGinLo = pbuf[14 + iOffset]; //通用分类标识序号低 组号
			BYTE byGinHi = pbuf[15 + iOffset]; //通用分类标识序号高 条目号
			BYTE byKOD = pbuf[16 + iOffset]; //描述类型
			BYTE byDataType = pbuf[17 + iOffset]; //数据类型
			BYTE byDataSize = pbuf[18 + iOffset]; //数据宽度
			BYTE byNum = pbuf[19 + iOffset]; //数据数目

			float fVal = 0.0;
			memcpy(&fVal, &pbuf[20 + iOffset ], 4);
			m_pMethod->SetYcData(m_SerialNo, i, fVal);
			m_SendStatus = C_YX_NA_3;
			printf("yc%d = %f\n", i, fVal);
		}
		else if (byGNo == 2) //组号2 为 遥信信息
		{
			int iOffset = i * 7;
			BYTE byGinLo = pbuf[14 + iOffset]; //通用分类标识序号低 组号
			BYTE byGinHi = pbuf[15 + iOffset]; //通用分类标识序号高 条目号
			BYTE byKOD = pbuf[16 + iOffset]; //描述类型
			BYTE byDataType = pbuf[17 + iOffset]; //数据类型
			BYTE byDataSize = pbuf[18 + iOffset]; //数据宽度
			BYTE byNum = pbuf[19 + iOffset]; //数据数目


			BOOL bVal = pbuf[20 + iOffset];
			BYTE open = 1, close = 2;
			if (bVal == close) //
				m_pMethod->SetYxData(m_SerialNo, i, 1);
			else if (bVal == open)
				m_pMethod->SetYxData(m_SerialNo, i, 0);
			m_SendStatus = C_YC_NA_3;
			printf("yx%02d = %d\n", i, bVal);
		}
	}

	return TRUE;
}

BOOL CIEC103_Nanzi_PST645::CallLevel1Data(BYTE *buf, int &len)
{/*{{{*/
	buf[0] = 0x10;
	buf[1] = ChangeFcb(0x7A, m_bFcb);
	buf[2] = m_wDevAddr;
	buf[3] = GetCs(&buf[1], 2);
	buf[4] = 0x16;

	len = 5;
	return TRUE;
}	

BOOL CIEC103_Nanzi_PST645::CallYcData(BYTE *buf, int &len)
{
	int index = 0;
	buf[index++] = 0x68;
	buf[index++] = 0x0d;
	buf[index++] = 0x0d;
	buf[index++] = 0x68;
	buf[index++] = ChangeFcb(0x73, m_bFcb);
	//buf[index++] = 0x73;
	buf[index++] = m_wDevAddr;
	
	buf[index++] = 0x15; //类型标识
	buf[index++] = 0x81; //可变结构限定词
	buf[index++] = 0x2A; //传输原因
	buf[index++] = 0x01; //公共地址
	buf[index++] = 0xFE; //功能类型
	buf[index++] = 0xF1; //信息序号
	buf[index++] = 0x15; //返回信息标识符
	buf[index++] = 0x01; //通用分类标识数目
	buf[index++] = 0x01; //通用分类标识序号(Lo)
	buf[index++] = 0x00; //通用分类标识序号(Hi)
	buf[index++] = 0x01; //描述类别
	buf[index++] = GetCs(&buf[4], 13);
	buf[index++] = 0x16;
	len = index;
	return TRUE;
}

BOOL CIEC103_Nanzi_PST645::CallYxData(BYTE *buf, int &len)
{
	int index = 0;
	buf[index++] = 0x68;
	buf[index++] = 0x0d;
	buf[index++] = 0x0d;
	buf[index++] = 0x68;
	buf[index++] = ChangeFcb(0x73, m_bFcb);
	buf[index++] = m_wDevAddr;

	buf[index++] = 0x15; //类型标识
	buf[index++] = 0x81; //可变结构限定词
	buf[index++] = 0x2A; //传输原因
	buf[index++] = 0x01; //公共地址
	buf[index++] = 0xFE; //功能类型
	buf[index++] = 0xF1; //信息序号
	buf[index++] = 0x17; //返回信息标识符
	buf[index++] = 0x01; //通用分类标识数目
	buf[index++] = 0x02; //通用分类标识序号(Lo)
	buf[index++] = 0x00; //通用分类标识序号(Hi)
	buf[index++] = 0x01; //描述类别
	buf[index++] = GetCs(&buf[4], 13);
	buf[index++] = 0x16;
	len = index;
	return TRUE;
}

BOOL CIEC103_Nanzi_PST645::GetSendBuf(BYTE *buf, int &len)
{
	if (m_bIsReSend)
	{
		memcpy(buf, m_byReSendBuf, m_wReSendLen);
		len = m_wReSendLen;
		m_byResendNumber++;
		if (m_byResendNumber > 5)
		{
			m_bIsReSend = FALSE;
			m_SendStatus = C_YC_NA_3;	//召唤遥测数据
			m_byResendNumber = 0;
			return FALSE;
		}
		
		return TRUE;
	}

	BOOL bRtn = TRUE;
	switch (m_SendStatus)
	{
	case C_PL1_NA:	//召唤一级数据
		CallLevel1Data(buf, len);
		break;

	case C_YC_NA_3:		//召唤遥测数据
		CallYcData(buf, len);
		break;
	case C_YX_NA_3:		//召唤遥信数据
		CallYxData(buf, len);
		break;
	default:
		break;
	}			

	return bRtn;
}		


BOOL CIEC103_Nanzi_PST645::ProcessBusMsg(PBUSMSG pBusMsg, BYTE *buf, int &len)
{/*{{{*/
		return FALSE;

}	

BOOL CIEC103_Nanzi_PST645::InitProtocolStatus()
{/*{{{*/
	m_bLinkStatus = FALSE;		//链接状态为断
	m_SendStatus = C_YC_NA_3;	//召唤遥测数据
	m_dwLinkTimeOut = 0;		//链接超时为0
	m_byResendNumber = 0;     //接收错误计数0
	m_bFcb = 0;					//FCB置0
	m_bIsReSend = FALSE;		//重发标识位0
	m_byResendCount = 0;		//重发次数清零
	m_bIsSending = FALSE;		//发送后置1 接收后值0
	m_bIsNeedResend = TRUE;		//是否需要重发

	m_wReSendLen = 0;
	m_byYkSendLen = 0;
	m_byRemoteBusNo = 0;
	m_byRemoteAddr = 0;
	memset(m_byReSendBuf, 0, IEC103_MAX_BUF_LEN);
	memset(m_byYkSendBuf, 0, sizeof(m_byYkSendBuf));
	memset(DebugBuf, 0, sizeof(DebugBuf));

	return TRUE;
}		/* -----  end of method CIEC103::InitProtocolStatus  ----- *//*}}}*/

BOOL CIEC103_Nanzi_PST645::GetDevCommState()
{/*{{{*/
	if (m_bLinkStatus)
		return COM_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}		/* -----  end of method CIEC103::GetDevCommState  ----- *//*}}}*/

BOOL CIEC103_Nanzi_PST645::ProcessProtocolBuf(BYTE *buf, int len)
{/*{{{*/
	int pos = 0;
	BOOL bRtn = TRUE;
	
	if (!WhetherBufValue(buf, len, pos))
	{
		print((char *)"CIEC103:ProcessProtocolBuf buf Recv err!!!\n");
		m_bIsReSend = TRUE;
		return FALSE;
	}

	if (buf[pos] == 0x10)
	{
		bRtn = ProcessHead10Buf(&buf[pos], len);
	}
	else if (buf[pos] == 0x68)
	{
		bRtn = ProcessHead68Buf(&buf[pos], len);
	}
	else
	{
		sprintf(DebugBuf, "CIEC103:ProcessProtocolBuf buf[0]=%x err!!!\n", buf[pos]);
		print(DebugBuf);
	}

	//此处只判断是否处理 不能因为子站传的正确报文而没有处理导致通讯异常
	if (!bRtn)
	{
		print((char *)"处理报文发生错误或未处理");
		m_bIsReSend = TRUE;
	}
	else
	{
		m_bLinkStatus = TRUE;
		m_dwLinkTimeOut = 0;
		m_bIsReSend = FALSE;
		m_byResendCount = 0;
		m_byResendNumber = 0;
	}
	m_bLinkStatus = TRUE;
	return bRtn;
}		/* -----  end of method CIEC103::ProcessProtocolBuf  ----- *//*}}}*/

BOOL CIEC103_Nanzi_PST645::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{/*{{{*/

	BOOL bRtn = TRUE;
	bRtn = GetSendBuf(buf, len);
	if (bRtn)
	{
		m_wReSendLen = len;
		memcpy(m_byReSendBuf, buf, m_wReSendLen);
		m_bIsReSend = TRUE;
	}

	return bRtn;
}		

BOOL CIEC103_Nanzi_PST645::Init(BYTE byLineNo)
{/*{{{*/
	if (!InitProtocolStatus())
	{
		print((char *)"CIEC103:InitProtocolStatus Err\n");
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CIEC103::Init  ----- *//*}}}*/
