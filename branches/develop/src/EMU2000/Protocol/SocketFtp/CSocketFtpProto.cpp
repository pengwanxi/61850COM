/*
 * =====================================================================================
 *
 *       Filename:  CSocketFtpProto.cpp
 *
 *    Description:  修改 ProcessDownData下 
		if( 0 ==  m_FtpFile.WriteFile( m_chFileName, buf+1, len-1 ))
		{
			//不返回错误 可能是空文件		mengqp		2015-11-05 10:53:49
			// SetErrorBit(  );
		}
 *
 *        Version:  1.1
 *        Created:  2015年09月24日 11时55分49秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:
 *		  
 *    Description:  创建
 *        Version:  1.0
 *        Created:  2015年09月24日 11时55分49秒
 *         Author:  mengqp 
 * =====================================================================================
 */
#include <stdio.h>
#include "CSocketFtpProto.h"
#include "../../share/global.h"

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  CSocketFtpProto
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CSocketFtpProto::CSocketFtpProto ()
{
	//初始化
	Init(  );
}  /* -----  end of method CSocketFtpProto::CSocketFtpProto  (constructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  ~CSocketFtpProto
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CSocketFtpProto::~CSocketFtpProto ()
{
}  /* -----  end of method CSocketFtpProto::~CSocketFtpProto  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  SetFileName
 * Description:  设置文件名字 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::SetFileName ( const char *pchFileName )
{
	if( NULL == pchFileName )
	{
		memset( m_chFileName, 0, 256 );
	}
	else
	{
		strcpy( m_chFileName, pchFileName );
		// memcpy( m_chFileName, pchFileName, strlen( pchFileName ) );
	}

}		/* -----  end of method CSocketFtpProto::SetFileName  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  SetErrorBit
 * Description:  设置错误位 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::SetErrorBit ( void )
{
	m_bySendBuf[3] |= 0x80;
}		/* -----  end of method CSocketFtpProto::SetErrorBit  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  SetResponseBit
 * Description:  设置回复位
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::SetResponseBit ( void )
{
	m_bySendBuf[3] |= 0x40;
}		/* -----  end of method CSocketFtpProto::SetResponseBit  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  SetFileBit
 * Description:  设置文件传送位 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::SetFileBit ( void )
{
	m_bySendBuf[3] |= 0x20;
}		/* -----  end of method CSocketFtpProto::SetFileBit  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  isFile
 * Description:  是否是文件装态 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::isFileState ( void ) const
{
	return ( m_bySendBuf[3] & 0x20 ) > 0;
}		/* -----  end of method CSocketFtpProto::isFile  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  SetDownLoadBit
 * Description:  设置上传下载位
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::SetDownLoadBit ( void )
{
	m_bySendBuf[3] |= 0x10;
}		/* -----  end of method CSocketFtpProto::SetDownLoadBit  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  IsLoad
 * Description:  是否是上传装态 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::IsLoad ( void ) const
{
	return ( m_bySendBuf[3] & 0x10 ) > 0 ;
}		/* -----  end of method CSocketFtpProto::IsLoad  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  SetFuncBit
 * Description:  设置功能位 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::SetFuncCode ( BYTE byFunc )
{
	m_bySendBuf[3] &= 0xf0;
	m_bySendBuf[3] |= ( byFunc & 0x0f );
}		/* -----  end of method CSocketFtpProto::SetFuncBit  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  GetFuncBit
 * Description:  获得功能码 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BYTE CSocketFtpProto::GetFuncCode ( void ) const
{
	return ( m_bySendBuf[3] & 0x0f  );
}		/* -----  end of method CSocketFtpProto::GetFuncBit  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  AddFrameHead
 * Description:  添加报文头
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CSocketFtpProto::AddFrameHead ( void )
{
	m_bySendBuf[0] = 0x68;
	m_bySendBuf[1] = HIBYTE( m_wSendLen );
	m_bySendBuf[2] = LOBYTE( m_wSendLen );
	m_wSendLen += 3;
}		/* -----  end of method CSocketFtpProto::AddFrameHead  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  IsFrameFormat
 * Description:  是否符合帧格式  判断报文格式时调用
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::IsFrameFormat ( BYTE *buf, int len )
{
	//判断数据长度
	if ( len > FTP_MAX_SEND_LEN || len < 4 )
	{
		return FALSE;
	}

	//判断报文头
	if( 0x68 != buf[0] )
	{
		return FALSE;
	}

	//判断报文长度
	if( MAKEWORD( buf[2], buf[1] ) != len - 3 )
	{
		return FALSE;
	}

	//判断是否是发送帧
	if( 0 != (buf[3] & 0x40) )
	{
		return FALSE;
	}

	return TRUE;
}		/* -----  end of method CSocketFtpProto::IsFrameFormat  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  ProcessStateByte
 * Description:  处理装态位 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::ProcessStateByte ( BYTE byState )
{
	if( 0 != ( 0x80 & byState ) )
	{
		//关闭socket
		return FALSE;
	}

	if( 0 != ( 0x20 & byState ) )
	{
		SetFileBit(  );
	}

	if( 0 != ( 0x10 & byState) )
	{
		SetDownLoadBit(  );
	}

	SetResponseBit(  );

	return TRUE;
}		/* -----  end of method CSocketFtpProto::ProcessStateByte  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD5_BeginTrans
 * Description:  处理数据类型5开始传输
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD_BeginTrans ( void )
{
	const BYTE byFuncPos = 3;
	SetFuncCode( START_TRANS );
	m_wSendLen = 1;

	m_bySendBuf[ byFuncPos + m_wSendLen++ ] = HIBYTE( HIWORD( EMU2000_VERSION ) );
	m_bySendBuf[ byFuncPos + m_wSendLen++ ] = LOBYTE( HIWORD( EMU2000_VERSION ) );
	m_bySendBuf[ byFuncPos + m_wSendLen++ ] = HIBYTE( LOWORD( EMU2000_VERSION ) );
	m_bySendBuf[ byFuncPos + m_wSendLen++ ] = LOBYTE( LOWORD( EMU2000_VERSION ) );

	AddFrameHead(  );
	return TRUE;
}		/* -----  end of method CSocketFtpProto::PLD5_BeginTrans  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD1_FileListInfo
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD_FileListInfo ( void )
{
	const BYTE byFuncPos = 3;
	char* pchFilesListPath  = (char *)"/mynand/config";

	m_FtpFile.DeleteDirFiles( FTP_FILE_LIST );
	m_FtpFile.WriteFileListFile( pchFilesListPath, FTP_FILE_LIST );
	m_dwFileSize = m_FtpFile.GetFileSize( FTP_FILE_LIST );
	SetFileName( FTP_FILE_LIST );
	m_wFileNum = 0;

	SetFuncCode( FILE_LIST_INFO );
	m_wSendLen = 1;
	m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize >>24) & 0xff;
	m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize >>16) & 0xff;
	m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize >>8) & 0xff;
	m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize ) & 0xff;

	AddFrameHead(  );

	return TRUE;
}		/* -----  end of method CSocketFtpProto::PLD1_FileListInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD3_File
 * Description:  处理上传数据3 文件
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD_File ( void )
{
	m_wSendLen = 1;
	DWORD dwReadLen = m_FtpFile.ReadFile( m_chFileName, 
			m_bySendBuf+4, 
			FTP_MAX_SEND_LEN - 4, 
			m_uiReadPos );

	m_wSendLen += dwReadLen;
	SetFileBit(  );
	SetFuncCode( NONE_FUNC );

	AddFrameHead(  );

	return TRUE;
}		/* -----  end of method CSocketFtpProto::PLD3_File  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD_MD5
 * Description:  获得md5
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD_MD5 ( void )
{
	m_uiReadPos = 0;
	SetFuncCode( MD5 );
	if( NULL !=  GetFileMD5( m_chFileName, m_bySendBuf+ 4) )
	{
		m_wSendLen += 16;
		AddFrameHead(  );
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CSocketFtpProto::PLD_MD5  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD_FileInfo
 * Description:  处理上传数据文件信息
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD_FileInfo ( void )
{
	m_wFileNum ++;
	m_wSendLen = 1;

	char chFileName[256];
	if( NULL != m_FtpFile.GetFileLineBuf( FTP_FILE_LIST, m_wFileNum, chFileName ) )
	{
		if( '\n' == chFileName[0]
				|| 0 == chFileName[0])
		{
			return FALSE;
		}

		chFileName[strlen(chFileName)-1] = 0;
		m_dwFileSize = m_FtpFile.GetFileSize( chFileName );
		SetFileName( chFileName );

		if( m_dwFileSize != 0xffffffff )
		{
			const BYTE byFuncPos = 3;
			m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize >>24) & 0xff;
			m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize >>16) & 0xff;
			m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize >>8) & 0xff;
			m_bySendBuf[byFuncPos+m_wSendLen++] = ( m_dwFileSize ) & 0xff;

			memcpy( m_bySendBuf+m_wSendLen+3, chFileName, strlen( chFileName ) );
			m_wSendLen += strlen( chFileName );
			SetFuncCode( FILE_INFO );
			return TRUE;
		}
		else
		{
			SetErrorBit(  );
			return TRUE;
		}
	}

	return FALSE;
}		/* -----  end of method CSocketFtpProto::PLD_FileInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD6
 * Description:  处理上传数据6
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD6 (  )
{
	
	if( PLD_FileInfo(  ) )
	{
	}
	else
	{
		m_wSendLen =1;
		SetFuncCode( END_TRANS );
	}

	AddFrameHead(  );
	return TRUE;
}		/* -----  end of method CSocketFtpProto::PLD6  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PLD15
 * Description:  处理上传数据15 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PLD15 ( void )
{
	m_wSendLen = 1;

	if( m_uiReadPos >= m_dwFileSize )
	{
		PLD_MD5(  );
	}
	else
	{
		PLD_File(  );
	}


	return TRUE;
}		/* -----  end of method CSocketFtpProto::PLD15  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  ProcessLoadData
 * Description:  处理上传报文 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::ProcessLoadData ( BYTE *buf, int len )
{
	//PLD 为ProcessLoadData
	BOOL bRtn = TRUE;
	switch ( GetFuncCode(  ) )
	{
		case START_TRANS:	
			PLD_FileListInfo(  );
			break;

		case END_TRANS:	
			SetFuncCode( START_DOWN );
			m_wSendLen = 1;
			m_FtpFile.DeleteDirFiles( FTP_FILE_LIST );
			AddFrameHead(  );
			break;

		case FILE_LIST_INFO:	
		case FILE_INFO:
			bRtn = PLD_File(  );
			break;

		case START_DOWN:	//处理数据类型数据5 开始传输	
			PLD_BeginTrans(  );
			break;

		case MD5:	
			bRtn = PLD6(  );
			break;

		case RECV_CONFIG:	
			bRtn = PLD15(  );
			break;

		case REBOOT:	
			m_wSendLen = 1;
			m_FtpFile.DeleteDirFiles( FTP_FILE_LIST );
			SetFuncCode( REBOOT );
			AddFrameHead(  );
			m_bReboot = 1;
			break;


		default:	
			printf ( "defalut\n" );
			m_wSendLen = 0;
			bRtn = FALSE;
			return FALSE;
			break;
	}				/* -----  end switch  ----- */

	return bRtn;
}		/* -----  end of method CSocketFtpProto::ProcessLoadData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PDD_endTrans
 * Description:  处理下载数据 结束传输 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PDD_endTrans ( void )
{
	m_wSendLen = 1;
	m_FtpFile.DeleteDirFiles( FTP_FILE_LIST );
	sync(  );
	system( "sync" );
	if( FTP_PRGM_TYPE == m_FileType )
	{
		m_FtpFile.UpdatePrgm(  );	
		system( "reboot" );
	}

	AddFrameHead(  );
	return TRUE;
}		/* -----  end of method CSocketFtpProto::PDD_endTrans  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PDD_FileListInfo
 * Description:  处理下载数据文件列表
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PDD_FileListInfo ( void )
{
	if( m_FtpFile.IsFileExist(  FTP_FILE_LIST) )
	{
		m_FtpFile.DeleteDirFiles( FTP_FILE_LIST );	
	}

	m_FileType = FTP_NONE_TYPE;
	m_wFileNum = 0;
	SetFileName( FTP_FILE_LIST);
	m_wSendLen = 1;

	AddFrameHead(  );
	return TRUE;
}		/* -----  end of method CSocketFtpProto::PDD_FileListInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PDD_FileInfo
 * Description:  处理下载数据文件
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PDD_FileInfo ( void )
{
	m_wSendLen = 1;
	if( NULL == m_FtpFile.GetDownFileName( m_FileType, m_chFileName ) )
	{
		SetErrorBit(  )	;
	}

	AddFrameHead(  );
	return TRUE;
}		/* -----  end of method CSocketFtpProto::PDD_FileInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  PDD6
 * Description:  处理下载数据6 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::PDD6 ( const char *chMd5Buf )
{
	m_wSendLen = 1;
	if( !IsFileMD5Right( m_chFileName, (char *)chMd5Buf ) )
	{
		SetErrorBit(  );	
		AddFrameHead(  );
		return TRUE;
	}

	char chLineBuf[256];

	//文件列表时删除相应文件
	if( 0 == strcmp( m_chFileName, FTP_FILE_LIST ) 
			&& FTP_NONE_TYPE == m_FileType)
	{
		char chTmp[256] = "";
		m_FileType = m_FtpFile.GetDownType( m_chFileName );	
		if( FTP_CFG_TYPE == m_FileType )
		{
			sprintf( chTmp, "%s%s", FTP_DOWN_CFG_FILE, "config/" );
			m_FtpFile.DeleteDirFiles( chTmp);
		}
		if( FTP_PRGM_TYPE == m_FileType )
		{
			if( !m_FtpFile.CheckFileList(  ) )
			{
				SetErrorBit(  );	
				AddFrameHead(  );
				return TRUE;
			}

			sprintf( chTmp, "%s%s", FTP_DOWN_PRGM_FILE, "bin/" );
			m_FtpFile.DeleteDirFiles( chTmp);
			sprintf( chTmp, "%s%s", FTP_DOWN_PRGM_FILE, "lib/" );
			m_FtpFile.DeleteDirFiles( chTmp);
			m_FtpFile.DeleteDirFiles( FTP_DOWN_USER_FILE );
		}

		m_wFileNum++;
	}
	//校验文件是否正确
	else if( NULL !=  m_FtpFile.GetFileLineBuf( FTP_FILE_LIST, m_wFileNum,chLineBuf ))
	{
		m_wFileNum++;
		char *p = strrchr( m_chFileName, '/' );

		if( NULL == strstr( chLineBuf, p ) )
		{
			printf ( "error\n" );
			SetErrorBit(  );	
		}
	}
	else
	{
		SetErrorBit(  );	
	}

	AddFrameHead(  );
	return TRUE;
}		/* -----  end of method CSocketFtpProto::PDD6  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  ProcessDownData
 * Description:  处理下载报文 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::ProcessDownData ( BYTE *buf, int len )
{
	BOOL bRtn = TRUE;
	/* 处理文件 */
	if( isFileState(  ) )
	{
		//处理文件
		if( 0 ==  m_FtpFile.WriteFile( m_chFileName, buf+1, len-1 ))
		{
			//可能是空文件		mengqp		2015-11-05 10:53:49
			// SetErrorBit(  );
		}
		SetFuncCode( RECV_CONFIG );
		m_wSendLen = 1;
		AddFrameHead(  );

	}
	else
	{
		switch ( GetFuncCode(  ) )
		{
			case START_TRANS: /* 开始传输 */
				{
					DWORD dwVersion = ( buf[1] << 24 ) 
						| ( buf[2]<<16 )  
						| ( buf[3] << 8 ) 
						| ( buf[4] );
					if( HIWORD( dwVersion ) != HIWORD( EMU2000_VERSION ) )
					{
						SetErrorBit(  );	
					}
					m_wSendLen = 1;
					AddFrameHead(  );
				}
				break;
			case END_TRANS: /* 结束传输 */
				PDD_endTrans(  );
				break;

			case FILE_LIST_INFO: /* 文件列表 */
				PDD_FileListInfo(  );
				break;


			case FILE_INFO: /* 传输文件 */
				SetFileName( (char *)( buf+5 ) );
				PDD_FileInfo(  );
				break;

			case MD5: /* md5 校验 */
				PDD6( (char *)( buf + 1 ) );
				break;

			case REBOOT:	
				m_wSendLen = 1;
				m_FtpFile.DeleteDirFiles( FTP_FILE_LIST );
				SetFuncCode( REBOOT );
				AddFrameHead(  );
				m_bReboot = TRUE;

			default:	
				bRtn = FALSE;
				break;
		}				/* -----  end switch  ----- */

	}

	return bRtn;
}		/* -----  end of method CSocketFtpProto::ProcessDownData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  ProcessProtoSendBuf
 * Description:  处理协议数据
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::ProcessProtoSendBuf ( BYTE *buf, int len )
{
	memset( m_bySendBuf, 0, FTP_MAX_SEND_LEN );
	m_wSendLen = 0;

	//判断数据的基本格式
	if ( !IsFrameFormat( buf, len ) )
	{
		printf ( "ProcessProtoSendBuf::fault format\n" );	
		return FALSE;
	}

	//判断数据的状态
	if( !ProcessStateByte( buf[3] & 0xf0 ) )
	{
		printf ( "ProcessProtoSendBuf::fault state\n" );	
		return FALSE;
	}

	SetFuncCode( buf[3] & 0x0f );

	//处理报文
	if( IsLoad(  ) )
	{
		//处理上传命令
		return ProcessLoadData( buf + 3, len - 3 )	;
	}
	else
	{
		//处理下载报文
		return ProcessDownData( buf + 3, len - 3 )	;
	}

	return FALSE;
}		/* -----  end of method CSocketFtpProto::ProcessProtoSendBuf  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketFtpProto
 *      Method:  Init
 * Description:  初始化协议
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CSocketFtpProto::Init ( void )
{
	memset( m_bySendBuf, 0, FTP_MAX_SEND_LEN );
	m_wSendLen = 0;
	memset( m_chFileName, 0 , 256 );
	m_uiReadPos = 0;
	m_wFileNum= 0;
	m_FileType = FTP_NONE_TYPE;
	m_dwFileSize = 0xffffffff;
	m_bReboot = FALSE;

	return TRUE;
}		/* -----  end of method CSocketFtpProto::Init  ----- */

