#include "CProtocol_LFP.h"
#include "Lfp_Nsa3000.h"

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


CProtocol_LFP::CProtocol_LFP()
{
    memset( m_sTemplatePath , 0 , sizeof( m_sTemplatePath ) ) ;
}

CProtocol_LFP::~CProtocol_LFP()
{
    //dtor
	int size = m_module.size() ;
	for(  int i = 0 ; i < size ; i++ )
	{
		delete m_module[ i ] ;
	}
	m_module.clear() ;
	//printf( "Delete All CProtocol_LFP OK . \n" );
}

BOOL CProtocol_LFP::Init( BYTE byLineNo )
{
	//增加ModBus 采集模块数据
	//通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo ;
	//读取模板文件
	m_ProtoType = PROTOCO_GATHER ;

	return GetDevData( ) ;
}

BOOL CProtocol_LFP::GetDevData( )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/LFP/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}

BOOL CProtocol_LFP::ProcessFileData( CProfile &profile )
{
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 100 ]={ "module" , "serialno" , "addr" , "name" , "template" } ;

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
		WORD wModule = 0 ;
		int  serialno = 0 ;
		WORD addr = 0 ;
		BOOL bRtn = FALSE;

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
			printf ( "Create LFP Module=%d serialno=%d addr=%d sName=%s stemplate=%s	Error \n",
				wModule, serialno, addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}

BOOL CProtocol_LFP::CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath )
{
	CProtocol_LFP * pProtocol = NULL ;
	// BOOL bRtn;

	switch ( iModule )
	{
	case NSA3000:
		{
			pProtocol = new CLfp_Nsa3000 ;
			pProtocol->m_byLineNo = m_byLineNo ;
			pProtocol->m_wModuleType = iModule ;
			pProtocol->m_wDevAddr = iAddr ;
			pProtocol->m_SerialNo = iSerialNo ;
			strcpy( pProtocol->m_sDevName , sName ) ;
			strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod ;
			pProtocol->m_ProtoType = PROTOCO_GATHER ;
			//初始化模板数据
			if( !pProtocol->Init( m_byLineNo ) ) 
			{
				printf( " Add bus = %d Addr = %d serialno = %d error!!!\n" , m_byLineNo , iAddr, iSerialNo ) ;
				return FALSE;
				
			}
			//printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
		}
		break;

	default:
		{
			printf( "LFP don't contain this module, Module No:%d, sName=%s \n", iModule, sName);
			return FALSE ;
		}
	}
	if(pProtocol)
		m_module.push_back( pProtocol ) ;

	return TRUE ;
}


WORD CProtocol_LFP::GetCrc( BYTE * pBuf , int len )
{
	WORD byRtn = 0x00;
	int i;

	if( pBuf == NULL || len <= 0 )
		return byRtn;

	for ( i=0; i<len ; i++ )
	{
		byRtn += pBuf[i];
	}

	return byRtn;
}

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_LFP
 *      Method:  print
 * Description:  打印到终端 或者 总线方便调试 调试时使用
 *       Input:	 缓冲区 长度
 *		Return:	
 *--------------------------------------------------------------------------------------
 */
void CProtocol_LFP::print ( const char *szBuf, int len )
{
#ifdef  LFPPRINT
	printf( "%s\n", szBuf );	
#endif     /* -----  not PRINT  ----- */

#ifdef  LFPDEBUG
	OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
#endif     /* -----  not DEBUG  ----- */
}		/* -----  end of method CProtocol_LFP::print  ----- */


