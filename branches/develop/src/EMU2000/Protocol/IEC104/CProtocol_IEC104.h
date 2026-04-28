#ifndef CPROTOCOL_IEC104_H
#define CPROTOCOL_IEC104_H

#include "../../share/Rtu.h"
#include "../../share/CMethod.h"


#define	IEC104PREFIXFILENAME			"/mynand/config/IEC104Slave/"	/* 104固定路径 */
#define MODULE_ESD_IEC104		1  //公司内部IEC104   
#define MODULE_IEC104				2  //标准IEC104

class CProtocol_IEC104 : public CRtuBase
{
	public:
		CProtocol_IEC104();
		virtual ~CProtocol_IEC104();
		BOOL GetDevData( );
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule ,char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath ) ;
		BOOL InitIEC104_Module( CProtocol_IEC104 * pProtocol , int iModule , char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath );
		virtual BOOL Init( BYTE byLineNo );

	protected:
		char m_sMasterAddr[ 200 ] ;//网络参数保存
	private:
};

#endif // CPROTOCOL_IEC104_H
