#include <stdio.h>
#include "CProtocol_ModBusTcp.h"

extern "C"
{
	CProtocol * CreateProtocol(  CMethod * pMethod ) ;
}

CProtocol * CreateProtocol(  CMethod * pMethod )
{
	CProtocol * pProtocol = NULL ;
	pProtocol = new CProtocol_ModBusTcp_Transfer;
	if( pProtocol )
		pProtocol->m_pMethod = pMethod ;
  
	return pProtocol ;
}
