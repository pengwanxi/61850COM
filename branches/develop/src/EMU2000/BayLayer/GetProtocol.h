// GetProtocol.h: interface for the CGetProtocol class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GETPROTOCOL_H__FCF96365_0A8A_48D7_87A9_29895F3546B7__INCLUDED_)
#define AFX_GETPROTOCOL_H__FCF96365_0A8A_48D7_87A9_29895F3546B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../share/CProtocol.h"
#include <vector>
#include "../share/CMethod.h"

//##ModelId=53CDE1B10002
class CGetProtocol
{
	public:
		//##ModelId=53CDE1B10003
		CGetProtocol();
		//##ModelId=53CDE1B10004
		virtual ~CGetProtocol();
		//##ModelId=53CDE1B10006
		CProtocol * GetProtoObj( char * pDllPath, CMethod *pMethod ) ;

	protected:
		//##ModelId=53CDE1B10014
		std::vector< void * > m_dllhandle ;
		//##ModelId=53CDE1B1001F
		void * OpenDll( char * pDllPath ) ;
		//##ModelId=53CDE1B10021
		void AddHandle( void * pHandle ) ;
		//##ModelId=53CDE1B10023
		void ReleaseHandle(  ) ;
		BOOL ModifyPath( ) ;

};

#endif // !defined(AFX_GETPROTOCOL_H__FCF96365_0A8A_48D7_87A9_29895F3546B7__INCLUDED_)

