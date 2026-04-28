#ifndef CPROTOCOL_H
#define CPROTOCOL_H

#include <string.h>
#include <vector>
#include "typedef.h"
#include "profile.h"
#include "semObj.h"

class CProtocol ;
class CMethod ;

typedef std::vector<CProtocol*> CPROTO_ARRAY;

//ģ��·��
#define SYSDATAPATH		"/mynand/config"
#define DEVNAME				"Bus"

//ͨѶ״̬
#define  COM_DEV_NORMAL	0  //װ��ͨѶ״̬����
#define  COM_NORMAL			0  //����ͨѶ״̬����
#define  COM_DEV_ABNORMAL  1 //װ��ͨѶ״̬�쳣
#define  COM_ABNORMAL			1 //����ͨѶ״̬�쳣

//Э������
#define PROTOCO_TRANSPROT	0
#define PROTOCO_GATHER			1

typedef struct _MSGSTNINFO
{
	BYTE byBusNo ; //���ߵ�ַ
	WORD wDevNo ;//װ�õ�ַ
} MSGSTNINFO;

typedef struct BUS_MSG
{/*{{{*/
	BUS_MSG( )
	{
		DataNum = 0;
		dwDataType = 0;
		DataLen = 0;
		byMsgType = 0 ;
		memset( &DstInfo, 0, sizeof( MSGSTNINFO ) );
		memset( &SrcInfo, 0, sizeof( MSGSTNINFO ) );
		pData = NULL ;
	}
	~BUS_MSG()
	{
		byMsgType = 0 ;
		if( pData != NULL )
		{
			//�ڶ��潫�ĳɶ�̬
			operator delete ( pData  ) ; //�����ù��캯�� Σ����Ҫ
			pData = NULL ;
		}
	}

	MSGSTNINFO DstInfo;//Ŀ��װ����Ϣ			//����Ϣ!
	MSGSTNINFO SrcInfo;//Դװ����Ϣ

	// BYTE byBusNo ; //���ߵ�ַ
	// WORD wDevNo ;//װ�õ�ַ
	// WORD wSerialNo ;//�͹����ڴ湲�õ�ģ���
	int DataNum;
	DWORD dwDataType ;//���õ��������� ң��Ԥ��
	// WORD dwDataNo;//���õ�������� ң�صڼ�·
	// DWORD dwDataVal ; //���õ�����ֵ ң���Ƿ� �Ǻ�

	BYTE byMsgType ;
	int DataLen;
	void * pData ; //��չ����--Ŀǰ����
}BUSMSG , *PBUSMSG;/*}}}*/

//��Ϣ
#define BROADCASET_PROTO		1
#define YK_PROTO						   2
#define THREAD_EXIT		3		/*�˳��߳�����  */
#define	DZ_PROTO		4		/*Э�鶨ֵ��Ϣ  */
#define  UNVARNISH_PROTO		5	 //͸����Ϣ

//ң�ض�������
#define YK_SEL				 1
#define YK_EXCT				2
#define YK_CANCEL			 3
#define YK_SEL_RTN		   4
#define YK_EXCT_RTN				5
#define YK_CANCEL_RTN		 6
#define YK_ERROR			0xFF

//��ֵ��Ϣ����
#define	DZZONE_CALL					1			/* �ٻ���ֵ�� */
#define	DZZONE_CALL_RTN				2			/* �ٻ���ֵ������ */
#define	DZZONE_SWITCH_PRESET		3			/* ��ֵ���л�Ԥ�� */
#define	DZZONE_SWITCH_PRESET_RTN	4			/* ��ֵ���л�Ԥ�÷��� */
#define DZZONE_SWITCH_EXCT			5			/* ��ֵ���л�ִ�� */
#define DZZONE_SWITCH_EXCT_RTN		6			/* ��ֵ���л�ִ�з��� */
#define DZZONE_SWITCH_CANCEL		7			/* ��ֵ���л�ȡ�� */
#define DZZONE_SWITCH_CANCEL_RTN	8			/* ��ֵ���л�ȡ������ */
#define	DZ_CALL						9			/* �ٻ���ֵ */
#define	DZ_CALL_RTN					10			/* �ٻ���ֵ���� */
#define	DZ_WRITE_PRESET				11			/* ��ֵдԤ�� */
#define	DZ_WRITE_PRESET_RTN			12			/* ��ֵдԤ�÷��� */
#define DZ_WRITE_EXCT				13			/* ��ֵдִ�� */
#define DZ_WRITE_EXCT_RTN			14			/* ��ֵдִ�з��� */
#define DZ_WRITE_CANCEL				15			/* ��ֵдȡ�� */
#define DZ_WRITE_CANCEL_RTN			16			/* ��ֵдȡ������ */
#define	DZZONE_ERROR				0xFF		/* ��ֵ���� */
#define	DZ_ERROR					0xFE		/* ��ֵ�� */

//͸������
#define VARNISH_CALL                1           /*send cmd*/
#define VARNISH_RTN                 2           //return cmd

typedef struct tagSetData
{
	WORD wSerialNo ; //װ�ñ��
	WORD wPnt ; //װ�õ��
	float fVal ; //ң��ʹ����ֵ
	WORD wVal ;  //ң��ʹ������
	tagSetData( )
	{
		wSerialNo = 0 ;
		wPnt = 0 ;
		fVal = 0.0f ;
		wVal = 0 ;
	}
}SETDATA , *PSETDATA;

/*lel*/
typedef struct tagSetData_Xml
{
	WORD wSerialNo ; //װ�ñ��
	WORD wPnt ; //װ�õ��
	WORD wNum ; //�����
	float fVal ; //ң��ʹ����ֵ
	WORD wVal ;  //ң��ʹ������
	tagSetData_Xml( )
	{
		wSerialNo = 0 ;
		wPnt = 0 ;
		wNum = 0 ;
		fVal = 0.0f ;
		wVal = 0 ;
	}
}SETDATA_XML , *PSETDATA_XML;
/*end*/

//��ȡʵ������ң�⣬ң�ţ�ң����ң�ص���
#define YC_SUM		0
#define YX_SUM		1
#define YM_SUM     2
#define YK_SUM		3

class CProtocol
{/*{{{*/
	public:
		CProtocol()
		{
			CProfile Profile( (char *)"/mynand/config/BusLine.ini" );
			m_TransDelay = Profile.GetProfileInt( (char *)"PROJECT" ,(char *)"transdelay" , 30) ;

		}
		virtual ~CProtocol() {}

		virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ){  return FALSE ;}
		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ){  return FALSE ; }
		virtual BOOL Init( BYTE byLineNo ) { return FALSE ;}
		virtual BOOL BroadCast( BYTE * buf , int &len ){  return FALSE ;}
		virtual void TimerProc(){printf("CProtocol\n"); return;  }
		virtual BOOL GetUnprocessBuf ( const BYTE *pBuf, const int iLen, void *pVoid ){return FALSE;}

		virtual BOOL WriteCIVal( WORD wSerialNo , WORD wPnt, float fVal){ return FALSE ;}
		virtual BOOL WriteAIVal(WORD wSerialNo ,WORD wPnt, WORD wVal){ return FALSE ; }
		virtual BOOL WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal){ return FALSE ; }
		virtual BOOL WritePIVal(WORD wSerialNo ,WORD wPnt, double dwVal){ return FALSE ;}
		virtual BOOL WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond){ return FALSE ;}
		/*lel*/
		virtual BOOL WriteAIVal_Xml(WORD wSerialNo, WORD wPnt, WORD wNum, float fVal){return FALSE;}
		virtual BOOL WriteDIVal_Xml(WORD wSerialNo, WORD wPnt, WORD wNum, WORD wVal){return FALSE;}
		virtual BOOL WritePIVal_Xml(WORD wSerialNo ,WORD wPnt, WORD wNum, double dwVal){ return FALSE ;}
		virtual BOOL WriteSOEInfo_Xml( WORD wSerialNo ,WORD wPnt, WORD wNum, WORD wVal, LONG lTime, WORD wMiSecond){ return FALSE ;}
		/*end*/
		virtual void ReadAnalogData(float *pData){}
		virtual void ReadDigitalData(BYTE *pData ){}
		virtual void ReadPulseData(QWORD *pData){}
		virtual BOOL GetDevCommState( ) { return FALSE  ; }
		virtual void  SetDevCommState( ) { return ; }
		virtual BOOL InitDevState( ) { return FALSE ; }
	public:

		int GetModuleNo( WORD wDevAddr )
		{
			int size = m_module.size() ;
			for( int i = 0 ; i < size ; i++ )
			{
				CProtocol * pProtocol = m_module[ i ] ;
				if( pProtocol->m_wDevAddr == wDevAddr )
					return i ;
			}
			return -1 ;
		}

	public:
		BYTE m_byLineNo ; //���ߺ�		����������"ͨѶ������"��"���"-1
		WORD m_wDevAddr ;//װ�õ�ַ		ͨѶ�������е��豸��ַ
		WORD m_SerialNo ; //�͹����ڴ�ӳ������к� ת��Э�鲻�øó�Ա����  ת����ʹ��վ�ţ����߹�����һ���� by cyz!
		WORD m_wModuleType ; //ģ������
		char m_sDevPath[ 200 ]; //װ���ļ�·��
		char m_sDevName[ 50 ]; //װ������
		char m_sTemplatePath[ 200 ] ;//ģ������
		char m_sysid[100];//�°�mqttʹ�ø��ֶ�
		CMethod *m_pMethod ; //��������
		CPROTO_ARRAY m_module ; //ģ�����
		BOOL m_ProtoType ; //Э���������ϲ㻹���²�
		//BYTE m_ProtoType; //Э���������ϲ㻹���²�
		WORD m_TransDelay;
};/*}}}*/
#endif // CPROTOCOL_H

