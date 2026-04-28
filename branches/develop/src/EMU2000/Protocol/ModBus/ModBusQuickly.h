// ModBusQuickly.h: interface for the CModBusQuickly class.
//
//////////////////////////////////////////////////////////////////////

//#if !defined(AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_)
//#define AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_
//#define MSGERROR (1)
//#define MSGTRUE (0)


//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000
#include "ModBusRTU.h"				//都已经包含ModBusRTU.h了还要我怎样!
//#include "../../share/rdbFun.h"
//#include "../../share/gDataType.h"

// #include "../../share/semObj.h"
//#include <vector>
//using namespace std;

#pragma pack(1)

//class QUICKCONF : public MODBUSCONF
//{[>{{{<]
	//public:
	////BYTE type;
	////BYTE func;
	////WORD registe;
	////WORD registe_num;
	////BYTE skew_byte;
	////WORD get_num;
	////WORD start_num;
	////BYTE data_len;
	////UINT mask_code;
	////BYTE data_form;
	////BYTE sign;
	////BYTE yk_form;
	////BYTE cir_flag;
	////WORD YkClose;
	////WORD YkOpen;
	////BYTE SetTimeFlag;
	////BYTE SoeFlag;
	////BYTE YkSelFlag;
	////BYTE YkExctFlag;

	////为快速报文添加字段如下:
	////这些字段在配置文件中的位置放在cir_flag之后，如果没有对应的采集量则配置0!
	//BYTE typesec;					//第二个数据类别
	//BYTE registenumsec;				//第二个寄存器数目
	//WORD getnumsec;					//第二个采集数目
	//WORD startpossec;
	//BYTE typethi;					//like above!
	//BYTE registenumthi;
	//WORD getnumthi;
	//WORD startposthi;
//};[>}}}<]

//typedef struct
//{[>{{{<]
	//BYTE pos;
	//WORD start_num;
	//WORD get_num;
//}INFO;[>}}}<]

class CModBusQuickly : public CModBusRTU
{/*{{{*/
	public:
		CModBusQuickly();
		virtual ~CModBusQuickly();

#if 0
		//start		似乎不可行，毕竟如果中间有一枚数据丢失则所有数据皆错位!
		WORD SumOfYc;
		WORD SumOfYm;
		WORD SumOfYx;
		//end
#endif
		MODBUSCONF yc_conf;					//如果首采的不是遥测，则遥测很多字段使用该配置!
		MODBUSCONF ym_conf;					//同上!
		MODBUSCONF yx_conf;					//同上!

		virtual BOOL ProcessProtocolBuf( BYTE * buf , int len ) ;

		virtual void DefaultValConfig(MODBUSCONF * mc);

		//读取配置文件
		virtual int ReadConf(char *filename);

		virtual BOOL Init(BYTE);

		//遥信，遥测，对时等发送报文
		virtual void SendBuf( MODBUSCONF modbusconf , BYTE * buf ,int *len);

		virtual void ModBusYxDeal(BYTE *, MODBUSCONF);

		//遥信安位处理
		virtual void ModBusYxBitDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//遥测处理
		virtual void ModBusYcDeal(unsigned char *buffer,MODBUSCONF modbusconf);

		//遥脉处理
		virtual void ModBusYmDeal(unsigned char *buffer,MODBUSCONF modbusconf);
};/*}}}*/
#pragma pack( )

//#endif // !defined(AFX_MODBUSRTU_H__B73038B4_D223_4CF3_BB34_5F1A02A9777E__INCLUDED_)
