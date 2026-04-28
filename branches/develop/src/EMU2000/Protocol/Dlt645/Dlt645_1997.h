/*
 * =====================================================================================
 *
 *       Filename:  Dlt645_1997.h
 *
 *    Description:  dlt645 1997版本协议 
 *
 *        Version:  1.0
 *        Created:  2014年11月12日 10时42分02秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (), 
 *   Organization:  
 *
 *		  history:
 *
 * =====================================================================================
 */

#ifndef  DLT645_1997_INC
#define  DLT645_1997_INC

#include "Dlt645_2007.h"

/*
 * =====================================================================================
 *        Class:  CDlt645_1997
 *  Description:  
 * =====================================================================================
 */
class CDlt645_1997 : public CDlt645_2007
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CDlt645_1997 ();                             /* constructor      */
		~CDlt645_1997 ();                            /* destructor       */


		//请求读电表数据
		virtual BOOL RequestReadData( BYTE *buf, int &len );
		//处理遥测数据
		virtual BOOL ProcessYcData( const BYTE *buf, int len );
		//处理遥脉数据
		virtual BOOL ProcessYmData( const BYTE *buf, int len );

	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	private:
		/* ====================  DATA MEMBERS  ======================================= */

}; /* -----  end of class CDlt645_1997  ----- */

#endif   /* ----- #ifndef DLT645_1997_INC  ----- */
