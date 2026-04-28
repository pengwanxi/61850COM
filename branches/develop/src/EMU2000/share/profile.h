/******************************************************************************
  profile.h : header file for Linux
  Copyright (C): 2011 by houpeng
 ******************************************************************************/
#ifndef _PROFILE_H__
#define _PROFILE_H__

#include <stdio.h>
#include <vector>
#include "rdbFun.h"
#include <string>
using namespace std;
/*****************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

/******************************************************************************
 * CDataBuffer Object
 */
class CDataBuffer
{
	public:
		CDataBuffer();
		CDataBuffer(char *pBuf, int nLen);
		virtual ~CDataBuffer();

		// Attributes
	public:
		int m_nLen;
		char *m_pBuff;

	public:
		CDataBuffer* m_objPrev;
		CDataBuffer* m_objNext;

		/* Implementation */
	public:
		int   GetBuffer(char *pBuf);
		char* GetBuffer(void);
		int   GetBufLen(void);
};

typedef std::vector<CDataBuffer*> CBufferArray;

/******************************************************************************
 * CMyList Object
 */
class CMyList
{
	public:
		CMyList();
		virtual ~CMyList();

		// Attributes
	protected:
		int m_nSize;
		CDataBuffer* m_objFirt;
		CDataBuffer* m_objLast;
		CDataBuffer* m_objActual;

		/* Implementation */
	public:
		int  GetSize(void);
		bool AddObj(CDataBuffer* pObj);
		CDataBuffer* GetFirt(void);
		CDataBuffer* GetLast(void);
		CDataBuffer* GetNext(CDataBuffer* pObj);
		CDataBuffer* GetPrev(CDataBuffer* pObj);
};

/******************************************************************************
 * CProfile Object
 */
class CProfile
{
	public:
		CProfile();
		CProfile(char* lpszName);
		~CProfile();

		// Attributes
	public:
		FILE*  m_fdFile;
		char   m_szFileName[64];
		CBufferArray m_LineArray;

		/* Implementation */
	public:
		bool IsValid();
		bool OpenFile(char* lpszName);
		static int GetProfileString( char* lpAppName,
				char* lpKeyName,
				char* lpDefault,
				char* lpszBuff, int nSize,
				FILE* fd );
		string GetProfileSection(string strSectionName, vector<string>&vKeyNameArray);
		static int GetProfileInt(char* lpAppName,
				char* lpKeyName,
				int   nDefault,
				FILE* fd );

		int FindProfileString( char* lpAppName,
				char* lpKeyName,
				char* lpDefault,
				char* lpszBuff, int nSize );
		int FindProfileInt( char* lpAppName,
				char* lpKeyName,
				int   nDefault );

		int  GetProfileString( char* lpAppName,
				char* lpKeyName,
				char* lpDefault,
				char* lpszBuff, int nSize );
		int  GetProfileInt( char* lpAppName,
				char* lpKeyName,
				int   nDefault );
};

BOOL PathFileExists( const char * pFileName);
BOOL WritePrivateProfileString(char * lpAppName, char* lpKeyName, char* lpString, char* lpFileName);

int remove_dir(const char *dir);
/*****************************************************************************/
#endif /* #ifndef _PROFILE_H__ */
