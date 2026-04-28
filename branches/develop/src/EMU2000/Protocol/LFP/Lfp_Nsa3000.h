/*
 * =====================================================================================
 *
 *       Filename:  Lfp_Nsa3000.h
 *
 *    Description:  Lfp_Nsa3000版本协议
 *
 *        Version:  1.0
 *        Created:  2017年01月13日
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */

#ifndef  _LFP_NSA3000_INC
#define  _LFP_NSA3000_INC

#include "CProtocol_LFP.h"

enum {
	NSA3000 = 1
};
//下发功能码
#define RCS_YC 0x50 /*申请遥测*/ //暂不需要读取
#define RCS_YX 0x54 /*申请全遥信*/
#define RCS_ACK 0x06
#define RCS_QUERY1 0x40
#define RCS_QUERY2 0x55
#define RCS_SWITCH 0x44	//开关量




//下发报文列表
const BYTE byValueCode[] = {RCS_SWITCH};


/*
 * =====================================================================================
 *        Class:  CLfp_Nsa3000
 *  Description:  
 * =====================================================================================
 */
class CLfp_Nsa3000 : public CProtocol_LFP
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CLfp_Nsa3000 ();                             /* constructor      */
		~CLfp_Nsa3000 ();                            /* destructor       */
		
		virtual BOOL Init( BYTE byLineNo );
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
		virtual void TimerProc() ;
		//获得装置通讯状态
		virtual BOOL GetDevCommState( ) ;
		
	protected:
		BOOL m_bNeedConfirm;	//是否需要确认
		BOOL m_bLinkStatus;
		BYTE m_bySendCount;
		BYTE m_byRecvCount;
		BYTE m_byCodeIndex;
		
	private:
		int PackMsg(BYTE * byBuf, BYTE byCode, BYTE * byData = NULL, int iDataLen = 0);		
}; /* -----  end of class CLfp_Nsa3000  ----- */

#endif   /* ----- #ifndef _LFP_NSA3000_INC  ----- */
