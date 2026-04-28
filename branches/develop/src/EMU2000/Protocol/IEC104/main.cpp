#include <stdio.h>
#include "CProtocol_IEC104.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_IEC104 ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		//printf( "IEC104 DLL  OK.\n " ) ;
	}

	return pProtocol ;
}
