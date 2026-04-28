/*
 * =====================================================================================
 *
 *       Filename:  CSocketDevCheck.h
 *
 *    Description:  利用socket 
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时53分44秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */
#include <iostream>
#include <vector> 
#include <map>
#include <math.h>
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "../../share/UdpPort.h"
#include "../../share/UdpPort.h"
#include "../../share/global.h"
#include "../../librtdb/rdbObj.h"
#include "../../share/profile.h"
/*
 * =====================================================================================
 *        Class:  CSocketDevCheck
 *  Description:  关于socket ftp的类
 * =====================================================================================
 */
class CSocketDevCheck
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CSocketDevCheck();                             /* constructor      */
		~CSocketDevCheck();                            /* destructor       */

	public:
		//初始化协议
		BOOL Init ( void );	
	private:
		//创建UDp服务器
		BOOL CreateUdpServer ( void );


}; /* -----  end of class CSocketDevCheck  ----- */



