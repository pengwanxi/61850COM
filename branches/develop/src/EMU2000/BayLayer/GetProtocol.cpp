// GetProtocol.cpp: implementation of the CGetProtocol class.
//
//////////////////////////////////////////////////////////////////////

#include "GetProtocol.h"
#include "globleDef.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <iostream>
using std::cout ;
using std::endl ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGetProtocol::CGetProtocol()
{
    /*{{{*/
    //更改程序加载路径
    ModifyPath( ) ;
}/*}}}*/

CGetProtocol::~CGetProtocol()
{
    /*{{{*/
    ReleaseHandle(  ) ;
}/*}}}*/

void CGetProtocol::AddHandle( void * pHandle )
{
    /*{{{*/
    if( pHandle == NULL )
        return ;

    m_dllhandle.push_back( pHandle ) ;
}/*}}}*/

void CGetProtocol::ReleaseHandle(  )
{
    /*{{{*/
    int size = m_dllhandle.size() ;
    for( int i = 0 ; i < size ; i++ )
    {
        void * handle = m_dllhandle[ i ] ;
        if( handle )
            dlclose(handle);
    }

    printf( "Close All Dll \n " );
    m_dllhandle.clear() ;
}/*}}}*/

CProtocol * CGetProtocol::GetProtoObj( char * pDllPath, CMethod * pMethod )
{
    /*{{{*/
    void *handle=NULL;
    char *error;
    //system( "pwd" );
    handle = OpenDll( pDllPath ) ;
    if( handle == NULL )
    {
        printf( "Open Dll = %s Failed ! \n " , pDllPath );
        return NULL ;
    }
    else
        printf( "Open Dll = %s Success ! \n " , pDllPath) ;

    typedef CProtocol * ( * ICreate )( CMethod * ) ;

    ICreate pObj = NULL ;
    pObj = ( ICreate )dlsym(handle, "CreateProtocol");				//调用动态库handle中的CreateProtocol函数!
    if( pObj == NULL )
    {
        printf( "Open Dll Failed ! \n " );
    }

    if ((error = dlerror()) != NULL)
    {
        printf ( "%s\n", error);
        dlclose( handle ) ;
        return NULL;
    }

    CProtocol * pProtocol = NULL ;
    pProtocol = pObj( pMethod ) ;
    if( pProtocol == NULL )
    {
        printf ( "Get ProtoObj Failed !  \n " );
        dlclose( handle ) ;
        return NULL;
    }

    AddHandle( handle ) ;	//dlclose(handle);

    return pProtocol ;
}/*}}}*/

BOOL CGetProtocol::ModifyPath( )
{
    /*{{{*/
    BOOL bFlag = FALSE ;
    int fd = open( SYSTEM_PATH ,O_RDONLY);
    if( fd < 0)
    {
        printf("Current Excut Path = %s Can't Open \n", SYSTEM_PATH );
        perror("open");
    }
    if( fchdir(fd) == 0 )
    {
        system("pwd");
        printf("Current Excut Path = %s Open Success \n", SYSTEM_PATH );
        bFlag = TRUE  ;
    }
    close(fd);

    return bFlag ;
}/*}}}*/

void * CGetProtocol::OpenDll( char * pDllPath )
{
    /*{{{*/
    void *handle=NULL;
    char *error = NULL;
    if( pDllPath == NULL )
        return NULL ;

    handle = dlopen ( pDllPath, RTLD_LAZY | RTLD_GLOBAL);

    if ((error = dlerror()) != NULL)
    {
        printf ( "%s\n", error);
        return NULL;
    }

    return handle ;
}/*}}}*/
