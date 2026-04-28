
#include <stdio.h>
#include "CProtocol_DDB.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_DDB ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "DDB DLL  OK.\n " ) ;
	}
  
	return pProtocol ;
}
