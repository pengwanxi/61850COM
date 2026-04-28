/// \文件:	MonitoringPlatformXML.h
/// \概要:	XML 协议继承类声明
/// \作者:	李恩来，lel1132473561@sina.com
/// \版本:	V1.0
/// \时间:	2018-03-22

#ifndef MONITORINGPLATFORMXML_H_
#define MONITORINGPLATFORMXML_H_

#include <ctime>
#include <sys/time.h>
#include <queue>
#include <vector>
#include <map>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "CProtocol_XML.h"
#include "../../share/gDataType.h"
#include "../../librtdb/rdbObj.h"
#include "../../BayLayer/CPublicMethod.h"
#include "Clog.h"

using namespace std;

#define XMLMAX_AI_LEN			20000
#define XMLMAX_DI_LEN			20000
#define XMLMAX_PI_LEN			20000

#define MAX_LEN					1024000

struct Id_Validate
{
	BYTE bSequence[6];			//随机序号
	BYTE bResult[20];			//返回结果
	WORD wYkOrder;				//遥控下发顺序号
	DWORD wDevId;				//遥控/定值下发设备号
	BYTE bYkValue;				//遥控下发值
	WORD wFacNo;				//遥控/定值厂站值
	BYTE byDzZoneNo;			//定值组号
	BYTE byDZStartOrder;        //mod 2模式下的定值起始序号
};

typedef struct Xml
{
	BYTE bType[21];
	struct Id_Validate uId_Validate;
}XML;


class MonitoringPlatformXML : public CProtocol_XML
{
public:
		MonitoringPlatformXML();
		~MonitoringPlatformXML();
		virtual void ReadMapConfig(LPCSTR lpszFile);
		virtual void ReadConfigOtherFuc(BYTE nType, char *strLine);
		virtual BOOL WriteDIVal(WORD wSerialNo, WORD wNum, WORD wVal);
		virtual BOOL WriteAIVal(WORD wSerialNo, WORD wNum, float fVal);
		virtual BOOL WritePIVal(WORD wSerialNo, WORD wPnt, QWORD dwVal);
		virtual BOOL WriteSOEInfo(WORD wSerialNo, WORD wNum, WORD wVal, LONG lTime, WORD wMiSecond);




		virtual BOOL GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg = NULL);
		virtual BOOL ProcessProtocolBuf(BYTE *buf, int len);




		virtual BOOL Init(BYTE byLineNo);
		virtual BOOL InitRtuBase();
		virtual void RelayEchoProc(BYTE byCommand, WORD wIndex,	BYTE byResult);
		virtual void RelayDzEchoProc(BYTE byCommand, WORD wIndex, BYTE *byResult);

		void AddAnalogEvt_Xml( WORD wSerialNo , WORD wPnt, WORD wNum, float fVal);
		void AddDigitalEvt_Xml( WORD wSerialNo , WORD wPnt, WORD wNum, WORD wVal);
		void AddSOEInfo_Xml( WORD wSerialNo , WORD wPnt, WORD wNum, WORD wVal, LONG lTime, WORD wMiSecond);

		BOOL GetAnalogEvt_Xml(WORD &wSerialNo , WORD& wPnt, WORD &wNum, float& fVal );
		BOOL GetDigitalEvt_Xml( WORD &wSerialNo , WORD& wPnt, WORD &wNum, WORD& wVal);
		BOOL GetSOEInfo_Xml(WORD &wSerialNo , WORD *wPnt, WORD *wNum, WORD *wVal, void *pTime, WORD *wMiSecond);

        void GetXMLPathDataToPaymentInfo(xmlDocPtr xDoc,char * pPath,paymentInfo &info );

		WORD convertToHex(char * pData, BYTE size);
		map<int, char*> mapDevName;
		map<int, int> mapDevId;
		map<int, int> mapComId;
		map<WORD, BYTE> mapDevId_Com;
		map<int, char*> mapTime;
		int m_iAllCall_Update;									//是总招还是随机上送

		BYTE *m_byDIBuf;
		float *m_wAIBuf;
		QWORD *m_dwPIBuf ;

		char m_XmlBuf[MAX_LEN];
		char m_XmlDITimeOutBuf[MAX_LEN];						//后台未回复，重发buf
		BYTE *m_pTX_Buf;
		BYTE *m_pRX_Buf;
		XML m_xml;
		BYTE m_bSequence[5];
		BYTE m_bDITimeOutSequence_YX[5];

		WORD m_wDataIndex;
		WORD m_wDataIndex1;
		BOOL m_bStartBit;
		WORD m_wExcmd;
		WORD m_wYkFlag;
		WORD m_wDzFlag;
		WORD m_wCommand;
		WORD m_wPreDevId;
		BYTE m_byDataStyle;
		BYTE m_byStaticLineNo;
		BYTE m_byStaticDevAddr;
		int m_iDataFlag;
		char m_UnvarnishRtnBuf[100];
		BYTE m_byTimeFlag;
		BYTE m_byDITimeOut;										//遥信实时上传超时处理
		BOOL m_YkFlag;

		int m_iDzDataNum;
		BYTE m_byDzType[10];
		DWORD m_dwDataType;
		WORD m_wDzDevAddr;
		DWORD m_wDZRecvAddr;
		WORD m_wDzRecvName;
		DZ_DATA *m_pDzData;
		BYTE bySource_BusNo;
		WORD wSource_DevNo;
		WORD m_wMsgDevID;

		time_t m_t1, m_t2, m_t3, m_t4, m_YKTime, m_DZTime;
		time_t m_DevStateTime;									//上一次上传装置状态时间
		time_t m_VaryDevStateTime;								//上一次上传装置状态时间
		time_t m_YxValueTime;									//上一次实时上传遥信时间
		time_t m_YcValueTime;									//上一次实时上传遥测时间
		time_t m_YmValueTime;									//上一次实时上传遥脉时间
        paymentInfo m_payment_info ;
		BYTE m_quest_id_forUnvarnish[ 10 ];		
		Clog m_log;

		void ReSetDataState();

		void SetTransIndex(WORD wIndex, WORD wDataStyle);
		void GetRandom();
		int DealBusMsgInfo(PBUSMSG pBusMsg);
		BOOL ReSetState();
		BOOL SetTTimer(BYTE byTime, BYTE byState);
		void SetXMLTime(time_t *tTime, BYTE byState, BYTE byBeginT, BYTE byEndT);

		BYTE GetTimeFlag();
		BOOL LoadXMLMessage(BYTE *pBuf, int &len, PBUSMSG pBusMsg);
		int PreProcess(PBUSMSG pBusMsg);
		void RequestPacketUnvarnish(XML TempXml, int &XmlBufLen);
		int StartedProcess();
		BOOL SetCommand(WORD wCommand);
		WORD GetCommand();

		/* 获得装置通讯状态 */
		virtual BOOL GetDevCommState();
		BOOL IsCanSendDataFromSerialNo(WORD wSerialNo);
		virtual void TimerProc();
		void ProcessXMLTime();

		void CalcRes(BYTE dataType, xmlNodePtr xData_Node, void * datatable, WORD wStartNo);
		BOOL GetComState(BYTE byLineNo);
		BOOL GetDevCommState(BYTE byLineNo, BYTE byAddr);
		BOOL IsDevNameFillwithDight(char * szDevName);
		BOOL IsDevHaveData(WORD wSerialNo);
		BOOL IsTransData(char *strname);
		int GetRealVal(BYTE byType, WORD wPnt, void *Value);
		BOOL XmlCommandProc(BYTE * pRecvBuf, int nlen);
		BOOL ChangeDataProcessPacket(XML TempXml, int &XmlBufLen);
		BOOL LoadAllDataPacket(XML TempXml, int &XmlBufLen);


		//BOOL LoadDigitalDataPacket(XML TempXml, int &XmlBufLen);
		BOOL LoadDigitalDataPacket(XML TempXml, int &XmlBufLen, int nSize, int *dataindex);


		BOOL LoadAnalogDataPacket(XML TempXml, int &XmlBufLen);
		BOOL LoadPulseDataPacket(XML TempXml, int &XmlBufLen);
	//	BOOL LoadAnalogDataPacket_Batch(XML TempXml, int &XmlBufLen);
	//	BOOL LoadPulseDataPacket_Batch(XML TempXml, int &XmlBufLen);


		BOOL LoadPulseDataPacket_Batch(XML TempXml, int &XmlBufLen, int nSize);
		void Pack_BatchData(BYTE dataType, BYTE dataTransType, xmlNodePtr xData_Node);

		void Pack_BatchData(BYTE dataType, BYTE dataTransType, xmlNodePtr xData_Node, int nSize);

		BOOL LoadAnalogDataPacket_Batch(XML TempXml, int &XmlBufLen, int nSize);


		BOOL LoadCommStatusPacket(XML TempXml, int &XmlBufLen);
		BOOL LoadComEFramePacket(XML TempXml, int &XmlBufLen);
		BOOL LoadDIEFramePacket(XML TempXml, int &XmlBufLen);
		BOOL LoadSOEFramePacket(XML TempXml, int &XmlBufLen);
		BOOL LoadAIEFramePacket(XML TempXml, int &XmlBufLen);
		BOOL AllDataEchoPacket(XML TempXml, int &XmlBufLen, char *szTemp);
		BOOL SysClockConfirmPacket(XML TempXml, int &XmlBufLen);
		BOOL YkPacket(XML TempXml, int &XmlBufLen);
		WORD GetDevAddrFromDev_id(DWORD wDevId);
		WORD GetDevNameFromDev_id(DWORD wDevId);
		WORD GetDevAddrAndBusFromDev_id(DWORD wDevId);
		WORD GetDevidFromBusnoAndDevNo(BYTE byBusNo, WORD wDevAddr);
		BOOL YkRelayEchoFrame(XML TempXml, int &XmlBufLen, char *szTemp);
		BOOL DzPacket(XML TempXml, int &XmlBufLen, PBUSMSG pBusMsg);
		BOOL DzRelayEchoFrame(XML TempXml, int &XmlBufLen, char *szTemp);
		BOOL DzReadRelayEchoFrame(XML TempXml, int &XmlBufLen);
		BOOL DzReadRelayEchoFrame_new(XML TempXml, int &XmlBufLen, PBUSMSG pBusMsg);
		BOOL RequestPacket(XML TempXml, int &XmlBufLen);
		BOOL RequestPacketReply(XML TempXml, int &XmlBufLen, char *szTemp);
		BOOL NotifyPacket(XML TempXml, int &XmlBufLen);
		BOOL HistoryAckPacket(XML TempXml, int &XmlBufLen);
		BOOL LoadDigitalDataPacket(XML TempXml, int &XmlBufLen);


		BOOL AnalyticMessage(XML *TempXml, char *XmlBuf, int len);
		void sendUnvarnishCmd(paymentInfo &m_payment_info);
		xmlXPathObjectPtr GetNodeSet(xmlDocPtr doc, const xmlChar *xPath);

		void CharacterSplit(XML *TempXml, char *szBuf);
		int RelaySelectProc(WORD wStn, WORD wDevId, WORD wPnt, BYTE byStatus);
		int RelayExecuteProc(WORD wStn, WORD wDevId, WORD wPnt, BYTE byStatus);
		int RelayCancelProc(WORD wStn, WORD wDevId, WORD wPnt, BYTE byStatus);
		BOOL Yk_IsCanSend(void);
		void Yk_PreSet(BOOL bPreSetFlag);
		void Yk_RtnConfirm(BOOL bFlag);
		void SaveDzData(DWORD dwDataType, int iDataNum, DZ_DATA *pData);
		void SysClockProc(BYTE *pDataBuf);
public:
		int modflag;
		vector<int>trans_id;
		void ReadMapConfig_transid(LPCSTR lpszFile);
		int dataindex;
		int dataindex_zyx;
		int fenbaoflag;
		int last_ycbuf_issending;
		int last_yxbuf_issending;
		int last_ymbuf_issending;
private:
        time_t time_reboot_flag;
		int reboot_flag;
		BOOL CanSendChange;
			
};

#endif
