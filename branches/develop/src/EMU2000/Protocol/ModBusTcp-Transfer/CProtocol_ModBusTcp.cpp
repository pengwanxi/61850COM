#include "CProtocol_ModBusTcp.h"
#include "ModBusTcp_Transfer.h"

#define MODULE_Tcp	1

/*///////////////////////////////////////////////////////////////////////////*/
CProtocol_ModBusTcp_Transfer::CProtocol_ModBusTcp_Transfer()
{
    //ctor
	memset( m_sTemplatePath , 0 , sizeof( m_sTemplatePath ) ) ;

}

CProtocol_ModBusTcp_Transfer::~CProtocol_ModBusTcp_Transfer()
{
    //dtor
	int size = m_module.size() ;
	for(  int i = 0 ; i < size ; i++ )
	{
		delete m_module[ i ] ;
	}
	m_module.clear() ;
	printf( "Delete All CProtocol_ModBusTcp_Transfer OK . \n" );
}

BOOL CProtocol_ModBusTcp_Transfer::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg )
{
	return FALSE ;
}

BOOL CProtocol_ModBusTcp_Transfer::ProcessProtocolBuf( BYTE * buf , int len )
{
	return FALSE ;
}

BOOL CProtocol_ModBusTcp_Transfer::Init( BYTE byLineNo )
{
	//增加ModBusTcp 采集模块数据
	//通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo ;
	//读取模板文件
//	m_ProtoType = PROTOCO_GATHER ;
	m_ProtoType = PROTOCO_TRANSPROT;

	return GetDevData( ) ;
}

BOOL CProtocol_ModBusTcp_Transfer::GetDevData( )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/KunMing/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );		//打印正常!
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}

BOOL CProtocol_ModBusTcp_Transfer::ProcessFileData( CProfile &profile )
{
	BOOL bRtn = FALSE;
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 100 ]={ "module" , "serialno" , "addr" , "name" , "template" } ;

	WORD wModule = 0 ;
	int  serialno=1 ;
	WORD addr =3 ;
	char sName[ 50 ] = { 0 };
	char stemplate[ 200 ] = { 0 };
	int iNum = 0 ;

	iNum = profile.GetProfileInt( sSect , (char *)"NUM"  , 0 ) ;					//有两个重载版本，不知道为什么ctags直接自以为是的跳到了错误的地方!
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

		//创建相应模块子类
		bRtn = CreateModule( wModule , serialno , addr , sName , stemplate ) ;
		if ( !bRtn )
		{
			printf ( "Create ModBusTcp Module=%d serialno=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, serialno, addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}
BOOL CProtocol_ModBusTcp_Transfer::CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath )
{
	CProtocol_ModBusTcp_Transfer * pProtocol = NULL ;
	BOOL bRtn = FALSE;
	switch ( iModule )
	{
	case MODULE_Tcp:
			pProtocol = new ModBusTcp_KunMing;
//			pProtocol = new ModBusTcp_Transfer;
			pProtocol->m_byLineNo = m_byLineNo ;
			pProtocol->m_wModuleType = iModule ;
			pProtocol->m_wDevAddr = iAddr ;
			pProtocol->m_SerialNo = iSerialNo ;
			//strcpy( pProtocol->m_sDevName , sName ) ;
			strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod ;
//			pProtocol->m_ProtoType = PROTOCO_GATHER;
			pProtocol->m_ProtoType = PROTOCO_TRANSPROT ;
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
		{
			printf( "ModBusTcp don't contain this module Failed .\n" );
			return FALSE ;
		}
	}
		m_module.push_back( pProtocol ) ;

	return TRUE ;
}

BOOL CProtocol_ModBusTcp_Transfer::BroadCast( BYTE * buf , int &len )
{
	//组织该模块的广播报文
	int index = 0 ;
	buf[ index++ ] = 0xFF ;
	buf[ index++ ] = 0x02 ;
	buf[ index++ ] = 0x03 ;
	buf[ index++ ] = 0x04 ;
/*
	WORD wCRC = GetCrc( buf, index );
    buf[ index++ ] = HIBYTE(wCRC);
    buf[ index++ ] = LOBYTE(wCRC);
*/
	len = index ;

	printf( "\n CProtocol_ModBusTcp_Transfer  TestBroadCast \n " ) ;
	return TRUE ;
}

