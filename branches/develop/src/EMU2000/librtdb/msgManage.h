
#if !defined(_MSGMANAGE_H__)
#define _MSGMANAGE_H__

#include "semObj.h"
#include "../share/msgdef.h"

#define	MSGBUSSEMKEY  20140101
/////////////////////////////////////////////////////////////////////////////
class CMsgManage
{
	public:
		CMsgManage();
		~CMsgManage();

	private:
		MSGSTORE *m_pMsgArea;
		CSemObj  m_hSemaphore;

	private:
		void  Init(void);
		int   GetBuffer();
		void  FreeBuffer( int nPos, int nCount=1 );
		void  CleanupSlot( int nSlotNo );
		int   LaunchMsg( MSGITEM* pMessage );

	public:
		void Open(char *pAddr);
		void Close(void);
		int  IsOpen(void);
		void TimerProc(void);
		void Dispatch(void);

		int FindDock(char *szName);
		int LoginBus(char *szName);
		int ExitBus(int nProcKey);
		int Subscribe(int nProcKey, unsigned int dwOption);
		int UnSubscribe(int nProcKey, unsigned int dwOption);
		int MessageSend(MSGITEM *pMessage, char *pDst=NULL);
		int MessageRecv(int nProcKey, MSGITEM *pMessage, int nSync=0);
};

/////////////////////////////////////////////////////////////////////////////
#endif
