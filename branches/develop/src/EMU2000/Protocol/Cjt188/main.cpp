
#include <stdio.h>
#include "CProtocol_Cjt188.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_Cjt188 ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "CJ/T188  OK.\n " ) ;
	}
  
	return pProtocol ;
}
