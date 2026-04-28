#if !defined(MODBUSTCP_)
#define MODBUSTCP_



//#include "../../share/CProtocol.h"
#include "CProtocol_ModBusTcp.h"

#define MODBUSTCPMAX_AI_LEN (6800)
#define MODBUSTCPMAX_PI_LEN (1599)
#define MODBUSTCPMAX_DI_LEN (9999)

#define MODBUSTCPMAX_MSG_LEN (1024)

typedef struct _MBS_BUSDEV
{
	BYTE busNo;
	WORD wAddr;
	_MBS_BUSDEV()
	{
		busNo = 0;
		wAddr = 0;
	}
}MBS_BUSDEV, *PMBS_BUSDEV;

class ModBusTcp  : public CProtocol_ModBusTcp
{
public:
	ModBusTcp();
	virtual ~ModBusTcp();

	BYTE SendFlag;
 	float    m_wYCBuf[MODBUSTCPMAX_AI_LEN];
    QWORD   m_dwYMBuf[MODBUSTCPMAX_PI_LEN];
	BYTE	m_byYXbuf[MODBUSTCPMAX_DI_LEN ] ;

	BYTE MsgBuf[MODBUSTCPMAX_MSG_LEN];
	WORD MsgLen;
	BYTE MsgFlag;
	BYTE FunNum;
	BYTE ErrorFlag;
	int m_wErrorTimer;
	int m_byPortStatus;
	BYTE yk_bufSerial[2];//ң�ر��ĵ�ǰ���ֽ����
	BYTE dz_buff[2];

	BYTE Yk_FunNum;		//����ң���·�֡�Ĺ�����
	BYTE dz_FunCode;
	WORD dzRegisterAddr; //�Ĵ�����ַ
	WORD dzRegisterNum; //�Ĵ�������

	BOOL yk_recv_flag;		//���յ�ң���·�֡��Ϊ�棬ң�ػظ�֮����Ϊ�٣�Ĭ�ϼ�
	UINT time_flag;			//�����յ�ң���·�֡��ʱ��

	virtual BOOL GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg = NULL ) ;
	virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;
	BOOL DzPacket(BYTE * buf, int len);
	virtual BOOL Init(BYTE byLineNo);
	virtual BOOL InitRtuBase();
	virtual BOOL WriteAIVal(WORD wSerialNo ,WORD wPnt, float wVal) ;
	virtual BOOL WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal) ;
	virtual BOOL WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal) ;
	virtual BOOL WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond) ;
	virtual  void RelayEchoProc(BYTE byCommand, WORD wIndex, BYTE byResult);

	//���װ��ͨѶ״̬ by	zhanghg 2014-9-23
	virtual BOOL GetDevCommState( ) ;
	BOOL InitDevState();
	BOOL GetDevState(WORD wYxNo, BYTE *byVal);
	virtual void TimerProc();

	BOOL YXPacket( BYTE * buf , int len ); 
	BOOL ProcessDevState(BYTE * pBuf, int len);
	BOOL ProcessYx(BYTE * buf, int len);
	BOOL YcYmPacket(BYTE * buf, int len);
	BOOL YKMsg( BYTE * buf , int len ); 
	BOOL YKPacket( int yk_register , int val ); 

	BOOL DealBusMsgInfo( PBUSMSG pBusMsg ); 

	void processDzRtn(PBUSMSG pBusMsg);
	BOOL ErrorPacket(BYTE * buf, BYTE errorflag);
	std::vector< PMBS_BUSDEV > m_busDevStateArray;

	
};

#endif // !defined(AFX_PROTOCOL_UPS_H__DB4E4A83_510B_4232_A294_B1B4EE1AF4FD__INCLUDED_)

