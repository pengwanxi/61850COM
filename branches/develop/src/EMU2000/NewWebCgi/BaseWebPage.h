#pragma once
#include "../share/typedef.h"
#include <json/json.h>
#include "../share/Clog.h"
#include "../share/md5.h"
#include "../share/global.h"
#include "../share/rdbFun.h"
#include "../share/profile.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <map>
#include <string>
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include<string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define BUFSZ 1024*10
#define	SHMPARSETBKEY  20210520 //参数设置页面共享内存key值

#define BUS_PATH	"/mynand/config/BusLine.ini"  //工程路径
#define MAX_LINE	200 //最大通讯行数据
#define	EMU2000_VERSION			0x01040100					/* EMU2000版本号 */
#define TRANS_PROTOCOL  0x01
#define GATHER_PROTOCOL 0x02
#define CONFIG_STATION_SUM  0x1001
#define CONFIG_RDBASE_SIZE  0x1002
#define CONFIG_EXTEND_SIZE  0x1003

#define CONFIG_STN_PARAM  0x2001
#define CONFIG_AI_PARAM  0x2101
#define CONFIG_DI_PARAM  0x2102
#define CONFIG_PI_PARAM  0x2103
#define CONFIG_DO_PARAM  0x2104
#define CONFIG_AO_PARAM  0x2105
#define CONFIF_DZ_PARAM 0x2106
#define ONE_EXTEND_PAGE 4096       //4K
#define MAX_EXTEND_SIZE 0x01000000 //16M

using namespace std;

typedef struct tagSTNDEF
{
	WORD  wNum;
	char  szName[16]; //装置名称
	WORD  wAICount; //遥测
	WORD  wDICount; //遥信
	WORD  wDOCount; //遥控
	WORD  wPICount; //遥脉
	WORD  wDZCount; //设点数量
} STNDEF;

/*lel*/
typedef struct tagSTNBusAddr
{
	BYTE byBusNo;	//总线
	WORD wDevAddr;	//地址

} STNBUSADDR;

class CBaseWebPage
{
public:
	CBaseWebPage();
	~CBaseWebPage();

	virtual BOOL getJSONStructFromWebPage(Json::Value &root) = 0;
	virtual BOOL procCmd( BYTE byCmd ) = 0;
	virtual void  Init() = 0;
	virtual void setLog(Clog * pLog) = 0;
	Clog * m_log;

public:
	string GetUptime();
	string GetVerSion();
	string GetSysTime();
	int ProtocolStyle(string str);
	void GetMemData(key_t key);



};

