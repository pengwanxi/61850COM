
#include <stdio.h>
#include "CProtocol_IEC615.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_IEC615 ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "IEC615 DLL OK.\n " ) ;
	}
  
	return pProtocol ;
}
