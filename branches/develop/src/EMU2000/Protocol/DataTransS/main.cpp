
#include <stdio.h>
#include "CProtocol_DataTrans.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_DataTrans ;
	if( pProtocol )
	{
		pProtocol->m_pMethod = pMethod ;
		printf( "libDataTrans.so  OK.\n " ) ;
	}

	return pProtocol ;
}
