/*
 * =====================================================================================
 *
 *       Filename:  CDataTrans.h
 *
 *    Description:  ����ESD�Զ�����ʷ�����ϴ� 
 *
 *        Version:  1.0
 *        Created:  2015��06��09�� 18ʱ28��24��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp
 *   Organization:  
 *
 *		  history:	Time								Author			version			Description
 *					2015��06��09�� 18ʱ29��13��         mengqp			1.0				created
 *
 * =====================================================================================
 */

#ifndef  CDATATRANS_INC
#define  CDATATRANS_INC

/* #####   HEADER FILE INCLUDES   ################################################### */
#include "CProtocol_DataTrans.h"

/* #####   TYPE DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ######################### */
/* #####   DATA TYPES  -  LOCAL TO THIS SOURCE FILE   ############################### */



/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ################################ */

/* #####   PROTOTYPES  -  LOCAL TO THIS SOURCE FILE   ############################### */

/*
 * =====================================================================================
 *        Class:  CDataTrans
 *  Description:  ���ݴ���Э��
 * =====================================================================================
 */
class CDataTrans : public CProtocol_DataTrans
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CDataTrans ();                             /* constructor      */
		virtual ~CDataTrans ();                            /* destructor       */

		/* ====================  METHODS       ======================================= */
		//ʱ�䴦������
		virtual void    TimerProc( void );
		//��ʼ��Э������
		virtual BOOL Init( BYTE byLineNo );
		//��ȡЭ�����ݻ���
		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
		//�����յ������ݻ��� 
		virtual BOOL ProcessProtocolBuf( BYTE * pBuf , int len ) ;

		virtual  int  GetRealVal(BYTE byType, WORD wPnt, void *v);
		virtual BOOL WriteAIVal(WORD wSerialNo ,WORD wPnt, float fVal) ;
		virtual BOOL WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal) ;
		virtual BOOL WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal) ;
		virtual BOOL WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond) ;		

	protected:
		/* ====================  DATA MEMBERS  ======================================= */

	private:
		//Э������� 
		void ProtocolErrorProc ( void );
		//�������ձ��� 
		BOOL ProcessRecvBuf ( BYTE *pBuf, int len );
		//���ý��ղ��� 
		void SetRecvParam ( void  );
		//��ȡ���ͱ��� 
		BOOL GetSendBuf ( BYTE *buf, int &len );
		//��������õ�Э��״̬
		BOOL GetProtocolState ( void );
		//�Ƿ���Ҫ�ط� 
		BOOL IsResend ( void ) const;
		//�Ƿ��б仯YX
		BOOL IsHaveChangeYX ( void  ) const;
		//�Ƿ���ȫ���������� 
		BOOL IsHaveAll ( void  ) const;
		//�Ƿ��������������� 
		BOOL IsHaveHeart ( void ) const;
		//Э��ʱ�䴦��
		void TimeToProtocol( void  );
		//�Ƿ�ʱ�䷢��ȫ������ 
		BOOL TimeToAll ( void );
		//�Ƿ�����ʱ��
		BOOL TimeToHeartbeat ( void );
		//��ȡ��Ӧ���͵�����
		BOOL GetSendTypeBuf ( BYTE *buf, int &len);
		//��ȡ�仯YX���� 
		BOOL GetChangeYXBuf ( BYTE *buf, int &len );
		//��֯YX���� 
		BOOL PackChangeYXBuf ( BYTE *buf, int &len );
		//��ȡȫ������
		BOOL GetAllDataBuf ( BYTE *buf, int &len );
		//��ȡyc���ݰ� 
		BOOL GetYCDataBuf ( BYTE *buf, int &len );
		//��֯YC���� 
		BOOL PackYCBuf ( BYTE *buf, int &len );
		//��ȡyx���ݰ� 
		BOOL GetYXDataBuf ( BYTE *buf, int &len );
		//��֯YX���� 
		BOOL PackYXBuf ( BYTE *buf, int &len );
		//��ȡyM���ݰ� 
		BOOL GetYMDataBuf ( BYTE *buf, int &len );
		//��֯YM���� 
		BOOL PackYMBuf ( BYTE *buf, int &len );
		//��֯��������
		BOOL GetHeartBuf ( BYTE *buf, int &len );
		//��ȡ�ط�����
		void GetResendBuf ( BYTE *buf, int &len );
		//�����ط�����
		void SaveResendBuf ( BYTE *buf, int len, BOOL byValid );
		//���÷��Ͳ���
		void SetSendParam ( BOOL bIsSendValid );
		//��ȡ����ģ����Ϣ
		BOOL ReadCfgInfo ( void );
		//��ȡ�����Ϣ 
		void ReadCfgMapInfo ( char *szPath );
		//������ģ�������������Ϣ
		BOOL ReadCfgOtherInfo ( char *szPath );
		//��ʼ��Э��״̬
		void InitProtocol ( void );
		//��ʼ��Э�����
		void InitProtocolState ( void );
		//��ʼ��ת����Ϣ
		void InitProtocolTransTab ( void );
		//��ʼ��Э������
		void InitProtocolData ( void );

		/* ====================  DATA MEMBERS  ======================================= */
		float    m_fYcBuf[DATATRANS_MAX_YC_NUM];
		QWORD   m_dwYmBuf[DATATRANS_MAX_YM_NUM];
		BYTE	m_byYxBuf[DATATRANS_MAX_YX_NUM] ;

		WORD m_wAllDataInterval;               /* ���������ϴ���� s */
		
		//�ط����
		BYTE m_byResendCount;
		int m_iResendLen;
		BYTE m_byResendBuf[DATATRANS_MAX_BUF_LEN];

		//ȫ��������λ��
		WORD m_wAllDataPos;

		//״̬���
		BOOL m_bSending;
		BYTE m_bySendCount;
		
		//�ȴ�ʱ��
		DWORD m_LocalAddTime;
		DWORD m_LocalSumTime;
		DWORD m_LocalHeartbeatTime;
		DWORD m_LocalHeartbeatAddTime;

		BYTE m_byTimerCount;

}; /* -----  end of class CDataTrans  ----- */

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ############################ */

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ##################### */


#endif   /* ----- #ifndef CDATATRANS_INC  ----- */
