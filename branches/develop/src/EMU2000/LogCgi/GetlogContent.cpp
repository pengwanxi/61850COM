#include "GetlogContent.h"
#include "unistd.h"



CGetlogContent::CGetlogContent()
{
	m_logPath = "/mnt/log";
	m_logList.reserve(200);
}


CGetlogContent::~CGetlogContent()
{
}

BOOL CGetlogContent::getLogList()
{
	FILE * fp = NULL;

	if ((fp = popen((" ls -lr " + m_logPath).c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}


	char buf[200];
	memset(buf, '\0', sizeof(buf));
	while (fgets(buf, 200, fp) != NULL)
	{
		string strBuf(buf);
		if (string::npos != strBuf.find("backup"))
		{
			getLogListFromBackup();
			continue;
		}
		
		size_t  pos = strBuf.find("curlog");
		if( string::npos == pos )
			continue;

		int len = strBuf.length();
		strBuf = strBuf.substr(0, len - 1).c_str();

		m_logList.push_back(strBuf.substr(pos));
	}
	fclose(fp);

	generateLogListJsonString();
	return TRUE;
}

void CGetlogContent::getLogListFromBackup()
{
	FILE * fp = NULL;

	if ((fp = popen((" ls " + m_logPath + "/backup" ).c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}


	char buf[200];
	memset(buf, '\0', sizeof(buf));
	while (fgets(buf, 200, fp) != NULL)
	{
		string strBuf(buf);
		if (string::npos == strBuf.find("txt"))
			continue;
		int len = strBuf.length();
		strBuf = strBuf.substr(0, len - 1).c_str();
		m_logList.push_back( ("backup/" + strBuf).c_str() );
	}
	fclose(fp);
}

void CGetlogContent::generateLogListJsonString()
{
	m_JSonString.clear();
	int size = m_logList.size();
	Json::Value data;
	for (int i = 0; i < size; i++)
	{
		data.append(m_logList[ i ]);
	}

	JVal["filelist"] = data;
}

void CGetlogContent::getFileContent(string &FilePath)
{
	string strPath;
	strPath = m_logPath + "/" + FilePath;
	strPath = strPath.substr(0, strPath.length() );
	if (access(strPath.c_str(), F_OK) != 0)
	{
		m_strFileContent = FilePath + " doesn't exist!";
		return;
	}

	char buf[1024 * 110];
	memset(buf, 0, sizeof(buf));


	FILE * fp = NULL;
	fp = fopen(strPath.c_str(), "r");
	fread(buf, sizeof( buf ) , 1, fp );
	m_strFileContent = buf;

	fclose(fp);

}

void CGetlogContent::getFileContentJSonString(string &strFilePath)
{
	getFileContent(strFilePath);

	m_JSonString.clear();
	JVal["filename"] = strFilePath;

	JVal["content"] = m_strFileContent;
	
	Json::FastWriter fast_writer;
	m_JSonString = fast_writer.write(JVal);
	
	printf("Content-type:application/json\n\n");
	printf(m_JSonString.c_str());
}