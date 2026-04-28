/*
 *
 * 			function:	yc、ym、yk、yx、soe、对时!
 * 			author:		Zhao Chunyun
 * 			date:		2017-01-06
 * 			explain:
 * 			点号和功能码关联，所以后台数据并不一定是从零开始!
 *
 */

#include "cdt.h"/*{{{*/
	//#include "CProtocol_Cdt.h"
#include <math.h>
#include <time.h>

#ifndef ERROR_CONST
	const int ERROR_CONST = 5;
#endif
#ifndef COMSTATUS_ONLINE
	const int COMSTATUS_ONLINE = 1;
#endif
#ifndef COMSTATUS_FAULT
	const int COMSTATUS_FAULT = 0;
#endif
#define CONFILEPATH  "/mynand/config/CDT/template/"					
	const UINT YCSIGNBIT = 1 << 11;				//遥测符号位!		是先求值再判断符号位或者溢出位等还是先判断位再求值，遥测选择了前者，遥脉选择了后者，以后再统一成后者!
	const UINT YCOVERFLOW = 1 << 14;				//遥测溢出位!
	const UINT YCINVALIDVAL = 1 << 15;			//遥测无效位!
	const BYTE YMINVALIDVAL = 1 << 7;			//遥脉无效位!
	const BYTE YMISBINARY = 1 << 5;			//遥脉数据是二进制!
	/*}}}*/

Cdt0::Cdt0()
{/*{{{*/			//谁能告诉我，有没有这样的笔，能画出一双双不报错的程序!
	yc_data.clear();
	yc_sec_data.clear();
	yc_gen_data.clear();
	cdt_conf.clear();
	m_wErrorTimer = ERROR_CONST + 1;
	m_byPortStatus = COMSTATUS_FAULT;
	line = 0;
	linesum = 0;
	timeflag = 0;
	byBusNo = 0;
	wDevNo = 0;
	wPnt = 0;
	byVal = 0;
}/*}}}*/

Cdt0::~Cdt0()
{/*{{{*/
	cdt_conf.clear();
}/*}}}*/

/*	获取本地时间	*/
struct tm *Cdt0::GetTime()
{/*{{{*/
	time_t timep;
	timep = time(NULL);
	return localtime(&timep);
}/*}}}*/

/*	判断是否是插入的遥信!	*/
BOOL Cdt0::InsertYx(BYTE *buf)
{/*{{{*/
	if(buf[0] >= 0xF0)
		if((buf[0] == buf[6]) && (buf[6] == buf[12]))
			if((buf[1] == buf[7]) && (buf[7] == buf[13]))
				if((buf[2] == buf[8]) && (buf[8] == buf[14]))
					if((buf[3] == buf[9]) && (buf[9] == buf[15]))
						if((buf[4] == buf[10]) && (buf[10] == buf[16]))
							if((buf[5] == buf[11]) && (buf[11] == buf[17]))
								return TRUE;
	return FALSE;
}/*}}}*/

/*	判断是否是插入的遥控返校信息字!	*/
/*	Added in 2017-02-10	*/
BOOL Cdt0::InsertYk(BYTE *buf)					//可以和上面的合并!
{/*{{{*/
	if(buf[0] == 0xE1)
		if((buf[0] == buf[6]) && (buf[6] == buf[12]))
			if((buf[1] == buf[7]) && (buf[7] == buf[13]))
				if((buf[2] == buf[8]) && (buf[8] == buf[14]))
					if((buf[3] == buf[9]) && (buf[9] == buf[15]))
						if((buf[4] == buf[10]) && (buf[10] == buf[16]))
							if((buf[5] == buf[11]) && (buf[11] == buf[17]))
								return TRUE;
	return FALSE;
}/*}}}*/

/*	获取遥控帧	*/
BOOL Cdt0::GetYkBuf(BYTE *buf, PBUSMSG pBusMsg)
{/*{{{*/
	buf[0] = buf[2] = buf[4] = 0xEB;
	buf[1] = buf[3] = buf[5] = 0x90;
	//	time_t seconds = 0;

	buf[6] = 0x71;							//控制字节
	if(pBusMsg->dwDataType == YK_SEL){		//遥控选择	有选择分和选择合!
		buf[7] = 0x61;						//帧类别!	0x61 for 选择	0xC2 for 执行	0xB3 for 撤销!
		buf[8] = 0x03;						//一个信息字四个字节
		buf[9] = cdt_conf[line].srcaddr;			//存在单点遥控和双点遥控吗?
		buf[10] = cdt_conf[line].dstaddr;
		buf[11] = GetCrc(buf + 6);

		buf[24] = buf[18] = buf[12] = 0xE0;		//功能码!
		//遥控预置!							//暂时没有考虑switchorder的BCD编码问题，
		if(byVal){								//预置合!
			buf[25] = buf[19] = buf[13] = buf[27] = buf[21] = buf[15] = 0xCC;
			buf[26] = buf[20] = buf[14] = buf[28] = buf[22] = buf[16] = cdt_conf[line].switchorder;				//开关序号!	这四种开关序号一样吗!或者我这几个命令是不是作用在一个开关上?
			buf[29] = buf[23] = buf[17] = GetCrc(buf + 12);
			yk_time_flag = time(NULL);
			yk_sel_flag = 0x01;
		}else{									//预置分
			buf[25] = buf[19] = buf[13] = buf[27] = buf[21] = buf[15] = 0x33;
			buf[26] = buf[20] = buf[14] = buf[28] = buf[22] = buf[16] = cdt_conf[line].switchorder;
			buf[29] = buf[23] = buf[17] = GetCrc(buf + 12);
			yk_time_flag = time(NULL);
			yk_sel_flag = 0x02;
		}
	}
//	else if(pBusMsg->dwDataType == YK_EXCT){
	else if((pBusMsg->dwDataType == YK_EXCT) && yk_sel_flag){		//只有预置成功了才能下发执行或取消命令!
		buf[9] = cdt_conf[line].srcaddr;
		buf[7] = 0xC2;
		buf[8] = 0x03;						//一个信息字四个字节
		buf[10] = cdt_conf[line].dstaddr;
		buf[11] = GetCrc(buf + 6);

		for(int i=0; i<3; i++){
			buf[12 + i*6] = 0xE2;
			buf[13 + i*6] = buf[15 + i*6] = 0xAA;
			buf[14 + i*6] = buf[16 + i*6] = cdt_conf[line].switchorder;
			buf[17 + i*6] = GetCrc(buf + 12 + i*6);
		}
		yk_sel_flag = 0;
	}
//	else if(pBusMsg->dwDataType == YK_CANCEL){
	else if((pBusMsg->dwDataType == YK_CANCEL) && yk_sel_flag){		//返校超时则遥控取消!
		buf[9] = cdt_conf[line].srcaddr;
		buf[7] = 0xB3;
		buf[8] = 0x03;
		buf[10] = cdt_conf[line].dstaddr;
		buf[11] = GetCrc(buf + 6);

		for(int i=0; i<3;i++){
			buf[12 + i*6] = 0xE3;
			buf[13 + i*6] = buf[15 + i*6] = 0x55;
			buf[14 + i*6] = buf[16 + i*6] = cdt_conf[line].switchorder;
			buf[17 + i*6] = GetCrc(buf + 12 + i*6);
		}
		yk_sel_flag = 0;
	}
	else
		return FALSE;
	return TRUE;
}/*}}}*/

/*	获取设置时钟帧	*/
void Cdt0::SetTime(BYTE *buf, int &len)
{/*{{{*/
	struct tm *timebridge;
	timebridge = GetTime();

	buf[len++] = 0x71;
	buf[len++] = 0x7A;
	buf[len++] = 0x02;			//两个信息字八个字节
	buf[len++] = cdt_conf[line].srcaddr;
	buf[len++] = cdt_conf[line].dstaddr;
	buf[len++] = GetCrc(buf + 6);

	buf[len++] = 0xEE;
	buf[len++] = 0x00;
	buf[len++] = 0x00;		//毫秒的高低位都置为零吧，
	buf[len++] = timebridge->tm_sec;
	buf[len++] = timebridge->tm_min;
	buf[len++] = GetCrc(buf + 12);

	buf[len++] = 0xEF;
	buf[len++] = timebridge->tm_hour;
	buf[len++] = timebridge->tm_mday;
	buf[len++] = timebridge->tm_mon + 1;
	buf[len++] = timebridge->tm_year - 100;			//这里可能出错，协议没有讲清楚!
	buf[len++] = GetCrc(buf + 18);
}/*}}}*/

/*	获取数据帧	*/
BOOL Cdt0::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{	//被动接收遥测、遥脉、遥信。主动发送遥控。/*{{{*/
	time_t seconds;
	seconds = time(NULL);
	if(pBusMsg != NULL){
#if 0
		cdt_conf[line].srcaddr = (pBusMsg->SrcInfo).wDevNo;
		cdt_conf[line].dstaddr = (pBusMsg->DstInfo).wDevNo;
		YK_DATA *yk_data;
		yk_data = (YK_DATA *)pBusMsg->pData;
		byBusNo = pBusMsg->SrcInfo.byBusNo;
		wDevNo = pBusMsg->SrcInfo.wDevNo;
		wPnt = yk_data->wPnt;
		byVal = yk_data->byVal;
#endif
		if(pBusMsg->byMsgType == YK_PROTO){
			cdt_conf[line].srcaddr = (pBusMsg->SrcInfo).wDevNo;
			cdt_conf[line].dstaddr = (pBusMsg->DstInfo).wDevNo;
			YK_DATA *yk_data;
			yk_data = (YK_DATA *)pBusMsg->pData;
			byBusNo = pBusMsg->SrcInfo.byBusNo;
			wDevNo = pBusMsg->SrcInfo.wDevNo;
			wPnt = yk_data->wPnt;
			byVal = yk_data->byVal;
			/*组装遥控帧*/
			if(GetYkBuf(buf, pBusMsg)){
				len = 30;
				return TRUE;
			}else{
				m_wErrorTimer++;
				return FALSE;
			}
		}
	}

	if((seconds - yk_time_flag >= 1200) && ((yk_sel_flag == 0x01) || (yk_sel_flag == 0x02))){	//如果预置20分钟之后还没有执行则取消之!
		pBusMsg->dwDataType = YK_CANCEL;
		GetYkBuf(buf, pBusMsg);
		yk_time_flag = seconds;
		len = 30;
		return TRUE;
	}

	buf[len++] = 0xEB;
	buf[len++] = 0x90;
	buf[len++] = 0xEB;
	buf[len++] = 0x90;
	buf[len++] = 0xEB;
	buf[len++] = 0x90;

	if((seconds - timeflag >= 1200)){				//20分钟对时一次!一开机就对时!
		SetTime(buf, len);
		timeflag = seconds;
	}

	m_wErrorTimer++;								//
	return TRUE;
}/*}}}*/

/*	将一个字节内的数据位进行反转	*/
//位和字节反转是非常低级的设置，但是既然设置了姑且如此吧!
void Cdt0::BitReverse(BYTE *array)		//扭转位序!
{/*{{{*/
	BYTE arr[8];
	for(int i = 0; i < 8; i++)
		arr[i] = ((*array >> i) & 0x01);
	*array = 0;
	for(int i = 0; i < 8; i++)
		*array += (arr[7 - i] << i);
}/*}}}*/

/*	将无符号整型数的四个字节反转	*/
void Cdt0::ByteReverse(UINT *array)		//扭转字节序!				//其实根本没有必要扭转，也不需要扭转位序!
{/*{{{*/
	BYTE *p;
	BYTE temp;
	p=(BYTE *)array;
	temp = p[3];
	p[3] = p[0];
	p[0] = temp;

	temp = p[1];
	p[1] = p[2];
	p[2] = temp;

	int i=0;
	while(i++ < 4)
		BitReverse(p++);
}/*}}}*/

/*	将无符号短整型数的两个字节反转	*/
void Cdt0::WordByteReverse(WORD *array)
{/*{{{*/
	BYTE *p;
	BYTE temp;
	p=(BYTE *)array;
	temp = p[1];
	p[1] = p[0];
	p[0] = temp;

	int i=0;
	while(i++ < 2)
		BitReverse(p++);
}/*}}}*/

/*	解析遥控返校数据帧	*/
void Cdt0::ResolvYkData(BYTE *buf)			//遥控返校,将返校信息返给上层，上层再下发命令以执行取消命令。应该是这样!
{		//遥控返校/*{{{*/
	/*只有遥控返校才有回复，所以只需要解析返校信息!*/
	//遥控返校的帧类别是0xf4!
	//	if(!InsertYx(buf))					//error take place here!
	//		return;

	/*	if(buf[0] >= 0xF0){					//如果插入了遥信则执行取消操作!
	//遥控取消!
	yk_time_flag = 0;
	yk_sel_flag = 0x01;
	return;
	}
	*/
	if((buf[0] != 0xE1) || (buf[0] != buf[6]) || (buf[0] != buf[12])){
		m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, wPnt, YK_ERROR);
		return;
	}
	if((buf[2] != cdt_conf[line].switchorder) || (buf[2] != buf[4]) || (buf[1] != buf[3]) || (buf[5] != GetCrc(buf))){
		m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, wPnt, YK_ERROR);
		return;
	}
	if((buf[2] != buf[8]) || (buf[2] != buf[14]))
		return;
	if((buf[1] != buf[7]) || (buf[1] != buf[13]))
		return;
	if((buf[1] == 0xCC) || (buf[1] == 0x33))
		m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, wPnt, byVal);
	else
		m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, wPnt, YK_ERROR);
}/*}}}*/

/*	解析遥测数据帧	*/
void Cdt0::ResolvYcData(BYTE *buf, const BYTE &info_len)					//info_len:信息字个数!
{/*{{{*/
	if((buf[0] > 0x7F) && (buf[0] < 0xF0) && (buf[0] != 0xE1))		//0xE1:遥控返校!
		return;
	if(buf[5] != GetCrc(buf))
		return;
	UINT valuetemp;
	float value;
	int m = 0;
	const int func_base = 0x00;
	WORD func_step = 0;

	if((info_len >= 0x03) && InsertYx(buf)){			//遥信插入
		ResolvYxData(buf, 1);
		ResolvYcData(buf + 18, info_len - 3);			//当初为什么这样做，如果直接m+3不是更好!	只能插入一次不需要考虑插入多次的情况!
		return;
	}else if(InsertYk(buf) && (info_len >= 0x03)){		//遥控返校插入!
		ResolvYkData(buf);
		ResolvYcData(buf + 18, info_len - 3);
		return;
	}

	while(m < info_len){
		func_step = buf[m * 6] - func_base;
		if(cdt_conf[line].yc_func[func_step / 32] & (UINT)pow(2, func_step % 32)){
			switch(buf[-5]){									//帧类别			没有区分重要遥测，一般遥测和普通遥测!
			case 0x61:
				if(buf[5 + m * 6] != GetCrc(buf + m * 6))
					return;
				valuetemp = buf[1 + 6 * m] | (buf[2 + 6 * m] << 8);
				if(valuetemp & YCINVALIDVAL)
					value = 0;
				else if(valuetemp & YCOVERFLOW)
					value = 0;
				else if(valuetemp & YCSIGNBIT)
					value = -(float)(~((valuetemp & 0xFFF) - 1) & 0xFFF);
				else
					value = (float)valuetemp;
				m_pMethod->SetYcData(m_SerialNo, func_step * 2, value);
				valuetemp = buf[3 + 6 * m] | (buf[4 + 6 * m] << 8);
				if(valuetemp & YCINVALIDVAL)
					value = 0;
				else if(valuetemp & YCOVERFLOW)
					value = 0;
				else if(valuetemp & YCSIGNBIT)
					value = -(float)(~((valuetemp & 0xFFF) - 1) & 0xFFF);
				else
					value = (float)valuetemp;
				m_pMethod->SetYcData(m_SerialNo, func_step * 2 + 1, value);
				break;
			case 0xC2:
				if(buf[5 + m * 6] != GetCrc(buf + m * 6))
					return;
				valuetemp = buf[1 + 6 * m] | (buf[2 + 6 * m] << 8);
				if(valuetemp & YCINVALIDVAL)
					value = 0;
				else if(valuetemp & YCOVERFLOW)
					value = 0;
				else if(valuetemp & YCSIGNBIT)
					value = -(float)(~((valuetemp & 0xFFF) - 1) & 0xFFF);
				else
					value = (float)valuetemp;
				m_pMethod->SetYcData(m_SerialNo, func_step * 2, value);
				valuetemp = buf[3 + 6 * m] | (buf[4 + 6 * m] << 8);
				if(valuetemp & YCINVALIDVAL)
					value = 0;
				else if(valuetemp & YCOVERFLOW)
					value = 0;
				else if(valuetemp & YCSIGNBIT)
					value = -(float)(~((valuetemp & 0xFFF) - 1) & 0xFFF);
				else
					value = (float)valuetemp;
				m_pMethod->SetYcData(m_SerialNo, func_step * 2 + 1, value);
				break;
			case 0xB3:
				if(buf[5 + m * 6] != GetCrc(buf + m * 6))
					return;
				value = 0;
				valuetemp = buf[1 + 6 * m] | (buf[2 + 6 * m] << 8);
				if(valuetemp & YCINVALIDVAL)
					value = 0;
				else if(valuetemp & YCOVERFLOW)
					value = 0;
				else if(valuetemp & YCSIGNBIT)
					value = -(float)(~((valuetemp & 0xFFF) - 1) & 0xFFF);
				else
					value = (float)valuetemp;
				m_pMethod->SetYcData(m_SerialNo, func_step * 2, value);
				valuetemp = 0;
				valuetemp = buf[3 + 6 * m] | (buf[4 + 6 * m] << 8);
				if(valuetemp & YCINVALIDVAL)
					value = 0;
				else if(valuetemp & YCOVERFLOW)
					value = 0;
				else if(valuetemp & YCSIGNBIT)
					value = -(float)(~((valuetemp & 0xFFF) - 1) & 0xFFF);
				else
					value = (float)valuetemp;
				m_pMethod->SetYcData(m_SerialNo, func_step * 2 + 1, value);
				break;
			default:
				return;
			}
			m++;
		}else
			m++;
	}
}/*}}}*/

/*	解析遥信数据帧	*/
void Cdt0::ResolvYxData(BYTE *buf, const BYTE &info_len)		//将遥信的两组数据作为一组处理!
{	//遥信点的位置是上层指定的吗?/*{{{*/
	UINT value;
	UINT i = 0;
	const int func_base = 0xF0;
	WORD func_step;

	for(int m = 0; m < info_len; m++){
		func_step = buf[6*m] - func_base;
		if(cdt_conf[line].yx_func & (WORD)pow(2, func_step)){
			if(buf[6*m + 5] != GetCrc(buf + 6*m))
				return;
			value = (buf[6*m + 4] << 24) | (buf[6*m + 3] << 16) | (buf[6*m + 2] << 8) | buf[6*m + 1];
			i = 0;
			while(i<=31){												//查看数据包中的所有位!
				if(cdt_conf[line].yx_bitmap & (UINT)pow(2, i)){			//指定位图掩码，指定为零则不取该位的值
					m_pMethod->SetYxData(m_SerialNo, func_step * 32 + i, (value & (UINT)pow(2, i)) ? 1 : 0);
				}
				i++;
			}
		}
		if((info_len >= 3) && InsertYx(buf))
			m += 2;
	}
}/*}}}*/

/*	解析遥脉数据帧	*/
void Cdt0::ResolvYmData(BYTE *buf, const BYTE &info_len)
{/*{{{*/
	if(((buf[0] < 0xA0) || (buf[0] > 0xDF)) && (buf[0] < 0xF0) && (buf[0] != 0xE1))
		return;
	UINT value;
	WORD m = 0;
	const int func_base = 0xA0;
	WORD func_step;

	if(InsertYx(buf) && (info_len >= 0x03)){			//遥信插入!
		ResolvYxData(buf, 1);
		ResolvYmData(buf + 18, info_len - 3);			//直接m+3就不用递归了。
		return;
	}else if(InsertYk(buf) && (info_len >= 0x03)){		//遥控返校插入!
		ResolvYkData(buf);
		ResolvYmData(buf + 18, info_len - 3);
		return;
	}

	while(m < info_len){
		func_step = buf[6 * m] - func_base;
		if(cdt_conf[line].ym_func[func_step / 32] & (UINT)pow(2, func_step % 32)){
			if(buf[5 + 6 * m] != GetCrc(buf + 6 * m))
				return;
			value = buf[6*m + 1] | (buf[6*m + 2] << 8) | (buf[6*m + 3] << 16) | (buf[6*m + 4] << 24);
			m_pMethod->SetYmData(m_SerialNo, func_step, (QWORD)value);
		}
		m++;
	}
}/*}}}*/

/*	解析BCD编码的遥脉数据帧	*/
void Cdt0::ResolvYmDataBCD(BYTE *buf, const BYTE &info_len)
{/*{{{*/
	if(((buf[0] < 0xA0) || (buf[0] > 0xDF)) && (buf[0] < 0xF0) && (buf[0] != 0xE1))
		return;
	UINT value;
	WORD m = 0;
	const int func_base = 0xA0;
	WORD func_step;

	if(InsertYx(buf) && (info_len >= 0x03)){			//遥信插入!
		ResolvYxData(buf, 1);
		ResolvYmDataBCD(buf + 18, info_len - 3);
		return;
	}else if(InsertYk(buf) && (info_len >= 0x03)){		//遥控返校插入!
		ResolvYkData(buf);
		ResolvYmDataBCD(buf + 18, info_len - 3);
		return;
	}

	while(m < info_len){
		func_step = buf[6 * m] - func_base;
		if(cdt_conf[line].ym_func[func_step / 32] & (UINT)pow(2, func_step % 32)){
			if(buf[5 + 6 * m] != GetCrc(buf + 6 * m))
				return;
			value = (buf[4 + 6*m] & 0x10) * 10000000;
			value = value + (buf[4 + 6*m] & 0x0F) * 1000000;
			value = value + (buf[3 + 6*m] & 0xF0) * 100000;
			value = value + (buf[3 + 6*m] & 0x0F) * 10000;
			value = value + (buf[2 + 6*m] & 0xF0) * 1000;
			value = value + (buf[2 + 6*m] & 0x0F) * 100;
			value = value + (buf[1 + 6*m] & 0xF0) * 10;
			value = value + (buf[1 + 6*m] & 0x0F);
			m_pMethod->SetYmData(m_SerialNo, func_step, (QWORD)value);
		}
		m++;
	}
}/*}}}*/

/*	解析soe	*/
void Cdt0::ResolvSoeData(BYTE *buf, const BYTE &info_len)			//这个怎么实现功能码与序号的对应，因为只有两个功能码而这两个信息字的数据共同表示一个数据。姑且搁置
{/*{{{*/
	if(buf[0] != 0x80)
		return;
	if(buf[5] != GetCrc(buf))
		return;
	if(buf[11] != GetCrc(buf + 6))
		return;
	WORD order = 0;
	BYTE m = 0;

	TIMEDATA time_soe;
	WORD soeframe;
	while(m < info_len){
		if((info_len >= 0x03) && InsertYx(buf) && buf[m * 6] == 0xE1){
			ResolvYkData(buf);
			m += 3;
		}

		soeframe = (buf[2 + m * 6] << 8) + buf[1 + m * 6];
		time_soe.MiSec = soeframe;
		time_soe.Second = buf[3 + m * 6];
		time_soe.Minute = buf[4 + m * 6];
		time_soe.Hour = buf[7 + m * 6];
		time_soe.Day = buf[8 + m * 6];
		time_soe.Month = 0;			//数据帧没有完全解析,底层暂时没有提供支持!
		time_soe.Year = 0;
//		byVal = (buf[10 + m * 6] & (1 << 15)) ? 1 : 0;
//		m_pMethod->SetYxDataWithTime(m_SerialNo, order++, byVal, &time_soe);
		m_pMethod->SetYxDataWithTime(m_SerialNo, order++, (buf[10 + m*6] & (1 << 15)) ? 1 : 0, &time_soe);
		m += 2;
	}
}/*}}}*/

/*	处理数据帧	*/
BOOL Cdt0::ProcessProtocolBuf(BYTE *buf, int len)
{/*{{{*/
	/*处理接受数据*/
	if(len == 0)
		return FALSE;

	if(((buf[0] != buf[2]) || (buf[0] != buf[4])) && ((buf[1] != buf[3]) || (buf[3] != buf[5])))
		return FALSE;
	if(buf[8] == 0){
		m_wErrorTimer = 0;
		return FALSE;
	}
	if(((buf[0] == 0xEB) && (buf[1] == 0x90)) || ((buf[0] == 0x09) && (buf[1] == 0xD7))){
		if((buf[6] == 0x71) && (buf[9] == cdt_conf[line].srcaddr) && (buf[10] == cdt_conf[line].dstaddr) || (buf[11] == GetCrc(buf + 6))){
			switch(buf[7]){
			case 0x61:
				//重要遥测
				ResolvYcData(buf + 12, buf[8]);
				break;
			case 0xC2:
				//重要遥测
				ResolvYcData(buf + 12, buf[8]);
				break;
			case 0xB3:
				//一般遥测处理
				ResolvYcData(buf + 12, buf[8]);
				break;
			case 0xF4:							//遥信，遥控返校，时钟返校!
				//遥信解析 & 遥控返校解析!
				if(buf[12] >= 0xF0)				//这样判断是不对的，如果遥控返校被遥信插入则整个数据帧都当成了遥信解析!
					ResolvYxData(buf + 12, buf[8]);
				else if(buf[12] == 0xE1)		//遥控返校
					ResolvYkData(buf + 12);
				else
					;
				break;
			case 0x85:
				//遥脉解析
				if(!(buf[16] & YMINVALIDVAL)){
					if(buf[16] & YMISBINARY)		//这里假定第一个信息字和最后一个信息字编码相同!
						ResolvYmDataBCD(buf + 12, buf[8]);
					else
						ResolvYmData(buf + 12, buf[8]);
				}
				break;
			case 0x26:
				//soe
				ResolvSoeData(buf + 12, buf[8]);
				break;
			default:
				return FALSE;
			}
			m_wErrorTimer = 0;
			return TRUE;
		}
	}
	return FALSE;
}/*}}}*/

/*	读取默认配置	*/
void Cdt0::DefaultValConfig(ConfigItem *configline)
{/*{{{*/
	configline->type= 0x00;
	configline->yc_func[0] = 0xFFFFFFFF;
	configline->yc_func[1] = 0xFFFFFFFF;
	configline->yc_func[2] = 0xFFFFFFFF;
	configline->yc_func[3] = 0xFFFFFFFF;
	configline->yx_func = 0xFFFF;
	configline->yx_bitmap = 0xFFFFFFFF;
	configline->ym_func[0] = 0xFFFFFFFF;
	configline->ym_func[1] = 0xFFFFFFFF;
	configline->switchorder = 0x00;	//不确定
	configline->srcaddr = 0x01;
	configline->dstaddr = 0x01;
}/*}}}*/

/*	读取配置文件	*/
int Cdt0::ReadConf(BYTE *filename)								//为什么读配置文件，底层不是已经读取了吗?
{/*{{{*/
	FILE *hFile;
	char szText[256];
	char *temp;
	BYTE i = 0;
	BYTE conflag = 0;						//表征什么?
	ConfigItem configline;
	if((hFile = fopen((const char *)filename, "r")) == NULL)
		return 0;

	while( fgets(szText, sizeof(szText), hFile) != NULL )			//将配置文件中每一行存到一个类对象中，再将这些类对象放入容器中。善哉善哉!
	{
		rtrim(szText);						//defined in share/global.cpp
		if( szText[0]=='#' || szText[0]==';' )
			continue;
		i = 0;
		conflag = 0;
		memset(&configline, 0, sizeof(configline));

		temp = strtok(szText,",");
		if((strtol(temp, NULL, 16) >= 0x00) && (strtol(temp, NULL, 16) <= 0xA0))
			configline.type = strtol(temp, NULL, 16);	//频繁的系统调用大大加重了系统的负担，如果先赋值再判断会好很多!
		if(temp == NULL)
			continue;
		while((temp = strtok(NULL, ",")))//该while用来给cdt_conf容器中的对象的成员赋值。
		{/*{{{*/
			switch(++i)						//i:配置文件字段的长度!
			{
			case 1:
				if((((UINT)strtoll(temp, NULL, 16) >= 0) && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF)))
					configline.yc_func[0] = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 2:
				if(((UINT)strtoll(temp, NULL, 16) >= 0 ) && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF))
					configline.yc_func[1] = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 3:
				if(((UINT)strtoll(temp, NULL, 16) >= 0) && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF))
					configline.yc_func[2] = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 4:
				if((UINT)strtoll(temp, NULL, 16) >= 0 && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF))
					configline.yc_func[3] = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 5:
				if(((UINT)strtoll(temp, NULL, 16) >= 0) && ((UINT)strtol(temp, NULL, 16) <= 0xFFFF))
					configline.yx_func = (WORD)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 6:
				if(((UINT)strtoll(temp, NULL, 16) >= 0) && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF))
					configline.yx_bitmap = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 7:
				if(((UINT)strtoll(temp, NULL, 16) >= 0) && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF))
					configline.ym_func[0] = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 8:
				if(((UINT)strtoll(temp, NULL, 16) >= 0) && ((UINT)strtoll(temp, NULL, 16) <= 0xFFFFFFFF))
					configline.ym_func[1] = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 9:
				if((UINT)(strtol(temp, NULL, 16) >= 0) && ((UINT)strtol(temp, NULL, 16) <= 0xFF ))		//范围是多少不知道，只知道占用一个字节，姑且先按0xFF计!
					configline.switchorder = (BYTE)strtol(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 10:
				if(((UINT)strtol(temp, NULL, 16) >= 0) && ((UINT)strtol(temp, NULL, 16) <= 0xFF))		//不知范围
					configline.srcaddr = (BYTE)strtol(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 11:
				if(((UINT)strtol(temp, NULL, 16) >= 0) && ((UINT)strtol(temp, NULL, 16) <= 0xFF))		//不知范围
					configline.dstaddr = (BYTE)strtol(temp, NULL, 16);
				else
					conflag = 1;
				break;
			default:
				conflag = 1;
				break;
			}
			if( conflag == 1 )
			{
				conflag = 1;
				printf("cdt config file error:\n");
				continue;
			}

		}/*}}}*/
		if( (conflag == 1) || (i < 11) )		//如果数据范围越界或者字段个数少于23个则读取默认配置。
		{
			printf("%s linesum is %d %d\n\n",filename,linesum, i+1);
			DefaultValConfig(&configline);
		}
		for(int i=0; i<4; i++)
			ByteReverse(configline.yc_func + i);
		WordByteReverse(&(configline.yx_func));
		ByteReverse(&(configline.yx_bitmap));
		ByteReverse(configline.ym_func);
		ByteReverse(configline.ym_func + 1);
		cdt_conf.push_back( configline );
		linesum++;							//总共的行数。
	}
	int freturn = fclose(hFile);//perror("fclose");
	if( freturn )
		perror("fclose");
	return linesum;
}/*}}}*/

/*	初始化协议	*/
BOOL Cdt0::Init(BYTE byLineNo)
{/*{{{*/
	/*读配置文件*/
	BYTE szFileName[128] = {0};		//不支持string头文件和类型，我无言以对!
	sprintf((char *)szFileName, "%s%s", CONFILEPATH, m_sTemplatePath);
	if(ReadConf(szFileName) <= 0){
		printf("Can't find %s\n", szFileName);
		ConfigItem conitem;
		DefaultValConfig(&conitem);		//指针传递!
		cdt_conf.push_back(conitem);
	}
	return TRUE;
}/*}}}*/

/*	时间片处理	*/
void Cdt0::TimerProc()
{/*{{{*/
	m_byPortStatus = (m_wErrorTimer > ERROR_CONST) ? COMSTATUS_FAULT : COMSTATUS_ONLINE;
}/*}}}*/

/*	子站状态信息	*/
BOOL Cdt0::GetDevCommState()		//从底层怎么的调用?这是个值得思考的问题!
{/*{{{*/
	return (m_byPortStatus == COMSTATUS_ONLINE) ? COM_DEV_NORMAL : COM_DEV_ABNORMAL;
}/*}}}*/
