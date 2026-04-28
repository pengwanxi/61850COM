/*
* =====================================================================================
*
*       Filename: 
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
#ifndef SPECIALL_UPS_H
#define SPECIALL_UPS_H

//#include "CProtocol_ModBusMaster.h"
#include "CProtocol_UpsMaster.h"

/*
* =====================================================================================
*        Class:  SPECIALLUPS
*  Description:  
* =====================================================================================
*/
class SPECIALLUPS : public  CProtocol_UpsMaster
{
public:
	/* ====================  LIFECYCLE     ======================================= */
	SPECIALLUPS();                             /* constructor      */
	~SPECIALLUPS();                            /* destructor       */

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
	BOOL ProcessRecvBuf_Status(BYTE *buf, int len);
	// 处理第二段报文
	BOOL ProcessRecvBuf_Data(BYTE *buf, int len);
	
private:
	/* ====================  DATA MEMBERS  ======================================= */
	BOOL m_bLinkStatus;
	BYTE m_bySendCount;
	BYTE m_byRecvCount;
	BYTE m_bySendPos;
	int m_byRecvflag;
	

}; /* -----  end of class SPECIALLUPS  ----- */

#endif  