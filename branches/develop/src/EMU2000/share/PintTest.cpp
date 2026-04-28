// PintTest.cpp: implementation of the CPintTest class.
//
//////////////////////////////////////////////////////////////////////

#include<sys/time.h>  /*是Linux系统的日期时间头文件*/  
#include<sys/socket.h>    /*对与引用socket函数必须*/  
#include<netdb.h> /*定义了与网络有关的结构，变量类型，宏，函数。函数gethostbyname()用*/  
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "PintTest.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPintTest::CPintTest()
{
	InitPing( ) ;
	m_dataLen = DEFAULT_LEN ;
}

CPintTest::~CPintTest()
{

}

void CPintTest::InitPing( )
{
	m_socket = ::socket( AF_INET , SOCK_RAW , IPPROTO_ICMP ) ;
}

BOOL CPintTest::Ping( char * pDestIp )
{
	/*
	   A.该ping命令函数有问题，目前不用，没有调试正常
	   B.等下个版本使用时，再调试正常
	   if( pDestIp == NULL )
	   return FALSE ;

	//获取发送ICMP包
	if( !PackICMP( pDestIp ) )
	return FALSE ;

	//发送ping包
	//SendICMP( pDestIp ) ;
	//接收ping包
	RecvICMP( pDestIp ) ;
	*/
	return TRUE ;
}

BOOL CPintTest::SendICMP( char * pDestIp )
{
	// 	struct sockaddr_in dest ;
	// 	dest.sin_family = AF_INET ;
	// 	dest.sin_port = htons( 0 ) ;
	// 	dest.sin_addr.s_addr = inet_addr( pDestIp ) ;
	// 	
	// 	sendto( m_socket , m_sbuf ,  , 0 , (struct sockaddr *)&dest,sizeof (dest) );
	return  TRUE ;
}

BOOL CPintTest::RecvICMP( char * pDestIp )
{
	if( pDestIp == NULL )
		return FALSE ;

	BYTE byRecv[1024] ;
	memset( byRecv , 0 , sizeof( byRecv ) ) ;
	struct sockaddr_in dest ;
	socklen_t len = 0 ;
	int iRet = 0 ;
	iRet = recvfrom( m_socket , byRecv , sizeof( byRecv ) , 0 , ( struct sockaddr * )&dest , &len ) ;

	if( iRet )
		return TRUE ;
	else
		return FALSE ;
}

BOOL CPintTest::PackICMP( char * pDestIP )
{
	if( pDestIP == NULL )
		return FALSE ;

	struct  NSPT::iphdr *ip_hdr;   /*iphdr为IP头部结构体*/
	struct icmphdr *icmp_hdr;   /*icmphdr为ICMP头部结构体*/  
	int len;  
	int len1; 

	/*ip头部结构体变量初始化*/  
	ip_hdr=(struct NSPT::iphdr *)m_sbuf; /*字符串指针*/
	ip_hdr->hlen=sizeof(struct NSPT::iphdr)>>2;  /*头部长度*/
	ip_hdr->ver=IPVERSION;   /*版本*/  
	ip_hdr->tos=0;   /*服务类型*/  
	ip_hdr->tot_len=IP_HSIZE+ICMP_HSIZE+m_dataLen; /*报文头部加数据的总长度*/  
	ip_hdr->id=0;    /*初始化报文标识*/  
	ip_hdr->frag_off=0;  /*设置flag标记为0*/  
	ip_hdr->protocol=IPPROTO_ICMP;/*运用的协议为ICMP协议*/  
	ip_hdr->ttl=255; /*一个封包在网络上可以存活的时间*/  
	ip_hdr->daddr= inet_addr( pDestIP );  /*目的地址*/  
	len1=ip_hdr->hlen<<2;  /*ip数据长度*/  

	/*ICMP头部结构体变量初始化*/  
	icmp_hdr=(struct icmphdr *)(m_sbuf+len1);  /*字符串指针*/  
	icmp_hdr->type=8;    /*初始化ICMP消息类型type*/  
	icmp_hdr->code=0;    /*初始化消息代码code*/  
	icmp_hdr->wRequestID=getpid( );   /*把进程标识码初始给icmp_id*/  
	icmp_hdr->wSequence= 0 ;  /*发送的ICMP消息序号赋值给icmp序号*/      
	gettimeofday((struct timeval *)icmp_hdr->timeStamp,NULL); /* 获取当前时间*/  

	len=ip_hdr->tot_len; /*报文总长度赋值给len变量*/  
	icmp_hdr->checksum=0;    /*初始化*/  
	icmp_hdr->checksum=checksum((BYTE *)icmp_hdr,len);  /*计算校验和*/  

	struct sockaddr_in dest ;
	dest.sin_family = AF_INET ;
	dest.sin_port = htons( 0 ) ;
	dest.sin_addr.s_addr = inet_addr( pDestIP ) ;

	sendto( m_socket , m_sbuf , len , 0 , (struct sockaddr *)&dest,sizeof (dest) );
	return TRUE ;
}

WORD CPintTest::checksum( BYTE *buf,int len)  
{  
	DWORD sum=0;  
	WORD *cbuf;  

	cbuf=(WORD *)buf;  

	while(len>1){  
		sum+=*cbuf++;  
		len-=2;  
	}  

	if(len)  
		sum+=*(BYTE *)cbuf;  

	sum=(sum>>16)+(sum & 0xffff);  
	sum+=(sum>>16);  

	return ~sum;  
}



