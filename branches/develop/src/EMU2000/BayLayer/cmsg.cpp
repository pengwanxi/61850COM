
#include <stdio.h>
#include <sys/msg.h>
#include <unistd.h>
#include "cmsg.h"

int CMsg::m_MsgID = -1 ;

CMsg::CMsg()
{/*{{{*/
	m_MsgKey = 2014 ;// 非法值
	m_MsgType = -1 ;//非法值
	//printf( "CMsg Constructor OK. \n" ) ;
	m_msgSize = sizeof( void * ) + sizeof( int ) ;
}/*}}}*/

CMsg::~CMsg()
{/*{{{*/
	//判断该消息队列是否有消息没处理
	//如果没有消息了就关闭消息通道
	//有消息等待消息处理完，在关闭该消息通道

	if( m_MsgID != -1 )
	{
		// 			int Return = -1;
		// 			struct msqid_ds get_info;
		// 			Return = msgctl( m_MsgID, IPC_STAT, &get_info );
		// 			if( Return == -1 )
		// 				printf("msgctl get stat defeat\n");
		// 			
		// 			if( get_info.msg_qnum == 0 )
		// 			{
		// 				CloseMsgQueue();
		// 				printf( "m_MsgType = %ld CMsg Destructor OK.\n",m_MsgType) ;
		// 				break;
		// 			}

		if( CloseMsgQueue() )
			printf( "\nCMsg Destructor OK.\n" ) ;
	}

}/*}}}*/

/*
 * -------------------------------------------------------------------------------------------------
 * class:	CMsg
 * funct:	CreateMsgQueue
 * descr:	创建消息队列!
 * param:	msgtyp,msgrcv之para3!
 * retur:	消息队列ID
 * -------------------------------------------------------------------------------------------------
 */
int CMsg::CreateMsgQueue( long msgChannel )
{/*{{{*/
	if( msgChannel <= 0 )
		return -1 ;
	else
		m_MsgType = msgChannel ;

	if( m_MsgID != -1 )
		return m_MsgID ;

	int msgID = -1 ;
	msgID = msgget( m_MsgKey, IPC_CREAT|0666);
	if( msgID != -1 )
		m_MsgID = msgID ;

	return msgID ;
}/*}}}*/

/*
 * -------------------------------------------------------------------------------------------------
 * class:	CMsg
 * funct:	IsMsgQueue
 * descr:	消息队列有效性判断!
 * param:	void
 * retur:	true:有效 false:无效!
 * -------------------------------------------------------------------------------------------------
 */
bool CMsg::IsMsgQueue(  )
{/*{{{*/
	if( m_MsgID == -1 || m_MsgType == -1 )
		return false ;
	else
		return true ;
}/*}}}*/

/*
 * -------------------------------------------------------------------------------------------------
 * class:	CMsg
 * funct:	CloseMsgQueue
 * descr:	关闭消息队列
 * param:	void
 * retur:	true:关闭成功 false:关闭失败!
 * -------------------------------------------------------------------------------------------------
 */
bool CMsg::CloseMsgQueue()
{/*{{{*/
	int Return = msgctl( m_MsgID, IPC_RMID, NULL ) ;
	if( Return == 0 )
	{
		m_MsgID = -1;
		printf( "\nClose Msg Queue Success OK. \n" );
		return true ;
	}
	else
		return false ;
}/*}}}*/

/*
 * -------------------------------------------------------------------------------------------------
 * class:	CMsg
 * funct:	SendMsg
 * descr:	发送消息!
 * param:	发送内容指针!
 * retur:	true:发送成功 false:发送失败!
 * -------------------------------------------------------------------------------------------------
 */
bool CMsg::SendMsg( void * pVoid )
{/*{{{*/
	if( pVoid == NULL || m_MsgID == -1 || !IsMsgQueue() )
		return false ;

	//IPC_NOWAIT 立即返回 非阻塞
	//0 阻塞
	PLMSG  pMsg = ( PLMSG  )pVoid ;
	pMsg->msgType = m_MsgType ;

	int iReturn = msgsnd( m_MsgID, pVoid , m_msgSize , IPC_NOWAIT ) ;
	if( iReturn == 0 )
		return true ;
	else
		return false ;
}/*}}}*/

/*
 * -------------------------------------------------------------------------------------------------
 * class:	CMsg
 * funct:	RecvMsg
 * descr:	接收消息!
 * param:	消息指针(保存着消息内容)!
 * retur:	true:发送成功 false:发送失败!
 * -------------------------------------------------------------------------------------------------
 */
bool CMsg::RecvMsg( void * pVoid )
{/*{{{*/
	if( pVoid == NULL || m_MsgID == -1 || !IsMsgQueue() )
		return false ;

	int recv_byte = msgrcv(m_MsgID, pVoid, m_msgSize, m_MsgType , IPC_NOWAIT );
	if( recv_byte <= 0)
		return false;
	else
		return true;
}/*}}}*/
