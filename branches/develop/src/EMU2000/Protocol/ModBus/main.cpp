
#include <stdio.h>
#include "CProtocol_ModBus.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_ModBus ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		//printf( "ModBus DLL  OK.\n " ) ;
	}
  
	return pProtocol ;
}
