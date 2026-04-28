#include "SysOperatePage.h"


CSysOperatePage::CSysOperatePage()
{
}


CSysOperatePage::~CSysOperatePage()
{
}

BOOL CSysOperatePage::getJSONStructFromWebPage(Json::Value &root)
{
	return TRUE;
}

BOOL CSysOperatePage::procCmd(BYTE byCmd)
{

	return FALSE;
}

void CSysOperatePage::Init()
{
	return ;
}

void CSysOperatePage::setLog(Clog * pLog)
{
	m_log = pLog;
}
void CSysOperatePage::SysRebootOperation()
{
	system("reboot");
}
void CSysOperatePage::SysRebootAndNewProject()
{


}
