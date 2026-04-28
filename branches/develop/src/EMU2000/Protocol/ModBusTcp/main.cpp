#include <stdio.h>
#include "CProtocol_ModBusTcp.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_ModBusTcp ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "ModBusTcp DLL  OK.\n " ) ;
	}
	
	return pProtocol ;	
}
