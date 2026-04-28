#ifndef CFTPCLIENT_H
#define CFTPCLIENT_H

#include "ftplib.h"
#include <string>
#include <vector>
#include "typedef.h"
using namespace std ;

typedef struct struct_File
{
	string strFileName;
	unsigned long ulFileSize;
	string strFileDate;
	string strFileTime;

	struct_File(){ }

	struct_File( const struct_File & fSource )
	{
		strFileName = fSource.strFileName;
		ulFileSize = fSource.ulFileSize;
		strFileDate = fSource.strFileDate;
		strFileTime = fSource.strFileTime;
	}

}FILE_PROPERTY;

typedef vector<FILE_PROPERTY>::iterator  FILE_MAP_ITOR ;

class CFtpClient
{
    public:
        CFtpClient();
        virtual ~CFtpClient();
		void init();
		//enum { FILE_NAME, FILE_SIZE, FILE_DATE, FILE_TIME };
		enum { FILE_ATTRIBUTE,FILE_DIRNUM,FILE_USERNAME,FILE_GROUPNAME,FILE_SIZE,FILE_DATE_MONTH,FILE_DATE_DAY,FILE_TIME,FILE_NAME };
        void setHostIP( string strIP ) ;
        void setHostPort( int portNo ) ;
        void setLoginInfo( string UserName ,string strPassWd );
		void setLocalDirectory( string strName );
		string localDirectory() { return m_strLocalDirect; }
		string hostIp() { return m_strServerIP; }
		virtual BOOL connect();
		virtual BOOL login();
		virtual BOOL chdir(string strPath);
		virtual BOOL fileList(string oFileName, string destDirect);
		virtual BOOL downLoad(const char * pFileName);
		void closeFtp();
		virtual  FILE_PROPERTY * GetLast();
		virtual  FILE_PROPERTY * GetPrev();
protected:
	BOOL parseFile(string strFileName);
	FILE_PROPERTY * getFileProperty(char * pBuf);
	BOOL addElement(FILE_PROPERTY *fProperty);
        netbuf * m_pNetbuf  ;
        string m_strServerIP ;
        string m_strUserName ;
        string m_strPassWd ;
		string m_strFileNameList; // save file name list
		vector<FILE_PROPERTY> m_fileVec;
		string m_strRemoteDirectory;
		string m_strPortNo  ;
		string  m_strLocalDirect;
		int m_index;
};

#endif // CFTPCLIENT_H
