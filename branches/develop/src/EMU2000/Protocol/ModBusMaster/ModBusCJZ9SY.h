/*
* =====================================================================================
*
*       Filename:  ModBusCJZ9SY.h
*
*    Description:  
*
*        Version:  1.0
*        Created:  2019年07月10日 13时21分18秒
*       Revision:  none
*       Compiler:  gcc
*
*         Author:  
*   Organization:
*
*		  history:
*
* =====================================================================================
*/


#ifndef  MODBUSCJZ9SY_INC
#define  MODBUSCJZ9SY_INC

#include "CProtocol_ModBusMaster.h"

#define				MODBUSMASTER_CJZ9SY_MAX_POS			5		/* 最大位置 */

/*
* =====================================================================================
*        Class:  CModBusSCJZ9SY
*  Description:  
* =====================================================================================
*/
class CModBusSCJZ9SY : public CProtocol_ModBusMaster
{
public:
	/* ====================  LIFECYCLE     ======================================= */
	CModBusSCJZ9SY();                             /* constructor      */
	~CModBusSCJZ9SY();                            /* destructor       */

	virtual BOOL GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg = NULL);
	virtual BOOL ProcessProtocolBuf(BYTE * buf, int len);
	virtual BOOL Init(BYTE byLineNo);

	virtual void TimerProc();
	//获得装置通讯状态
	virtual BOOL GetDevCommState();

protected:
	/* ====================  DATA MEMBERS  ======================================= */

private:
	// 判断报文有效性
	BOOL WhetherBufValue(BYTE *buf, int &len);
	// 处理报文
	BOOL ProcessRecvBuf(BYTE *buf, int len);
	// 处理第一段报文
	BOOL ProcessRecvBuf_One(BYTE *buf, int len);
	// 处理第二段报文
	BOOL ProcessRecvBuf_Two(BYTE *buf, int len);
	// 处理第三段报文
	BOOL ProcessRecvBuf_Three(BYTE *buf, int len);
	// 处理第四段报文
	BOOL ProcessRecvBuf_Four(BYTE *buf, int len);
	// 处理第五段报文
	BOOL ProcessRecvBuf_Five(BYTE *buf, int len);
	
	//改变发送位置
	void ChangeSendPos(void);
	//获取发送位置
	BYTE GetSendPos(void);
private:
	/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;
	BYTE m_bySendPos;
	int m_byRecvflag;
	BYTE buf_fz[4]; //存 放 辅 助 信 息

}; /* -----  end of class CModBusSCJZ9SY  ----- */

#endif   /* ----- #ifndef MODBUSCJZ9SY_INC  ----- */
