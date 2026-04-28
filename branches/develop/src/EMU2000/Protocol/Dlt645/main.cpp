
#include <stdio.h>
#include "CProtocol_Dlt645.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_Dlt645 ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "Dlt645 DLL  OK.\n " ) ;
	}
  
	return pProtocol ;
}
