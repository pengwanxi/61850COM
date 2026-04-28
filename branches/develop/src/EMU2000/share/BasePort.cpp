/******************************************************************************
*  BasePort.cpp: implementation of the BasePort class.
*  Copyright (C): 2008-2038 by houpeng
******************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#include "BasePort.h"
#include "global.h"

/*****************************************************************************/
CBasePort::CBasePort()
{
	m_hComm = 0;
	m_bValid = FALSE;
	m_wPortState = 0;

    m_byEnable = 1;
    m_uThePort = 2044;
}

CBasePort::~CBasePort()
{
	ClosePort();
}

char *Strstr(const char *s1, const char *s2)
{
	const char *p = s1;
	const size_t len = strlen(s2);
	for(; (p = strchr(p, *s2)) != 0; p++)
		if(strncmp(p,s2,len) == 0)
			return (char *)p;
	return NULL;
}

int CBasePort::GetCommAttrib( const char* p, char* lpszAttr, UINT& uPort )
{
	const char* lpszCtrl = p;
//	char * pReturn = strstr( lpszCtrl , "_" ) ;
	if(lpszCtrl == NULL)
		return -1;
	char *pReturn = Strstr(lpszCtrl, "_");
	if( pReturn == NULL )
		return 0 ;

	//获得通讯类型
	int  result;
	result = pReturn - lpszCtrl + 1;

	char cTypeBuf[ 30 ] ;
	memset( cTypeBuf  , 0 , sizeof( cTypeBuf ) ) ;
	memcpy( cTypeBuf , lpszCtrl , result ) ;

	BYTE byComType = 0 ;
	if (!strcmp(cTypeBuf, COMRS_232))
		byComType = COMRS232;
	else if (!strcmp(cTypeBuf, COMRS_485))
		byComType = COMRS485;
	else if (!strcmp(cTypeBuf, COMRS_422))
		byComType = COMRS422;
	else if (!strcmp(cTypeBuf, LAN_TCP))
	{
		byComType = SOCKETTCP;
		//获得端口号
		pReturn++;
		uPort = atoi(pReturn);
		strcpy(lpszAttr, "NULL");
		return byComType;
	}
	else if (!strcmp(cTypeBuf, LAN_TCP_CLIENT))
		byComType = TCP_CLIENT;
	else if (!strcmp(cTypeBuf, LAN_TCP_CLIENT_SHORT))
		byComType = TCP_CLIENT_SHORT;
	else if (!strcmp(cTypeBuf, COM_CAN_NET))
		byComType = CAN_NET;
	else if (!strcmp(cTypeBuf, COM_LORA))
		byComType = LORA_WIRELESS;
	else
		return 0 ;

	//获得端口号
	pReturn++ ;
	char * pPort = strstr( pReturn , ":" ) ;
	if( pPort == NULL )
		return 0 ;

	result = pPort - pReturn  ;
	memset( cTypeBuf  , 0 , sizeof( cTypeBuf ) ) ;
	memcpy( cTypeBuf , pReturn , result ) ;
	uPort = atoi( cTypeBuf ) ;

	//获得属性
	pPort++ ;
	strcpy( lpszAttr , pPort ) ;

	return byComType ;
}

int CBasePort::GetCommAttrib_original( const char* lpszCtrl, char* lpszAttr, UINT& uPort )
{
	int	  i, nLen;
	int   k, nStyle = -1;
	char  szParam[32];
	const char* p = lpszCtrl;

	k = 0;
	nLen = strlen(lpszCtrl);
	for( i=0; i<nLen; i++, p++ )
	{
		if( *p==' ' || *p=='\t' ) { continue; }
		else if( !isdigit(*p) ) { szParam[k++] = toupper(*p); }
		else { szParam[k] = '\0'; break; }
	}
	sprintf( lpszAttr, "%s", p );

	if(strcmp(szParam, "COM")==0)      { nStyle = 0; }
	else if(strcmp(szParam, "TCP")==0) { nStyle = 1; }
	else if(strcmp(szParam, "UDP")==0) { nStyle = 2; }
	else if(strcmp(szParam, "CAN")==0) { nStyle = 3; }

	for( k=0; i<nLen; i++, p++ )
	{
		if( *p != ':' ) { szParam[k++] = *p; }
		else { szParam[k] = '\0'; p++; break; }
	}
	sprintf( lpszAttr, "%s", p );
	uPort = (UINT)atoi(szParam);

	return nStyle;
}

void CBasePort::ReceiveProc( void )
{
}

BOOL CBasePort::IsPortValid( void )
{
	return FALSE;
}

void CBasePort::ClosePort( void )
{
}

BOOL CBasePort::OpenPort( char* lpszError )
{
	return FALSE;
}

BOOL CBasePort::SetQueue( DWORD dwInQueueSize, DWORD dwOutQueueSize )
{
	return FALSE;
}

int	CBasePort::GetInQueue( void )
{
	return 0;
}

int CBasePort::GetOutQueue( void )
{
	return 0;
}

int	CBasePort::ReadPort( BYTE *pBuf, int nRead )
{
	return 0;
}

int	CBasePort::WritePort( BYTE *pBuf, int nWrite )
{
	return 0;
}

int	CBasePort::AsyReadData( BYTE *pBuf, int nRead )
{
	return 0;
}

int CBasePort::AsySendData( BYTE *pBuf, int nWrite )
{
	return 0;
}
/*****************************************************************************/
