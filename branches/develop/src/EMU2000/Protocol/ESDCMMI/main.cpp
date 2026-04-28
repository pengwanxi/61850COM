#include <stdio.h>
#include "CProtocol_ESDCMMI.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_ESDCMMI ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "ESDCMMI DLL  OK.\n " ) ;
	}
	
	return pProtocol ;	
}
