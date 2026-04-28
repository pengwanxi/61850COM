#pragma once
#include "BaseWebPage.h"
#include "../share/Clog.h"
#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <vector>

using namespace std ;

class CManager
{
public:
	CManager();
	~CManager();

	void init();
	void addPage(CBaseWebPage * pPage);
	void getJSonStructString();
	UINT sendTo();
	void Get_Input();
	BOOL WriteDataToMemory(char *str,  key_t key);

protected:
	vector <CBaseWebPage*> m_vecPage;
	Clog m_log;
	string m_JSonString;	
};

