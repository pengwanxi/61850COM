#include "CFtpClient.h"
#include "ftplib.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rdbDef.h"

using namespace std ;

CFtpClient::CFtpClient()
{
    //ctor
    init( );
	m_index = 0;
	m_strServerIP.clear();
}

CFtpClient::~CFtpClient()
{
    //dtor
    FtpClose( m_pNetbuf ) ;
}

void CFtpClient::init( )
{
	m_pNetbuf = NULL;
	m_strPortNo = "10000";
	m_strRemoteDirectory.empty();
   FtpInit( ) ;
}

void CFtpClient::setHostIP( string strIP )
{
	if (strIP.empty())
		return;

	m_strServerIP = strIP;
}

void CFtpClient::setHostPort( int portNo )
{
	if (portNo != atoi(m_strPortNo.c_str()) && portNo)
	{
		char buf[100];
		sprintf(buf, "%d", portNo);
		m_strPortNo = buf;
	}
}

void CFtpClient::setLoginInfo( string userName ,string strPassWd )
{
    if( userName.empty() || strPassWd.empty() )
        return ;

	m_strUserName = userName ;
    m_strPassWd = strPassWd ;
}

void CFtpClient::setLocalDirectory(string strName)
{
	if (strName.empty())
		return;

	m_strLocalDirect = strName;
	if (!m_strServerIP.empty())
	{

		m_strLocalDirect += "/" + m_strServerIP;
		string tt = m_strLocalDirect ;
		mode_t ret = umask(0);
		UNUSED( ret );
		if (mkdir(m_strLocalDirect.c_str(), S_IRWXU))
			perror("setLocalDirectory");
	}
}

BOOL CFtpClient::connect()
{
	if (m_strServerIP.empty())
	{
		cout << "ftpClient server ip is empty" << endl;
		return FALSE;
	}

	string str = m_strServerIP + ":" + m_strPortNo;
	int ret = FtpConnect(str.c_str(), &m_pNetbuf);
	return ret ? TRUE : FALSE;
}

BOOL CFtpClient::login()
{
	if (m_strUserName.empty() || m_strPassWd.empty())
	{
		cout << "ftpClient username or passwd is empty" << endl;
		return FALSE;
	}

	int ret = FtpLogin(m_strUserName.c_str(), m_strPassWd.c_str() , m_pNetbuf);
	return ret ? TRUE : FALSE;
}

BOOL CFtpClient::chdir( string strPath )
{
	if (!m_pNetbuf || strPath.empty())
		return FALSE;

	int ret = FtpChdir(strPath.c_str(), m_pNetbuf);
	return  ret ? TRUE : FALSE;
}

BOOL CFtpClient::fileList(string oFileName, string destDirect)
{
	if (oFileName.empty() || destDirect.empty())
		return FALSE;

	string of = m_strLocalDirect + "/" + oFileName; //mynand/FaultRecorder/192.168.1/192.168..txt
	int ret = FtpDir(of.c_str( ) , destDirect.c_str() , m_pNetbuf );
	if (!ret)
        return FALSE ;

    m_strFileNameList = of;

	parseFile(m_strFileNameList);
	return (BOOL)ret;
}

BOOL CFtpClient::parseFile(string strFileName)
{
	if (strFileName.empty())
		return FALSE;

	ifstream file(strFileName.c_str());
	if (!file.is_open())
		return FALSE ;

	m_fileVec.clear();

	char buffer[200];
	memset(buffer, 0, sizeof(buffer));
	while (!file.eof())
	{
		file.getline(buffer, 200);
        cout << buffer << " "<< strlen( buffer ) << endl ;

		int size = strlen(buffer);
		if( !size )
			continue;
		FILE_PROPERTY *pfp = getFileProperty(buffer);
		if (pfp)
		{
			std::string name = pfp->strFileName;
			int flag=0;
			if (name.find(".dat") != std::string::npos) { 
				printf("----dat find\n");  
				flag=1;       
			}  
			if (name.find(".cfg") != std::string::npos) { 
				printf("----cfg find\n");  
				flag=1;  
			} 
			if(flag==1)
			addElement(pfp);
			delete pfp;
		}
	}

	file.close();

	return FALSE;
}

FILE_PROPERTY * CFtpClient::getFileProperty(char * pBuf)
{
	if (pBuf == NULL || !strlen( pBuf ) )
		return NULL;

	char * buffer = pBuf;
	int i = 0;
	
	char * ptmp = NULL;
	ptmp = strtok(buffer, " ");
	if (!strcmp(ptmp, ".") || !strcmp(ptmp, ".."))
		return NULL;
	/*
	int datflag=0,cfgflag=0;
	printf("---%s--++\n",ptmp);
	string tempstr= buffer;
	if(tempstr.size()<3)
	 return NULL;
	 if (tempstr.find(".dat") != std::string::npos) { 
		printf("----dat find\n");
		datflag=1;         
    }  
	if (tempstr.find(".cfg") != std::string::npos) { 
		printf("----cfg find\n"); 
       cfgflag=1;
    } 
	if(datflag==0&&cfgflag==0)
	 return NULL;
  */
   
	FILE_PROPERTY * pfp = new FILE_PROPERTY;
	while (ptmp != NULL)
	{
		printf("%d  %s\n",i,ptmp);
		switch (i)
		{
		case FILE_NAME:
			pfp->strFileName = ptmp;
			break;
		case FILE_SIZE:
			pfp->ulFileSize = atol(ptmp);
			break;
		case FILE_DATE_DAY:
			pfp->strFileDate = ptmp;
			break;
		case FILE_TIME:
			pfp->strFileTime = ptmp;
			break;
		}
		i++;
        ptmp = strtok(NULL, " ");
	}

	return pfp;
}

BOOL CFtpClient::addElement( FILE_PROPERTY *pfProperty )
{
	if (!pfProperty)
		return FALSE;

	cout << pfProperty->strFileDate << endl;
	m_fileVec.push_back(*pfProperty);

	return TRUE;
}

BOOL CFtpClient::downLoad( const char * pFileName )
{
	if (!m_pNetbuf)
		return FALSE;

	string strPath(m_strLocalDirect);
	strPath += "/";
	strPath += pFileName;
	int ret = FtpGet(strPath.c_str(), pFileName, FTPLIB_BINARY, m_pNetbuf );
	return ( BOOL )ret ;
}

void CFtpClient::closeFtp()
{
	if (m_pNetbuf)
	{
		FtpClose(m_pNetbuf);
		m_pNetbuf = NULL;
	}

	return ;
}

FILE_PROPERTY *  CFtpClient::GetLast()
{
	int size = m_fileVec.size();
	if (size == 0)
		return NULL;

	m_index = size - 1;
	for(int i=0; i < size; i++)
	{
		printf("%s", m_fileVec[i].strFileName.c_str());
	}
	printf("*********%d %s****%d********\n", __LINE__, __FILE__,m_index );
	

	return &m_fileVec[ m_index ];
}

FILE_PROPERTY * CFtpClient::GetPrev()
{
	int size = m_fileVec.size();
	if (size == 0 || m_index >=size || m_index <= 0 )
		return NULL;

	m_index--;
	return &m_fileVec[ m_index ];
}
