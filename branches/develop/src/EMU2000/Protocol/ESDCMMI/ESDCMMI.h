#if !defined(ESDCMMI_)
#define ESDCMMI_



//#include "../../share/CProtocol.h"
#include "CProtocol_ESDCMMI.h"

#define ESDCMMIMAX_AI_LEN (9216)
#define ESDCMMIMAX_PI_LEN (9216)
#define ESDCMMIMAX_DI_LEN (9216)

#define ESDCMMIMAX_MSG_LEN (1024)

typedef struct STRUCT_YK_DATA /*��У��ִ��������Ϣ*/	
{
	BYTE	m_byLineNo;
	BYTE	m_byAddress;
	BYTE	m_byPointNo;
	BYTE	m_byValid;		/*�Ƿ���Ч��0-��Ч��1-��Ч*/
	time_t	m_tm; 			/*д����ʱ��ʱ��*/		
	BYTE	m_byYkCmd;		/*ң������*/
	BYTE	m_byYkAction;	/*�ֺϣ�0x00-�֣�0xFF-��*/
	BYTE	m_byStatus;		//1.��ײ㷢�ͣ�2.�ײ㷵�أ�3.���Ϸ���
}ESDCMMI_YK_STRUCT;



class ESDCMMI  : public CProtocol_ESDCMMI
{
public:
	ESDCMMI();
	virtual ~ESDCMMI();

	BOOL FirstFlag;
	BYTE bySendType;
	
 	float m_fYCBuf[ESDCMMIMAX_AI_LEN];
    QWORD m_dwYMBuf[ESDCMMIMAX_PI_LEN];
	BYTE m_byYXbuf[ESDCMMIMAX_DI_LEN ] ;

	int m_wErrorTimer;
	int m_byPortStatus;

	BOOL m_bRetTime;
	BYTE curSendType_normal;
	BYTE curSendType_prior;
	WORD ESDCMMI_sendOrder;
	ESDCMMI_YK_STRUCT ESDCMMI_Yk_Data;
	
	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
	virtual BOOL Init( BYTE byLineNo );
	virtual BOOL InitRtuBase();
	virtual BOOL WriteAIVal(WORD wSerialNo ,WORD wPnt, float wVal) ;
	virtual BOOL WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal) ;
	virtual BOOL WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD  dwVal) ;
	virtual BOOL WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond) ;


	//���װ��ͨѶ״̬ by	zhanghg 2014-9-23
	virtual BOOL GetDevCommState( ) ;
	virtual void TimerProc() ;

private:
	void ESDCMMI_addSyncWord( BYTE *pBuf );
	void ESDCMMI_addControlWord(BYTE *pBuf,BYTE frameKind,BYTE infoWords);
	BOOL ESDCMMI_assembleYxFrame( BYTE * pBuf , int &len ); 
	BOOL ESDCMMI_assembleYcFrame( BYTE * pBuf , int &len ); 
	BOOL ESDCMMI_assembleYmFrame( BYTE * pBuf , int &len ); 
	BOOL ESDCMMI_assembleSoeFrame( BYTE * pBuf , int &len ); 
	BOOL ESDCMMI_assembleFailFrame( BYTE * pBuf , int &len ); 
	WORD ESDCMMI_addPriorWord( BYTE *pBuf , WORD nByte );
	void ESDCMMI_InsertWord(BYTE *pBuf,BYTE func_high,BYTE func_low,BYTE module_actionNo,BYTE byLineNo,BYTE module_addr,BYTE module_ykNo);
	BYTE ESDCMMI_assembleYkRevise(BYTE* pBuf);
	void ESDCMMI_assemSubRetTime(BYTE* pBuf);
	BOOL ESDCMMI_GetYxWordFromYxChP(WORD YxOrder,BYTE* pBuf);
	BOOL ESDCMMI_generateYxDWord(WORD wSendOrder,DWORD * pYxDWord,BYTE YxNum);
	BOOL ESDCMMI_insertYxChp(WORD YxChp,BYTE* pBuf);
	BOOL ESDCMMI_InsertFail(BYTE byLineNo,BYTE ModuleNoAddOne,BYTE *pBuf);
	BOOL ESDCMMI_isSyncWord( BYTE * pWord );
	BOOL ESDCMMI_setSubstationTime(BYTE* pBuf);
	BOOL ESDCMMI_handleYkCmd(BYTE* pBuf,BYTE cmdType);

	BOOL DealBusMsgInfo( PBUSMSG pBusMsg ); 

};

#endif // !defined(AFX_PROTOCOL_ESDCMMI_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)

