
#include <stdio.h>
#include "CProtocol_LFP.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_LFP ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		//printf( "LFP OK.\n " ) ;
	}
  
	return pProtocol ;
}
