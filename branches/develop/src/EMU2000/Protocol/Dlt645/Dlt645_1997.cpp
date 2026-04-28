/*
 * =====================================================================================
 *
 *       Filename:  Dlt645_1997.c
 *
 *    Description:  dlt645 1997版本协议 
 *
 *        Version:  1.0
 *        Created:  2014年11月12日 10时46分34秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */

#include "Dlt645_1997.h"

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_1997
 *      Method:  CDlt645_1997
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_1997::CDlt645_1997 ()
{
	memset( m_bySlaveAddr, 0xAA, sizeof( m_bySlaveAddr ) );
}  /* -----  end of method CDlt645_1997::CDlt645_1997  (constructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_1997
 *      Method:  ~CDlt645_1997
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDlt645_1997::~CDlt645_1997 ()
{
}  /* -----  end of method CDlt645_1997::~CDlt645_1997  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_1997
 *      Method:  ProcessYcData
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_1997::ProcessYcData ( const BYTE *buf, int len )
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	DWORD dwYcVal = 0;
	const BYTE *pointer;
	if ( len < 14 )
		return FALSE;

	if ( buf[8] != 0x81 )
		return FALSE;
	
	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	
	pointer = buf + 12;
	while( byDataNum > 0 )
	{
		float fYcVal;

		CalFormatData( pointer, byDataFormat, byDataLen, dwYcVal);
		fYcVal = (float)dwYcVal;
		m_pMethod->SetYcData( m_SerialNo, wPnt, fYcVal );

		pointer += byDataLen;
		wPnt++;
		byDataNum--;

		sprintf( m_szPrintBuf, "yc pnt:%d value:%f", wPnt, fYcVal );
		print( m_szPrintBuf );
	}
	
	return TRUE;
}		/* -----  end of method CDlt645_1997::ProcessYcData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_1997
 *      Method:  ProcessYmData
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_1997::ProcessYmData ( const BYTE *buf, int len )
{
	BYTE byDataNum = 0;
	BYTE wPnt = 0;
	DWORD dwYmVal = 0;
	BYTE byDataFormat = 0;
	BYTE byDataLen = 0;
	const BYTE *pointer;
	if ( len < 14 )
		return FALSE;

	if ( buf[8] != 0x81 )
		return FALSE;
	
	byDataNum = m_CfgInfo[m_bySendPos].byDataNum;
	wPnt = (WORD)m_CfgInfo[m_bySendPos].byStartIndex;
	byDataFormat = m_CfgInfo[m_bySendPos].byDataFormat;
	byDataLen = m_CfgInfo[m_bySendPos].byDataLen;
	
	pointer = buf + 12;
	while( byDataNum > 0 )
	{
		CalFormatData( pointer, byDataFormat, byDataLen, dwYmVal );
		m_pMethod->SetYmData( m_SerialNo, wPnt, static_cast<QWORD>(dwYmVal));

		pointer += byDataLen;
		wPnt++;
		byDataNum--;

		sprintf( m_szPrintBuf, "ym pnt:%d value:%lu", wPnt, dwYmVal );
		print( m_szPrintBuf );
	}
	
	return TRUE;
}		/* -----  end of method CDlt645_1997::ProcessYmData  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDlt645_1997
 *      Method:  RequestReadData
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDlt645_1997::RequestReadData ( BYTE *buf, int &len )
{
	len = 0;
	for ( int i=0; i<m_CfgInfo[m_bySendPos].byFENum; i++)
	{
		buf[len++] = 0xfe;
	}
	buf[len++] = 0x68;
	//地址位
	for ( int i=0; i<6; i++)
	{
		buf[len++] = m_bySlaveAddr[i];
	}
	buf[len++] = 0x68;
	buf[len++] = 0x01;	//读数据
	buf[len++] = 0x02;	//数据长度
	//2007为4个标识符
	buf[len++] = m_CfgInfo[m_bySendPos].byDI0 + 0x33;
	buf[len++] = m_CfgInfo[m_bySendPos].byDI1 + 0x33;
	buf[len++] = GetCs( buf + m_CfgInfo[m_bySendPos].byFENum , 12 );
	buf[len++] = 0x16;

	return TRUE;
}		/* -----  end of method CDlt645_1997::RequestReadData  ----- */
