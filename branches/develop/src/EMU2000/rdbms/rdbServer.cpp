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

#include "../share/rdbFun.h"
#include "../share/global.h"

#define RUN_LED_NUM  2
/*****************************************************************************/
char g_szAppPath[64] = {"/mynand"};
BOOL g_bAppRun = TRUE;   //进程运行
int  g_nBusKeyID = -1;	 //消息总线key
// WORD g_wAppLED = 0x0001; //进程LED
// long g_hLedPort = -1;    //ADC设备

/*****************************************************************************/
#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */
	int already_running(const char *filename);

	void SignHandler(int signum, siginfo_t *pInfo, void *pReserved)
	{
		g_bAppRun = FALSE;
	}

	void RTDB_Timer_Proc(unsigned short wSecond);
	int SignalHook(int iSigNo, LPSIGPROC func);
	void init_daemon(void);

#ifdef	__cplusplus
}
#endif	/* __cplusplus */

/*****************************************************************************/
int main(int argc, char **argv)
{
	int  nTicks=0;
	MSGITEM  msgRecv;
	if ( already_running( "/mynand/config/rdbSever.lock" ) )
	{
		printf ( "the rdbSever is running \n" );
		return -1;
	}
	else
		printf ( "the rdbSever Begin to Run\n" );

	if( argc >= 2 ) sprintf(g_szAppPath, "%s", argv[1]);
	if( argc >= 3 )
	{
		if( atoi(argv[2]) > 0 )
		{
			init_daemon();
		}
	}
	SignalHook(SIGTERM, SignHandler);
	g_bAppRun = TRUE;
	SignalHook(SIGINT,  SignHandler); /*CTRL-C*/
	SignalHook(SIGQUIT, SignHandler); /*CTRL-\*/
	SignalHook(SIGSTOP, SignHandler); /*CTRL-Z*/
	//	signal(SIGINT,  SIG_IGN);
	//	signal(SIGQUIT, SIG_IGN);

	printf("----------------start RTDB Server----------------\n");
	if( Create_SHM_DBase(g_szAppPath, 0) < 0 )
	{
        printf("create db error\n");
		if( Open_SHM_DBase() < 0 )
			exit(-1);
	}

	ShowRTDBInfo();
	// Open_QMBLED_Device();

	//初始化管理消息队列
	g_nBusKeyID = LoginMessageBus((char *)"RDBServer");
	//绑定该进程要使用消息管理
	MessageSubscribe(g_nBusKeyID, 0x00000C00);
	printf("---------------RTDB Server Running---------------\n");

	struct timeval tv;
	while(g_bAppRun)
	{
		while( MessageRecv(g_nBusKeyID, &msgRecv, 0) > 0 )
		{
			//RecvMsgProc(&msgRecv);
		}
		if( ++nTicks >= 4 )
		{
			nTicks = 0;
			RTDB_Timer_Proc(1);
			// Set_QMBLED_Status(RUN_LED_NUM, g_wAppLED==0 ? 0 : 1);
			// g_wAppLED ^= 0x0001;
		}
		tv.tv_sec = 0;
		tv.tv_usec = 250000;
		select(0, NULL, NULL, NULL, &tv);
	}

	printf("----------------Exit RTDB Server----------------\n");
	ExitMessageBus(g_nBusKeyID);
	Close_SHM_DBase();
	// Set_QMBLED_Status(RUN_LED_NUM, 0);
	// Close_QMBLED_Device();

	for( int i=0; i<6; i++ )
	{
		printf ( "rtdbsever exit leave %ds\n", 6-i );
		usleep(1000000);
	}
	printf("**ByeBye**\n");

	return 0;
}
/*****************************************************************************/
