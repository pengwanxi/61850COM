// Protocol_ModBusSlave.cpp: implementation of the CProtocol_ModBusSlave class.
//
//////////////////////////////////////////////////////////////////////

#include "Protocol_ModBusSlave.h"
#include "Protocol_ESD_ModBusSlave.h"
#include "Protocol_SultanModBusSlave.h"

#define  MODULE_ESD_MODBUS_SLAVE			1 //ESD��׼ģ��
#define  MODULE_SULTAN_MODBUS_SLAVE		2 //������յ�ģ��

CProtocol_ModBusSlave::CProtocol_ModBusSlave()
{

}

CProtocol_ModBusSlave::~CProtocol_ModBusSlave()
{

}

BOOL CProtocol_ModBusSlave::Init( BYTE byLineNo )
{
	m_byLineNo = byLineNo ;
	m_ProtoType = PROTOCO_TRANSPROT ;
	//��ȡģ���ļ�
	return GetDevData( ) ;
}

BOOL CProtocol_ModBusSlave::GetDevData( )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/ModBusSlave/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}

BOOL CProtocol_ModBusSlave::ProcessFileData( CProfile &profile )
{
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 50 ]={ "module" , "addr" , "name" , "masteraddr" , "template" , "ycdead" , "ycProperty" , "timing"} ;

	BOOL bRtn;
	WORD wModule = 0 ;//ģ���ʶ

	WORD addr =0 ;//װ�õ�ַ
	char sName[ 50 ] = { 0 };//ģ������
	char stemplate[ 200 ] = { 0 };//ģ��·��
	int iNum = 0 ;//վ����
	char sMasterAddr[ 200 ] = { 0 } ;//��վIP��ַ�Ͷ˿�

	iNum = profile.GetProfileInt( sSect , (char *)"NUM"  , 0 ) ;
	if( iNum == 0 )
	{
		printf( "Get DEVNUM Failed ! \n " );
		return FALSE ;
	}

	BYTE byIndex = 0 ;
	for( int i = 0 ; i < iNum ; i++ )
	{
		sprintf( sSect , "%s%03d" ,  "DEV" , i + 1 );

		wModule = profile.GetProfileInt( sSect , sKey[ byIndex++ ] , 0 ) ;
		addr = profile.GetProfileInt( sSect , sKey[ byIndex++ ] , 0 ) ;

		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL"  , sName , sizeof( sName ) ) ;
		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL" , sMasterAddr , sizeof( sMasterAddr ) ) ;
		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL" , stemplate , sizeof( stemplate ) ) ;

		//������Ӧģ������
		bRtn = CreateModule( wModule ,sMasterAddr , addr , sName , stemplate ) ;
		if ( !bRtn )
		{
			printf ( "Create ModBusSlave Module=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule,  addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}

BOOL CProtocol_ModBusSlave::CreateModule( int iModule , char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath )
{
	CProtocol * pProtocol = NULL ;

	switch ( iModule )
	{
	case MODULE_ESD_MODBUS_SLAVE:
		pProtocol = new CProtocol_ESD_ModBusSlave ;
		break;
	case MODULE_SULTAN_MODBUS_SLAVE:
		pProtocol = new CProtocol_SultanModBusSlave ;
		break;
	default:
		{
			printf( "%s don't contain this module Failed .\n" , "ModBusSlave"  );
			return FALSE ;
		}
	}

	if( !InitMS_Module( pProtocol , iModule , sMasterAddr , iAddr , sName , stplatePath ) )
			return FALSE ;

	m_module.push_back( pProtocol ) ;
	return TRUE ;
}

BOOL CProtocol_ModBusSlave::InitMS_Module( CProtocol* pProtocol , int iModule , char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath )
{
	if( pProtocol == NULL )
		return FALSE ;

	pProtocol->m_byLineNo = m_byLineNo ;
	pProtocol->m_wModuleType = iModule ;
	pProtocol->m_wDevAddr = iAddr ;
	printf("Addr=%d m_byLineNo=%d\n",  iAddr, m_byLineNo);
	strcpy( pProtocol->m_sDevName , sName ) ;
	strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
	//strcpy( pProtocol->m_sMasterAddr , sMasterAddr ) ;
	m_pMethod->m_pRtuObj = pProtocol;
	pProtocol->m_pMethod = m_pMethod ;
	pProtocol->m_ProtoType = PROTOCO_TRANSPROT ;
	//��ʼ��ģ������
	if( !pProtocol->Init( m_byLineNo ) )
				return FALSE ;
	printf( " Add bus = %d Addr = %d ProtocolName = %s \n" , m_byLineNo , iAddr , sName ) ;

	return TRUE ;
}

WORD CProtocol_ModBusSlave::GetCRC(BYTE *pBuf,WORD nLen)
{
	WORD Genpoly=0xA001;
	WORD CRC=0xFFFF;
	WORD index;
	while(nLen--)
	{
		CRC=CRC^(WORD)*pBuf++;
		for(index=0;index<8;index++)
		{
			if((CRC & 0x0001)==1)
				CRC=(CRC>>1)^Genpoly;
			else
				CRC=CRC>>1;
		}
	}
	return CRC;
}

