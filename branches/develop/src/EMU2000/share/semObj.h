/***************************************************************************
  semObj.h  -  description
 ***************************************************************************/
#ifndef _SEMOBJ_H_
#define _SEMOBJ_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>


#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
	int val;                    /* value for SETVAL */
	struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
	unsigned short int *array;  /* array for GETALL, SETALL */
	struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif

/*****************************************************************************/

class CSemObj
{
	protected:
		int m_iSemId;   //信号量标示符
		key_t m_tKey;   //信号量键值
		bool m_bCreate; //创建标志

	public:
		CSemObj();
		CSemObj(key_t tKey, int iMode = 0600);
		virtual ~CSemObj();

		int  Create(key_t tKey, int iMode = 0600);
		void Remove();
		bool semTake();
		void semGive();
		void SetValue(int iValue);
		int  GetValue();

		bool IfCreate() { return m_bCreate; }
};
/*****************************************************************************/
#endif
