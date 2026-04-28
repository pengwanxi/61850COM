// BusManger.h: interface for the CBusManger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUSMANGER_H__8FD47E1A_DCA2_4AB0_8996_B31E7B86EC02__INCLUDED_)
#define AFX_BUSMANGER_H__8FD47E1A_DCA2_4AB0_8996_B31E7B86EC02__INCLUDED_

#include "main.h"
#include "GetProtocol.h"
#include "../share/Ftp_FaultRecorder.h"

class CBusManger
{
	public:
		CBusManger( );
		virtual ~CBusManger( );
		BOOL AddBus( CProtocol * pProtocol , CBasePort * pBasePort , WORD wInterval , BYTE byMsgType , 
				CMethod * pMethod , char *pNetCard, char *pRemoteIp, DWORD dwPortNum);
		CGetProtocol *m_GetProtocol ;
	public:
		BUSARRAY m_sbus ;			//每一枚元素都是包含一枚总线基本信息的结构体! PAUSE也会被保存!
		char m_sysDns[20];
		CBusDebug m_Debug;
		BOOL RemoveBus( ) ;
		BOOL AddPauseBus( ) ;
		BOOL AddClientPort( CBasePort * pPortTcpClient ) ;
		BOOL GetClientSize( ) ;
		vector<CBasePort*>*GetSanServerVector( ) ;
		BOOL SetNetCardParam( NETWORKPARAM *pNetParam ) ;
		BOOL GetNetCardParam( char * strNetCardName , NETWORKPARAM *pNetParam ) ;
		BOOL EnableNetCardParam( );
		BOOL setFaultRecorderCfg(UINT port, char * pType);
		BOOL GetFaultRecorderSteupFlag();
		BOOL setupFaultRecorder();
		BOOL ChangeNetMask(char *ip, char *netmask);
private:
		vector<CBasePort*> m_VectorTcpClient;
		NETWORKPARAM_ARRAY  m_NetParamArray ;
		CFtp_FaultRecorder m_faultRecorderFtp;
		BOOL m_bfaultRecorder;
		
};

#endif // !defined(AFX_BUSMANGER_H__8FD47E1A_DCA2_4AB0_8996_B31E7B86EC02__INCLUDED_)

