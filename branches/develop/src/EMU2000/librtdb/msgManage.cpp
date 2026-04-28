/*******************************************************************************

  msgManage.cpp : Defines the application routines for the message buf.

 *******************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>  /* Standard libarary definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <fcntl.h>   /* File control definitions */
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/mman.h>

#include "msgManage.h"

///////////////////////////////////////////////////////////////////////////////
CMsgManage::CMsgManage()
{/*{{{*/
	m_pMsgArea = NULL;
}/*}}}*/

CMsgManage::~CMsgManage()
{/*{{{*/
	Close();
}/*}}}*/

int CMsgManage::IsOpen(void)
{/*{{{*/
	if( m_pMsgArea ) return 1;
	return 0;
}/*}}}*/

void CMsgManage::Open(char *pAddr)
{/*{{{*/
	if( pAddr == NULL ) return;
	m_pMsgArea = (MSGSTORE*)pAddr;
	m_hSemaphore.Create( MSGBUSSEMKEY );
	if( m_pMsgArea->nQuality != 0xAA55)
	{
		Init();
		m_pMsgArea->nQuality = 0xAA55;
	}
}/*}}}*/

void CMsgManage::Close(void)
{/*{{{*/
	m_hSemaphore.Remove();
	m_pMsgArea = NULL;
}/*}}}*/

void CMsgManage::Init()
{/*{{{*/
	int     i;
	MSGLIST *pList;
	m_hSemaphore.semTake();
	for(i=0; i<MSG_SLOT_SUM; i++)
	{
		m_pMsgArea->slot[i].nStatus  = 0;
		m_pMsgArea->slot[i].nProcKey = -1;
		m_pMsgArea->slot[i].dwMsgCtrl = 0;
		m_pMsgArea->slot[i].szProcName[0] = '\0';
		m_pMsgArea->slot[i].nMsgPos = -1;
	}
	m_pMsgArea->nFreeNum = MSG_POOL_SUM;
	m_pMsgArea->nFreePos  = 0;
	m_pMsgArea->nFreeTail = MSG_POOL_SUM - 1;
	for(i=0; i<MSG_POOL_SUM; i++)
	{
		m_pMsgArea->pool[i].nSrcKey = -1;
		m_pMsgArea->pool[i].nActive = 0;
		m_pMsgArea->pool[i].wLevel = 0;
		m_pMsgArea->pool[i].wTypes = 0;
		m_pMsgArea->pool[i].wMsgLen = 0;
		pList = &m_pMsgArea->vect[i];
		pList->self = i;
		if( i < (MSG_POOL_SUM - 1)  )
		{
			pList->next = i+1;
		}
		else
		{
			pList->next = -1;
		}
	}
	m_hSemaphore.semGive();
}/*}}}*/

int CMsgManage::GetBuffer()
{/*{{{*/
	if( m_pMsgArea->nFreePos < 0 ) return -1;
	MSGLIST *pList = &m_pMsgArea->vect[m_pMsgArea->nFreePos];
	m_pMsgArea->nFreePos = pList->next;
	if( m_pMsgArea->nFreePos < 0 ) 
		m_pMsgArea->nFreeTail = -1;
	m_pMsgArea->nFreeNum--;
	pList->next = -1;
	return pList->self;
}/*}}}*/

void CMsgManage::FreeBuffer( int nPos, int nCount )
{/*{{{*/
	MSGLIST *pList;
	if( nPos < 0 ) return;
	if( m_pMsgArea->nFreePos < 0 )
	{
		m_pMsgArea->nFreePos = nPos;
		pList = &m_pMsgArea->vect[nPos];
		while( pList->next >= 0 )
		{
			pList = &m_pMsgArea->vect[pList->next];
		}
		m_pMsgArea->nFreeTail = pList->self;
	}
	else
	{
		m_pMsgArea->vect[m_pMsgArea->nFreeTail].next = nPos;
		pList = &m_pMsgArea->vect[nPos];
		while( pList->next >= 0 )
		{
			pList = &m_pMsgArea->vect[pList->next];
		}
		m_pMsgArea->nFreeTail = pList->self;
	}
	m_pMsgArea->nFreeNum += nCount;
}/*}}}*/

void CMsgManage::CleanupSlot( int nSlotNo )
{/*{{{*/
	if( nSlotNo<0 || nSlotNo>=MSG_SLOT_SUM ) return;
	PROCSLOT *pSlot = &m_pMsgArea->slot[nSlotNo];
	if( pSlot->nMsgPos >= 0 )
	{
		m_hSemaphore.semTake();
		int nCount=1;
		MSGLIST *pList = &m_pMsgArea->vect[pSlot->nMsgPos];
		while( pList->next >= 0 )
		{
			nCount++;
			pList = &m_pMsgArea->vect[pList->next];
		}
		FreeBuffer(pSlot->nMsgPos, nCount);
		m_hSemaphore.semGive();
	}
}/*}}}*/

int CMsgManage::LaunchMsg( MSGITEM* pMessage )
{/*{{{*/
	return 0;
}/*}}}*/

void CMsgManage::TimerProc(void)
{/*{{{*/
	int i, nPos, k;
	if( !m_pMsgArea ) return;
	m_hSemaphore.semTake();
	for( i=0; i<MSG_SLOT_SUM; i++ )
	{
		PROCSLOT *pSlot = &m_pMsgArea->slot[i];
		nPos = pSlot->nMsgPos;
		while( nPos >= 0 )
		{
			MSGITEM *pMsg  = &m_pMsgArea->pool[nPos];
			MSGLIST *pList = &m_pMsgArea->vect[nPos];
			pMsg->nActive--;
			if( pMsg->nActive < 0 )
			{
				k = nPos;
				if( pSlot->nMsgPos == nPos )
					pSlot->nMsgPos = pList->next;
				nPos = pList->next;
				pList->next = -1;
				FreeBuffer( k );
			}
			else
			{
				nPos = pList->next;
			}
		}
	}
	m_hSemaphore.semGive();
}/*}}}*/

int CMsgManage::FindDock(char *szName)
{/*{{{*/
	if( !m_pMsgArea ) return -1;
	for(int i=0; i<MSG_SLOT_SUM; i++)
	{
		if( m_pMsgArea->slot[i].nProcKey <= 0 ) continue;
		if( strcmp(m_pMsgArea->slot[i].szProcName, szName) != 0 ) continue;
		return m_pMsgArea->slot[i].nProcKey;
	}
	return -2;
}/*}}}*/

///////////////////////////////////////////////////////////////////////////////
int CMsgManage::LoginBus(char *szName)
{/*{{{*/
	if( !m_pMsgArea ) return -1;
	for(int i=0; i<MSG_SLOT_SUM; i++)
	{
		if( m_pMsgArea->slot[i].nProcKey > 0 )
		{
			if( strcmp(m_pMsgArea->slot[i].szProcName, szName) != 0 ) continue;
			CleanupSlot( i );
		}
		else
		{
			sprintf(m_pMsgArea->slot[i].szProcName, szName);
		}
		m_pMsgArea->slot[i].nStatus  = 0;
		m_pMsgArea->slot[i].nProcKey = i+1;
		m_pMsgArea->slot[i].dwMsgCtrl = 0;
		m_pMsgArea->slot[i].nMsgPos = -1;

		return m_pMsgArea->slot[i].nProcKey;
	}
	return -2;
}/*}}}*/

int CMsgManage::Subscribe(int nProcKey, unsigned int dwOption)
{/*{{{*/
	if( !m_pMsgArea ) return -1;
	if( nProcKey<=0 || nProcKey>MSG_SLOT_SUM ) return -2;
	PROCSLOT *pSlot = &m_pMsgArea->slot[nProcKey-1];
	if( pSlot->nProcKey <= 0 ) return -3;

	pSlot->dwMsgCtrl |= dwOption;

	return 1;
}/*}}}*/

int CMsgManage::UnSubscribe(int nProcKey, unsigned int dwOption)
{/*{{{*/
	if( !m_pMsgArea ) return -1;
	if( nProcKey<=0 || nProcKey>MSG_SLOT_SUM ) return -2;
	PROCSLOT *pSlot = &m_pMsgArea->slot[nProcKey-1];
	if( pSlot->nProcKey <= 0 ) return -3;
	pSlot->dwMsgCtrl &= ~dwOption;
	return 1;
}/*}}}*/

int CMsgManage::ExitBus(int nProcKey)
{/*{{{*/
	if( !m_pMsgArea ) return -1;
	if( nProcKey<=0 || nProcKey>MSG_SLOT_SUM ) return -1;
	//	if( nProcKey != pSlot->nProcKey ) return -2;
	PROCSLOT *pSlot = &m_pMsgArea->slot[nProcKey-1];
	if( pSlot->nMsgPos >= 0 )
	{
		m_hSemaphore.semTake();
		int nCount=1;
		MSGLIST *pList = &m_pMsgArea->vect[pSlot->nMsgPos];
		while( pList->next >= 0 )
		{
			nCount++;
			pList = &m_pMsgArea->vect[pList->next];
		}
		FreeBuffer(pSlot->nMsgPos, nCount);
		m_hSemaphore.semGive();
	}
	pSlot->nStatus   = 0;
	pSlot->nProcKey  = -1;
	pSlot->dwMsgCtrl = 0;
	pSlot->nMsgPos   = -1;
	pSlot->szProcName[0] = '\0';
	return 0;
}/*}}}*/

int CMsgManage::MessageSend(MSGITEM *pMessage, char *pDst)
{/*{{{*/
	PROCSLOT *pSlot;
	MSGITEM  *pMsgBuf;
	int i, nPos, nCount=0;

	if( !m_pMsgArea ) return -1;
	if( !pMessage || pMessage->wMsgLen>MSG_BODY_LEN ) return -2;
	m_hSemaphore.semTake();
	for( i=0; i<MSG_SLOT_SUM; i++ )
	{
		pSlot = &m_pMsgArea->slot[i];
		if( pSlot->nProcKey < 0 ) continue;
		if( pSlot->nProcKey == pMessage->nSrcKey ) continue;
		if( (pSlot->dwMsgCtrl & (1<<pMessage->wTypes)) == 0 ) continue;

		nPos = GetBuffer();
		if( nPos < 0 ) break;
		pMsgBuf = &m_pMsgArea->pool[nPos];
		memcpy(pMsgBuf, pMessage, 12+pMessage->wMsgLen);
		if( pSlot->nMsgPos < 0 )
		{
			pSlot->nMsgPos = nPos;
		}
		else
		{
			MSGLIST *pList = &m_pMsgArea->vect[pSlot->nMsgPos];
			while( pList->next >= 0 )
			{
				pList = &m_pMsgArea->vect[pList->next];
			}
			pList->next = nPos;
		}
		pSlot->nStatus++;
		nCount++;
	}
	m_hSemaphore.semGive();
	return nCount;
}/*}}}*/

int CMsgManage::MessageRecv(int nProcKey, MSGITEM *pMessage, int nSync)
{/*{{{*/
	MSGLIST *pList;
	MSGITEM *pBuff;
	int nPos, nLen=0;

	if( !m_pMsgArea ) return -1;
	if( !pMessage || nProcKey<=0 || nProcKey>MSG_SLOT_SUM ) return -2;
	PROCSLOT *pSlot = &m_pMsgArea->slot[nProcKey-1];
	if( pSlot->nProcKey < 0 ) return -3;
	if( pSlot->nMsgPos < 0 ) return 0;

	m_hSemaphore.semTake();
	nPos = pSlot->nMsgPos;
	pBuff = &m_pMsgArea->pool[nPos];
	nLen = pBuff->wMsgLen;
	memcpy(pMessage, pBuff, 12+nLen);

	pList = &m_pMsgArea->vect[nPos];
	pSlot->nMsgPos = pList->next;
	pList->next = -1;
	FreeBuffer( nPos );
	pSlot->nStatus--;
	m_hSemaphore.semGive();
	return nLen;
}/*}}}*/

///////////////////////////////////////////////////////////////////////////////
