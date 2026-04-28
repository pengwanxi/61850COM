/*
* =====================================================================================
*
*       Filename:
*
*        Version:  1.0
*        Created:  2019年07月12日 13时15分10秒
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  mengqp (),
*   Organization:
*
*		  history:
* =====================================================================================
*/
#include  <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include"Speciall_UPS.h"

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

/*
*--------------------------------------------------------------------------------------
*       Class:  SPECIALLUPS
*      Method:  SPECIALLUPS
* 
*--------------------------------------------------------------------------------------
*/

SPECIALLUPS::SPECIALLUPS()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_bySendPos = 0;
	m_byRecvflag = 0;
	

}  /* -----  end of method SPECIALLUPS::SPECIALLUPS  (constructor)  ----- */

/*
*--------------------------------------------------------------------------------------
*       Class:   FJ_UPS
*      Method:  ~ FJ_UPS
* Description:  destructor
*--------------------------------------------------------------------------------------
*/
SPECIALLUPS::~SPECIALLUPS()
{
} 
/*
*--------------------------------------------------------------------------------------
*       Class:  FJ_UPS
*      Method:  WhetherBufValue
* Description:  查看接收报文有效性
*       Input:  缓冲区 长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL SPECIALLUPS::WhetherBufValue(BYTE *buf, int &len)
{
	if (m_byRecvflag == 1)//状态 命令返回
	{
		if (buf[0] == 0x53 && buf[len - 2] == 0x0D&&buf[len - 1] == 0x0A)//简单判断一下头和尾
			return TRUE;
		else
			return FALSE;
	}
	else if(m_byRecvflag == 2)//数据 命令返回
	{
		if (buf[0] == 0x44 && buf[len - 2] == 0x0D && buf[len - 1] == 0x0A)//简单判断一下头和尾
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		return FALSE;
	}
}
/*
*--------------------------------------------------------------------------------------
*       Class:  SPECIALLUPS
*      Method:  ProcessRecvBuf
* Description:  处理报文  更新数据
*       Input:  缓冲区长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL SPECIALLUPS::ProcessRecvBuf(BYTE *buf, int len)
{
	int i;
	char szBuf[256];
	switch (m_byRecvflag)
	{
	case 1:
		ProcessRecvBuf_Status(buf, len);
		break;
	case 2:
		ProcessRecvBuf_Data(buf, len);
		break;
	default:
		sprintf(szBuf, "SPECIALLUPS recv no this type\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		break;
	}	
	return FALSE;
}		
//yc：0 - 4  
BOOL SPECIALLUPS::ProcessRecvBuf_Status(BYTE *buf, int len)
{
	printf("\n^^^^^^yx data^^^^^\n");
	/*
	BYTE byval=0;
	byval = buf[3] - 0x30;
	if (byval == 1 || byval == 0)
	{
		m_pMethod->SetYxData(m_SerialNo, 0, byval);//yx0  SOK0/SOK1   SOK0:UPS正常  SOK1:UPS异常
		printf("---0----%d\n",byval);

	}
	byval = buf[9] - 0x30;
	if (byval == 1 || byval == 0)
	{
		m_pMethod->SetYxData(m_SerialNo, 1,byval);//yx1  SBP0:逆变 SBP1:旁路
		printf("---1----%d\n", byval);

	}
	
	byval = buf[15] - 0x30;
	if (byval == 1 || byval == 0)
	{
		m_pMethod->SetYxData(m_SerialNo, 2,byval);//yx2  SUF0:市电供电 SUF1:市电掉电 
		printf("---2----%d\n", byval);

	}
	
	byval = buf[21] - 0x30;
	if (byval == 1 || byval == 0)
	{
		m_pMethod->SetYxData(m_SerialNo, 3,byval);//yx3 SBL0:电压正常 SBL1:电压低压
		printf("---3----%d\n", byval);

	}

	byval = buf[27] - 0x30;
	if (byval == 1 || byval == 0)
	{
		m_pMethod->SetYxData(m_SerialNo, 4,byval);//yx4  SBM0:逆变 SBM1:放电
		printf("---4----%d\n", byval);

	}
	*/
	char *szBuf = new char[1024];
	int checkflag = 0;
	int wpnt = 0;
	BYTE yxdata ;
	memcpy(szBuf, buf, len);
	for (int i = 0; i < len; i++, szBuf++)
	{
		if ((isdigit(*szBuf)) && checkflag == 0)
		{
			checkflag++;
			yxdata = atoi(szBuf);
			if (yxdata == 1 || yxdata == 0)
			{
				printf("-----wpnt=%d-----yxdata=%d---\n", wpnt, yxdata);
				m_pMethod->SetYxData(m_SerialNo, wpnt, yxdata);

			}
			wpnt++;
		

		}
		else/* if (isalpha(*szBuf))*/{
			checkflag = 0;

		}

	}
	szBuf = NULL;
	delete szBuf;



	return TRUE;
}
//RD<cr>
BOOL SPECIALLUPS::ProcessRecvBuf_Data(BYTE *buf, int len)
{
	printf("\n*********yc data**********\n");
	char *szBuf = new char[1024];
	int checkflag = 0;
	int wpnt = 0;
	float ycdata = 0.0;
	memcpy(szBuf, buf , len );
	for (int i = 0; i < len; i++, szBuf++)
	{
		if ((isdigit(*szBuf) || (*szBuf == 0x2B) || (*szBuf == 0x2D)) && checkflag == 0)
		{
			checkflag++;
			ycdata = atof(szBuf);
			printf("-----wpnt=%d-----ycdata=%f---\n", wpnt, ycdata);
			m_pMethod->SetYcData(m_SerialNo, wpnt++, ycdata);
			
		}
		else if (isalpha(*szBuf) && ((*szBuf != 0x45) || (*szBuf) != 0x65)){
			checkflag = 0;

		}

	}
	szBuf = NULL;
	delete szBuf;

}

/*
*--------------------------------------------------------------------------------------
*       Class:  SPECIALLUPS
*      Method:  GetProtocolBuf
* Description:  获取发送报文
*       Input:  
*		Return:  BOOL
*---------------------------------------------------------------------------------------
*/
BOOL SPECIALLUPS::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	char szBuf[256] = "";
	m_byRecvflag = 0;
	len = 0;	
	if (pBusMsg != NULL)
	{
		return FALSE;
	}
	switch (m_bySendPos)
	{
		case 0:	//Command:RS <cr> description:状态
		{
				buf[len++] = 0x52;   
				buf[len++] = 0x53;
				m_byRecvflag = 1;

		}break;
		case 1://Command:RD <cr>  description:数据
		{
				 buf[len++] = 0x52;
				 buf[len++] = 0x44;
				 m_byRecvflag = 2;

		}break;
		
	   default:
		   sprintf(szBuf, "%s", "SPECIALLUPS send buf err !!!\n");
		   OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		   break;
	}	
	buf[len++] = 0x0D;   //归位符
	m_bySendPos++;
	if (m_bySendPos > 1)
		m_bySendPos = 0;
	m_bySendCount++;

	return TRUE;
}		/* -----  end of method SPECIALLUPS::GetProtocolBuf  ----- */



/*
*--------------------------------------------------------------------------------------
*       Class:  FJ_UPS
*      Method:  ProcessProtocolBuf
* Description:  处理接收报文
*       Input:  缓冲区长度
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL SPECIALLUPS::ProcessProtocolBuf(BYTE *buf, int len) 
{
	if (!WhetherBufValue(buf, len))
	{
		char szBuf[256] = "";
		sprintf(szBuf, "%s", "SPECIALLUPS recv buf err !!!\n");
		OutBusDebug(m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2);
		m_byRecvCount++;
		return FALSE;
	}	
	ProcessRecvBuf(buf, len);
	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	return TRUE;
}		/* -----end of method SPECIALLUPS::ProcessProtocolBuf ------- */
/*
*--------------------------------------------------------------------------------------
*       Class:  SPECIALLUPS
*      Method:  Init
* Description:  初始化协议
*       Input:  总线号
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL SPECIALLUPS::Init(BYTE byLineNo)
{
	return TRUE;
}		/* -----  end of method SPECIALLUPS::Init  -------*/

/*
*--------------------------------------------------------------------------------------
*       Class:  SPECIALLUPS
*      Method:  TimerProc
* Description:  时钟处理
*       Input:   void
*		Return:  void
*---------------------------------------------------------------------------------------
*/
void SPECIALLUPS::TimerProc(void)
{
	if (m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if (m_bLinkStatus)
		{
			m_bLinkStatus = FALSE;
			OutBusDebug(m_byLineNo, (BYTE *)"SPECIALLUPS:unlink\n", 30, 2);
		}
	}
}		/* -----  end of method SPECIALLUPS::TimerProc  ----- */


/*
*--------------------------------------------------------------------------------------
*       Class:  SPECIALLUPS
*      Method:  GetDevCommState
* Description:  获取装置状态
*       Input:  void
*		Return:  BOOL
*--------------------------------------------------------------------------------------
*/
BOOL SPECIALLUPS::GetDevCommState(void)
{
	if (m_bLinkStatus)
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}		/* -----  end of method SPECIALLUPS::GetDevCommState  ----- */

