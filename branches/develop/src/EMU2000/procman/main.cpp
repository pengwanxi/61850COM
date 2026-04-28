#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <sys/reboot.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>

#include "proc.h"
#include "../share/global.h"
#include "CWatchDog.h"
#include "../Protocol/SocketFtp/CSocketFtp.h"
#include "../Protocol/DevCheck/CSocketDevCheck.cpp"

#define WATCHDOG
#define RUN_LED_NUM  1  //1-8
#define WDOG_TIMEOUT 5

#define CMD_SET_LEDRUN_ON  21 //LED运行灯 开
#define CMD_SET_LEDRUN_OFF  22 //LED运行灯 关

/*****************************************************************************/
//配置文件路径
char g_szAppPath[64] = {"/mynand"};
//环境变量参数
char g_szEnviVar[160] = {"LD_LIBRARY_PATH=/mynand/usrlib"};

BOOL g_bAppRun = TRUE;   //进程运行
WORD g_wAppLED = 0x0001; //进程LED
long g_hLedPort = -1;    //ADC设备

int  g_nWDogTout = 0;    //看门狗时间
int  g_hWatchDog = -1;   //看门狗设备

PROC_DB  g_ProcDB;       //进程数据
CWatchDog g_Watchdog ;

/*****************************************************************************/
#ifdef	__cplusplus
extern "C" {/*{{{*/
#endif	/* __cplusplus */

	int already_running(const char *filename);
	void ltrim(char *s);
	void rtrim(char *s);
	void OutPromptText(char *lpszText);
	void LogPromptText(const char *fmt, ...);
	void OutMessageText(char *szSrc, unsigned char *pData, int nLen);
	int SignalHook(int iSigNo, LPSIGPROC func);
	void init_daemon(void);
	void SignHandler(int signum, siginfo_t *pInfo, void *pReserved)
	{
		g_bAppRun = FALSE;
	}
#ifdef	__cplusplus
}/*}}}*/
#endif	/* __cplusplus */

/*===========================================================================*/
void InitProcParam(void)
{/*{{{*/
	int i, j;
	g_ProcDB.wCount = 0;
	g_ProcDB.wflag  = 0;
	for(i=0; i<PROC_MAX_SUM; i++)
	{
		g_ProcDB.proc[i].wProcType = 0;
		g_ProcDB.proc[i].wStartMode = 0;
		g_ProcDB.proc[i].lStartTime = 0;
		g_ProcDB.proc[i].szDescribe[0] = '\0';
		g_ProcDB.proc[i].szExecPath[0] = '\0';
		g_ProcDB.proc[i].szProcName[0] = '\0';
		g_ProcDB.proc[i].szParam[0] = '\0';
		g_ProcDB.proc[i].szSerialSum[0] = '\0';
		for(j=0; j<PARA_MAX_SUM+2; j++)
			g_ProcDB.proc[i].argv[j] = 0;
		g_ProcDB.proc[i].hProcess  = -1;
		g_ProcDB.proc[i].nRunState = 0;
		g_ProcDB.proc[i].nErrTimer = 0;
		g_ProcDB.proc[i].nRestoreNum = 0;
	}
}/*}}}*/

#define CONFIG_ENV_VAR     0x1001
#define CONFIG_PROC_STYLE  0x2001
#define CONFIG_PROC_PARAM  0x2002

int ParseConfigItem(char *strItem, WORD *pwNum)
{/*{{{*/
	char  strType[32];
	int   i, nLen;

	if( strstr(strItem, "environ") ) return CONFIG_ENV_VAR;
	i = 0;
	nLen = strlen(strItem);
	while( !isdigit(strItem[i]) && i<(int)sizeof(strType) )
	{
		strType[i] = toupper(strItem[i]);
		if( ++i >= nLen ) break;
	}

	strType[i] = '\0';
	if( i >= nLen ) *pwNum = 0;
	else *pwNum = (WORD)atoi(&strItem[i]);

	if( strcmp(strType, "PROC") == 0 )
		return CONFIG_PROC_STYLE;
	if( strcmp(strType, "PARA") == 0 )
		return CONFIG_PROC_PARAM;
	return -1;
}/*}}}*/

char* GetFileName(char* szPath)
{/*{{{*/
	int i, nLen;
	char *p = szPath;

	nLen = strlen(szPath);
	if( nLen <= 0 ) return NULL;
	for(i=nLen-1; i>0; i--)
	{
		if( szPath[i]=='\\' ||
				szPath[i]=='/' )
		{
			p = &szPath[i+1];
			break;
		}
	}
	return p;
}/*}}}*/

BOOL SetProcAttrib(WORD wNum, char* szParam)
{/*{{{*/
	int  i = 0;
	char *tok, *p;
	if( wNum >= PROC_MAX_SUM ) return FALSE;
	if( strlen(szParam) <= 0 ) return FALSE;

	tok = strtok(szParam, ",");
	while( tok )
	{
		rtrim(tok);
		switch(i)
		{
			case 0: //进程描述
				sprintf(g_ProcDB.proc[wNum].szDescribe, tok);
				break;
			case 1: //执行路径
				sprintf(g_ProcDB.proc[wNum].szExecPath, tok);
				p = GetFileName(tok);
				if( p ) sprintf(g_ProcDB.proc[wNum].szProcName, p);
				g_ProcDB.proc[wNum].argv[0] = g_ProcDB.proc[wNum].szProcName;
				break;
			case 2: //进程类型
				g_ProcDB.proc[wNum].wProcType = (WORD)atoi(tok);
				break;
			case 3: //启动模式
				g_ProcDB.proc[wNum].wStartMode = (WORD)atoi(tok);
				break;
			case 4: //等待时间
				g_ProcDB.proc[wNum].lStartTime = (LONG)atol(tok);
				break;
			case 5: //串口个数
				sprintf(g_ProcDB.proc[wNum].szSerialSum, tok);
				g_ProcDB.proc[wNum].argv[1] = g_ProcDB.proc[wNum].szSerialSum;
				break;
			default:
					break;
		}
		tok = strtok(NULL, ",");
		i++;
	}
	return TRUE;
}/*}}}*/

void GetParamArray(WORD wNum, char *lpszText)
{/*{{{*/
	int  i  = 0;
	char *s = lpszText;
	char *p = g_ProcDB.proc[wNum].szParam;
	g_ProcDB.proc[wNum].argv[1] = p;

	while( (*p = *s++) != 0 )
	{
		if( *p==' ' && (*s==' ' || *s=='\0') )
		{
			continue;
		}
		if( *p=='[' || *p==']' || *p=='(' || *p==')' )
		{
			continue;
		}

		if( *p != ' ' && *p != '\t' )
		{
			p++;
		}
		else
		{
			*p++ = '\0';
			if(i>=PARA_MAX_SUM)
				break;

			g_ProcDB.proc[wNum].argv[2+i] = p;
			i++;
		}
	}
	*p++ = '\0';

	/*
	   printf("[%s]\n", lpszText);
	   for(i=0; i<PARA_MAX_SUM+2; i++)
	   {
	   if( g_ProcDB.proc[wNum].argv[i] )
	   printf("%d = %s\n", i, g_ProcDB.proc[wNum].argv[i]);
	   }
	   */
}/*}}}*/

BOOL ReadProcConfig(void)
{/*{{{*/
	FILE *hFile;
	char szText[160];
	char *pItem, *pParam;
	WORD wNum;

	//读配置信息
	sprintf(szText, "%s/procman.conf", g_szAppPath);
	hFile = fopen(szText, "r");
	if( hFile != NULL  )
	{
		LogPromptText("Open file %s ok.\n", szText);
		while( fgets(szText, sizeof(szText), hFile) )
		{

			rtrim(szText);
			//ltrim(szText);
			if( szText[0]==';' || szText[0]=='#' ) continue;
			//分离文本行
			pItem = strtok(szText, "=");
			if( pItem == NULL ) continue;
			pParam = strtok(NULL, "@");

			if( pParam )
			{
				ltrim(pParam);
				//解析配置行
				int nType = ParseConfigItem(pItem, &wNum);

				switch( nType )
				{
				case CONFIG_ENV_VAR:
					sprintf(g_szEnviVar, pParam);
					break;
				case CONFIG_PROC_STYLE:
					SetProcAttrib(g_ProcDB.wCount, pParam);
					g_ProcDB.wCount++;
					break;
				case CONFIG_PROC_PARAM:
					GetParamArray(wNum-1, pParam);
					break;
				}
			}
		}
		fclose(hFile);
	}
	else
		return FALSE ;

	return TRUE ;
}/*}}}*/

pid_t StartChildProc(char* szPath, char *argv[])
{/*{{{*/
	pid_t child;

	child = fork();
	if( child < 0 )
	{
		perror("Create child process fail");
		return -1;
	}

	/*子进程*/
	else if( child == 0 )
	{
	//char buf[256]="/bin/valgrind";										//of mine!
	//char *buf0[8]={"--leak-check=full", szPath};										//of mine!
		if( execv(szPath, argv) < 0 )
		//if( execve(buf, buf0, argv) < 0 )			//added by cyz!
		{
			perror( szPath );				//noted by cyz!
			//perror( buf );
			exit(-2);
		}
		return 0;
	}

	return child;
}/*}}}*/

void StartAppProc(void)
{/*{{{*/
	int    i;
	pid_t  child;

	for(i=0; i<g_ProcDB.wCount; i++)
	{
		//if(g_ProcDB.proc[i].wStartMode==0) continue;
		if(g_ProcDB.proc[i].lStartTime>0)
			sleep( g_ProcDB.proc[i].lStartTime );
		LogPromptText("................................start process(%s)................................\n",
				g_ProcDB.proc[i].szExecPath);
		child = StartChildProc(g_ProcDB.proc[i].szExecPath, g_ProcDB.proc[i].argv);
		if( child > 0 )
		{
			LogPromptText("process(%s) started(pid=%u) \n", g_ProcDB.proc[i].szDescribe, child);
			g_ProcDB.proc[i].hProcess  = child;
			g_ProcDB.proc[i].nRunState = 1;
			g_ProcDB.proc[i].nErrTimer = 0;
			g_ProcDB.proc[i].nRestoreNum = 0;
		}
	}

	g_ProcDB.wflag = 1;
}/*}}}*/

typedef struct statstruct_proc
{/*{{{*/
	int           pid;                      /** The process id. **/
	char          exName[_POSIX_PATH_MAX];  /** The filename of the executable **/
	char          state; /** 1 **/          /** R is running, S is sleeping,
											  D is sleeping in an uninterruptible wait,
											  Z is zombie, T is traced or stopped **/
	unsigned      euid,                     /** effective user id **/
				  egid;                     /** effective group id */
	int           ppid;                     /** The pid of the parent. **/
	int           pgrp;                     /** The pgrp of the process. **/
	int           session;                  /** The session id of the process. **/
	int           tty;                      /** The tty the process uses **/
	int           tpgid;                    /** (too long) **/
	unsigned int  flags;                    /** The flags of the process. **/
	unsigned int  minflt;                   /** The number of minor faults **/
	unsigned int  cminflt;                  /** The number of minor faults with childs **/
	unsigned int  majflt;                   /** The number of major faults **/
	unsigned int  cmajflt;                  /** The number of major faults with childs **/
	int           utime;                    /** user mode jiffies **/
	int           stime;                    /** kernel mode jiffies **/
	int           cutime;                   /** user mode jiffies with childs **/
	int           cstime;                   /** kernel mode jiffies with childs **/
	int           counter;                  /** process's next timeslice **/
	int           priority;                 /** the standard nice value, plus fifteen **/
	unsigned int  timeout;                  /** The time in jiffies of the next timeout **/
	unsigned int  itrealvalue;              /** The time before the next SIGALRM is sent to the process **/
	int           starttime; /** 20 **/     /** Time the process started after system boot **/
	unsigned int  vsize;                    /** Virtual memory size **/
	unsigned int  rss;                      /** Resident Set Size **/
	unsigned int  rlim;                     /** Current limit in bytes on the rss **/
	unsigned int  startcode;                /** The address above which program text can run **/
	unsigned int  endcode;                  /** The address below which program text can run **/
	unsigned int  startstack;               /** The address of the start of the stack **/
	unsigned int  kstkesp;                  /** The current value of ESP **/
	unsigned int  kstkeip;                  /** The current value of EIP **/
	int           signal;                   /** The bitmap of pending signals **/
	int           blocked; /** 30 **/       /** The bitmap of blocked signals **/
	int           sigignore;                /** The bitmap of ignored signals **/
	int           sigcatch;                 /** The bitmap of catched signals **/
	unsigned int  wchan;  /** 33 **/        /** (too long) **/
	int        sched,              /** scheduler **/
			   sched_priority;     /** scheduler priority **/
} procinfo;/*}}}*/

int GetProcessInfo( pid_t pid, procinfo *pinfo )
{/*{{{*/
	FILE  *fp;
	char  szFileName[FILENAME_MAX];
	char  szStat[2048], *s, *t;
	struct stat  st;

	sprintf( szFileName, "/proc/%d/stat", pid );
	//检查是否存在
	if( -1 == access(szFileName, F_OK) ) return -1;
	//检查是否有权限可读
	if( -1 == access(szFileName, R_OK) ) return 0;
	//获取文件信息
	if( -1 != stat(szFileName, &st) )
	{
		pinfo->euid = st.st_uid;
		pinfo->egid = st.st_gid;
	}
	else
	{
		pinfo->euid = pinfo->egid = 0;
	}
	//打开文件
	if( (fp = fopen(szFileName, "r")) == NULL ) return 0;
	//获取状态串
	if( (s = fgets(szStat, sizeof(szStat), fp)) == NULL )
	{
		fclose(fp);
		return 0;
	}
	/** 得到pid **/
	sscanf(szStat, "%u", &(pinfo->pid));
	/** 得到exename **/
	s = strchr(szStat, '(') + 1;
	t = strchr(szStat, ')');
	strncpy(pinfo->exName, s, t-s);
	pinfo->exName[t-s] = '\0';
	/** 获得进程信息 **/
	sscanf( t + 2, "%c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
			/*       1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33*/
			&(pinfo->state),
			&(pinfo->ppid),
			&(pinfo->pgrp),
			&(pinfo->session),
			&(pinfo->tty),
			&(pinfo->tpgid),
			&(pinfo->flags),
			&(pinfo->minflt),
			&(pinfo->cminflt),
			&(pinfo->majflt),
			&(pinfo->cmajflt),
			&(pinfo->utime),
			&(pinfo->stime),
			&(pinfo->cutime),
			&(pinfo->cstime),
			&(pinfo->counter),
			&(pinfo->priority),
			&(pinfo->timeout),
			&(pinfo->itrealvalue),
			&(pinfo->starttime),
			&(pinfo->vsize),
			&(pinfo->rss),
			&(pinfo->rlim),
			&(pinfo->startcode),
			&(pinfo->endcode),
			&(pinfo->startstack),
			&(pinfo->kstkesp),
			&(pinfo->kstkeip),
			&(pinfo->signal),
			&(pinfo->blocked),
			&(pinfo->sigignore),
			&(pinfo->sigcatch),
			&(pinfo->wchan) );

	fclose(fp);
	return 1;
}/*}}}*/

int GetProcessStat( pid_t pid )
{/*{{{*/
	FILE  *fp;
	char  szFileName[FILENAME_MAX];
	char  szStat[1024], *s;//, *t;
	pid_t idProc;
	char  szExeName[64], state='X';

	sprintf( szFileName, "/proc/%d/stat", pid );
	//检查是否存在
	if( -1 == access(szFileName, F_OK) ) return -1;
	//检查是否有权限可读
	if( -1 == access(szFileName, R_OK) ) return 0;
	//打开文件
	if( (fp = fopen(szFileName, "r")) == NULL ) return 0;
	//获取状态串
	if( (s = fgets(szStat, sizeof(szStat), fp)) == NULL )
	{
		fclose(fp);
		return 0;
	}
	/** 得到pid **/
	//	sscanf(szStat, "%u", &idProc);
	/** 得到exename **/
	//	s = strchr(szStat, '(') + 1;
	//	t = strchr(szStat, ')');
	//	strncpy(szExeName, s, t-s);
	//	szExeName[t-s] = '\0';
	/** 获得进程信息 **/
	sscanf( szStat, "%u %s %c", &idProc, szExeName, &state );
	fclose(fp);

	//	printf("ProcessInfo: %u %s %c\n", idProc, szExeName, state);
	return state;
}/*}}}*/

void WatchAppProc(WORD wSecond)
{/*{{{*/
	int    i, state;
	pid_t  child;

	for( i=0; i<g_ProcDB.wCount; i++ )
	{
		if( g_ProcDB.proc[i].hProcess < 0 ) continue;
		state = GetProcessStat(g_ProcDB.proc[i].hProcess);
		/* R is running, S is sleeping, D is sleeping in an uninterruptible wait,
		   Z is zombie, T is traced or stopped */
		if( state < 0 || state=='Z' ) //进程异常
		{
			g_ProcDB.proc[i].nRunState = 3;
			g_ProcDB.proc[i].nErrTimer += wSecond;
			if( g_ProcDB.proc[i].nErrTimer < 30 ) continue;

			g_ProcDB.proc[i].nErrTimer = 0;
			g_ProcDB.proc[i].nRestoreNum++;
			if( g_ProcDB.proc[i].wProcType < 2 )
			{
				child = StartChildProc(g_ProcDB.proc[i].szExecPath, g_ProcDB.proc[i].argv);
				if( child > 0 ) //启动成功
				{
					LogPromptText("restart process(%s:%u) ok \n", g_ProcDB.proc[i].szDescribe, child);
					g_ProcDB.proc[i].hProcess  = child;
					g_ProcDB.proc[i].nRunState = 1;
					g_ProcDB.proc[i].nErrTimer = 0;
					g_ProcDB.proc[i].nRestoreNum = 0;
				}
				else			//启动失败
				{
					switch(g_ProcDB.proc[i].wProcType)
					{
					case 0: //普通进程
						break;
					case 1: //重要进程
						if(g_ProcDB.proc[i].nRestoreNum>3)
						{
							g_ProcDB.wflag = 2;
							g_bAppRun = FALSE;
							return;
						}
						break;
					}
				}
			}
			else //关键进程
			{
				g_ProcDB.wflag = 2;
				g_bAppRun = FALSE;
				return;
			}
		}
		else if(state=='R' || state=='S') //进程正常
		{
			if( g_ProcDB.proc[i].nRunState != 2 )
				g_ProcDB.proc[i].nRunState = 2;
			g_ProcDB.proc[i].nErrTimer = 0;
			g_ProcDB.proc[i].nRestoreNum = 0;
		}
	}
}/*}}}*/

// by zhanghg
//开关运行灯
int gpiofd = -1 ;
static void OpenRunLed(  )
{/*{{{*/
	/*open gpio drv*/
	// gpiofd = open( "/dev/gpio_drv" , O_RDWR);
	gpiofd = open( "/dev/esd_gpout" , O_RDWR);
	if( gpiofd < 0 )
	{
		gpiofd = -1 ;
		return;
	}
}/*}}}*/

static void CloseRunLen( )
{/*{{{*/
	if( gpiofd >= 0 )
	{
		close(gpiofd);
		gpiofd = -1 ;
	}
}/*}}}*/

static void RunLed( )
{/*{{{*/
	static BYTE byVal = 0 ;
	byVal ^= 1 ;

	int bOn = CMD_SET_LEDRUN_OFF ;
	if( byVal )
		bOn = CMD_SET_LEDRUN_ON ;

	// ioctl(gpiofd, bOn, 0);
    char data[2];
    data[0] = 0;
    data[1] = bOn == CMD_SET_LEDRUN_ON ? 1 : 0;
    write(gpiofd, data, sizeof(data));
}/*}}}*/

/*
 * ===  FUNCTION  ======================================================================
 *         Name:  CreateSocketFtp
 *  Description:
 *		  Param:
 *		 Return:
 * =====================================================================================
 */
static void CreateSocketFtp ( CSocketFtp **pSocketFtp  )
{/*{{{*/
	mkdir( "/myapp/bak", 0755 );
	mkdir( "/mynand/bak",0755 );
	mkdir( "/myapp/downuser", 0755 );
	mkdir( "/mynand/downprgm", 0755 );
	// CSocketFtp *pSocketFtp = NULL;
	*pSocketFtp = new CSocketFtp(  );
	if( NULL == ( *pSocketFtp ) )
	{
		printf ( "CreateSocketFtp failed\n" );
		return;
	}

	if( !( *pSocketFtp )->Init() )
	{
		printf ( "CreateSocketFtp Init failed\n" );
		return;
	}

	if( !( *pSocketFtp )->m_pProto->m_FtpFile.IsFileExist( FTP_FILE_LIST ) )
	{
		( *pSocketFtp )->m_pProto->m_FtpFile.UpdateCfg();
	}

	( *pSocketFtp )->CreateThread(  );


	return ;
}


/*
* ===  FUNCTION  ======================================================================
*         Name:  CreateSocketDevCheck
*  Description:
*		  Param:
*		 Return:
* =====================================================================================
*/
static void CreateSocketDevCheck(CSocketDevCheck **pSocketDevCheck)
{/*{{{*/
	*pSocketDevCheck = new CSocketDevCheck( );
	if (NULL == (*pSocketDevCheck))
	{
		printf("CreateSocketFtp failed\n");
		return;
	}
	if (!(*pSocketDevCheck)->Init())
	{
		printf("CreateSocketDevCheck Init failed\n");
		return;
	}
	return;
}

/*****************************************************************************/
int main(int argc, char **argv)
{/*{{{*/

	int    nSecond = 5;
	struct timeval  tv;

	if ( already_running( "/mynand/config/procman.lock" ) )
	{
		printf ( "the Procman is running \n" );
		return -1;
	}
	else
		printf ( "the Procman Begin to Run\n" );

	printf("------------Start Process Manager------------\n");
	if( argc>=4 && atoi(argv[3])>0 )
	{
		init_daemon();
	}

	//	SignalHook(SIGKILL, SignHandler);
	SignalHook(SIGTERM, SignHandler);

	g_bAppRun = TRUE;
	/*捕捉的信号*/
	SignalHook(SIGINT,  SignHandler); /*CTRL-C*/
	SignalHook(SIGQUIT, SignHandler); /*CTRL-\*/
	SignalHook(SIGSTOP, SignHandler); /*CTRL-Z*/
	/*忽略的信号*/
	signal(SIGCHLD, SIG_IGN); /*子进程终止*/

	//初始化进程参数
	InitProcParam();

	//获得参数路径
	if( argc >= 2 ) sprintf(g_szAppPath, "%s", argv[1]);
	if( argc >= 3 ) g_nWDogTout = atoi(argv[2]);

	printf("---------------Init ProcTable----------------\n");
	//读取进程参数
	if( !ReadProcConfig() )
	{
		printf( "Read Parameter File Failed! Exit! \n" );
		exit( -1 ) ;
	}

	CSocketFtp *pSocketFtp = NULL;					//2 of mine!
	CreateSocketFtp( &pSocketFtp );


	CSocketDevCheck *pDevCheck = NULL;            //DevCheck 服务
	CreateSocketDevCheck( &pDevCheck );

	printf("Process Sum = %d\n", g_ProcDB.wCount);


	//改变环境变量
	//	if( putenv(g_szEnviVar) == 0 )
	//		printf("putenv(%s) ok.\n", g_szEnviVar);

	printf("---------------Manager Running---------------\n");
	//启动应用进程
	StartAppProc();
	sleep(1);

	//打开运行灯
	OpenRunLed( ) ;

	//打开看门狗设备
#ifdef WATCHDOG
	if( g_nWDogTout > 0 )
	{
		g_hWatchDog = g_Watchdog.OpenWatchDog() ;
	}
#endif
	//进程主循环
	nSecond = 0;
	while( g_bAppRun )
	{
		tv.tv_sec = 0;
		tv.tv_usec = 500 * 1000 ;//500000;
		select(0, NULL, NULL, NULL, &tv);
#ifdef WATCHDOG
		//复位看门狗
		g_Watchdog.FeedDog() ;
#endif
		//进程监视
		if( ++nSecond >= 6 )
		{
			nSecond = 0;
			WatchAppProc(3);
		}

		//运行灯闪烁
		RunLed() ;
	}
	printf("----------------Exit Manager----------------\n");

	//关闭运行灯
	CloseRunLen() ;
	delete pSocketFtp;							//of mine!
	//系统软复位
	if(g_ProcDB.wflag==2)
	{
		sync();
		reboot(RB_AUTOBOOT);
		return 1;
	}

#ifdef WATCHDOG
	//关闭看门狗
	if( g_hWatchDog >= 0 )
		//此处为关闭描述符，不能关闭看门狗
		g_Watchdog.closeWatchDog() ;
#endif


	return 0;
}/*}}}*/

int GetRealSysMemory( )
{/*{{{*/
	struct sysinfo s_info;
	int error = -1 ;

	error = sysinfo(&s_info);
	printf( "\n RAM: total %ld  free %ld  \n" , s_info.totalram, s_info.freeram);

	return error ;
}/*}}}*/

/*****************************************************************************/
