/***************************************************************************
  semaphore.cpp  -  description
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include "semObj.h"
/*****************************************************************************/

CSemObj::CSemObj()
{
	m_iSemId = -1;
	m_bCreate = false;
	m_tKey = ftok(".", 's');
}

CSemObj::CSemObj(key_t tKey, int iMode)
	: m_tKey(tKey)
{
	m_bCreate = false;
	Create(m_tKey, iMode);
}

CSemObj::~CSemObj()
{
	if(m_bCreate)
		Remove();
}

int CSemObj::Create(key_t tKey, int iMode)
{
	m_tKey = tKey;
	if( (m_iSemId = semget(m_tKey, 1, IPC_CREAT | IPC_EXCL | iMode)) < 0 )
	{
		if(errno != EEXIST)
		{
			perror("semget(IPC_CREAT) fail");
			return -1;
		}
		if( (m_iSemId = semget(m_tKey, 1, iMode)) < 0 )
		{
			perror("semget() fail");
			return -2;
		}		
		m_bCreate = false;
	}
	else
	{
		SetValue(1);
		m_bCreate = true;
	}
	return 0;
}

void CSemObj::Remove()
{
	union semun unSem;
	semctl(m_iSemId, 0, IPC_RMID, unSem);
	m_iSemId = -1;
	m_bCreate = false;
}

bool CSemObj::semTake()
{
	struct sembuf semb;
	semb.sem_num = 0;
	semb.sem_op  = -1;
	semb.sem_flg = SEM_UNDO;
	if( semop(m_iSemId, &semb, 1) < 0 )
	{
		if(errno == EAGAIN || errno == EINTR)
		{
			return false;
		}
		return false;	
	}
	return true;
}

void CSemObj::semGive()
{
	struct sembuf semb;
	semb.sem_num = 0;
	semb.sem_op  = 1;
	semb.sem_flg = SEM_UNDO;
	semop(m_iSemId, &semb, 1);
}

void CSemObj::SetValue(int iValue)
{
	semun unSem;
	unSem.val = iValue;
	semctl(m_iSemId, 0, SETVAL, unSem);
}

int CSemObj::GetValue()
{
	semun unSem;
	int iValue = semctl(m_iSemId, 0, GETVAL, unSem);
	return iValue;
}
