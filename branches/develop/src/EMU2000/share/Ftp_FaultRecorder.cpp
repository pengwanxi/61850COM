#include "Ftp_FaultRecorder.h"
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include "rdbDef.h"
#include "C61850TransferFile.h"

using namespace std ;


CFtp_FaultRecorder::CFtp_FaultRecorder():VECTOR_SIZE( 4 ), m_vec(1)
{
	m_pFtpClient = NULL;
	m_pFtpServer = NULL;
	m_pTcpPortServer = NULL;
	m_listenPort = 0;
	m_listenIp.clear();
	m_strErr.clear();
	m_threadId = 0;
	m_byProtoType = p_invalid;
	m_ftpServerPort = 22310;
	m_pftpServerUser = NULL;

	const char FTP_TMEP_DIRECTORY[] = { "/mynand/FaultRecorder" };
	strcpy( FILE_DIRECTORY , FTP_TMEP_DIRECTORY );
	strcpy(FILE_READED_NAME, "FaultRecorderRead.ini");
	system("mkdir -p /mynand/FaultRecorder");
	createFaultRecorderIni();
}


CFtp_FaultRecorder::~CFtp_FaultRecorder()
{
	closeThread();
	deleteftpobj();
	printf( "~CFtp_FaultRecorder delelte \n" ) ;
}

void CFtp_FaultRecorder::closeThread()
{
	sendCloseMsg();
	if( m_threadId )
	pthread_join(m_threadId , NULL );
}

void CFtp_FaultRecorder::sendCloseMsg()
{
	if (!m_pTcpPortServer)
		return;

	LMSG msg;
	msg.pVoid = ( void * )CTcpPortServer::eexit;
	m_pTcpPortServer->sendMsg(&msg);
}

void CFtp_FaultRecorder::deleteftpobj( )
{
	if (m_pTcpPortServer)
		delete m_pTcpPortServer;
	if (m_pFtpClient)
	{
		m_pFtpClient->closeFtp();
		delete m_pFtpClient;
	}
	if (m_pFtpServer)
		delete m_pFtpServer;
}

void CFtp_FaultRecorder::createFaultRecorderIni()
{
	string strFN = FILE_DIRECTORY;
	strFN += "/" ;
	strFN += FILE_READED_NAME;
	ifstream ifs(strFN.c_str());
	if (!ifs.is_open())
	{
		ofstream of(strFN.c_str(), ofstream::out);
		if (of.is_open())
			of.close();
		return;
	}

	ifs.close();
}

BOOL CFtp_FaultRecorder::initListenServer( int port , char * pType , string strIp)
{
	if ( strIp.empty() || !pType )
		return FALSE;
	m_listenIp = strIp;
	m_listenPort = port;
	m_typeCom = pType;
	return TRUE;
}

BOOL CFtp_FaultRecorder::startUpFaultReorder()
{
	initFtp();

	BOOL bflag = FALSE;
	if (m_pTcpPortServer)
	{
		m_pTcpPortServer->setTcpParam(m_listenIp.c_str(), m_listenPort);
		char err[200] = { 0 };
		bflag = m_pTcpPortServer->OpenPort( err );
	}

	initThread();
	return bflag;
}


BOOL CFtp_FaultRecorder::initThread()
{
	int ret = pthread_create(&m_threadId, NULL, threadFtp, this);
	if (ret != 0)
	{
		cout << "CFtp_FaultRecorder create thread failing" << endl;
		return FALSE;
	}

	return TRUE ;
}

void * CFtp_FaultRecorder::threadFtp(void * pvoid )
{
	if (!pvoid)
		return 0;

	CFtp_FaultRecorder * pFtp = static_cast<CFtp_FaultRecorder *>(pvoid);
	CTcpPortServer * pTcpServer =  pFtp->m_pTcpPortServer;
	if (!pTcpServer)
		return 0;

	while ( 1 )
	{
		LMSG msg;
		if (pTcpServer->recvMsg(&msg))
		{
			unsigned long ninfo = (unsigned long)msg.pVoid;
			if (ninfo == CTcpPortServer::eexit )
			{
				cout << endl << " a client connected " << endl;
				break;
			}
		}

		pFtp->ProcessFtp();

		usleep(1000 * 100);
	}

	return 0;
}


void CFtp_FaultRecorder::ProcessFtp()
{
	if (!m_pTcpPortServer->IsPortValid())
		return;

	BYTE buf[1024] = { 0 };
	int len = sizeof(buf);

	int rRead = m_pTcpPortServer->AsyReadData(buf, len);
	if (rRead <= 0)
		return;

	if (!ProcessProto(buf, rRead))
	{
		sendErrorStr("FF ");
		return;
	}

	cout << m_pftpServerUser << endl;
	if (getProto(buf, len))
	{
		sendStr(buf, len);
	}
	else
		sendErrorStr("FF ");

	return;
}

void CFtp_FaultRecorder::sendErrorStr( string strCode )
{
	if (strCode.empty())
		return;
	char sendErr[1024] = { 0 } ;
	int len = strCode.length();
	strCode.copy(sendErr, len, 0);

	getErrString(sendErr, len);
	sendStr(( BYTE *)sendErr, len);
}

void CFtp_FaultRecorder::sendStr(BYTE * pstr, int size )
{
	if (!pstr || !size)
		return;

	m_pTcpPortServer->AsySendData((BYTE *)pstr, size);
}

BOOL CFtp_FaultRecorder::getProto(BYTE * pBuf, int &len)
{
	if (!pBuf || !len)
		return FALSE;
	memset(pBuf, 0, len);
	cout << m_pftpServerUser << endl;
	BOOL bflag = FALSE;
	switch (m_byProtoType)
	{
		case p_ready:
		{
			bflag =  getTransmitFileReady(pBuf, len);
			m_byProtoType = p_invalid;
		}
		break;
		default:
			break;
	}

	return bflag ;
}


BOOL CFtp_FaultRecorder::getTransmitFileReady(BYTE *pBuf, int &len)
{
	if (!pBuf)
		return FALSE;

	//startup ftp server
	if (!initFtpServer())
		return FALSE;

	//delete
	deleteDevFileListFromFtpDirectory();

	return getSuccessProto(pBuf, len);
}

BOOL CFtp_FaultRecorder::initFtpServer()
{
	if (!m_pFtpServer && !m_pFtpClient )
		return FALSE;

	string str = m_pFtpClient->localDirectory();
	if (str.empty())
		return FALSE;

	// device has independent ftp directory
	// so invoke deleteuser firstly and create it's own directory
	if (m_pftpServerUser)
	{
		m_pFtpServer->DeleteUser(m_pftpServerUser);
	}

	m_pftpServerUser = m_pFtpServer->AddUser("admin", "admin", str.c_str() );
	if (!m_pftpServerUser)
		return FALSE;

	m_pftpServerUser->SetMaxNumberOfClient(0);
	m_pftpServerUser->SetPrivileges(CFtpServer::READFILE | CFtpServer::LIST );
	if (!m_pFtpServer->StartListening(INADDR_ANY, m_ftpServerPort))
	{
		setError("listen ftpserver fail");
		return FALSE;
	}

	if (!m_pFtpServer->StartAccepting())
		return FALSE;

	return TRUE;
}

BOOL CFtp_FaultRecorder::getSuccessProto(BYTE * pBuf, int &len)
{
	if (!pBuf || !len)
		return FALSE;

	string str = "02";
	char szPort[20] = { 0 };
	sprintf(szPort, ":%u:", m_ftpServerPort);
	str += szPort;
	int strlen = str.length();
	if (strlen >= len)
		return FALSE;

	str += " files have been ready";
	str.copy((char *)pBuf, strlen, 0);

	return TRUE;
}

void CFtp_FaultRecorder::setError(string strErr)
{
	if (strErr.empty())
		return;

	m_strErr = strErr;
	cout << endl << m_strErr << endl;
}

void CFtp_FaultRecorder::getErrString(char * pErr, int &szLen)
{
	if (m_strErr.empty() || !pErr || !szLen )
		return;

	m_strErr.insert(0, pErr, szLen);
	int strLen = m_strErr.length();
	m_strErr.copy(pErr, strLen , 0);
	szLen = strLen ;
}

BOOL CFtp_FaultRecorder::ProcessProto(BYTE * pBuf, int len)
{
	if (!pBuf || !len)
		return FALSE;

	const BYTE dnLoad_Len = 8;
	char szDown[] = "download";
	char szSuccess[] = "00";
	char szFailed[] = "FF";
	BOOL bFlag = FALSE;
	UNUSED( bFlag ) ;
	if (len >= dnLoad_Len && !memcmp(pBuf, szDown, strlen(szDown)))
	{
		bFlag =  ProcessDownLoad(pBuf, len);
	}
	else if (len >= 2 && !memcmp(pBuf, szSuccess, strlen(szSuccess)))
	{
		cout << "client dowload scceuss" << endl ;
		saveFileInfoToLocal(m_vDownLoadDevRecord);
		deleteDevDirectoryFromFtpDirectory();
	}
	else if (len >= 2 && !memcmp(pBuf, szFailed, strlen(szFailed)))
	{
		cout << "client download failed" << endl;
		return FALSE;
	}
	else
		setError("protocol is wrong!pleaes ask for corresponding company");

	return bFlag;
}

BOOL CFtp_FaultRecorder::deleteDevFileListFromFtpDirectory()
{
	if (!m_pFtpClient)
		return FALSE;

	string str = m_pFtpClient->localDirectory();
	if (str.empty())
		return FALSE;
	str += "/" + m_pFtpClient->hostIp() + ".txt";
	if (PathFileExists(str.c_str()))
	{
        if(-1 == remove(str.c_str()) )
         {
            perror( "deleteDevFileListFromFtpDirectory" ) ;
            return FALSE ;
         }
	}

    return TRUE ;
}

BOOL CFtp_FaultRecorder::deleteDevDirectoryFromFtpDirectory()
{
	if (!m_pFtpClient)
		return FALSE;

	string str = m_pFtpClient->localDirectory();
	if (str.empty())
		return FALSE;
	if (PathFileExists(str.c_str()))
		remove_dir(str.c_str());
	return TRUE;
}

BOOL CFtp_FaultRecorder::ProcessDownLoad( BYTE * pBuf , int len)
{
	if (!pBuf || !len)
		return FALSE;

	const BYTE dnLoad_Len = 8;
	// download is 8 characters
	if (len < dnLoad_Len)
		return FALSE;

	resetConfig();

	//download 192.168.1.50:21 : ENGINEER : remote0003
	if (!split(m_vec, (char *)pBuf))
	{
		setError("protocol is wrong!pleaes ask for corresponding company");
		return FALSE;
	}

	if (!downloadFromDev())
	{
      deleteDevDirectoryFromFtpDirectory();
      return FALSE;
	}

	m_byProtoType = p_ready;
	return TRUE;
}

BOOL CFtp_FaultRecorder::resetConfig()
{
	if (m_pFtpClient)
	{
		m_pFtpClient->closeFtp();
	}

    if (!m_pFtpServer)
        return FALSE ;

	// FaultRecorderRead.ini can be deleted by web cmd
	// so each connection need check it
	createFaultRecorderIni();

	return TRUE ;
}

BOOL CFtp_FaultRecorder::downloadFromDev()
{
	if (m_vec.size() < VECTOR_SIZE)
		return FALSE;

	if (!initFtpClient())
	{
		cout << "connect client failing" << endl;
		setError("connect client failing") ;
		return FALSE;
	}

	if (!downloadLatestFaultFile())
	{
		return FALSE;
    }

	return TRUE ;
}

BOOL CFtp_FaultRecorder::downloadLatestFaultFile()
{
	if (!m_pFtpClient)
		return FALSE;
	const FILE_PROPERTY * pFtpFile = m_pFtpClient->GetLast() ;
	if (!pFtpFile)
		return FALSE;
	CProfile * pProfile = GetProfile();
	if (!pProfile)
	{
		cout << "read faultrecorderread.ini fail.";
		return FALSE;
	}
	char ip[20] = { 0 };
	GetDevIp( ip , sizeof( ip ) ) ;

	//vector<string> m_vDownLoadDevRecord;
	m_vDownLoadDevRecord.clear();
	BOOL bRet = FALSE;
	while (pFtpFile)
	{
		if (!isNeedDownLoad(pFtpFile, ip, pProfile))
		{
			pFtpFile = m_pFtpClient->GetPrev();
			continue;
		}
        m_vDownLoadDevRecord.push_back(pFtpFile->strFileName);
		if (!m_pFtpClient->downLoad(pFtpFile->strFileName.c_str()))
		{
			string str = "download " + pFtpFile->strFileName + "error \n download stop .";
			setError( str );
			break;
		}
		const FILE_PROPERTY * pNewFtpFile = m_pFtpClient->GetPrev();
		if (isSameFileName(pFtpFile, pNewFtpFile))
		{
			pFtpFile = pNewFtpFile;
		}
		else
		{
			// save filename to FaultRecorderRead.ini
			//if (saveFileInfoToLocal(m_vDownLoadDevRecord))
			{
				bRet = TRUE;
				break;
			}
		}
	}

	if (pProfile)
		delete pProfile;
	printf("*********%d %s************\n", __LINE__, __FILE__);
	if( !bRet )
		setError("no new file need to download. or empty flag");

	return bRet;
}

string CFtp_FaultRecorder::getFaultRecorderReadiniPath()
{
	string strPath(FILE_DIRECTORY);
	strPath += "/";
	strPath += FILE_READED_NAME;
	return strPath;
}

BOOL CFtp_FaultRecorder::saveFileInfoToLocal(vector<string>& vec)
{
	int size = vec.size();
	if (!size)
		return FALSE;

	string strFileN = getleftstring(vec[0], ".");
	if (strFileN.empty())
		return FALSE;

	char szPath[200] = { 0 };
	string strPath = getFaultRecorderReadiniPath();
	strPath.copy(szPath, sizeof(szPath), 0);

	char szSec[20] = { 0 };
	GetDevIp( szSec , sizeof( szSec ) ) ;

	char szKey[100] = { 0 };
	strFileN.copy(szKey, sizeof(szKey), 0);

	string strVal;
	vector<string>::iterator it = vec.begin();
	while ( it != vec.end() )
	{
		strVal += *it;
		strVal += ",";
		it++;
	}

	char szVal[1024] = { 0 };
	strVal.copy(szVal, sizeof(szVal), 0);
	WritePrivateProfileString(szSec, szKey, szVal, szPath );
	return TRUE;
}

BOOL CFtp_FaultRecorder::isSameFileName( const FILE_PROPERTY * pOld, const FILE_PROPERTY  * pNew)
{
	if (!pOld || !pNew)
		return FALSE;

	string strdelimiter = ".";
	string strold = getleftstring(pOld->strFileName ,strdelimiter );
	string strNew = getleftstring(pNew->strFileName, strdelimiter);

	if ( strold == strNew )
		return TRUE ;

	return FALSE;
}

string CFtp_FaultRecorder::getleftstring(string strDest, string strdelimiter)
{
	if (strDest.empty() || strdelimiter.empty())
		//return FALSE;
		return "";

	size_t index = strDest.find(".");
	if (index == string::npos)
		//return FALSE;
		return "";

	string strOld = strDest.substr(0 , index );
	return strOld;
}

BOOL CFtp_FaultRecorder::isNeedDownLoad(const FILE_PROPERTY * pFile, const char * pdevIp , CProfile * pProfile )
{
	if (!pdevIp || !pProfile || !pProfile->IsValid() )
		return FALSE;

	int len = strlen(pdevIp);
	if (len == 0)
		return FALSE;

	char buffer[100] = { 0 };
	int size = sizeof(buffer);
	char szName[100] = { 0 } ;
	string strName="" ;
	strName = getleftstring( pFile->strFileName , ".") ;
	printf("*********%d %s***%s*********\n", __LINE__, __FILE__,pFile->strFileName.c_str());
	strName.copy(szName, sizeof(szName), 0);
	if( strName.empty()|| strName=="")
	{

		return FALSE ;
	}
	char szDevip[20] = { 0 };
	memcpy(szDevip, pdevIp, len);
	char szNULL[] = { "NULL" };
	pProfile->GetProfileString(szDevip, szName, szNULL, buffer, size);
	BOOL bRet = FALSE;
	if (!strcmp(buffer, "NULL"))
		bRet = TRUE;
	return bRet;
}

void CFtp_FaultRecorder::GetDevIp( char * ip , int len)
{

	if (!m_vec.size() || !ip || !len || len < 16 )
		return ;

	string strip = m_vec[eip];
	strip.copy(ip, len, 0);
	return;
}

UINT CFtp_FaultRecorder::GetDevPort()
{
	if (!m_vec.size())
		return 0;

	return atoi(m_vec[eport].c_str());
}

// return Cprofile must delete
CProfile * CFtp_FaultRecorder::GetProfile()
{
	string strFN = FILE_DIRECTORY;
	strFN += "/";
	strFN += FILE_READED_NAME;
	char buf[100]={ 0 } ;
	strFN.copy(buf, strFN.length(), 0);
	CProfile *pfile = new CProfile(buf);
	if (!pfile->IsValid())
	{
		cout << strFN << "file not exist" << endl;
		return NULL ;
	}

	return pfile;
}

BOOL CFtp_FaultRecorder::initFtpClient()
{
	//download 192.168.1.50:21 : ENGINEER : remote0003
	if (m_vec.size() <  VECTOR_SIZE  )
		return FALSE;

	string ip = m_vec[eip];
	UINT port = atol(m_vec[eport].c_str());
	string userName = m_vec[euser];
	string passwd = m_vec[epasswd];

	m_pFtpClient->setHostIP(ip);
	m_pFtpClient->setHostPort(port);
	m_pFtpClient->setLoginInfo(userName, passwd);
	m_pFtpClient->setLocalDirectory(FILE_DIRECTORY);  ///mynand/FaultRecorder
	if (!m_pFtpClient->connect())
		return FALSE;

	if (!m_pFtpClient->login())
		return FALSE;

	char szPath[] = "/COMTRADE/";

	if (!m_pFtpClient->chdir(szPath))
	{
		strcpy(szPath, "/");
		if (!m_pFtpClient->chdir(szPath))
			return FALSE;
		//return FALSE;
	}

	if (!m_pFtpClient->fileList( ip + ".txt", szPath))
		return FALSE;
  printf("-------------------------------------------\n");
	return TRUE;
}

BOOL CFtp_FaultRecorder::split( vector<string>&vec , char * pStrDest)
{
	if ( !pStrDest )
		return FALSE;

	vec.clear();
	char * pBuf = pStrDest;
	const char * delimiter = " :";
	char * pTmp = strtok(pBuf, delimiter);
	//first data is discarded
	while (pTmp!=NULL)
	{
		pTmp = strtok(NULL, delimiter);
		if( pTmp )
			vec.push_back(pTmp);
	}
	return TRUE;
}

BOOL CFtp_FaultRecorder::initFtp()
{
	if (!strcmp(m_typeCom.c_str(), "ABB615"))
	{
		if (!m_pFtpClient)
			m_pFtpClient = new CFtpClient;
	}
	else if (!strcmp(m_typeCom.c_str(), "SIEMENS686"))
	{
		if (!m_pFtpClient)
			m_pFtpClient = new C61850TransferFile;
	}
	else
	    	printf(" fault recorder com type error!!\n" );

	if (!m_pFtpServer)
		m_pFtpServer = new CFtpServer;

	if (!m_pTcpPortServer)
		m_pTcpPortServer = new CTcpPortServer;

	cout << m_pftpServerUser << endl;
	return TRUE;
}
