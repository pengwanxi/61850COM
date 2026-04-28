#include <stdio.h>
#include <math.h>
#include <vector>
#include <map>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <time.h>
#include <sys/time.h>
#include "GetlogContent.h"
using namespace std;
CGetlogContent m_getLog;

int main(int argc, char **argv)
{
	//argv[1] 第一次为空 以后每次为文件名
	if (argc != 2)
		return 0;

	m_getLog.getLogList();

	string strFilePath(argv[1]);
	if (strFilePath.empty())
		strFilePath = m_getLog.m_logList[0];

	m_getLog.getFileContentJSonString(strFilePath);

	return 0;
} 



