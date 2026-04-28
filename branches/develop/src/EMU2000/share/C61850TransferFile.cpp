#include "C61850TransferFile.h"
#include <stdio.h>

#define ARM
C61850TransferFile::C61850TransferFile()
{
}


C61850TransferFile::~C61850TransferFile()
{

}

BOOL C61850TransferFile::downLoad(const char * pFileName)
{
#ifdef ARM
	IedClientError error;
	IedConnection con = IedConnection_create();
	int portNo = atoi(m_strPortNo.c_str());
	IedConnection_connect(con, &error, m_strServerIP.c_str(), portNo);

	string strLocalPath(m_strLocalDirect);
	strLocalPath += "/";
	strLocalPath += pFileName;

	char devPath[200] = { "/COMTRADE/" };
	strcat(devPath, pFileName ) ;
	getFile(con , strLocalPath.c_str() , devPath);
	IedConnection_destroy(con);
#endif
	return true;
}

BOOL C61850TransferFile::fileList(string oFileName, string destDirect)
{
#ifdef ARM
	IedClientError error;
	IedConnection con = IedConnection_create();
	int portNo = atoi(m_strPortNo.c_str());
	IedConnection_connect(con, &error, m_strServerIP.c_str(), portNo );
	showDirectory(con);
	IedConnection_destroy(con);
#endif
	return true;
}


BOOL C61850TransferFile::split(vector<string>&vec, char * pStrDest)
{
	if (!pStrDest)
		return FALSE;

	vec.clear();
	char * pBuf = pStrDest;
	const char * delimiter = "/";
	char * pTmp = strtok(pBuf, delimiter);
	//first data is discarded
	while (pTmp != NULL)
	{
		pTmp = strtok(NULL, delimiter);
		if (pTmp)
			vec.push_back(pTmp);
	}
	return TRUE;
}

string C61850TransferFile::getleftstring(string strDest, string strdelimiter)
{
	if (strDest.empty() || strdelimiter.empty())
		return FALSE;

	size_t index = strDest.find(".");
	if (index == string::npos)
		return FALSE;

	string strOld = strDest.substr(0, index);
	return strOld;
}

#ifdef ARM
void C61850TransferFile::showDirectory(IedConnection con)
{


    m_fileVec.clear();
	IedClientError error;

	/* Get the root directory */
	LinkedList rootDirectory =
		IedConnection_getFileDirectory(con, &error, NULL);

	if (error != IED_ERROR_OK) {
		printf("Error retrieving file directory\n");
	}
	else {
		LinkedList directoryEntry = LinkedList_getNext(rootDirectory);

		while (directoryEntry != NULL) {

			FileDirectoryEntry entry = (FileDirectoryEntry)directoryEntry->data;

			printf("%s %i\n", FileDirectoryEntry_getFileName(entry), FileDirectoryEntry_getFileSize(entry));
			string strFullName = FileDirectoryEntry_getFileName(entry);
			char szName[100] = { 0 };
			strFullName.copy(szName , sizeof( szName) , 0 );

			vector<string> vet;
			split( vet, szName );

			FILE_PROPERTY fProperty;
			fProperty.strFileName = vet[0];
			fProperty.strFileDate = "";
			fProperty.strFileTime = "";
			fProperty.ulFileSize = 0 ;
			m_fileVec.push_back(fProperty);
			directoryEntry = LinkedList_getNext(directoryEntry);
		}

		LinkedList_destroyDeep(rootDirectory, (LinkedListValueDeleteFunction)FileDirectoryEntry_destroy);
	}

}
#endif

static bool downloadHandler(void* parameter, uint8_t* buffer, uint32_t bytesRead)
{
	FILE* fp = (FILE*)parameter;

	printf("received %i bytes\n", bytesRead);

	if (fwrite(buffer, bytesRead, 1, fp) == 1)
		return true;
	else {
		printf("Failed to write local file!\n");
		return false;
	}

}
#ifdef ARM
void  C61850TransferFile::getFile(IedConnection con, const char * localFilename , const char * devPath )
{

	IedClientError error;
	FILE* fp = fopen(localFilename, "w");

	if (fp != NULL) {

		/* Download a file from the server */
		IedConnection_getFile(con, &error, devPath, downloadHandler, (void*)fp);

		if (error != IED_ERROR_OK)
			printf("Failed to get file!\n");

		fclose(fp);
	}
	else
		printf("Failed to open file %s\n", localFilename);

}
#endif

FILE_PROPERTY *  C61850TransferFile::GetLast()
{
	int size = m_fileVec.size();
	if (size == 0)
		return NULL;

	m_index = 0;
	return &m_fileVec[m_index];
}
FILE_PROPERTY * C61850TransferFile::GetPrev()
{
	int size = m_fileVec.size();
	if (size == 0 || ( m_index >= size - 1 ) || m_index < 0)
		return NULL;

	return &m_fileVec[++m_index];
}
