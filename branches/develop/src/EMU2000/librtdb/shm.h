/***************************************************************************
  shm.h  -  description
 ***************************************************************************/
#ifndef _SHM_H_DEF_
#define _SHM_H_DEF_

#include <sys/ipc.h>
#include <sys/shm.h>

/***************************************************************************/
class CShm
{
	public:
		CShm();
		CShm(key_t tKey, int iSize, int iMode=0600);
		virtual ~CShm();

	protected:
		key_t m_tShmKey;
		int   m_iShmSize;

		int  m_bCreate;
		int  m_iShmId;
		char* m_pchShm;

	public:
		int  GetShm(int iMode=0600);
		long  AttachShm();
		void DetachShm();
		void RemoveShm();
		void InitShmVal();
		bool IfCreate() { return m_bCreate; }
		char* GetMemory() { return m_pchShm; }
		int   GetMemSize() { return m_iShmSize; }

		int  Open(key_t tKey, int iSize, int iMode=0600);
		int  Create(key_t tKey, int iSize, int iMode=0600);
};

/***************************************************************************/
#endif
