/*
 * =====================================================================================
 *
 *       Filename:  global.cpp
 *
 *    Description: 定义全局函数
 *
 *        Version:  1.0
 *        Created:  2014年09月11日 11时18分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/reboot.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>


#include "global.h"

#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*******************************************************************************
 * 函数名:IsBigEndian
 * 功能描述:判断大小端，大端返回true 小端返加false
 * 参数:void
 * 返回值:bool
 ******************************************************************************/
bool IsBigEndian(void)
{
	int i=1;

	char *p=(char *)&i;

	if ( 1 == *p)
	{
		return false;
	}
	else
	{
		return true;
	}

	return true;
}   /*-------- end IsBigEndian -------- */

/*******************************************************************************
 * 函数名:GlobalCopyByEndian
 * 功能描述:根据大小端模式拷贝buf
 * 参数: char *dest 目标buf
 * 参数: char *src 源buf
 * 参数: int num 要拷贝的数量
 * 返回值:bool
 ******************************************************************************/
bool GlobalCopyByEndian( unsigned char *dest, unsigned char *src, unsigned int num )
{
	if ( NULL == dest || NULL == src )
	{
		return false;
	}

	// 不拷虑dest和src内存重合
	if ( IsBigEndian( ) )
	{
		src += num-1;
		while( 0 != num-- )
		{
			*( dest++ ) = *( src-- );
		}
	}
	else
	{
		while( 0 != num-- )
		{
			*( dest++ ) = *( src++ );
		}
	}

	return true;
}   /*-------- end GlobalCopyByEndian -------- */


/*  set advisory lock on file */
static int lockfile(int fd)
{
	struct flock fl;

	fl.l_type = F_WRLCK;  /*  write lock */
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;  //lock the whole file
	return(fcntl(fd, F_SETLK, &fl));
}

int already_running(const char *filename)
{
	int fd;
	char buf[16];

	fd = open(filename, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0)
	{
		printf( "can't open %s: %m\n", filename);
		exit(1);
	}
	/*  先获取文件锁 */
	if (lockfile(fd) == -1)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			printf( "file: %s already locked", filename);
			close(fd);
			return 1;
		}

		printf("can't lock %s: %m\n", filename);
		exit(1);

	}
	/*  写入运行实例的pid */
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return 0;
}

void SetSerialConfigMode(  int byMode )
{
	BYTE byType = 0 ;
	switch( byMode )
	{
	case 1:
		{
			byType = CMD_SET_UT1_NON_RS485 ;
			printf( "Set RS01 = RS232 \n" );
		}
		break;
	case 2:
		{
			byType = CMD_SET_UT2_NON_RS485 ;
			printf( "Set RS02 = RS232 \n" );
		}
		break;
	case 8:
		{
			byType = CMD_SET_UT3_NON_RS485 ;
			printf( "Set RS03 = RS422 \n" );
		}
		break;
	case 4:
		{
			byType = CMD_SET_UT4_NON_RS485 ;
			printf( "Set RS04 = RS422 \n" );
		}
		break;
	default:
		return ;
	}

	int gpiofd;
	gpiofd = open(GPIO_DEV_NAME, O_RDWR);
	if( gpiofd < 1  ) return;
	ioctl(gpiofd, byType, 0);
	close(gpiofd);
}


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  TransYxTimeToStructTm
 *  Description:  struct tm
 * =====================================================================================
 */
struct tm  SetStructTm ( TIMEDATA srcTime, struct tm &t, long &lTime )
{
	t.tm_year = srcTime.Year;
	t.tm_mon = srcTime.Month - 1;
	t.tm_mday = srcTime.Day;
	t.tm_hour = srcTime.Hour;
	t.tm_min = srcTime.Minute;
	t.tm_sec = srcTime.Second;

	lTime = mktime( &t ) ;
	return t;
}		/* -----  end of function TransYxTimeToStructTm  ----- */


/*
 * ===  FUNCTION  ======================================================================
 *         Name:  TransLongTimeToStructTm
 *  Description:
 * =====================================================================================
 */
void GetOwnStructTm ( long lTime, struct tm *t )
{
	localtime_r((time_t *)&lTime, t);
	t->tm_mon = t->tm_mon + 1;
}		/* -----  end of function TransLongTimeToStructTm  ----- */

/*
 * ===  FUNCTION  ======================================================================
 *  Description: 复制一些重复的 公用的函数
 * =====================================================================================
 */
#ifdef	__cplusplus
extern "C" {
#endif	/* __cplusplus */
	BOOL g_bDebugApp = TRUE; //调试状态
	BOOL g_bAppRun = TRUE;   //进程运行
	//删除字符串左边空格及\t
	void ltrim(char *s)
	{
		char *p = s;
		if( *p==' ' || *p=='\t' )
		{
			while( *p==' ' || *p=='\t' ) p++;
			while( (*s++ = *p++) );
		}
	}

	//删除字符串右边空格及'\t','\r', '\n'
	void rtrim(char *s)
	{
		char *p = s;
		while( *p ) p++;
		if( p != s )
		{
			p--;
			while( *p==' ' || *p=='\t' || *p=='\r' || *p=='\n' ) p--;
			*(++p) = 0;
		}
	}

	void OutPromptText(char *lpszText)
	{
		if( !g_bDebugApp ) return;
		printf("%s\n", lpszText);
	}

	void LogPromptText(const char *fmt, ...)
	{
		if( !g_bDebugApp ) return;
		va_list args;
		va_start(args, fmt);
		vprintf(fmt, args);
		va_end(args);
	}

	void OutMessageText(char *szSrc, unsigned char *pData, int nLen)
	{
		int i, k;
		char szBuff[96];

		if( !g_bDebugApp ) return;
		k = 0;
		sprintf(szBuff, "\n%s\n", szSrc);
		printf(szBuff);
		for( i=0; i<nLen; i++ )
		{
			k += sprintf(&szBuff[k], " %02X", pData[i]);
			if((i+1)%24==0)
			{
				printf(szBuff);
				szBuff[0] = '\n';
				k = 1;
			}
		}
		if( k > 1 ) printf(szBuff);
		/*
		   if( nLen > 0 )
		   printf("\n------------------------------------------------------------");
		   */
	}


	int SignalHook(int iSigNo, LPSIGPROC func)
	{
		struct sigaction act, oact;

		act.sa_sigaction = func;
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO|SA_RESTART;

		if(sigaction(iSigNo, &act, &oact) < 0)
			return -1;
		return 0;//(int)oact.sa_handler;
	}

	/*创建守护进程*/
	void init_daemon(void)
	{
		int   i;
		pid_t child1, child2;
		/*创建第一子进程*/
		child1 = fork();
		if( child1 < 0 )
		{
			perror("Create first child process fail!\n");
			exit(1);
		}
		else if(child1>0) exit(0); /*结束父进程*/
		/*第一子进程成为新的会话组长和进程组长*/
		setsid();
		/*第一子进程与控制终端分离*/
		child2 = fork();
		if( child2 < 0 )
		{
			perror("Create second child process fail!\n");
			exit(2);
		}
		else if(child2>0) exit(0); /*结束第一子进程*/
		/*第二子进程继续, 第二子进程不再是会话组长*/
		/*关闭打开的文件描述符*/
		for(i=0; i<NOFILE; ++i)
			close(i);
		/*改变工作目录到/tmp*/
		chdir("/tmp");
		/*重设文件创建掩模*/
		umask(0);
		/*处理SIGCHLD信号*/
		signal(SIGCHLD, SIG_IGN);
	}

#ifdef	__cplusplus
}
#endif	/* __cplusplus */

/*===========================================================================*/
