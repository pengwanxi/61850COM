/*
 * =====================================================================================
 *
 *       Filename:  CGroupBroadProto.cpp
 *
 *    Description:  
 *
 *       Compiler:  gcc
 *
 *        Version:  1.0
 *        Created:  2015年10月13日 16时54分57秒
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/ioctl.h> 
#include <sys/socket.h> 
#include <net/if.h> 
#include <netinet/in.h> 
#include <net/if_arp.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <linux/mii.h>
#ifdef SOLARIS 
#include <sys/sockio.h> 
#endif 

#include "CGroupBroadProto.h"
#include "GroupBroad.h"
#include "../../share/profile.h"
#include "../../share/rdbFun.h"
#include "../../BayLayer/main.h"


static const BYTE ASK_IP = 0x01;
static const char g_GroupIp[16] = "224.0.0.100";

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  CGroupBroadProto
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CGroupBroadProto::CGroupBroadProto ()
{
	char chPrjNameFile[] =  BUS_PATH;
	for( int i=0; i<4; i++ )
	{
		m_IsAddRoute[i] = FALSE;
	}
	memset( m_bySendBuf, 0, GROUP_BROAD_PROTO_MAX_LEN );
	m_wSendLen = 0;
	m_lTimeFlag = 0;
	m_RoutePort = 0;
	memset( m_chPrjName, 0, 32 );
	m_iSocketFd = -1;

	CProfile Profile( chPrjNameFile );
	Profile.GetProfileString( (char *)"PROJECT" , 
			(char *)"name" , 
			(char *)"uninit name" ,
			m_chPrjName , 
			32 ) ;

	// printf ( "工程名字:%s\n", m_chPrjName );
}  /* -----  end of method CGroupBroadProto::CGroupBroadProto  (constructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  ~CGroupBroadProto
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CGroupBroadProto::~CGroupBroadProto ()
{
	if( m_iSocketFd > 0 )
	{
		close( m_iSocketFd );
	}

	for ( int i=0; i<4; i++)
	{
		if( m_IsAddRoute[i] )
		{
			char chBuf[256] = "";
			sprintf( chBuf, 
					"route del -net %s netmask 255.255.255.255 dev eth%d" , 
					g_GroupIp, i);
			m_IsAddRoute[ i-1 ] = FALSE;
			system( chBuf );

		}
	}
}  /* -----  end of method CGroupBroadProto::~CGroupBroadProto  (destructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  SetTimeFlag
 * Description:  设置时间标识 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CGroupBroadProto::SetTimeFlag ( long lTimeFlag )
{
	m_lTimeFlag = lTimeFlag;
}		/* -----  end of method CGroupBroadProto::SetTimeFlag  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  CheckTimeFlag
 * Description:  校验时间标识 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::CheckTimeFlag ( long lTimeFlag )
{
	return ( lTimeFlag == m_lTimeFlag );
}		/* -----  end of method CGroupBroadProto::CheckTimeFlag  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  GetNetCardNum
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
#define MAXINTERFACES 16 /* 最大接口数 */
BYTE CGroupBroadProto::GetNetCardNum (  )
{
	struct ifreq buf[MAXINTERFACES]; /* ifreq结构数组 */
	struct ifconf ifc; 

	if( m_iSocketFd < 0 )
	{
		m_iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	}

	printf("m_iSocketFd:%d	line:%d\n", m_iSocketFd, __LINE__);
	if (m_iSocketFd >= 0) 
	{ 
		ifc.ifc_len = sizeof buf; 
		ifc.ifc_buf = (caddr_t) buf; 
		if (!ioctl (m_iSocketFd, SIOCGIFCONF, (char *) &ifc)) 
		{ 
			register int intrface = 0;
			//获取接口数量信息
			intrface = ifc.ifc_len / sizeof (struct ifreq); 
			return intrface;
			//根据借口信息循环获取设备IP和MAC地址
		} 
		else 
		{
			close (m_iSocketFd); 
			perror ("cpm: ioctl"); 
			return 0;
		}
	} 
	else 
	{
		perror ("cpm: socket"); 
	}
		
	return 0;
}		/* -----  end of method CGroupBroadProto::GetNetCardNum  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  GetNetState
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::GetNetState ( const char *if_name,
		int phy_id,
		int reg_num )
{
	struct ifreq ifr; 
	struct mii_ioctl_data *mii; 
	BOOL value;	
	if( m_iSocketFd < 0 )
	{
		m_iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	}

	if (m_iSocketFd  < 0)
	{
		perror("socket");
		close(m_iSocketFd);
		return -1; 
	}

	bzero(&ifr, sizeof(ifr));
	strncpy(ifr.ifr_name, if_name, IFNAMSIZ-1); 
	ifr.ifr_name[IFNAMSIZ-1] = 0; 

	if (ioctl(m_iSocketFd, SIOCGMIIPHY, &ifr) < 0)
	{ 
		perror("ioctl");
		close(m_iSocketFd);
		return -1; 
	}

	mii = (struct mii_ioctl_data *)&ifr.ifr_data;
	mii->reg_num = reg_num;//0x01
	if (ioctl(m_iSocketFd, SIOCGMIIREG, &ifr) < 0)
	{ 
		perror("ioctl");
		close(m_iSocketFd);
		return -1;
	}

	value = ((mii->val_out&0x04)>>2);

	return value;
}		/* -----  end of method CGroupBroadProto::GetNetState  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  GetNetIpAndState
 * Description:  获得网卡ip及状态
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::GetNetIpAndState ( const BYTE byNetCard, 
		char *pchBuf, 
		BOOL &bState)
{
	struct ifreq buf[MAXINTERFACES]; /* ifreq结构数组 */
	struct ifconf ifc; 

	if( m_iSocketFd < 0 )
	{
		m_iSocketFd = socket (AF_INET, SOCK_DGRAM, 0);
	}

	if ((m_iSocketFd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) 
	{ 
		ifc.ifc_len = sizeof buf; 
		ifc.ifc_buf = (caddr_t) buf; 
		if (!ioctl (m_iSocketFd, SIOCGIFCONF, (char *) &ifc)) 
		{ 
			register int intrface = 0;
			//获取接口数量信息
			intrface = ifc.ifc_len / sizeof (struct ifreq); 
			//根据借口信息循环获取设备IP和MAC地址
			if( byNetCard > 0 )
			{
				intrface = byNetCard;
			}
			//获取设备名称
			// printf ("net device %s\n", buf[intrface].ifr_name); 
			// //判断网卡类型 
			// if (!(ioctl (m_iSocketFd, SIOCGIFFLAGS, (char *) &buf[intrface]))) 
			// { 
				// if (buf[intrface].ifr_flags & IFF_PROMISC) 
				// { 
					// puts ("the interface is PROMISC"); 
				// } 
			// } 
			// else 
			// { 
				// char str[256]; 
				// sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name); 
				// perror (str); 
			// } 

			// //判断网卡状态 
			// printf ( "state=%x\n", buf[intrface].ifr_flags );
			// if (  (buf[intrface].ifr_flags & IFF_UP) 
					// && (buf[intrface].ifr_flags & IFF_RUNNING))
			// { 
				// bState = TRUE;
				// puts("the interface status is UP"); 
			// } 
			// else 
			// { 
				// bState = FALSE;
				// puts("the interface status is DOWN"); 
			// } 
			bState = GetNetState( buf[intrface].ifr_name, 0x10, 0x01 ); 

			//获取当前网卡的IP地址 
			if (!(ioctl (m_iSocketFd, SIOCGIFADDR, (char *) &buf[intrface]))) 
			{ 
				// printf("IP address is:"); 
				strcpy( pchBuf,  (char *)inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
				// puts((char *)inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr)); 
				//printf("\n%d\n"buf[intrface].ifr_addr))->sin_addr.s_addr); 
				//puts (buf[intrface].ifr_addr.sa_data); 
			} 
			else 
			{ 
				char str[256]; 
				sprintf (str, "cpm: ioctl device %s", buf[intrface].ifr_name); 
				perror (str); 
				return FALSE;
			} 
			/* this section can't get Hardware Address,I don't know whether the reason is module driver*/ 
		} 
		else 
		{
			perror ("cpm: ioctl"); 
			return FALSE;
		}
	} 
	else 
	{
		perror ("cpm: socket"); 
		return FALSE;
	}

	return TRUE;; 
}		/* -----  end of method CGroupBroadProto::GetNetIpAndState  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  IpToByte
 * Description:  IP地址转BYTE报文
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
inline void CGroupBroadProto::IpToByte ( char *pchBuf, BYTE *byIpBuf )
{/*{{{*/
	int a, b, c, d;
	sscanf( pchBuf, "%3d.%3d.%3d.%3d", &a, &b, &c, &d );
	byIpBuf[0] = a & 0xff;
	byIpBuf[1] = b & 0xff;
	byIpBuf[2] = c & 0xff;
	byIpBuf[3] = d & 0xff;
}		/* -----  end of method CGroupBroadProto::IpToByte  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  SetIpState
 * Description:  设置ip状态
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
inline	void CGroupBroadProto::SetIpState ( BYTE &byStateByte, 
		BOOL bState, 
		BYTE byNetCard)
{/*{{{*/
	if( bState )
	{
		byStateByte |= ( 1 << byNetCard );
	}
}		/* -----  end of method CGroupBroadProto::SetIpState  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  AddFrameFormat
 * Description:  添加固定帧格式
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CGroupBroadProto::AddFrameFormat ( BYTE *byDataBuf, WORD wDataLen )
{/*{{{*/
	//头
	m_bySendBuf[ HEAD_START ] = 0x68;
	m_bySendBuf[ HEAD_HILEN ] = HIBYTE( wDataLen );
	m_bySendBuf[ HEAD_LOLEN ] = LOBYTE( wDataLen );
	m_bySendBuf[ HEAD_RESTART ] = 0x68;

	//数据帧
	for ( WORD i=0; i<wDataLen; i++ )
	{
		m_bySendBuf[ DATA_FUNC + i ] = byDataBuf[ i ];
	}

	m_bySendBuf[ DATA_FUNC ] |= 0x80;

	m_bySendBuf[ DATA_FUNC + wDataLen ]
		= GetCheckByte( &m_bySendBuf[ DATA_FUNC ], wDataLen );

	//尾
	m_bySendBuf[ DATA_FUNC + wDataLen + 1 ] = 0x16;
	m_wSendLen = wDataLen + 6;
}		/* -----  end of method CGroupBroadProto::AddFrameFormat  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  GetAnsIpFrame
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::GetAnsIpFrame ( void )
{/*{{{*/
	WORD wDataLen = 0;
	BYTE byDataBuf[ GROUP_BROAD_PROTO_MAX_LEN - 6 ] = "";
	char chPrjNameFile[] =  BUS_PATH;

	byDataBuf[ wDataLen ++ ] = 0x01;            /* 功能码 */
	byDataBuf[ wDataLen ++ ] = HIBYTE( HIWORD( m_lTimeFlag ) ); /* 时间标识 */
	byDataBuf[ wDataLen ++ ] = LOBYTE( HIWORD( m_lTimeFlag ) ); /* 时间标识 */
	byDataBuf[ wDataLen ++ ] = HIBYTE( LOWORD( m_lTimeFlag ) ); /* 时间标识 */
	byDataBuf[ wDataLen ++ ] = LOBYTE( LOWORD( m_lTimeFlag ) ); /* 时间标识 */

	CProfile Profile( chPrjNameFile );
	Profile.GetProfileString( (char *)"PROJECT" , 
			(char *)"name" , 
			(char *)"uninit name" ,
			m_chPrjName , 
			32 ) ;
	memcpy( &byDataBuf[wDataLen], m_chPrjName, 32 ); /* 工程名 */
	wDataLen += 32;

	BYTE byNetCardNum = GetNetCardNum(  );
	//排除127.0.0.1
	byDataBuf[ wDataLen ++ ] = byNetCardNum-1;    /* 网卡数量 */

	BYTE byStateByte = 0;
	for ( BYTE i=1; i<byNetCardNum; i++)
	{
		//i=0 是127.0.0.1
		char chIpBuf[16] = "";
		BOOL bState;
		if( GetNetIpAndState( i, chIpBuf, bState ) )
		{
			IpToByte( chIpBuf, &byDataBuf[ wDataLen ] ); /* 网址 */
			wDataLen += 4;
			SetIpState( byStateByte, bState, i-1 );
		}
		else
		{
			printf ( "GetAnsIpFrame::ip state not find\n" );
			return FALSE;
		}
	}

	byDataBuf[ wDataLen ++ ] = byStateByte;        /* 状态位 */
	AddFrameFormat( byDataBuf, wDataLen );
	
	return TRUE;
}		/* -----  end of method CGroupBroadProto::GetAnsIpFrame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  PDF_AskIp
 * Description:  处理询问ip数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::PDF_AskIp ( BYTE *buf, int len )
{/*{{{*/
	BYTE *pPoint = buf + DATA_FUNC;

	// 校验时间标识
	pPoint ++;
	long lTimeFlag = ( *pPoint << 24 ) 
		|( *( pPoint + 1 ) << 16 )
		|( *( pPoint+ 2 ) << 8 )
		|( *( pPoint + 3 ) );

	if( CheckTimeFlag( lTimeFlag ) )
	{
		return FALSE;	
	}
	else
	{
		SetTimeFlag( lTimeFlag );
	}

	return GetAnsIpFrame(  );
}		/* -----  end of method CGroupBroadProto::PDF_AskIp  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  ProcessDataFrame
 * Description:  处理数据帧
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::ProcessDataFrame ( BYTE *buf, int len )
{/*{{{*/

	switch ( buf[DATA_FUNC] )
	{
		case FUNC_ASK_IP:	
			return PDF_AskIp( buf, len );
			break;

		default:	
			break;
	}				/* -----  end switch  ----- */

	return FALSE;
}		/* -----  end of method CGroupBroadProto::ProcessDataFrame  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  GetCheckByte
 * Description:  获得校验和 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
inline BYTE CGroupBroadProto::GetCheckByte ( BYTE *buf, int len )
{/*{{{*/
	BYTE byCheckByte = 0;

	for ( int i=0; i<len; i++)
	{
		byCheckByte+=buf[i]	;
	}

	return byCheckByte;
}		/* -----  end of method CGroupBroadProto::GetCheckByte  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  CheckFrameFormat
 * Description:  校验帧格式 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::CheckFrameFormat ( BYTE *buf, int len )
{/*{{{*/
	if( NULL == buf 
			|| 6 > len)
	{
		return FALSE;
	}

	if( 0x68 != buf[HEAD_START] 
			&& buf[HEAD_START] != buf[HEAD_RESTART])
	{
		printf ( "head error\n" );
		return FALSE;
	}

	WORD wRecvLen = MAKEWORD( buf[HEAD_LOLEN], buf[HEAD_HILEN] );
	if( ( len-6 ) !=  wRecvLen)
	{
		printf ( "len error\n" );
		return FALSE;
	}

	if( 0x16 != buf[len-1] )
	{
		printf ( "0x16 error\n" );
		return FALSE;
	}

	BYTE byCheckByte = GetCheckByte( buf+DATA_FUNC, wRecvLen );
	if( byCheckByte != buf[len-2] )
	{
		printf ( "check error %x %x\n", byCheckByte, buf[len-2] );
		return FALSE;	
	}

	if( buf[DATA_FUNC] & 0x80 )
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CGroupBroadProto::CheckFrameFormat  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  ProcessProtoData
 * Description:  处理收到的协议报文 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CGroupBroadProto::ProcessProtoData ( BYTE *buf, int len )
{/*{{{*/
	if( !CheckFrameFormat( buf, len ) )
	{
		return FALSE;
	}
	return ProcessDataFrame( buf, len );
}		/* -----  end of method CGroupBroadProto::ProcessProtoData  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  GetOneNetPort
 * Description:  获得一个网线状态的端口 从eth0 - eht4 依次获得
 *       Input:
 *		Return:  0表示错误 1-4表示有效
 *--------------------------------------------------------------------------------------
 */
BYTE CGroupBroadProto::GetOneNetPort ( void )
{/*{{{*/
	BYTE byNetCardNum = GetNetCardNum(  );

	//i=0 为 127.0.0.1
	for ( BYTE i=1; i < byNetCardNum;i++ )
	{
		char chBuf[256] = "";
		sprintf( chBuf, "eth%d", ( i-1 ) );
		BOOL bState = GetNetState( chBuf, 0x10, 0x01 ); 
		if( bState )
		{
			m_IsAddRoute[i-1] = TRUE;
			return i;
		}
	}

	return 0;
}		/* -----  end of method CGroupBroadProto::GetOneNetPort  ----- *//*}}}*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CGroupBroadProto
 *      Method:  SetRoutePort
 * Description:  设置该端口加入route 其它删除route
 *       Input:  emu2000 byPort为1-4 采集器为1-2
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CGroupBroadProto::SetRoutePort ( BYTE byPort )
{/*{{{*/
	if( 0 == byPort || 4 < byPort )
	{
		return;
	}

	if( byPort == m_RoutePort )
	{
		return;
	}

	BYTE byNetCardNum = GetNetCardNum(  );
	for ( BYTE i=1; i<byNetCardNum; i++)
	{
		char chBuf[256]= "";
		if( byPort == i )	
		{
			sprintf( chBuf, 
					"route add -net %s netmask 255.255.255.255 dev eth%d" , 
					g_GroupIp, i-1);
			m_RoutePort = byPort;
			system( chBuf );					//完全没有作用!
			GroupBroadInit(  );
		}
		else
		{
			if( m_IsAddRoute[ i-1 ] )
			{
				sprintf( chBuf, 
						"route del -net %s netmask 255.255.255.255 dev eth%d" , 
						g_GroupIp, i-1);

				m_IsAddRoute[ i-1 ] = FALSE;
				system( chBuf );
			}
		}
	}
}		/* -----  end of method CGroupBroadProto::SetRoutePort  ----- *//*}}}*/
