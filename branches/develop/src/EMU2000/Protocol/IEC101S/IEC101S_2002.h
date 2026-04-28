/*
 * =====================================================================================
 *
 *       Filename:  IEC101S_2002.h
 *
 *    Description:  IEC101��վ 2002 ��
 *
 *        Version:  1.0
 *        Created:  2014��11��18�� 13ʱ30��03��
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


#ifndef  IEC101S_2002_INC
#define  IEC101S_2002_INC


#include "CProtocol_IEC101S.h"

#define		IEC101S_2002_MAX_YC_NUM      4096		/* ���ң������ */
#define		IEC101S_2002_MAX_YX_NUM      8192		/* ���ң������ */
#define		IEC101S_2002_MAX_YM_NUM      1024		/* ���ң������ */


#define		IEC101S_2002_TOTAL_TYPE		 1			/* �������� */
#define		IEC101S_2002_CHANGE_TYPE	 2			/* �仯�������� */
#define		IEC101S_2002_SOE_TYPE		 3			/* SOE ���� */
#define		IEC101S_2002_YKSINGLE_TYPE   4			/* ����ң������ */
#define		IEC101S_2002_YKDOUBLE_TYPE   5			/* ˫��ң������ */

#define		IEC101S_2002_MAX_SEND_COUNT	 3			/* ����ͼ��� */
#define		IEC101S_2002_MAX_RESEND_COUNT	 3		/* ����ط����� */

/*
 * =====================================================================================
 *        Class:  CIEC101S_2002
 *  Description�� 2002�� 101��վ�� 
 * =====================================================================================
 */
class CIEC101S_2002 : public CProtocol_IEC101S
{
	public:
		/* ====================  LIFECYCLE     ======================================= */
		CIEC101S_2002 ();                             /* constructor      */
		~CIEC101S_2002 ();                            /* destructor       */

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
		/* ����Э�鱨�� */
		//�������տ����ֹ���
		virtual BOOL ProcessCtlBit( BYTE c );
		//���ý��ղ���
		void SetRecvParam( void );
		//����ͷΪ0x10�ı���
		BOOL ProcessHead10Buf( const BYTE *pBuf, int len );
		//����ң�ز���
		void SetYkParam( BYTE byType, BYTE byCot, WORD wStn, WORD wPnt, BYTE byStatus );
		//��ȡң�ز���
		BOOL IsYkParamTrue( BYTE byType, BYTE byCot, WORD wStn, WORD wPnt, BYTE byStatus ) const; 
		//����ң������
		BOOL ProcessYkBuf( const BYTE *buf, int len );
		//������������
		BOOL ProcessTotalCallBuf( const BYTE *buf, int len );
		//����ң������
		BOOL ProcessYMCallBuf( const BYTE *buf, int len );
		//������ʱ����
		BOOL ProcessTimeSyncBuf( const BYTE *buf, int len );
		//����ͷΪ0x68�ı���
		BOOL ProcessHead68Buf( const BYTE *pBuf, int len );
		//�������ձ���
		BOOL ProcessRecvBuf( const BYTE *pBuf, int len );

		/* ��ȡЭ�鱨�� */
		//����������Ϣ
		BOOL DealBusMsg( PBUSMSG pBusMsg );
		//����68ͷβ֡
		int Add68HeadAndTail( const BYTE *byAsduBuf, int iAsduLen, BYTE *buf );
		//��ȡ��·״̬����
		BOOL GetLinkStatusBuf( BYTE *buf, int &len );
		//��ȡ��·��λȷ�ϱ���
		BOOL GetReconitionBuf( BYTE *buf, int &len );
		//�û����� ȷ��
		BOOL GetUserDataBuf( BYTE *buf, int &len );
		//û�����ݵĻظ�����
		BOOL GetNoneDataBuf( BYTE *buf, int &len );
		//��ȡ���ٻ�ȷ�ϱ���
		BOOL GetTotalCallRecoBuf( BYTE *buf, int &len, BYTE byCot );
		//��ȡ��ʱȷ�ϱ���
		BOOL GetTimeSyncRecoBuf( BYTE *buf, int &len, BYTE byCot );
		//��ȡ�ۼ���ȷ�ϱ���
		BOOL GetCallYmRecoBuf( BYTE *buf, int &len, BYTE byCot );

		//��ȡ���ٲ���ֵ ��һ��ֵ
		BOOL Get_M_ME_NA_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ�仯������ ��һ��ֵ
		BOOL Get_M_ME_NA_1_ChangeFrame( BYTE *buf, int &len );
		//��ȡ����ֵ ��һ��ֵ
		BOOL Get_M_ME_NA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ��ͻ�������� ��һ��ֵ
		BOOL Get_M_ME_TA_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ�����ֵ ��һ��ֵ
		BOOL Get_M_ME_TA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ���ٲ���ֵ ��Ȼ�ֵ
		BOOL Get_M_ME_NB_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ�仯������ ��Ȼ�ֵ
		BOOL Get_M_ME_NB_1_ChangeFrame( BYTE *buf, int &len );
		//��ȡ����ֵ ��Ȼ�ֵ
		BOOL Get_M_ME_NB_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ��ͻ�������� ��Ȼ�ֵ
		BOOL Get_M_ME_TB_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ�����ֵ ��Ȼ�ֵ
		BOOL Get_M_ME_TB_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ���ٲ���ֵ �̸�����
		BOOL Get_M_ME_NC_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ�仯������ �̸�����
		BOOL Get_M_ME_NC_1_ChangeFrame( BYTE *buf, int &len );
		//��ȡ����ֵ �̸�����
		BOOL Get_M_ME_NC_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ��ͻ�������� �̸�����
		BOOL Get_M_ME_TC_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ�����ֵ �̸�����
		BOOL Get_M_ME_TC_1_Frame( BYTE *buf, int &len, int iFlag );

		// ����ֵ�� ����Ʒ�������ʵĹ�һ��ֵ����
		BOOL Get_M_ME_ND_1_TotalFrame ( BYTE *buf, int &len );
		// ����ֵ�� ����Ʒ�������ʵĹ�һ��ֵ�仯 
		BOOL Get_M_ME_ND_1_ChangeFrame ( BYTE *buf, int &len );
		// ����ֵ�� ����Ʒ�������ʵĹ�һ��ֵ 
		BOOL Get_M_ME_ND_1_Frame ( BYTE *buf, int &len, int iFlag );

		//��ȡ��CP56Time2aͻ�������� ��һ��ֵ
		BOOL Get_M_ME_TD_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��CP56Time2a����ֵ ��һ��ֵ
		BOOL Get_M_ME_TD_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��CP56Time2aͻ�������� ��Ȼ�ֵ
		BOOL Get_M_ME_TE_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��CP56Time2a����ֵ ��Ȼ�ֵ
		BOOL Get_M_ME_TE_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��CP56Time2aͻ�������� �̸�����
		BOOL Get_M_ME_TF_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��CP56Time2a����ֵ �̸�����
		BOOL Get_M_ME_TF_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ����ʱ������ٵ�����Ϣ
		BOOL Get_M_SP_NA_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ����ʱ��ı仯������Ϣ
		BOOL Get_M_SP_NA_1_ChangeFrame( BYTE *buf, int &len );
		//����ʱ��ĵ�����Ϣ
		BOOL Get_M_SP_NA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ���ͻ�䵥����Ϣ
		BOOL Get_M_SP_TA_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ��ĵ�����Ϣ
		BOOL Get_M_SP_TA_1_Frame( BYTE *buf, int &len, int iFlag );


		//��ȡ����ʱ�������˫����Ϣ
		BOOL Get_M_DP_NA_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ����ʱ��ı仯˫����Ϣ
		BOOL Get_M_DP_NA_1_ChangeFrame( BYTE *buf, int &len );
		//����ʱ���˫����Ϣ
		BOOL Get_M_DP_NA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ���ͻ��˫����Ϣ
		BOOL Get_M_DP_TA_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ���˫����Ϣ
		BOOL Get_M_DP_TA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ��CP56Time2a��ͻ�䵥����Ϣ
		BOOL Get_M_SP_TB_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ��CP56Time2a�ĵ�����Ϣ
		BOOL Get_M_SP_TB_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ��CP56Time2a��ͻ��˫����Ϣ
		BOOL Get_M_DP_TB_1_SoeFrame( BYTE *buf, int &len );
		//��ȡ��ʱ��CP56Time2a��˫����Ϣ
		BOOL Get_M_DP_TB_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ����ʱ��������ۼ���
		BOOL Get_M_IT_NA_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ����ʱ��ı仯�ۻ���
		BOOL Get_M_IT_NA_1_ChangeFrame( BYTE *buf, int &len );
		//����ʱ����ۻ���
		BOOL Get_M_IT_NA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��ʱ��������ۼ���
		BOOL Get_M_IT_TA_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ��ʱ���ͻ���ۻ���
		BOOL Get_M_IT_TA_1_SoeFrame( BYTE *buf, int &len );
		//��ʱ����ۻ���
		BOOL Get_M_IT_TA_1_Frame( BYTE *buf, int &len, int iFlag );

		//��ȡ��CP56Time2aʱ��������ۼ���
		BOOL Get_M_IT_TB_1_TotalFrame( BYTE *buf, int &len );
		//��ȡ��CP56Time2aʱ���ͻ���ۻ���
		BOOL Get_M_IT_TB_1_SoeFrame( BYTE *buf, int &len );
		//��ʱ��CP56Time2a���ۻ���
		BOOL Get_M_IT_TB_1_Frame( BYTE *buf, int &len, int iFlag );

		//ң�ط��ر���
		BOOL GetYkRtnDataFrame ( BYTE *buf, int &len, int byYkRtnType );

		//�鿴�Ƿ��б仯ң��
		BOOL IsHaveChangeYcData( void );
		//��ȡң�����ݱ���
		BOOL GetChangeYcData( BYTE *buf, int &len );
		//��ȡ��������
		BOOL GetLevel2Data( BYTE *buf, int &len );
		//�鿴�Ƿ���1������
		BOOL IsHaveLevel1Data( void );
		//�鿴�Ƿ�����������
		BOOL IsHaveSpecialData( void ) const;
		//����ң������
		BOOL GetTotalYxData( BYTE *buf, int &len );
		//����ң������
		BOOL GetTotalYcData( BYTE *buf, int &len );
		//����ң������
		BOOL GetTotalYmData( BYTE *buf, int &len );
		//��ȡ��������
		BOOL GetSpecialData( BYTE *buf, int &len );
		//�鿴�Ƿ��б仯ң��
		BOOL IsHaveChangeYxData( void );
		//�Ƿ���ң��SOE
		BOOL IsHaveYxSoeData ( void ) const;
		//��ȡң�����ݱ���
		BOOL GetChangeYxData( BYTE *buf, int &len );
		//��ȡң��soe����
		BOOL GetSoeYxData ( BYTE *buf, int &len );
		//��ȡһ������
		BOOL GetLevel1Data( BYTE *buf, int &len );
		//ң�ط�����Ϣ�Ƿ���Ч
		BOOL IsYkRtnBusMsgValid ( PBUSMSG pBusMsg, DWORD dwYkType );
		//��ȡң�ط��ر���
		BOOL GetYkRtnData( BYTE *buf, int &len );
		//��ȡ���ͱ���
		BOOL GetSendBuf( BYTE *buf, int &len );
		//���÷��Ͳ���
		void SetSendParam( void );
		/* ��ʼ������ */
		//��ȡģ����Ϣ
		BOOL ReadCfgTemplate ( void );
		// ��ȡ�����ļ���Ϣ
		BOOL ReadCfgInfo ( void );
		// ��ȡ�����Ϣ
		BOOL ReadCfgMap ( void );
		// ��ʼ��Э�����
		void InitProtocolState ( void );
		// ��ʼ�������Ϣ
		void InitProtocolTransTab( void );
		// ��ʼ��������Ϣ
		void InitProtocolData( void );
		// ��ʼ��Э��״̬
		void InitProtocol( void );

	protected:
		/* ====================  DATA MEMBERS  ======================================= */

		//�������ݵ���ʼ��ַ
		WORD m_wYcStartAddr;
		WORD m_wYxStartAddr;
		WORD m_wYkStartAddr;
		WORD m_wYmStartAddr;
		WORD m_wComStateAddr;

		//�ɱ仯�����ݳ���
		BYTE m_byCotLen;
		BYTE m_byAddrLen;
		BYTE m_byInfoAddrLen;

		//������������
		BYTE m_byTotalCallYx;
		BYTE m_byTotalCallYc;
		BYTE m_byTotalCallYm;

		//�仯��������
		BYTE m_byChangeYx;
		BYTE m_bySoeYx;
		BYTE m_byChangeYc;
		BYTE m_byYkType;

	private:
		/* ====================  DATA MEMBERS  ======================================= */

		//ң�ز������
		BYTE m_byYKAsduType;
		BYTE m_byYkCot;
		WORD m_wYkStn;
		WORD m_wYkPnt;
		BYTE m_byYkStatus;

		WORD m_wDataIndex;		//���ݲ���

		BOOL m_bLinkStatus;		//����״̬
		BOOL m_bDataInit;		//�����Ƿ��ʼ��
		BOOL m_bSending;        //����״̬
		BOOL m_bReSending;      //�ط�״̬

		BYTE m_bySendCount;     //���ͼ���
		BYTE m_byRecvCount;     //���ռ���
		BYTE m_byResendCount;	//�ط�����

		BYTE m_byResendBuf[256];
		BYTE m_byResendLen;

		float    m_fYcBuf[IEC101S_2002_MAX_YC_NUM];
		QWORD   m_dwYmBuf[IEC101S_2002_MAX_YM_NUM];
		BYTE	m_byYxBuf[IEC101S_2002_MAX_YX_NUM] ;

}; /* -----  end of class CIEC101S_2002  ----- */



#endif   /* ----- #ifndef IEC101S_2002_INC  ----- */
