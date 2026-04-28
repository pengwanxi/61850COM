/******************************************************************************
 *
 *  TcpListen.h: interface for the CTcpListen class for Linux
 *
 ******************************************************************************/
#ifndef _TCPLISTEN_H_
#define _TCPLISTEN_H_

#include <vector>
#include <netinet/in.h>
#include <pthread.h>

#include "../share/typedef.h"

typedef void (*ACCEPTPROC)(int, unsigned short, char*, char* );
/*****************************************************************************/
/* This class is interface for callback */
class CDelegateBase
{
	public:
		CDelegateBase(){}
		virtual BOOL AcceptProc(int hSocket, unsigned short nPort,
				char* lpszRemote, char* lpszLocal) {close(hSocket); return FALSE;}
		virtual ~CDelegateBase() { }
};

/*****************************************************************************/
/* This class is the tcp server */
class CTcpListen
{
	/*Constructors / Destructors*/
	public:
		CTcpListen(void);
		virtual ~CTcpListen(void);
		char* ClassName(){return (char *)"CTcpListen";}

		/* Attributes */
	private:
		int m_nAddrSize;
		struct sockaddr_in m_ClientAddr;

	protected:
		CDelegateBase* m_pAcceptObj;/*连接处理的函数对象指针*/
		HANDLE  m_hSocket;          /*服务端Socket文件描述符*/

	public:
		unsigned short m_nThePort; /*Listen端口号*/
		char  m_szLocalAddr[24];   /*服务端IP地址*/
		char  m_szRemoteAddr[24];  /*客户端IP地址*/

		BOOL  m_bTaskRun;          /*任务运行*/
		int	  m_tTaskID;           /*处理任务ID*/
		pthread_t  m_tThreadID;

		/* Implementation */
	public:
		void RunExit( void );
		BOOL IsObjValid( void );
		BOOL CreateObj( char* lpszError=NULL );
		void ReleaseObj( void );
		BOOL StartRun( CDelegateBase* lpAcceptObj );
		void WorkTaskProc( void );
};

typedef std::vector<CTcpListen*> CTcpPtrVector;
typedef CTcpPtrVector::iterator TcpIterator;

/*****************************************************************************/
#endif
