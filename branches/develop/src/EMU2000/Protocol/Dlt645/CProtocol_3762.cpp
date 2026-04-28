#include "CProtocol_3762.h"

CProtocol_3762::CProtocol_3762()
{
	m_sendmsg_sequence = 0;
}

CProtocol_3762::~CProtocol_3762()
{

}

BOOL CProtocol_3762::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg /*= NULL */)
{
	WORD index = 0;
	buf[index++] = 0x68;
	buf[index++] = 0; //长度
	buf[index++] = 0; //长度
	buf[index++] = 0x43;//控制域 从站方向
	buf[index++] = 0x04;
	buf[index++] = 0x00;//部分信道 不编码
	buf[index++] = 0x00;//延时等待时间 160
	buf[index++] = 0x00;//50bps 通信波特率
	buf[index++] = 0;//0表示bps 1表示kbps
	buf[index++] = m_sendmsg_sequence++;//0到255 报文序号
	//主节点地址
	buf[index++] = 0x01;
	buf[index++] = 0x00;
	buf[index++] = 0x00;
	buf[index++] = 0x00;
	buf[index++] = 0x00;
	buf[index++] = 0x00;
	//目的节点地址
	buf[index++] = m_bySlaveAddr[0];
	buf[index++] = m_bySlaveAddr[1];
	buf[index++] = m_bySlaveAddr[2];
	buf[index++] = m_bySlaveAddr[3];
	buf[index++] = m_bySlaveAddr[4];
	buf[index++] = m_bySlaveAddr[5];
	//路由数据转发
	buf[index++] = 0x13;
	//路由请求抄读内容
	buf[index++] = 0x01;
	buf[index++] = 0x00;
	//抄读内容
	buf[index++] = 0x02;//可以抄读
	buf[index++] = 0x00;
	buf[index++] = 0x00;//数据长度

	CDlt645_2007::GetProtocolBuf(buf+index + 1 , len, pBusMsg);

	buf[index ] = len;//填充数据长度
	index = index + len+1;
	buf[index++] = GetCs(buf + 3, index); //cs校验码
	buf[index++] = 0x16; //结束符

	len = index;

	buf[1] = LOBYTE(index);
	buf[2] = HIBYTE(index);


	printf("send len=%d\n: ", len );
	for (int i = 0; i < len; i++)
	{
		printf("%02x ", buf[i]);
	}
	printf("\n");

	return TRUE;
}

BOOL CProtocol_3762::ProcessProtocolBuf(BYTE * buf, int len)
{	
	printf("recv len %d\n: len ");
	for (int i = 0 ; i <len ; i++ )
	{
		printf("%02x ", buf[i]);
	}
	printf("\n");

	int msg_len = buf[1];
	if (msg_len != len)
	{
		printf("process msg_len != len\n");
		return TRUE;
	}

	//直接跳到数据域
	BYTE * pData = buf + 29;
	int dlt645len = buf[28];
	//取出Dlt645报文
	CDlt645_2007::ProcessProtocolBuf(pData , dlt645len );


	return TRUE;
}
