/*	昆明三号线!	*/
#ifndef MODBUSTcp_TRANSFER_H
#define MODBUSTcp_TRANSFER_H
#include <vector>
#include "CProtocol_ModBusTcp.h"
typedef unsigned int UINT;
typedef unsigned short int USHORT;
typedef unsigned long ULONG;
using namespace std;

class ConfigItem{
	public:
		BYTE type;
		BYTE busno;
		BYTE devno;
		BYTE bus_count;
		BYTE dev_count;
		UINT mask;
		WORD register_addr;
};

class ModBusTcp_KunMing : public CProtocol_ModBusTcp_Transfer{
	public:
		ModBusTcp_KunMing();
		virtual ~ModBusTcp_KunMing();

		virtual BOOL GetProtocolBuf(BYTE*, int &, PBUSMSG pBusMsg = NULL);
		virtual void ReOrder(vector<DWORD> &);
		void ReOrder(vector<DWORD> &, vector<DWORD> &);
		BOOL FetchYcYmData();
		void GetYcYmFrame(BYTE *, int &);
		BOOL FetchYxData();
		void GetYxFrame(BYTE *, int &);
		virtual BOOL ProcessProtocolBuf(BYTE *, int);
		BYTE ReadConf(BYTE *);
		void DefaultConfig(ConfigItem *);
		virtual BOOL Init(BYTE);
		virtual void TimerProc();
		virtual BOOL GetDevCommState();

	private:
		WORD affair_type;
		vector<DWORD> vec_yc_ym;
		vector<WORD> vec_yx;
		vector<BYTE> LineNo;
		vector<BYTE> DevNo;							//和LineNo一起用于生产用于遥信的顺序号!
		vector<ConfigItem> vec_conf;
		BYTE m_byPortStatus;
		BYTE linesum;
		BYTE line;
		int loopflag;
		const int ERROR_CONST;
		const int COMSTATUS_ONLINE;
		const int COMSTATUS_FAULT;
};

#endif
