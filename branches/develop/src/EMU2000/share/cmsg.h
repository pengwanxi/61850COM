#ifndef _CMSG_H__
#define _CMSG_H__

#include "rdbFun.h"
#pragma pack( 1 )
typedef struct tagMsg
{/*{{{*/
	tagMsg( )
	{
		msgType = 0 ;
		pVoid = NULL ;
		msgSize = sizeof( void * ) + sizeof( int ) ;
	}
	~tagMsg()
	{}

	long msgType ;
	void * pVoid ;
	int msgSize ; //msgSize 为wMsgAutoType pVoid msgSize的大小总和
}LMSG, *PLMSG ;/*}}}*/

class CMsg
{/*{{{*/
	public:
		CMsg() ;
		~CMsg()	;
		enum { msg_uid = 2014 };
		enum{ msgType_tcpportServer = 3015 };
	protected:
		static int m_MsgID ; //通信标示符，唯一代表消息队列
		int m_MsgKey ;//创建消息通道键值
		int m_msgSize ;
		
		//msg_gather  Assign to gather 
		//msg_tcpportserver   Assign to tcpportServer 

	public:
		int  CreateMsgQueue( long msgChannel );
		bool SendMsg( void * pVoid ) ;
		bool RecvMsg( void * pVoid ) ;
		bool CloseMsgQueue() ;
		bool IsMsgQueue(  ); //消息队列是否可用
		long m_MsgType ;//消息通道
};/*}}}*/
#pragma pack(  )
#endif

