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
#include "Manager.h"

using namespace std;
CManager g_Manager;

int main(int argc, char **argv)
{
	g_Manager.init();
	g_Manager.getJSonStructString();
	//g_Manager.Get_Input();
	g_Manager.sendTo();	
	return 0;
} 
