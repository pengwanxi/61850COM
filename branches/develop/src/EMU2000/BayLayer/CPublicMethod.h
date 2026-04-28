/*
 * =====================================================================================
 *
 *       Filename:  publicmethod.h
 *
 *    Description:   ͨѶ�������ڴ湲���ռ�֮���ṩ��������
 *
 *        Version:  1.0
 *        Created:  2014��07��17�� 08ʱ56��50��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp (),
 *        Company:  esdtek
 *
 * =====================================================================================
 */

#ifndef  _PUBLICMETHOD_H__
#define  _PUBLICMETHOD_H__


#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<sys/time.h>

#include	"../librtdb/rdbObj.h"
#include	"../share/CMethod.h"
#include	"../share/Rtu.h"
#include	"BusManger.h"

//˫������״̬
/*����״̬*/
#define        STATUS_SLAVE                1 /*�ӻ�*/
#define        STATUS_MASTER               0 /*����*/

class CPublicMethod : public CMethod
{/*{{{*/
	public:

		/* ====================  LIFECYCLE     ======================================= */
		CPublicMethod ();                             /* constructor      */
		virtual ~CPublicMethod ();                            /* destructor       */

		/* ====================  MUTATORS      ======================================= */

		/* ====================  real method      ======================================= */
		/* ====================  virtual method      ======================================= */
		/* ====================  public  method      ======================================= */
		// ͨ��wSerialNo ��ȡ�����ߺź���Ӧ��ַ
		virtual  BOOL GetBusLineAndAddr(WORD wSerialNum, BYTE &byBusNo, WORD &wDevAddr, char *pDevName /*= NULL*/);
		// ͨ�����ߺź�װ�õ�ַ�Ż�ȡ ������
		virtual int GetSerialNo ( BYTE byBusNo, WORD wDevAddr );
		/*==================================ң�⴦��=========================================*/
		virtual	void YcUpdate ( WORD SerialNo, YC_DATA YcData[], UINT YcNum );
		//���ô���һ��ң��
		virtual void SetYcData ( WORD wSerialNo, WORD wPnt, float fVal );
		//���ô���һ����ʱ���ң��
		virtual	void SetYcDataWithTime ( WORD wSerialNo, WORD wPnt, float fVal, TIMEDATA *pTime  );
		// ��ȡ���е�ң��ֵ
		virtual void ReadAllYcData ( float *pData );
		//��ȡ�����ڴ��е�һ��ң��Դ��ֵ
		virtual DWORD ReadYcData ( WORD wSerialNO, WORD wPnt );
		/*==================================ң�Ŵ���=========================================*/
		virtual void YxUpdate (  WORD SerialNo, YX_DATA YxData[], UINT YxNum);
		//���ô���һ��ң��
		virtual	void SetYxData ( WORD wSerialNo, WORD wPnt, BYTE byVal );
		////���ô���һ��ң��ֵ��ֵ����Ϊ0 1 2 3 ...��
		virtual	void SetYxVariousData(WORD wSerialNo, WORD wPnt, WORD byVal);
		//���ô���һ����ʱ���ң��
		virtual	void SetYxDataWithTime ( WORD wSerialNo, WORD wPnt, BYTE byVal, TIMEDATA *pTime );
		//��ȡ���е�ң��ֵ
		virtual void ReadAllYxData ( BYTE *pData ) ;
		//��ȡ�����ڴ��е�һ��ң��Դ��ֵ
		virtual int ReadYxData ( WORD wSerialNO, WORD wPnt, WORD *pwVal);
		/*==================================ң������=========================================*/
		virtual void YmUpDate ( WORD SerialNo, YM_DATA YmData[], UINT YmNum );
		//���ô���һ��ң��
		virtual	void SetYmData ( WORD wSerialNO, WORD wPnt, float fVal );
		virtual	void SetYmData ( WORD wSerialNO, WORD wPnt, double dVal );
		virtual	void SetYmData ( WORD wSerialNO, WORD wPnt, QWORD dVal );
		//��ȡһ��ң������
		virtual BOOL GetYmData( WORD wSerialNo, WORD wPnt, QWORD &dwVal );
		//��ȡ���е��ֵ
		virtual void ReadAllYmData ( QWORD *pdwData );
		//��ȡ�����ڴ��е�һ��ң��Դ��ֵ
		virtual int ReadYmData ( WORD wSerialNO, WORD wPnt, QWORD *pdwVal );
		/*==================================ң�ش���=========================================*/
		virtual	void SetVarsListData ( WORD wSerialNo, VARSLIST varslist );
		//ң��ѡ�񷵻�
		virtual void SetYkSelRtn (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal );
		//ң��ִ�з���
		virtual void SetYkExeRtn (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal );
		//ң��ȡ������
		virtual void SetYkCancelRtn (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal );
		//ң��ѡ��
		virtual void SetYkSel (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal );
		//ң��ִ��
		virtual void SetYkExe (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal );
		//ң��ȡ��
		virtual void SetYkCancel (const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal );
		/*==================================��ֵ����=========================================*/
		//�ٻ���ֵ��
		virtual void SetDzZoneCall(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//�ٻ���ֵ������
		virtual void SetDzZoneCallRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ���л�Ԥ��
		virtual void SetDzZoneSwitchPreset(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ���л�Ԥ�÷���
		virtual void SetDzZoneSwitchPresetRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ���л�ִ��
		virtual void SetDzZoneSwitchExct(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ���л�ִ�з���
		virtual void SetDzZoneSwitchExctRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ���л�ȡ��
		virtual void SetDzZoneSwitchCancel(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ���л�ȡ������
		virtual void SetDzZoneSwitchCancelRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo );
		//��ֵ����
		virtual void SetDzZoneError(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo );
		//�ٻ���ֵ
		virtual void SetDzCall(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);

		//�ٻ���ֵ--ͨ����ʼ���
		virtual void SetDzCall_By_StartOrder(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDZStartOrder, DZ_DATA DzData[], int iDzDataNum);

		//�ٻ���ֵ����
		virtual void SetDzCallRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵдԤ��
		virtual void SetDzWritePreset(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵдԤ�÷���
		virtual void SetDzWritePresetRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵдִ��
		virtual void SetDzWriteExct(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵдִ�з���
		virtual void SetDzWriteExctRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵдȡ��;
		virtual void SetDzWriteCancel(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵдȡ������
		virtual void SetDzWriteCancelRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);
		//��ֵ��
		virtual void SetDzError(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum);

		virtual void Unvarnished(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, char * pCmd, int len, int iFlag);

		virtual void UnvarnishedRtn(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, char * pCmd, int len, int iFlag);

		virtual BYTE GetBusLineProtocolType(BYTE byLineNo);

		//�ر������׽���
		virtual void CloseSocket( BYTE byBusLine ) ;
		//�򿪱������׽���
		virtual void OpenSocket ( BYTE byBusLine );
		virtual BOOL IsPortValid( ) ;
		virtual BOOL IsSoeTime ( UINT uiMilSec, UINT uiSec, UINT uiMin, UINT uiHour, UINT uiDay, UINT uiMonth, UINT uiYear) const;

		//���װ��ͨѶ״̬
		virtual BOOL GetDevCommState( BYTE byLineNo , WORD wDevNo ) ;
		virtual BOOL GetDevCommState( WORD wSerialNo ) ;
		//�������״̬
		virtual BOOL GetCommState( BYTE byLineNo ) ;

		//����ܹ��ж���������
		virtual BYTE GetToTalBusNum( ) ;
		//���ÿ�������ж���װ��
		virtual BYTE GetDevNum( BYTE byBusNo );
		//ͨ�����ߺź�ģ��ŵõ�װ�õ�ַ
		virtual WORD GetAddrByLineNoAndModuleNo ( BYTE byLineNo, WORD wModuleNo );
		/*lel*/
		//ͨ�����ߺź�ģ��ŵõ�װ�ÿ����ַ(����)
		virtual char* GetDevNameByLineNoAndModuleNo ( BYTE byLineNo, WORD wModuleNo );
		/*end*/

		//��ȡ���вɼ����ߵ�װ��������������ת��������������װ�ã�
		WORD GetGatherDevCount( ) ;
		//�������вɼ����ߵ�װ������
		BOOL SetGatherDevCount( WORD wCount ) ;
		//��ȡ�����ɼ����ߵ�װ������
		BYTE GetSingleGatherDevCount( BYTE byBusNo , BYTE byDevIndex = 0 , WORD * pAddr = NULL ) ;
		//��ȡ����
		virtual PBUSMANAGER GetBus( BYTE byIndex ) ;
	private:
		// ң�ش�������
		void SetYkDeal ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal, int iFlag );
		//��ֵ����
		void SetDzDeal (  const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr,BYTE byDzZoneNo, DZ_DATA DzData[], int iDzDataNum, int iFlag );

		void SetDzDeal_By_StartOrder(const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDZStartOrder, DZ_DATA DzData[], int iDzDataNum, int iFlag);

		//��ֵ������
		void SetDzZoneDeal ( const CProtocol *pProtocol, BYTE byBusNo, WORD wDevAddr, BYTE byDzZoneNo, int iFlag );
		//�ж��Ƿ�������
		BOOL IsLeapYear( UINT uiYear ) const;
		BOOL ProcessDDB( const CProtocol *pProtocol , BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal , int iFlag );
		void  SetDDBYkDeal ( const CProtocol *pProtocol , BYTE byBusNo, WORD wDevAddr, void *pVoid , int iFlag );

	public:
		static void SetDDBProtocol( ) ;
		static void SetDDBSyncState( BYTE bySyncState ) ;
		static BOOL IsHaveDDB( ) ;
		static BYTE GetDDBSyncState( ) ;
		static void SetDDBBusAndAddr( BYTE byBusNo , WORD wAddr ) ;
		static BOOL GetDDBBusAndAddr( BYTE &byBusNo , WORD &wAddr ) ;
		static void SetDDBDevBusAndAddr( BYTE byBusNo , WORD wAddr ) ;
		static BOOL GetDDBDevBusAndAddr( BYTE &byBusNo , WORD &wAddr ) ;
		// ˫������ͨѶ״̬���
		// ��������״̬
		static BOOL SetDDBBusLinkStatus( BYTE byBusNo, BOOL bStatus);
		// �������״̬
		static BOOL GetDDBBusLinkStatus( BYTE byBusNo, BOOL &bStatus);
		// ����װ��״̬
		static BOOL SetDDBStnLinkStatus( WORD wSerialNo , BOOL bStatus);
		// ���װ��״̬
		static BOOL GetDDBStnLinkStatus( WORD wSerialNo, BOOL &bStatus);

	private:
		static BOOL m_IshaveDDB ; //ϵͳ�Ƿ�����˫������Э��
		static BYTE m_DDBState ; //��װ�õ�˫������Э���״̬
		static BYTE m_DDBBusNo ; //����˫���������ߺ�
		static WORD m_DDBwAddr ; //����˫�������ַ��
		static BYTE m_DDBDevBusNo ; //����˫���������ߺ�
		static WORD m_DDBDevwAddr ; //����˫�������ַ��
		// ˫������ͨѶ״̬���
		static BOOL m_bDDBBusLinkStatus[ MAX_LINE ];//����״̬
		static BOOL m_bDDBStnLinkStatus[ MAX_STN_SUM ];//װ��״̬
		WORD m_wGatherDevCount ; //���вɼ������µ�װ��
		void SetDDBYkRtnDeal ( const CProtocol *pProtocol , BYTE byBusNo, WORD wDevAddr, WORD wPnt, BYTE byVal ,int iFlag  );
	public:
			//ͨ����Ż�ȡģ��
			CProtocol * GetProtocolMoudle(WORD wSerialNo);
}; /* -----  end of class CPublicMethod  ----- *//*}}}*/

#endif   /* ----- #ifndef _PUBLICMETHOD_H__  ----- */
