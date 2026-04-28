#include "CWatchDog.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#include <unistd.h>

CWatchDog::CWatchDog()
{
	//ctor
	fd = -1 ;
}

CWatchDog::~CWatchDog()
{
	//dtor
}

int CWatchDog::OpenWatchDog(  )
{
	fd = open( "/dev/watchdog" ,O_RDWR);
	if (fd < 0)
	{
		printf("Open WDT Dev Error!\n");
		return -1;
	}

	SetWatchDogTimeOut( ) ;
	SetWatchDogOpt( ) ;

	return fd ;
}

int CWatchDog::SetWatchDogTimeOut( )
{
	if( GetWatchDogState( ) < 0 )
	{
		printf( "WatchDog Is not Open !\n" ) ;
		return -1 ;
	}

	int cmd = -1 ;
	cmd = WDIOC_SETTIMEOUT;
	int arg = 60; //看门狗20s起作用
	int iReturn = ioctl(fd, cmd , &arg ) ;

	if (iReturn < 0)
	{
		printf("Call cmd Settimerout fail\n");
		return -1;
	}

	return iReturn ;
}

int CWatchDog::SetWatchDogOpt( )
{
	if( GetWatchDogState( ) < 0 )
	{
		printf( "WatchDog Is not Open !\n" ) ;
		return -1 ;
	}

	int cmd = -1 ;
	cmd = WDIOC_SETOPTIONS;
	int iReturn = ioctl(fd, cmd) ;

	if ( iReturn < 0)
	{
		printf("Call cmd Start WDT fail\n");
		return -1;
	}

	return iReturn ;
}

int CWatchDog::FeedDog( )
{/*{{{*/
	if( GetWatchDogState( ) < 0 )
	{
		//printf( "WatchDog Is not Open !\n" ) ;
		return -1 ;
	}

	int cmd = -1 ;
	cmd = WDIOC_KEEPALIVE;
	int iReturn = ioctl(fd, cmd) ;

	if ( iReturn < 0)
	{
		printf("Call cmd Feedwatchdog fail\n");
		return -1;
	}

	return iReturn ;
}/*}}}*/

int CWatchDog::closeWatchDog( )
{/*{{{*/
	if( GetWatchDogState( ) < 0 )
	{
		printf( "WatchDog Is not Open !\n" ) ;
		return -1 ;
	}


	int iRe = close( fd ) ;
	return  iRe ;
}/*}}}*/

int CWatchDog::GetWatchDogState( )
{/*{{{*/
	return fd ;
}/*}}}*/