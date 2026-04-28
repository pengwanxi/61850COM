#include "CProtocol_IEC615.h"
#include "IEC615.h"
#include <stdlib.h>
#include <fstream>
#include <string>
using namespace std ;

#define _IEC615_	1
#if 0
#define _IEC615_0	2
#define _IEC615_1	3
#define _IEC615_2	4
#define _IEC615_3	5
#define _IEC615_4	6
#define _IEC615_5	7
#define _IEC615_6	8
#define _IEC615_7	9
#define _IEC615_8	10
#define _IEC615_9	11
#define _IEC615_A	12
#define _IEC615_B	13
#define _IEC615_C	14
#define _IEC615_D	15
#define _IEC615_E	16
#define _IEC615_F	17
#define _IEC615_G	18
#define _IEC615_H	19
#define _IEC615_I	20
#define _IEC615_J	21
#define _IEC615_K	22
#define _IEC615_L	23
#define _IEC615_M	24
#define _IEC615_N	25
#define _IEC615_O	26
#define _IEC615_P	27
#define _IEC615_Q	28
#define _IEC615_R	29
#define _IEC615_S	30
#define _IEC615_50	31
#endif

/*///////////////////////////////////////////////////////////////////////////*/
CProtocol_IEC615::CProtocol_IEC615()
{/*{{{*/
	//ctor
	memset( m_sTemplatePath , 0 , sizeof( m_sTemplatePath ) ) ;

}/*}}}*/

CProtocol_IEC615::~CProtocol_IEC615()
{/*{{{*/
	//dtor
	int size = m_module.size() ;
	for(  int i = 0 ; i < size ; i++ )
	{
		delete m_module[ i ] ;
	}
	m_module.clear() ;
	printf( "Delete All CProtocol_IEC615 OK . \n" );
}/*}}}*/

BOOL CProtocol_IEC615::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg )
{/*{{{*/
	return FALSE ;
}/*}}}*/

BOOL CProtocol_IEC615::ProcessProtocolBuf( BYTE * buf , int len )
{/*{{{*/
	return FALSE ;
}/*}}}*/

BOOL CProtocol_IEC615::Init( BYTE byLineNo )
{/*{{{*/
	//增加ModBus 采集模块数据
	//通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo ;
	//读取模板文件
	m_ProtoType = PROTOCO_GATHER ;

	return GetDevData( ) ;
}/*}}}*/

BOOL CProtocol_IEC615::GetDevData( )
{/*{{{*/
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/IEC61850/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}/*}}}*/

BOOL CProtocol_IEC615::ProcessFileData( CProfile &profile )
{/*{{{*/
	BOOL bRtn = FALSE;
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 100 ]={ "module" , "serialno" , "addr" , "name" , "template", "ip" };		//为61850新增字段ip

	WORD wModule = 0 ;
	WORD addr =3 ;
	int  serialno=1 ;
	char sName[ 50 ] = { 0 };
	char stemplate[ 200 ] = { 0 };
	char _ip[16] = {0};
	int iNum = 0 ;

	iNum = profile.GetProfileInt( sSect , (char *)"NUM"  , 0 ) ;
	if( iNum == 0 )
	{
		printf( "Get DEVNUM Failed ! \n " );
		return FALSE ;
	}

	for( int i = 0 ; i < iNum ; i++ )
	{
		sprintf( sSect , "%s%03d" ,  "DEV" , i + 1 );

		wModule = profile.GetProfileInt( sSect , sKey[ 0 ] , 0 ) ;
		serialno = profile.GetProfileInt( sSect , sKey[ 1 ] , 0 ) ;
		addr = profile.GetProfileInt( sSect , sKey[ 2 ] , 0 ) ;
		profile.GetProfileString( sSect , sKey[ 3 ] , (char *)"NULL"  , sName , sizeof( sName ) ) ;
		profile.GetProfileString( sSect , sKey[ 4 ] , (char *)"NULL" , stemplate , sizeof( stemplate ) ) ;
		profile.GetProfileString(sSect, sKey[5], (char *)"NULL", _ip, sizeof(_ip));

		//创建相应模块子类
		bRtn = CreateModule( wModule , serialno , addr , sName , stemplate, _ip) ;
		if ( !bRtn )
		{
			printf ( "Create IEC615 Module=%d serialno=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, serialno, addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}/*}}}*/

//BOOL CProtocol_IEC615::CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath, char *ip)
//{[>{{{<]
//CProtocol_IEC615 * pProtocol = NULL ;
//BOOL bRtn = FALSE;
//switch ( iModule )
//{
//case _IEC615_:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.10", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_0:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.11", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_1:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.12", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_2:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.13", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_3:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.14", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_4:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.15", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_5:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.16", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_6:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.17", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_7:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.18", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_8:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.19", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_9:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.20", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_A:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.21", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_B:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.22", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_C:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.23", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_D:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.24", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_E:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.25", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_F:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.26", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_G:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.27", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_H:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.28", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_I:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.29", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_J:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.30", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_K:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.31", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_L:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.32", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_M:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.33", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_N:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.34", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_O:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.35", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_P:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.36", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_Q:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.37", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_R:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.38", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_S:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.39", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//case _IEC615_50:
//{[>{{{<]
//pProtocol = new IEC615 ;
//pProtocol->m_byLineNo = m_byLineNo ;
//pProtocol->m_wModuleType = iModule ;
//pProtocol->m_wDevAddr = iAddr ;
//pProtocol->m_SerialNo = iSerialNo ;
////strcpy( pProtocol->m_sDevName , sName ) ;
//strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
//m_pMethod->m_pRtuObj = pProtocol;
//pProtocol->m_pMethod = m_pMethod ;

////memcpy(pProtocol->host_name, (char *)"192.168.2.50", sizeof(host_name));
//memcpy(pProtocol->host_name, ip, sizeof(host_name));

//pProtocol->m_ProtoType = PROTOCO_GATHER ;
////初始化模板数据
//bRtn = pProtocol->Init( m_byLineNo ) ;
//if ( !bRtn )
//{
//printf ( "Init Error \n");
//return FALSE;
//}
//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
//}[>}}}<]
//break;
//default:
//{[>{{{<]
//printf( "IEC615 don't contain this module Failed .\n" );
//return FALSE ;
//}[>}}}<]
//}
//m_module.push_back( pProtocol ) ;

//return TRUE ;
//}[>}}}<]

BOOL CProtocol_IEC615::CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath, char *ip)
{/*{{{*/
	CProtocol_IEC615 * pProtocol = NULL ;
	BOOL bRtn = FALSE;
	switch(iModule){
	case _IEC615_:
		pProtocol = new IEC615 ;
		pProtocol->m_byLineNo = m_byLineNo ;
		pProtocol->m_wModuleType = iModule ;
		pProtocol->m_wDevAddr = iAddr ;
		pProtocol->m_SerialNo = iSerialNo ;
		strcpy( pProtocol->m_sDevName , sName ) ;
		strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
		m_pMethod->m_pRtuObj = pProtocol;
		pProtocol->m_pMethod = m_pMethod ;
		memcpy(pProtocol->host_name, ip, sizeof(host_name));
		pProtocol->m_ProtoType = PROTOCO_GATHER ;
		//初始化模板数据
		bRtn = pProtocol->Init( m_byLineNo ) ;
		if ( !bRtn )
		{
			printf ( "Init Error \n");
			return FALSE;
		}
		printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
		break;
	default:
		printf("CProtocol_IEC615 don't contain this module Failed!");
		return FALSE;
	}
	m_module.push_back( pProtocol ) ;
	return TRUE;
}/*}}}*/

//WORD CProtocol_IEC615::GetCrc( BYTE * pBuf , int len )
//{[>{{{<]
//#if 0
//unsigned char uchCRCHi = 0xFF;			[> 高CRC字节初始化 <]
//unsigned char uchCRCLo = 0xFF;			[> 低CRC 字节初始化 <]
//unsigned uIndex;						[> CRC循环中的索引 <]

//while( len-- )						[> 传输消息缓冲区 <]
//{
//uIndex = uchCRCHi ^ *pBuf++ ;	[> 计算CRC <]
//uchCRCHi = uchCRCLo ^ AuchCRCHi[uIndex];
//uchCRCLo = AuchCRCLo[uIndex] ;
//}
//#endif
//return (uchCRCHi << 8 | uchCRCLo) ;
//}[>}}}<]

//BOOL CProtocol_IEC615::BroadCast( BYTE * buf , int &len )
//{[>{{{<]
////组织该模块的广播报文
//#if 0
//int index = 0 ;
//buf[ index++ ] = 0xFF ;
//buf[ index++ ] = 0x02 ;
//buf[ index++ ] = 0x03 ;
//buf[ index++ ] = 0x04 ;

//WORD wCRC = GetCrc( buf, index );
//buf[ index++ ] = HIBYTE(wCRC);
//buf[ index++ ] = LOBYTE(wCRC);

//len = index ;

//printf( "\n CProtocol_IEC615  TestBroadCast \n " ) ;
//#endif
//return TRUE ;
//}[>}}}<]
