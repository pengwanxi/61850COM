#include "CProtocol_IEC101S.h"
#include "../../share/CProtocol.h"
#include "IEC101S_2002.h"
#include "IEC101S_1997.h"
#include "string.h"
#include <assert.h>


extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

CProtocol_IEC101S::CProtocol_IEC101S()
{
	memset( m_sMasterAddr, 0, sizeof( m_sMasterAddr ) );
	memset( m_szPrintBuf, 0, sizeof( m_szPrintBuf ) );
    //ctor
}

CProtocol_IEC101S::~CProtocol_IEC101S()
{
    //dtor
}

BOOL CProtocol_IEC101S::Init( BYTE byLineNo )
{
	m_byLineNo = byLineNo ;
	m_ProtoType = PROTOCO_TRANSPROT ;
	//��ȡģ���ļ�

	return GetDevData( ) ;
}

BOOL CProtocol_IEC101S::GetDevData( )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/IEC101Slave/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}

BOOL CProtocol_IEC101S::ProcessFileData( CProfile &profile )
{
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 50 ]={ "module" , "addr" , "name" , "masteraddr" , "template" , "ycdead" , "ycProperty" , "timing"} ;


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
		BOOL bRtn;
		WORD wModule = 0 ;//ģ���ʶ
		WORD addr =0 ;//װ�õ�ַ
		sprintf( sSect , "%s%03d" ,  "DEV" , i + 1 );

		wModule = profile.GetProfileInt( sSect , sKey[ byIndex++ ] , 0 ) ;
		addr = profile.GetProfileInt( sSect , sKey[ byIndex++ ] , 0 ) ;

		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL"  , sName , sizeof( sName ) ) ;
		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL" , sMasterAddr , sizeof( stemplate ) ) ;
		profile.GetProfileString( sSect , sKey[ byIndex++ ] , (char *)"NULL" , stemplate , sizeof( stemplate ) ) ;

		//������Ӧģ������
		bRtn = CreateModule( wModule ,sMasterAddr , addr , sName , stemplate ) ;
		if ( !bRtn )
		{
			printf ( "Create IEC101S Module=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule,  addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}

BOOL CProtocol_IEC101S::CreateModule( int iModule , char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath )
{
	CProtocol_IEC101S * pProtocol = NULL ;

	switch ( iModule )
	{
	case MODULE_IEC101S_2002:
		{
			pProtocol = new CIEC101S_2002;
			if( !InitIEC101S_Module( pProtocol , iModule , sMasterAddr , iAddr , sName , stplatePath ) )
				return FALSE ;
		}
		break;
	case MODULE_IEC101S_1997:
		{
			pProtocol = new CIEC101S_1997;
			if( !InitIEC101S_Module( pProtocol , iModule , sMasterAddr , iAddr , sName , stplatePath ) )
				return FALSE ;
		}
		break;
	default:
		{
			printf( "%s don't contain this module Failed .\n" , "IEC101S"  );
			return FALSE ;
		}
	}

	m_module.push_back( pProtocol ) ;
	return TRUE ;
}

BOOL CProtocol_IEC101S::InitIEC101S_Module( CProtocol_IEC101S * pProtocol , int iModule , char * sMasterAddr , WORD iAddr , char * sName , char * stplatePath )
{
	if( pProtocol == NULL )
		return FALSE ;

	pProtocol->m_byLineNo = m_byLineNo ;
	pProtocol->m_wModuleType = iModule ;
	pProtocol->m_wDevAddr = iAddr ;
	printf("iddr=%d m_byLineNo=%d\n",  iAddr, m_byLineNo);
	strcpy( pProtocol->m_sDevName , sName ) ;
	strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
	strcpy( pProtocol->m_sMasterAddr , sMasterAddr ) ;
	m_pMethod->m_pRtuObj = pProtocol;
	pProtocol->m_pMethod = m_pMethod ;
	pProtocol->m_ProtoType = PROTOCO_TRANSPROT ;
	//��ʼ��ģ������
	if( !pProtocol->Init( m_byLineNo ) )
				return FALSE ;
			printf( " Add bus = %d Addr = %d ProtocolName = %s \n" , m_byLineNo , iAddr , sName ) ;

	return TRUE ;
}


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_IEC101S
 *      Method:  print
 * Description:  �ڲ���ӡ����
 *       Input:	 ������ ����
 *		Return:	 void
 *--------------------------------------------------------------------------------------
 */
void CProtocol_IEC101S::print ( const char *szBuf, int len  )
{
#ifdef  IEC101S_PRINT
	printf ( "%s\n" , szBuf);
#endif     /* -----  not IEC101SPRINT  ----- */


#ifdef  IEC101S_DEBUG
	OutBusDebug( m_byLineNo, (BYTE *)szBuf, strlen(szBuf), 2 );
#endif     /* -----  not IEC101S_DEBUG  ----- */
}		/* -----  end of method CProtocol_IEC101S::print  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_IEC101S
 *      Method:  GetCs
 * Description:  ��У�� 
 *       Input:	 ������ ���������� ���л���������Ϊ�� ���Ȳ���Ϊ0
 *		Return:	 ����У��� BYTE
 *--------------------------------------------------------------------------------------
 */

BYTE CProtocol_IEC101S::GetCs ( const BYTE *pBuf, int len )
{
	BYTE byRtn = 0x00;
	assert( pBuf != NULL );
	assert( len > 0 );

	for ( int i=0; i<len ; i++ )
	{
		byRtn += pBuf[i];
	}

	return byRtn;
}		/* -----  end of method CProtocol_IEC101S::GetCs  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_IEC101S
 *      Method:  ProcessJudgeFlag
 * Description:  �жϿ����ֵı�ʶλ
 *       Input:  ������
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_IEC101S::ProcessJudgeFlag ( BYTE c )
{
	//�ж�FCV ֡������Чλ
	if( c & 0x10 )
	{
		BOOL bFcb = ( c & 0x20 ) >> 5;
		// sprintf( m_szPrintBuf, "bFcb=%d m_bFcb=%d", bFcb, m_bFcb );
		// print( m_szPrintBuf );
		//�ж�FCB -֡����λ	
		if ( bFcb != m_bFcb)
		{
			m_SendStatus = RESEND;
			return FALSE;
		}
		else
		{
			m_bFcb ^= 1;
		}
	}

	return TRUE;
}		/* -----  end of method CProtocol_IEC101S::ProcessJudgeFlag  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_IEC101S
 *      Method:  WhetherBufValid
 * Description:  �жϱ����Ƿ���� �����ж�
 *       Input:	 ������ ���� λ��
 *		Return:	 BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_IEC101S::WhetherBufValid ( const BYTE *buf, int &len )
{
	const BYTE *pointer = buf;
	int datalen;
	BYTE byCs;
	int pos = 0;

	if ( buf == NULL || len <= 0 )
	{
		sprintf( m_szPrintBuf, "IEC101S WhetherBufValue err buf==NULL or len = %d" , len);
		print( m_szPrintBuf );
		return FALSE;
	}
	
	while ( len > 0 )
	{
		switch(*pointer)
		{
			case 0x68:  //�жϿɱ�֡
				{
					//�ж�68 x x 68������
					if( (*(pointer+3) != *pointer)
							|| (*(pointer+1) != *(pointer+2)))
					{
						goto DEFAULT;
					}

					//�ж����ݳ��Ⱥ�����
					datalen=*(pointer+1);
					if( datalen+6>len  )
					{
						sprintf(m_szPrintBuf, "IEC101S recv len error datalen=%d len=%d", datalen, len );
						print( m_szPrintBuf );
						goto DEFAULT;
					}

					//�ж�У��λ �����һ���ֽ�0x16
					byCs=GetCs(pointer+4,datalen);
					if(*(pointer+datalen+4)!=byCs
							|| *(pointer+datalen+5)!=0x16)
					{
						sprintf(m_szPrintBuf, "IEC101S recv cs error GetCs=%d cs=%d or last byte != 0x16 =%x", byCs, *(pointer+datalen+4), *(pointer+datalen+5));
						print( m_szPrintBuf );
						goto DEFAULT;
					}

					//�жϵ�ַ
					if ( *(pointer + 5) != m_wDevAddr && *(pointer + 5) != 0xff )
					{
						sprintf(m_szPrintBuf, "IEC101S recv addr error Getaddr=%d addr=%d ", *(pointer + 5), m_wDevAddr );
						print( m_szPrintBuf );
						goto DEFAULT;
					}

					//�жϿ�����PRMλ 
					if ( 0 == (*(pointer + 4) & 0x40) )
					{
						sprintf(m_szPrintBuf, "IEC101S recv PRM error = %d  ", *(pointer + 4) & 0x40 );
						print( m_szPrintBuf );
						goto DEFAULT;
					}

					//��ȡ��ȷ�ı��ĳ���
					len = datalen + 6;
					buf = buf + pos;
					return TRUE;
				}
				break;
			case 0x10: //�жϹ̶�֡
				{
					//�ж�У��λ �����һ���ֽ�0x16
					byCs=GetCs(pointer+1,2);
					if( *(pointer+3)!=byCs || *(pointer+4)!=0x16 )
					{
						sprintf(m_szPrintBuf, "IEC101S recv cs error GetCs=%d cs=%d or last byte != 0x16 =%x", byCs, *(pointer+datalen+3), *(pointer+datalen+4));
						print( m_szPrintBuf );
						goto DEFAULT;
					}

					//�жϵ�ַ
					if ( *(pointer+2) != m_wDevAddr )
					{
						sprintf(m_szPrintBuf, "IEC101S recv addr error Getaddr=%d addr=%d ", *(pointer+2), m_wDevAddr );
						print( m_szPrintBuf );
						goto DEFAULT;
					}

					buf = buf + pos;
					len = 5;
					return TRUE;
				}
				break;
			default:
				{
					goto DEFAULT;
				}
				break;
		}
DEFAULT:
		len--;
		pointer++;
		pos ++;

	}

	return FALSE;
}		/* -----  end of method CProtocol_IEC101S::WhetherBufValid  ----- */

