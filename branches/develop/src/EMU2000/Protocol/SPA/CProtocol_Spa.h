/// \文件:	CProtocol_SPA.h
/// \概要:	ABB Spa协议头文件
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2017-09-25

#ifndef _CPROTOCOL_SPA_H
#define _CPROTOCOL_SPA_H

#include <time.h>
#include <sys/time.h>
#include "../../share/CProtocol.h"
#include "../../share/CMethod.h"

#define SPA_FCB_0			0xdf			/*FCB位0 1101 1111 */
#define SPA_FCB_1			0x20			/*FCB位1 0010 0000 */

#define	SPA_YX_DATATYPE	1				/* 遥信数据类型 */
#define	SPA_YC_DATATYPE	2				/* 遥测数据类型 */
#define	SPA_YM_DATATYPE	3				/* 遥脉数据类型 */
#define	SPA_YK_DATATYPE	4				/* 遥控数据类型 */
#define	SPA_DD_DATATYPE	5				/* 定值数据类型 */

// --------------------------------------------------------
/// \概要:	ABB Spa协议类，继承CProtocol 类
// --------------------------------------------------------
class CProtocol_SPA : public CProtocol
{
	public:
		CProtocol_SPA();
		~CProtocol_SPA();
		virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);
		virtual BOOL Init(BYTE byLineNo);
		/*获取校验和*/
		virtual UINT GetCs(BYTE *pBuf, int len);
		virtual BYTE HEXTOASCII(BYTE temp);
		/*广播报文*/
		virtual BOOL BroadCast(BYTE *buf, int &len);
		//判断报文有效性
		virtual BOOL WhetherBufValue(BYTE *buf, int &len, int &pos);
	protected:
		BOOL GetDevData();
		BOOL ProcessFileData(CProfile &profile);
		BOOL CreateModule(int iModule, int iSerialNo, WORD wAddr, char *sName, char *sTemplatePath);
};

#endif
