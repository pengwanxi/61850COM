/*
 * =====================================================================================
 *
 *       Filename:  CProtocol_DataTrans.c
 *
 *    Description:  关于ESD自定义历史数据上传 
 *
 *        Version:  1.0
 *        Created:  2015年06月09日 15时33分15秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:	Time						Author			version			Description
 *					2015-06-09 14:25:52         mengqp			1.0				created
 * =====================================================================================
 */

/* #####   HEADER FILE INCLUDES   ################################################### */
#include "CProtocol_DataTrans.h"
#include "CDataTrans.h"
#include "CDTWithTime.h"


/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ################################### */

#define		MODULE_DATATRANS			1			/*  */
#define		MODULE_DATATRANSWITHTIME	2			/*  */

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################ */
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  CProtocol_DataTrans
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CProtocol_DataTrans::CProtocol_DataTrans ()
{
	memset( m_sTemplatePath, 0, sizeof( m_sTemplatePath ) );
	memset( m_szPrintBuf , 0, sizeof(m_szPrintBuf) );
	memset( m_sMasterAddr, 0, sizeof( m_sMasterAddr ) );
	
	m_ProtocolState = 0;
	printf ( "CProtocol_DataTrans constructor\n" );
}  /* -----  end of method CProtocol_DataTrans::CProtocol_DataTrans  (constructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  ~CProtocol_DataTrans
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CProtocol_DataTrans::~CProtocol_DataTrans ()
{
	//删除已创建的模块
	unsigned long size = m_module.size() ;
	for(  unsigned long i = 0 ; i < size ; i++ )
	{
		delete m_module[ i ] ;
	}
	m_module.clear() ;

	printf( "Delete All CProtocol_DataTrans OK . \n" );
}  /* -----  end of method CProtocol_DataTrans::~CProtocol_DataTrans  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  GetProtocolBuf
 * Description:  获取协议报文 
 *       Input:  buf:存取报文的缓冲区
 *				 len:缓冲区长度
 *				 pBusMsg:消息指针 没有时为空
 *		Return:  
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::GetProtocolBuf ( BYTE *buf, int &len, PBUSMSG pBusMsg )
{
	return FALSE;
}		/* -----  end of method CProtocol_DataTrans::GetProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  ProcessProtocolBuf
 * Description:  处理协议报文 
 *       Input:  buf:处理报文的缓冲区
 *				 len:缓冲区长度
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::ProcessProtocolBuf ( BYTE *buf, int len )
{
	return FALSE;
}		/* -----  end of method CProtocol_DataTrans::ProcessProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  Init
 * Description:  初始化总线模块
 *       Input:  byLineNo 总线号
 *		Return:  
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::Init ( BYTE byLineNo )
{
	//总线号
	m_byLineNo = byLineNo;
	//协议类型
	m_ProtoType = PROTOCO_TRANSPROT;

	return GetDevData(  );
}		/* -----  end of method CProtocol_DataTrans::Init  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  GetDevData
 * Description:  获取总线中的装置数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::GetDevData ( void )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/DataTrans/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	return ProcessFileData( profile );
}		/* -----  end of method CProtocol_DataTrans::GetDevData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  ProcessFileData
 * Description:  处理配置文件数据
 *       Input:  profile :文件名
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::ProcessFileData ( CProfile &profile )
{

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 50 ]={ "module" , "addr" , "name" , "masteraddr" , "template" , "ycdead" , "ycProperty" , "timing"} ;
	char sName[ 50 ] = { 0 };//模块名字
	char stemplate[ 200 ] = { 0 };//模板路径
	int iNum = 0 ;//站数量
	char sMasterAddr[ 200 ] = { 0 } ;//主站IP地址和端口

	iNum = profile.GetProfileInt( sSect , (char *)"NUM"  , 0 ) ;
	if( iNum == 0 )
	{
		printf( "Get DEVNUM Failed ! \n " );
		return FALSE ;
	}

	BYTE byIndex = 0 ;
	for( int i = 0 ; i < iNum ; i++ )
	{
		BOOL bRtn;
		int  iModule = 0 ;//模块标识
		int addr =0 ;//装置地址
		sprintf( sSect , "%s%03d" ,  "DEV" , i + 1 );

		iModule = profile.GetProfileInt( sSect , sKey[ byIndex++ ] , 0 ) ;
		addr = profile.GetProfileInt( sSect , sKey[ byIndex++ ] , 0 ) ;

		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL"  , sName , sizeof( sName ) ) ;
		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL" , sMasterAddr , sizeof( stemplate ) ) ;
		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL" , stemplate , sizeof( stemplate ) ) ;

		//创建相应模块子类
		bRtn = CreateModule( iModule ,
				sMasterAddr ,
				addr , 
				sName , 
				stemplate ) ;
		if ( !bRtn )
		{
			printf ( "Create IEC101S Module=%d addr=%d sName=%s stemplate=%s \
					Error \n", iModule,  addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}		/* -----  end of method CProtocol_DataTrans::ProcessFileData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  CreateModule
 * Description:  创建模块
 *       Input:  iModule: 模块号
 *				 sMasterAddr:主站地址
 *				 iAddr:本机地址
 *				 sName:模块名
 *				 stplatePath:配置软件路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::CreateModule ( int iModule , 
		char * sMasterAddr ,
		int iAddr ,
		char * sName ,
		char * stplatePath )
{
	CProtocol_DataTrans * pProtocol = NULL ;

	switch ( iModule )
	{
		case MODULE_DATATRANS:
			{
				pProtocol = new CDataTrans;
				if( !InitModule( pProtocol , iModule , sMasterAddr , iAddr , sName , stplatePath ) )
					return FALSE ;
			}
			break;
		case MODULE_DATATRANSWITHTIME:	
			{
				pProtocol = new CDTWithTime;
				if( !InitModule( pProtocol , iModule , sMasterAddr , iAddr , sName , stplatePath ) )
					return FALSE ;
			}
			break;

		default:
			{
				printf( "%s don't contain this module Failed .\n" , "IEC101S"  );
				return FALSE ;
			}
	}

	m_module.push_back( pProtocol ) ;
	return TRUE ;
}		/* -----  end of method CProtocol_DataTrans::CreateModule  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  InitModule
 * Description:  初始化模块参数
 *       Input:	 pProtocol:协议指针   
 *				 iModule: 模块号
 *				 sMasterAddr:主站地址
 *				 iAddr:本机地址
 *				 sName:模块名
 *				 stplatePath:配置软件路径
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::InitModule ( CProtocol_DataTrans * pProtocol ,
		int iModule , 
		char * sMasterAddr ,
		int iAddr ,
		char * sName ,
		char * stplatePath )
{
	if( pProtocol == NULL )
		return FALSE ;

	pProtocol->m_byLineNo = m_byLineNo ;
	pProtocol->m_wModuleType = iModule ;
	pProtocol->m_wDevAddr = iAddr ;
	printf("iddr=%d m_byLineNo=%d\n",  iAddr, m_byLineNo);
	strcpy( pProtocol->m_sDevName , sName ) ;
	strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
	strcpy( pProtocol->m_sMasterAddr , sMasterAddr ) ;
	m_pMethod->m_pRtuObj = pProtocol;
	pProtocol->m_pMethod = m_pMethod ;
	pProtocol->m_ProtoType = PROTOCO_TRANSPROT ;
	//初始化模板数据
	if( !pProtocol->Init( m_byLineNo ) )
				return FALSE ;
			printf( " Add bus = %d Addr = %d ProtocolName = %s \n" , m_byLineNo , iAddr , sName ) ;

	return TRUE ;
}		/* -----  end of method CProtocol_DataTrans::InitModule  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  print
 * Description:  内部打印函数
 *       Input:  缓存区
 *		Return:
 *--------------------------------------------------------------------------------------
 */
#define ESD_DATATRANS_PRINT 1
#define ESD_DATATRANS_DEBUG 1
//Above of them defined by cyz. Just for debug!
void CProtocol_DataTrans::print ( const char *szBuf )
{
#ifdef  ESD_DATATRANS_PRINT
	printf ( "%s\n" , szBuf);
#endif     /* -----  not ESD_DATATRANS_PRINT  ----- */

#ifdef  ESD_DATATRANS_DEBUG
	OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
#endif     /* -----  not ESD_DATATRANS_DEBUG  ----- */
}		/* -----  end of method CProtocol_DataTrans::print  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  WhetherBufValid
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::WhetherBufValid ( const BYTE *buf, 
		int &len,
		int &pos )
{
	const BYTE *pointer = buf;
	pos = 0;

	if ( buf == NULL || len <= 0 )
	{
		sprintf( m_szPrintBuf, "CProtocol_DataTrans WhetherBufValue err buf==NULL or len = %d" , len);
		print( m_szPrintBuf );
		return FALSE;
	}
	
	while ( len > 1 )
	{
		switch(*pointer)
		{
			case 0x68:  //判断可变帧
				switch ( *( pointer + 1 ) )
				{
					case 0xF1:	
					case 0xF3:	
					case 0xF5:	
					case 0xF7:	
						len = 2;
						return TRUE;
						break;

					default:	
						sprintf ( m_szPrintBuf, 
								"CProtocol_DataTrans WhetherBufValid func code %x is err!!!",
								*(pointer + 1) );
						print( m_szPrintBuf );
						goto DEFAULT;
				}				/* -----  end switch  ----- */
				break;
			default:
				{
					goto DEFAULT;
				}
				break;
		}
DEFAULT:
		len--;
		pointer++;
		pos ++;

	}

	return FALSE;
}		/* -----  end of method CProtocol_DataTrans::WhetherBufValid  ----- */

/* #####   other 其它部分   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  SetState
 * Description:  设置状态 
 *       Input:  dwState:要设置的状态
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CProtocol_DataTrans::SetState ( DWORD dwState )
{
	m_ProtocolState |= dwState;
	// printf ( "set m_ProtocolState=%lu %lu\n", m_ProtocolState, dwState );
}		/* -----  end of method CProtocol_DataTrans::SetState  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  UnsetState
 * Description:  取消状态
 *       Input:  dwState:要取消的状态
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CProtocol_DataTrans::UnsetState ( DWORD dwState )
{
	m_ProtocolState &= ~dwState;
}		/* -----  end of method CProtocol_DataTrans::UnsetState  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  IsHaveState
 * Description:  查看是否有该类型
 *       Input:  dwState:要查看的状态
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_DataTrans::IsHaveState ( DWORD dwState ) const
{
	if( (m_ProtocolState & dwState) )
	{
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CProtocol_DataTrans::IsHaveState  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  OpenLink
 * Description:  打开链接
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CProtocol_DataTrans::OpenLink ( void )
{
	if( IsHaveState(  DATATRANS_LINK_STATE ) )
	{
		return ;
	}

	//执行open
	m_pMethod->OpenSocket( m_byLineNo );

	print( (char *)"CProtocol_DataTrans open link" );
	SetState( DATATRANS_LINK_STATE );
}		/* -----  end of method CProtocol_DataTrans::OpenLink  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_DataTrans
 *      Method:  CloseLink
 * Description:  关闭链接
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CProtocol_DataTrans::CloseLink ( void )
{
	m_pMethod->CloseSocket( m_byLineNo );

	if( IsHaveState(  DATATRANS_LINK_STATE ) )
	{
		//执行close
		print( (char *)"CProtocol_DataTrans close link" );
		UnsetState( DATATRANS_LINK_STATE );
	}

}		/* -----  end of method CProtocol_DataTrans::CloseLink  ----- */

