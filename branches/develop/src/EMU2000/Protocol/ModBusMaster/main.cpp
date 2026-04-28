
#include <stdio.h>
#include "CProtocol_ModBusMaster.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_ModBusMaster ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "ModBusMaster DLL  OK.\n " ) ;
	}
	
	return pProtocol ;
}
