
#ifndef CPROTOCOL_IEC103_H
#define CPROTOCOL_IEC103_H

#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"
#include <time.h>
#include <sys/time.h>


#define	IEC103_FCB_1			0x20			/* FCB位1 0010 0000 */
#define	IEC103_FCB_0			0xdf			/* FCB位0 1101 1111 */

#define	IEC103_YX_DATATYPE		1				/* 遥信数据类型 */
#define	IEC103_YC_DATATYPE		2				/* 遥测数据类型 */
#define	IEC103_YM_DATATYPE		3				/* 遥脉数据类型 */
#define	IEC103_YK_DATATYPE		4				/* 遥控数据类型 */
#define	IEC103_DD_DATATYPE		5				/* 定值数据类型 */


class CProtocol_IEC103 : public CProtocol
{
    public:
        CProtocol_IEC103();
        virtual ~CProtocol_IEC103();
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL );
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len );
		virtual BOOL Init( BYTE byLineNo  ) ;
		//获取校验和
		virtual BYTE GetCs( BYTE * pBuf , int len );
		virtual BOOL BroadCast( BYTE * buf , int &len ) ;
		virtual void TimerProc(){ return;  } 
		//判断报文有效性
		virtual BOOL WhetherBufValue(BYTE *buf, int &len, int &pos );

	protected:
		BOOL GetDevData( ) ;
		//改变校验位
		BYTE ChangeFcb(BYTE byCtlBit, BOOL &bFCB);
	protected:
		BOOL ProcessFileData( CProfile &profile );
		BOOL CreateModule( int iModule , int iSerialNo , WORD iAddr , char * sName , char * stplatePath ) ;

};


#endif // CPROTOCOL_IEC103_H

