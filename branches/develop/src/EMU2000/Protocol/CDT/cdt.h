#ifndef CDT_H
#define CDT_H

#include "CProtocol_Cdt.h"
using namespace std;

#ifndef BYTE
typedef unsigned char BYTE;
#endif
#ifndef UINT
typedef unsigned int UINT;
#endif
typedef unsigned long long int ULONG;

typedef struct{			//没必要指定个数!
	BYTE type;			//预留									//0-0xA0
	UINT yc_func[4];	//四个字节128位，表示128个功能码!		//用于过滤指定的功能码! 全部 0-0xFFFFFFFF
	WORD yx_func;		//遥信16个功能码!						//0-0xFFFF
	UINT yx_bitmap;		//用于指定取哪些位!						//0-0xFFFFFFFF
	UINT ym_func[2];	//遥脉64个功能码!						//全部0-0xFFFFFFFF
	BYTE switchorder;	//开关顺序!默认0						//0-0xFF
	BYTE srcaddr;		//默认1						//源地址与目的地址都已经通过PBUSMSG结构体指定了，该成员就没有必要存在于此了吧!	答曰:PBUSMSG结构体中的地址应该既含有设备地址又含有总线号。而cdt协议只有一个字节指定数据那么该地址是指的什么暂时尚不清楚! 0-0xFF
	BYTE dstaddr;		//默认1						//源地址和目的地址，应该从上层获得。	//0-0xFF
}ConfigItem;

class Cdt0 : public CProtocol_Cdt{
	public:
		Cdt0();
		virtual ~Cdt0();
		virtual struct tm *GetTime();
		virtual BOOL InsertYx(BYTE *);
		virtual BOOL InsertYk(BYTE *);
		virtual BOOL GetYkBuf(BYTE *, PBUSMSG);
		virtual void SetTime(BYTE *, int &);
		virtual BOOL GetProtocolBuf(BYTE *, int &, PBUSMSG);
		virtual void BitReverse(BYTE *);
		virtual void ByteReverse(UINT *);
		virtual void WordByteReverse(WORD *);
		virtual void ResolvYkData(BYTE *);
		virtual void ResolvYcData(BYTE *, const BYTE &);
		virtual void ResolvYxData(BYTE *, const BYTE &);
		virtual void ResolvYmData(BYTE *, const BYTE &);
		virtual void ResolvYmDataBCD(BYTE *, const BYTE &);
		virtual void ResolvSoeData(BYTE *, const BYTE &);
		virtual BOOL ProcessProtocolBuf(BYTE *, int);
		virtual void DefaultValConfig(ConfigItem *);
		virtual int  ReadConf(BYTE *);
		virtual BOOL Init(BYTE);				//以后再换成有参数的!
		virtual void TimerProc();
		virtual BOOL GetDevCommState();
	protected:
		vector<float> yc_data;
		vector<float> yc_sec_data;
		vector<float> yc_gen_data;
		BOOL yc_yet;
		BOOL yc_sec_yet;
		BOOL yc_gen_yet;
		vector<ConfigItem> cdt_conf;			//暂时没什么用!
		int m_wErrorTimer;
		int m_byPortStatus;

		BYTE line;			//表征第几行配置!	没什么用了。
		BYTE linesum;		//配置行数!
		ULONG timeflag;		//设置时钟标志!
		ULONG yk_time_flag;	//遥控逾时取消标志
		WORD srcaddr;
		WORD dstaddr;
		BYTE byBusNo;
		BYTE wDevNo;
		BYTE wPnt;
		BYTE byVal;
		BYTE yk_sel_flag;	//1:预置合	2:预置分
};

#endif
