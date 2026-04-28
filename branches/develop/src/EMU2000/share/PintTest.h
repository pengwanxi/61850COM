// PintTest.h: interface for the CPintTest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PINTTEST_H__90ECF2C5_8928_41E7_A471_A56514CF8119__INCLUDED_)
#define AFX_PINTTEST_H__90ECF2C5_8928_41E7_A471_A56514CF8119__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "typedef.h"
//ICMP消息头部
struct icmphdr {
	BYTE type;     /*定义消息类型*/
	BYTE code;    /*定义消息代码*/
	WORD checksum;   /*定义校验*/

	WORD wRequestID ; //请求ID号
	WORD wSequence ; //序列号
	DWORD timeStamp ; //时间
	icmphdr( )
	{
		type = 0 ;
		code = 0 ;
		checksum = 0 ;
		wRequestID = 0 ;
		wSequence = 0 ;
		timeStamp = 0 ;
	}
};

namespace NSPT
{
	struct iphdr {
		BYTE hlen : 4, ver : 4;   /*定义4位首部长度，和IP版本号为IPV4*/
		BYTE tos;     /*8位服务类型TOS*/
		WORD tot_len;    /*16位总长度*/
		WORD id;         /*16位标志位*/
		WORD frag_off;   /*3位标志位*/
		BYTE ttl;         /*8位生存周期*/
		BYTE protocol;    /*8位协议*/
		WORD check;      /*16位IP首部校验和*/
		DWORD saddr;      /*32位源IP地址*/
		WORD daddr;      /*32位目的IP地址*/
	};
}

#define IP_HSIZE sizeof(struct NSPT::iphdr)   /*定义IP_HSIZE为ip头部长度*/
#define IPVERSION  4   /*定义IPVERSION为4，指出用ipv4*/
#define ICMP_HSIZE sizeof(struct icmphdr)
#define BUFSIZE 1500    /*发送缓存最大值*/
#define DEFAULT_LEN 56  /*ping消息数据默认大小*/

class CPintTest
{
	public:
		CPintTest();
		virtual ~CPintTest();
		BOOL Ping( char * pDestIp ) ;

	protected:
		void InitPing( ) ;
		BOOL PackICMP( char * pDestIP ) ;
		BOOL SendICMP( char * pDestIp );
		BOOL RecvICMP( char * pDestIp );
		WORD checksum( BYTE *buf,int len)  ;
		icmphdr *m_pICMP_hdr ;
		BYTE m_sbuf[ BUFSIZE ] ;
		WORD m_wRequestID ;
		HANDLE m_socket ;
		int m_dataLen ;
};

#endif // !defined(AFX_PINTTEST_H__90ECF2C5_8928_41E7_A471_A56514CF8119__INCLUDED_)
