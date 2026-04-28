/***************************************************************************
  shm.cpp  -  description
 ***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <errno.h>

#include "shm.h"

/***************************************************************************/
CShm::CShm()
{
	m_pchShm  = NULL;
	m_bCreate = false;
	m_tShmKey = ftok(".", 'm');
	m_iShmSize = 0;
}

CShm::CShm(key_t tKey, int iSize, int iMode)
	: m_tShmKey(tKey), m_iShmSize(iSize)
{
	GetShm(iMode);
	AttachShm();
}

CShm::~CShm()
{
	DetachShm();
	RemoveShm();
}

int CShm::GetShm(int iMode)
{
	//try to create
	if( (m_iShmId = shmget(m_tShmKey, m_iShmSize, IPC_CREAT | IPC_EXCL | iMode)) < 0 )
	{
		if(errno != EEXIST)
		{
			perror("shmget(IPC_CREAT) fail");
			return -1;
		}

		if( (m_iShmId = shmget(m_tShmKey, m_iShmSize, iMode)) < 0 )
		{
			perror("shmget() fail");
			return -2;
		}
		m_bCreate = false;
		return 0;
	}
	m_bCreate = true;
	return 1;
}

int CShm::Open(key_t tKey, int iSize, int iMode)
{
	m_tShmKey  = tKey;
	m_iShmSize = iSize;
	if( (m_iShmId = shmget(m_tShmKey, m_iShmSize, iMode)) < 0 )
	{
		perror("shmget() fail");
		return -2;
	}
	m_bCreate = false;
	return 0;
}

int CShm::Create(key_t tKey, int iSize, int iMode)
{
	m_tShmKey  = tKey;
	m_iShmSize = iSize;
	if( (m_iShmId = shmget(m_tShmKey, m_iShmSize, IPC_CREAT | IPC_EXCL | iMode)) < 0 )
	{
		perror("shmget(IPC_CREAT) fail");
		return -1;
	}
	m_bCreate = true;
	return 1;
}

long CShm::AttachShm()
{
	m_pchShm = NULL;
	m_pchShm = (char*)shmat(m_iShmId, NULL, 0);
	//printf(" shmid = %d m_pchshm = %d \n", m_iShmId , m_pchShm );
	if( (long)m_pchShm == -1 )
	{
		perror("shmat fail");
		printf("get sharememory error\n");
	}
	return ((long)m_pchShm);
}

void CShm::DetachShm()
{
	if( (long)m_pchShm <= 0 ) return;
	if( shmdt(m_pchShm) < 0 )
	{
		perror("shmdt fail");
		return;
	}
	m_pchShm = NULL;
}

void CShm::RemoveShm()
{
	if( !m_bCreate ) return;
	if( shmctl(m_iShmId, IPC_RMID, NULL) < 0 )
	{
		perror("shmctl(IPC_RMID) fail");
		return;
	}
	m_bCreate = false;
}

void CShm::InitShmVal()
{
	memset(m_pchShm, 0, m_iShmSize);
}
