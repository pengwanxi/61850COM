#include "CProtocol_DDB.h"
#include "DDB.h"

#define MODULE_RTU	1

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

CProtocol_DDB::CProtocol_DDB()
{
    //ctor
	memset( m_sTemplatePath , 0 , sizeof( m_sTemplatePath ) ) ;

}

CProtocol_DDB::~CProtocol_DDB()
{
    //dtor
	int size = m_module.size() ;
	for(  int i = 0 ; i < size ; i++ )
	{
		delete m_module[ i ] ;
	}
	m_module.clear() ;
	printf( "Delete All CProtocol_DDB OK . \n" );
}

BOOL CProtocol_DDB::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg )
{
	return FALSE ;
}

BOOL CProtocol_DDB::ProcessProtocolBuf( BYTE * buf , int len )
{
	return FALSE ;
}

BOOL CProtocol_DDB::Init( BYTE byLineNo )
{
	//通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo ;
	//读取模板文件
	m_ProtoType = PROTOCO_TRANSPROT	;

	return GetDevData( ) ;
}

BOOL CProtocol_DDB::GetDevData( )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/DDB/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}

BOOL CProtocol_DDB::ProcessFileData( CProfile &profile )
{
	BOOL bRtn = FALSE;
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 100 ]={ "module" ,   "addr" , "name" , "template" } ;

	WORD wModule = 0 ;
	WORD addr =3 ;
	char sName[ 50 ] = { 0 };
	char stemplate[ 200 ] = { 0 };
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
		addr = profile.GetProfileInt( sSect , sKey[ 1 ] , 0 ) ;
		profile.GetProfileString( sSect , sKey[ 2 ] , (char *)"NULL"  , sName , sizeof( sName ) ) ;
		profile.GetProfileString( sSect , sKey[ 3 ] , (char *)"NULL" , stemplate , sizeof( stemplate ) ) ;

		//创建相应模块子类
		bRtn = CreateModule( wModule  , addr , sName , stemplate ) ;
		if ( !bRtn )
		{
			printf ( "Create ModBus Module=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}

BOOL CProtocol_DDB::CreateModule( int iModule , WORD iAddr , char * sName , char * stplatePath )
{
	CProtocol_DDB * pProtocol = NULL ;

	switch ( iModule )
	{
	case MODULE_RTU:
		{
			pProtocol = new CDDB ;
			pProtocol->m_byLineNo = m_byLineNo ;
			pProtocol->m_wModuleType = iModule ;
			pProtocol->m_wDevAddr = iAddr ;
			//strcpy( pProtocol->m_sDevName , sName ) ;
			strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod ;
			pProtocol->m_ProtoType = PROTOCO_TRANSPROT ;
			//初始化模板数据
			if( !pProtocol->Init( m_byLineNo ) )
				return FALSE ;
			printf( " Add bus = %d Addr = %d \n" , m_byLineNo , iAddr ) ;
		}
		break;
	default:
		{
			printf( "ModBus don't contain this module Failed .\n" );
			return FALSE ;
		}
	}
		m_module.push_back( pProtocol ) ;

	return TRUE ;
}


BOOL CProtocol_DDB::BroadCast( BYTE * buf , int &len )
{
	return TRUE ;
}

