
#include <stdio.h>
#include "CProtocol_IEC103.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}



CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_IEC103 ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "IEC103 DLL  OK.\n " ) ;
	}
  
	return pProtocol ;
}
