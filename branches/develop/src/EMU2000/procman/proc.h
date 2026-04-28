/******************************************************************************
  proc.h : header file
  Copyright (C): 2011 by houpeng
 ******************************************************************************/
#ifndef _PROCDEF_H__
#define _PROCDEF_H__

#include "../share/typedef.h"

#define	PROC_MAX_SUM   16
#define	PROC_NAME_LEN  24
#define	PROC_PATH_LEN  64
#define PROC_SERIAL_LEN 10

#define	PARA_MAX_SUM   4
#define	PROC_PARA_LEN  96

#if defined(__unix)
#define	END_PROC_SIGNAL		SIGKILL
#elif defined(WIN32)
#define END_PROC_SIGNAL		WM_CLOSE
#endif
/*******************************************************************/
//进程启动方式
enum AUTOSTARTMODE
{
	NOTSTART = 0,	//不启动
	RESPAWN,		//启动守护
	STARTAT,		//定时启动
	STARTCYCLE		//周期启动
};
//进程状态
enum ProcState
{
	PROC_NOEXIST = 0,	/*不存在*/
	PROC_START,			/*启动*/
	PROC_RUN,			/*在线*/
	PROC_ERROR,			/*异常退出*/
	PROC_STOP,			/*正常退出*/
	//	PROC_DEAD			/*无响应*/
};

#pragma pack(1)

//进程管理数据结构
typedef struct
{
	WORD wCount;  //进程数量(0-PROC_MAX_SUM)
	WORD wflag;   //状态标志 0=备用 1=运行 2=故障
	struct _tagProc
	{
		WORD  wProcType;  //进程类型 0=普通进程 1=重要进程 2=关键进程
		WORD  wStartMode; //启动模式 0=不启动   1=启动守护
		LONG  lStartTime; //启动时间
		char  szDescribe[PROC_NAME_LEN];	//进程描述
		char  szExecPath[PROC_PATH_LEN];	//执行路径
		char  szProcName[PROC_NAME_LEN];	//进程名称
		char  szParam[PROC_PARA_LEN];	//启动参数
		char  szSerialSum[PROC_SERIAL_LEN];	//串口个数
		//argv[0]程序名字，argv[1]进程参数文件全路径,argv[2~6]该进程其他参数
		char  *argv[PARA_MAX_SUM+2];

		pid_t hProcess;		//进程标识(pid)
		short nRunState;	//进程状态 0=无效 1=启动 2=在线 3=异常 4=退出
		short nErrTimer;    //错误计时(秒)
		short nRestoreNum;  //恢复计数
	}proc[PROC_MAX_SUM];
}PROC_DB;

int GetRealSysMemory( ) ;
#pragma pack()

/*****************************************************************************/
#endif /* #ifndef _PROCDEF_H__ */
