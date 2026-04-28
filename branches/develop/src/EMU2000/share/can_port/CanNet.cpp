#include "CanNet.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <linux/can.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/can/raw.h>


CCanNet::CCanNet()
{

}


CCanNet::~CCanNet()
{
    ClosePort() ;
}

void CCanNet::modifyBaudrate()
{

}

BOOL CCanNet::IsPortValid(void)
{
	if (m_socket != INVALID_HANDLE)
		return true;

	return false;
}

BOOL CCanNet::OpenPort(char* lpszError /*= NULL*/)
{
	m_port = m_uThePort;
	m_baudrate = atoi(m_szAttrib);
	modifyBaudrate();

	return openSocket();
}

bool CCanNet::openSocket()
{
	struct sockaddr_can addr;
	struct ifreq ifr;

	if ((m_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		perror("can-socket");
		return false;
	}

	char canName[10] = { 0 };
	sprintf(canName, "can%d", m_port);
	strcpy(ifr.ifr_name, canName);
	ioctl(m_socket, SIOCGIFINDEX, &ifr);

	addr.can_family = PF_CAN ;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("can-bind");
		return false;
	}

	return true;
}

void CCanNet::ClosePort(void)
{
	if (m_socket != INVALID_HANDLE)
	{
		shutdown(m_socket, 2);
		close(m_socket);
		m_socket = INVALID_HANDLE;
	}
}

int CCanNet::ReadPort(BYTE *pBuf, int nRead)
{
	if (!pBuf || ( UINT )nRead < sizeof(struct can_frame))
		return -1;

	if (m_socket == INVALID_HANDLE)
		return -1;

    struct can_frame frame;
	int nbytes = read( m_socket, &frame, sizeof ( frame ) );
    if ( nbytes < 0 )
     {
        return -1;
     }

	memcpy(pBuf, &frame, sizeof(frame));
	return nbytes ;

}

int CCanNet::WritePort(BYTE *pBuf, int nWrite)
{
	if ( !pBuf || (UINT)nWrite == 0  )
		return -1;

	if (m_socket == INVALID_HANDLE)
		return -1;

    int len = write( m_socket, pBuf , nWrite );
    if( len == -1 )
    perror( "can write" );
	return len ;
}

int CCanNet::AsyReadData(BYTE *pBuf, int nRead)
{
	int  nBytes = 0;
	fd_set rfds;
	struct timeval tv;

	if( !IsPortValid() || nRead<=0 ) return -1;
	FD_ZERO(&rfds);
	FD_SET(m_socket, &rfds);
	tv.tv_sec  = 0;
	tv.tv_usec = 10000;
	switch( select(m_socket +1, &rfds, NULL, NULL, &tv) )
	{
	case ERROR:
		return -2;
	case 0:
		break;
	case 1:
		if(FD_ISSET(m_socket, &rfds))
		{
            struct can_frame frame;
            nBytes = read( m_socket, &frame, sizeof ( frame ) );
            if ( nBytes < 0 )
             {
                return -1;
             }

            memcpy(pBuf, &frame, sizeof(frame));
			if( nBytes == ERROR )
				return 0;
			else if( nBytes == 0 )
				ClosePort() ;
		}
		break;
	}

	return nBytes;

//	return ReadPort(pBuf, nRead);
}

int CCanNet::AsySendData(BYTE *pBuf, int nWrite)
{
    return 0 ;
}
