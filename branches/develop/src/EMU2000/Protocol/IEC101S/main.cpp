
#include <stdio.h>
#include "CProtocol_IEC101S.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_IEC101S ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "IEC101Slave DLL  OK.\n " ) ;
	}

	return pProtocol ;
}
