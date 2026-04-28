/*
 * =====================================================================================
 *
 *       Filename:  CBusDebug.c
 *
 *    Description:  设置总线端口打印数据
 *
 *        Version:  1.0
 *        Created:  2014年08月01日 11时58分52秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *        Company:  esdtek
 *
 * =====================================================================================
 */

#include	"CBusDebug.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#include "../share/global.h"

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CBusDebug
 *      Method:  CBusDebug
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CBusDebug::CBusDebug ()
{/*{{{*/
	pUdpObj = NULL;
	m_binit = FALSE;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CBusDebug
 *      Method:  ~CBusDebug
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CBusDebug::~CBusDebug ()
{/*{{{*/
	delete pUdpObj;
	// delete pUdp1Obj;
	// delete pUdp2Obj;
	// delete pUdp3Obj;
	printf ( "CBusDebug destructor\n" );
}  /* -----  end of method CBusDebug::~CBusDebug  (destructor)  ----- *//*}}}*/

#define MAXINTERFACES 16 /*   最大接口数 */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CBusDebug
 *      Method:  CBusDebug
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CBusDebug::GetLocalIp ( char *pNetCard )
{/*{{{*/
	register int fd, intrface;  //retn = 0;
	struct ifreq buf[MAXINTERFACES]; /* ifreq结构数组 */
	// struct arpreq arp;
	struct ifconf ifc;
	if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0)
	{
		ifc.ifc_len = sizeof buf;
		ifc.ifc_buf = (caddr_t) buf;
		if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc))
		{
			//获取接口数量信息
			intrface = ifc.ifc_len / sizeof (struct ifreq);
			/* printf("interface num is intrface=%d\n",intrface);  */
			//根据借口信息循环获取设备IP和MAC地址
			while ( (intrface--) > 0)
			{
				//获取设备名称
				/* printf ("net device %s\n", buf[intrface].ifr_name);   */
				if(strcmp(buf[intrface].ifr_name, pNetCard) == 0)
				{
					//p = buf[intrface].ifr_name;
				}
				else
					continue;
				if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
				{
					strcpy(m_szLocalAddr, (char *)inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                    close (fd);
					return TRUE;
				}
				else
				{
					char str[256];
					sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name);
					perror (str);
				}

			} //while
		}
		else
			perror ("cpm: ioctl");
	}
	else
		perror ("cpm: socket");
	close (fd);
	return FALSE;
}		/* -----  end of method CBusDebug::GetLocalIp  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CBusDebug
 *      Method:  Init
 * Description:  初始化CbusDebug 定义基本数据
 *		 Input:  总线号 //带修改
 *		Return:  0 成功 1 失败
 *--------------------------------------------------------------------------------------
 */

BOOL CBusDebug::Init ( BYTE byBusline, char *pNetCard, char *pRemoteIp, DWORD dzPortNum )
{/*{{{*/

	/*if (m_binit == TRUE)
		return FALSE;*/

	char ErrorBuf[256];
	if(pUdpObj  == NULL)
		pUdpObj = new CUdpPort;
	else
		return FALSE;

	/*printf("%s,%d,netcard = %s, remoteip = %s,port = %d\n", __FILE__, __LINE__
			,pNetCard , pRemoteIp , dzPortNum );*/
	if( GetLocalIp( pNetCard ) )
	{
		pUdpObj->m_uThePort = dzPortNum + byBusline;
		strcpy(pUdpObj->m_szLocalAddr, m_szLocalAddr);
		strcpy(m_szRemoteAddr, pRemoteIp);
		pUdpObj->OpenPort();
		printf("CBusDebug:NetCard=%s, LocalIp=%s RemoteIp=%s Port=%d\n", pNetCard, m_szLocalAddr, m_szRemoteAddr, pUdpObj->m_uThePort);
		//return TRUE;
	}

	//m_binit = TRUE;
	return FALSE;
}/*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CBusDebug
 *      Method:  SendDebugMsg
 * Description:  发送信息到相应ip和端口
 *		 Input:  buf 要发送的信息 Len 发送信息的长度
 *		Return:  发送的字节数
 *--------------------------------------------------------------------------------------
 */
int CBusDebug::SendDebugMsg ( BYTE *buf, int Len )
{/*{{{*/
	int SendLen = 0;
	if( buf == NULL || Len < 0 || Len  > MAX_DEBUG_BUFFER )
	{
		printf("SendDebugMsg return -1\n");
		return -1;
	}
	if(Len >= MAX_DEBUG_BUFFER)
		Len = MAX_DEBUG_BUFFER;

	if (pUdpObj)
	{
		SendLen = pUdpObj->WriteTo(buf, Len, m_szRemoteAddr, pUdpObj->m_uThePort);

		//printf("%s,%d, %s , %d \n", __FILE__, __LINE__,m_szRemoteAddr , pUdpObj->m_uThePort);
		//将报文打印到网页
		//pUdpObj->WriteTo(buf, Len, "127.0.0.1", pUdpObj->m_uThePort);
	}
	return SendLen;

}
