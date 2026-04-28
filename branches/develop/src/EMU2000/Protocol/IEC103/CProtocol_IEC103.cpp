#include "CProtocol_IEC103.h"
#include "IEC103.h"
#include "iec103_simenzi_ym.h"
#include "iec103_nanzi_pst645.h"
#include "IEC103_zhongneng.h"

#define IEC103_SIMENZI	    1
#define IEC103_SIMENZI_YM	2
#define IEC103_NANZI_PST645  3
#define IEC103_NANZI_ZHONGNENG  4 //中能化成项目

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);

CProtocol_IEC103::CProtocol_IEC103()
{
    //ctor
	memset( m_sTemplatePath , 0 , sizeof( m_sTemplatePath ) ) ;

}

CProtocol_IEC103::~CProtocol_IEC103()
{
    //dtor
	int size = m_module.size() ;
	for(  int i = 0 ; i < size ; i++ )
	{
		delete m_module[ i ] ;
	}
	m_module.clear() ;
	printf( "Delete All CProtocol_IEC103 OK . \n" );
}

BOOL CProtocol_IEC103::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg )
{
	return FALSE ;
}

BOOL CProtocol_IEC103::ProcessProtocolBuf( BYTE * buf , int len )
{
	return FALSE ;
}

BOOL CProtocol_IEC103::Init( BYTE byLineNo )
{
	//增加ModBus 采集模块数据
	//通过总线号获取读取的装置文件路径
	m_byLineNo = byLineNo ;
	//读取模板文件
	m_ProtoType = PROTOCO_GATHER ;

	return GetDevData( ) ;
}

BOOL CProtocol_IEC103::GetDevData( )
{
	memset( m_sDevPath , 0 , sizeof( m_sDevPath ) ) ;
	sprintf( m_sDevPath , "%s/IEC103/%s%02d.ini" , SYSDATAPATH , DEVNAME , m_byLineNo + 1 );
	CProfile profile( m_sDevPath ) ;

	return ProcessFileData( profile ) ;
}

BOOL CProtocol_IEC103::ProcessFileData( CProfile &profile )
{
	BOOL bRtn = FALSE;
	if( !profile.IsValid() )
	{
		printf( "Open file %s Failed ! \n " , profile.m_szFileName );
		return FALSE ;
	}

	char sSect[ 200 ] = "DEVNUM" ;
	char sKey[ 20 ][ 100 ]={ "module" , "serialno" , "addr" , "name" , "template" } ;

	WORD wModule = 0 ;
	int  serialno=1 ;
	WORD addr =3 ;
	char sName[ 50 ] = { 0 };
	char stemplate[ 200 ] = { 0 };
	int iNum = 0 ;

	iNum = profile.GetProfileInt( sSect , (char *)"NUM"  , 0 ) ;
	if( iNum == 0 )
	{
		printf( "Get DEVNUM Failed ! \n " );
		return FALSE ;
	}

	for( int i = 0 ; i < iNum ; i++ )
	{
		sprintf( sSect , "%s%03d" ,  "DEV" , i + 1 );

		wModule = profile.GetProfileInt( sSect , sKey[ 0 ] , 0 ) ;
		serialno = profile.GetProfileInt( sSect , sKey[ 1 ] , 0 ) ;
		addr = profile.GetProfileInt( sSect , sKey[ 2 ] , 0 ) ;
		profile.GetProfileString( sSect , sKey[ 3 ] , (char *)"NULL"  , sName , sizeof( sName ) ) ;
		profile.GetProfileString( sSect , sKey[ 4 ] , (char *)"NULL" , stemplate , sizeof( stemplate ) ) ;

		//创建相应模块子类
		bRtn = CreateModule( wModule , serialno , addr , sName , stemplate ) ;
		if ( !bRtn )
		{
			printf ( "Create ModBus Module=%d serialno=%d addr=%d sName=%s stemplate=%s \
					Error \n", wModule, serialno, addr, sName, stemplate );
			return FALSE;
		}
	}

	return TRUE ;
}

BOOL CProtocol_IEC103::CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath )
{
    CProtocol_IEC103 * pProtocol = NULL ;

    switch ( iModule )
    {
        case IEC103_SIMENZI:
            {
                pProtocol = new CIEC103 ;
                pProtocol->m_byLineNo = m_byLineNo ;
                pProtocol->m_wModuleType = iModule ;
                pProtocol->m_wDevAddr = iAddr ;
                pProtocol->m_SerialNo = iSerialNo ;
                strcpy( pProtocol->m_sDevName , sName ) ;
                strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
                m_pMethod->m_pRtuObj = pProtocol;
                pProtocol->m_pMethod = m_pMethod ;
                pProtocol->m_ProtoType = PROTOCO_GATHER ;
                //初始化模板数据
                if( !pProtocol->Init( m_byLineNo ) )
                    return FALSE ;
                printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
            }
            break;

        case IEC103_SIMENZI_YM:
            {
                pProtocol = new Ciec103_simenzi_ym ;
                pProtocol->m_byLineNo = m_byLineNo ;
                pProtocol->m_wModuleType = iModule ;
                pProtocol->m_wDevAddr = iAddr ;
                pProtocol->m_SerialNo = iSerialNo ;
                strcpy( pProtocol->m_sDevName , sName ) ;
                strcpy( pProtocol->m_sTemplatePath , stplatePath ) ;
                m_pMethod->m_pRtuObj = pProtocol;
                pProtocol->m_pMethod = m_pMethod ;
                pProtocol->m_ProtoType = PROTOCO_GATHER ;
                //初始化模板数据
                if( !pProtocol->Init( m_byLineNo ) )
                    return FALSE ;
                printf( " Add bus = %d Addr = %d serialno = %d\n" , m_byLineNo , iAddr, iSerialNo ) ;
            }
            break;
		case IEC103_NANZI_PST645:
		{
			pProtocol = new CIEC103_Nanzi_PST645;
			pProtocol->m_byLineNo = m_byLineNo;
			pProtocol->m_wModuleType = iModule;
			pProtocol->m_wDevAddr = iAddr;
			pProtocol->m_SerialNo = iSerialNo;
			strcpy(pProtocol->m_sDevName, sName);
			strcpy(pProtocol->m_sTemplatePath, stplatePath);
			m_pMethod->m_pRtuObj = pProtocol;
			pProtocol->m_pMethod = m_pMethod;
			pProtocol->m_ProtoType = PROTOCO_GATHER;
			//初始化模板数据
			if (!pProtocol->Init(m_byLineNo))
				return FALSE;
			printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		}
		break;

		case IEC103_NANZI_ZHONGNENG:
		{
			 pProtocol = new CIEC103_ZN;
			 pProtocol->m_byLineNo = m_byLineNo;
		     pProtocol->m_wModuleType = iModule;
		     pProtocol->m_wDevAddr = iAddr;
		     pProtocol->m_SerialNo = iSerialNo;
			 strcpy(pProtocol->m_sDevName, sName);
			 strcpy(pProtocol->m_sTemplatePath, stplatePath);
			 m_pMethod->m_pRtuObj = pProtocol;
			 pProtocol->m_pMethod = m_pMethod;
			 pProtocol->m_ProtoType = PROTOCO_GATHER;
							   //初始化模板数据
			 if (!pProtocol->Init(m_byLineNo))
					 return FALSE;
			  printf(" Add bus = %d Addr = %d serialno = %d\n", m_byLineNo, iAddr, iSerialNo);
		}
			break;
        default:
            {
                printf( "ModBus don't contain this module Failed .\n" );
                return FALSE ;
            }
    }
    m_module.push_back( pProtocol ) ;

    return TRUE ;
}

BYTE CProtocol_IEC103::GetCs( BYTE * pBuf , int len )
{
	BYTE byRtn = 0x00;
	int i;

	if( pBuf == NULL || len <= 0 )
		return byRtn;

	for ( i=0; i<len ; i++ )
	{
		byRtn += pBuf[i];
	}

	return byRtn;
}

BOOL CProtocol_IEC103::BroadCast( BYTE * buf , int &len )
{
	//组织该模块的广播报文
	int index = 0 ;
	buf[ index++ ] = 0xFF ;
	buf[ index++ ] = 0x02 ;
	buf[ index++ ] = 0x03 ;
	buf[ index++ ] = 0x04 ;

	WORD wCRC = GetCs( buf, index );
    buf[ index++ ] = HIBYTE(wCRC);
    buf[ index++ ] = LOBYTE(wCRC);

	len = index ;

	printf( "\n CProtocol_IEC103  TestBroadCast \n " ) ;
	return TRUE ;
}


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_IEC103
 *      Method:  WhetherBufValue
 * Description:  判断103报文的有效性 基本判断 并从缓存中获取一帧正确报文（如果有)
 *       Input:  收到的缓存区buf 收到的数据长度len 缓存区有效位置pos
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CProtocol_IEC103::WhetherBufValue (BYTE *buf, int &len, int &pos )
{
	BYTE *pointer = buf;
	int datalen = 0;
	BYTE byCrc;
	char PrintBuf[256];
	pos = 0;

	if( buf == NULL || len <= 0 )
		return FALSE;

	printf("%02x\n",*pointer);
	while ( len > 0 )
	{
		switch(*pointer)
		{
			case 0xE5:  //ABB 61850-103 单字节错误
				{
					len--;
					pointer++;
					pos ++;
				}
				break;
			case 0x68:  //判断可变帧
				{
					//判断68 x x 68合理性
					if( (*(pointer+3) != *pointer)
							|| (*(pointer+1) != *(pointer+2)))
					{
						len--;
						pointer++;
						pos ++;
						continue;

					}

					//判断数据长度合理性
					datalen=*(pointer+1);
					if( datalen+6>len  )
					{
						len--;
						pointer++;
						pos ++;
						sprintf(PrintBuf, "IEC103 recv len error datalen=%d len=%d", datalen, len );
						OutBusDebug( m_byLineNo, (BYTE *)PrintBuf, strlen(PrintBuf), 2 );
						continue;
					}

					//判断校验位 及最后一个字节0x16
					byCrc=GetCs(pointer+4,datalen);
					if(*(pointer+datalen+4)!=byCrc
							|| *(pointer+datalen+5)!=0x16)
					{
						len--;
						pointer++;
						pos ++;
						sprintf(PrintBuf, "IEC103 recv cs error GetCrc=%d crc=%d or last byte != 0x16 =%x", byCrc, *(pointer+datalen+4), *(pointer+datalen+5));
						OutBusDebug( m_byLineNo, (BYTE *)PrintBuf, strlen(PrintBuf), 2 );
						continue;
					}

					//判断地址
					if ( *(pointer + 5) != m_wDevAddr && *(pointer + 5) != 0xff )
					{
						len--;
						pointer++;
						pos ++;
						sprintf(PrintBuf, "IEC103 recv addr error Getaddr=%d addr=%d ", *(pointer + 5), m_wDevAddr );
						OutBusDebug( m_byLineNo, (BYTE *)PrintBuf, strlen(PrintBuf), 2 );
						continue;
					}

					//获取正确的报文长度
					len = datalen + 6;
					buf = buf + pos;
					return TRUE;
				}
				break;
			case 0x10: //判断固定帧
				{
					//判断校验位 及最后一个字节0x16

				   printf("*********%d %s************\n", __LINE__, __FILE__);
					byCrc=GetCs(pointer+1,2);
					if( *(pointer+3)!=byCrc || *(pointer+4)!=0x16 )
					{
						len--;
						pointer++;
						pos ++;
						sprintf(PrintBuf, "IEC103 recv cs error GetCrc=%d crc=%d or last byte != 0x16 =%x", byCrc, *(pointer+3), *(pointer+4));
						OutBusDebug( m_byLineNo, (BYTE *)PrintBuf, strlen(PrintBuf), 2 );
						continue;
					}

					//判断地址
					if ( *(pointer+2) != m_wDevAddr )
					{

						printf("*********%d %s************\n", __LINE__, __FILE__);
						len--;
						pointer++;
						pos ++;
						sprintf(PrintBuf, "IEC103 recv addr error Getaddr=%d addr=%d ", *(pointer+2), m_wDevAddr );
						OutBusDebug( m_byLineNo, (BYTE *)PrintBuf, strlen(PrintBuf), 2 );
						continue;
					}
					buf = buf + pos;
				}
				return TRUE;
				break;
			default:
				{
					len--;
					pointer++;
					pos ++;
					continue;
				}
				break;
		}
	}

	return FALSE;
}		/* -----  end of method CProtocol_IEC103::WhetherBufValue  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CProtocol_IEC103
 *      Method:  ChangeFcb
 * Description:
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BYTE CProtocol_IEC103::ChangeFcb ( BYTE byCtlBit, BOOL &bFCB )
{
	if( bFCB )
		byCtlBit &= IEC103_FCB_0;
	else
		byCtlBit |= IEC103_FCB_1;

	bFCB ^= 1;

	return byCtlBit;
}		/* -----  end of method CProtocol_IEC103::ChangeFcb  ----- */
