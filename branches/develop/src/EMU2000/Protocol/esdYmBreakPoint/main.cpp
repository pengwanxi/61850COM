
#include <stdio.h>
#include "Protocol_esdYmBreakPoint.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_esdYmBreakPoint;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "esdYmBreakPoint DLL  OK.\n " ) ;
	}

	return pProtocol ;
}
