#include "ModBusTcp_Gather.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
const UINT ERROR_CONST = 5;
const UINT COMSTATUS_ONLINE = 1;
const UINT COMSTATUS_FAULT = 0;
#define MODBUSTcpPREFIXNAME "/mynand/config/MBTcp/template/"

ModBusTcp_Gather::ModBusTcp_Gather()
{/*{{{*/
	yk_data = NULL;
	yktype = 0;				//暂不做处理!
	byBusNo = 0;
	wDevNo = 0;
	wPnt = 0;
	byVal = 0;
	errortimes = ERROR_CONST + 1;
	linesum = 0;
	line = 0;
	lineflag = 0;
	portstatus = COMSTATUS_FAULT;
	time_flag = 0;
	yk_pos_num = 0;
	ykline = 0;
	m_timeIndex = 0xFF;
	yk_info.reserve(500);
	//yk_sel_flag = FALSE;
	//yc_sum = 0;
	//ym_sum = 0;
	//yx_sum = 0;
	//yc_order = 0;
	//ym_order = 0;
	//yx_order = 0;
}/*}}}*/

ModBusTcp_Gather::~ModBusTcp_Gather()
{}

BOOL ModBusTcp_Gather::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{/*{{{*/
	buf[len++] = vec_conf[line].affair >> 8;
	buf[len++] = vec_conf[line].affair;
	buf[len++] = vec_conf[line].protype >> 8;
	buf[len++] = vec_conf[line].protype;
	len += 2;
	buf[len++] = m_wDevAddr;

	if(pBusMsg != NULL){
		if(pBusMsg->byMsgType == YK_PROTO){	//遥控!
			//if(vec_conf[line].type == 0x04){
			yk_data = (YK_DATA *)pBusMsg->pData;
			yktype 	= pBusMsg->dwDataType;
			byBusNo = pBusMsg->SrcInfo.byBusNo;
			wDevNo 	= pBusMsg->SrcInfo.wDevNo;
			wPnt 	= yk_data->wPnt;
			byVal 	= yk_data->byVal;				//表示分合!
			//printf("========line:%d byVal:%d========\n", __LINE__, byVal);
			//printf("========line:%d wPnt:%d========\n", __LINE__, wPnt);
			getykframe(buf, len);			//遥控数据帧!
			buf[4] = (len - 6) >> 8;
			buf[5] = (len - 6);

			char szbuf[300] = { 0 };
			int k = 0;
			for (int i = 0; i < len; i++)
			{
				k += sprintf(&szbuf[k], " %02X", buf[i]);
			}
			printf("\n%s\n", szbuf);

			if(errortimes++ > 40000)
				errortimes = ERROR_CONST + 1;
			return TRUE;
			//}
		}
	}
	time_t now;
	time(&now);
	if(now - time_flag > 1800 && m_timeIndex != 0xFF ){
		getsynframe(buf, len);
		buf[4] = (len - 6) >> 8;
		buf[5] = len - 6;
		time_flag = now;
		return TRUE;
	}
	switch(vec_conf[line].type){		//这里假定一种量使用一个功能码!如果要求一种量可以使用其他类型功能码那么以后再实现!
	case 0:								//遥测
		getycframe(buf, len);
		break;
	case 1:								//遥脉
		getymframe(buf, len);
		break;
	case 2:								//遥信
		getyxframe(buf, len);
		break;
	case 3:								//soe
		getsoeframe(buf, len);
		break;
	default:
		line++;
		return FALSE;
	}
	buf[4] = (len - 6) >> 8;
	buf[5] = len - 6;
	if(errortimes++ > 40000)			//以后再处理!
		errortimes = ERROR_CONST + 1;
	lineflag = line;
	for(BYTE i = 0; i < linesum; i++){
		line = (++line) % linesum;			//配置换行!
		if((vec_conf[line].type == 1) || (vec_conf[line].type == 2) || (vec_conf[line].type == 3) || (vec_conf[line].type == 0))
			break;
	}

	return TRUE;
}/*}}}*/

void ModBusTcp_Gather::getykframe(BYTE *buf, int &len)		//遥控帧涉及到预置，执行和取消以及遥控类型。以后再做处理!
{/*{{{*/
#if 0
	buf[len++] = vec_conf[line].func;
	if((yktype == YK_SEL) && (byVal == 0x01)){							//预置合
		buf[len++] = vec_conf[line].register_addr_yk[0] >> 8;			//不能使用line，因为line不一定是遥控行!
		buf[len++] = vec_conf[line].register_addr_yk[0];
		buf[len++] = vec_conf[line].yk_instruction >> 24;
		buf[len++] = vec_conf[line].yk_instruction >> 16;
	}
	else if((yktype == YK_SEL) && (byVal == 0x00)){						//预置分
		buf[len++] = vec_conf[line].register_addr_yk[1] >> 8;
		buf[len++] = vec_conf[line].register_addr_yk[1];
		buf[len++] = vec_conf[line].yk_instruction >> 8;
		buf[len++] = vec_conf[line].yk_instruction;
	}
	else if((yktype == YK_EXCT) && (byVal == 0x01)){					//执行合 & 通过6功能码写寄存器!
		buf[len++] = vec_conf[line].register_addr_yk[2] >> 8;
		buf[len++] = vec_conf[line].register_addr_yk[2];
		if(vec_conf[line].func == 0x06){
			buf[len++] = vec_conf[line].preset_value >> 8;				//这个成员应该合并到yk_instructor!
			buf[len++] = vec_conf[line].preset_value;
			return;
		}else{
			buf[len++] = vec_conf[line].yk_instruction >> 24;
			buf[len++] = vec_conf[line].yk_instruction >> 16;
			return;
		}
	}
	else if((yktype == YK_EXCT) && (byVal == 0x00)){					//执行分
		buf[len++] = vec_conf[line].register_addr_yk[3] >> 8;
		buf[len++] = vec_conf[line].register_addr_yk[3];
		buf[len++] = vec_conf[line].yk_instruction >> 8;
		buf[len++] = vec_conf[line].yk_instruction;
	}
	else if((yktype == YK_CANCEL) && (byVal == 0x01)){					//取消合操作
		buf[len++] = vec_conf[line].register_addr_yk[4] >> 8;
		buf[len++] = vec_conf[line].register_addr_yk[4];
		buf[len++] = vec_conf[line].yk_instruction >> 24;
		buf[len++] = vec_conf[line].yk_instruction >> 16;
	}
	else if((yktype == YK_CANCEL) && (byVal == 0x00)){					//取消分操作!
		buf[len++] = vec_conf[line].register_addr_yk[5];
		buf[len++] = vec_conf[line].register_addr_yk[5];
		buf[len++] = vec_conf[line].yk_instruction >> 8;
		buf[len++] = vec_conf[line].yk_instruction;
	}
#endif

	WORD i = 0;
	for(i = 0; i < yk_pos_num; i++){
		if(yktype == YK_SEL){
			if(yk_data->wPnt == yk_info[i].start_order){
				//(byVal == 1) ? sel_close(buf, len, vec_conf[yk_info[i].pos]) : sel_open(buf, len, vec_conf[yk_info[i]i.pos]);
				//ykcommand(buf, len, yk_info[i].pos, byVal);
				//ABB设备没有预置，但是我们的后台必须预置才能执行
				//yk_sel_flag = TRUE;
				 m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, yk_data->wPnt, yk_data->byVal);
				return;
			}
		}else if(yktype == YK_EXCT){
			if(yk_data->wPnt == yk_info[i].start_order){
				ykcommand(buf, len, yk_info[i].pos, byVal);
				return;
			}
		}else if(yktype == YK_CANCEL){
			if(yk_data->wPnt == yk_info[i].start_order){
				//ykcommand(buf, len, yk_info[i].pos, 1);
				return;
			}
		}
	}
	//printf("======== line:%d i=%d ========\n", __LINE__, i);
}/*}}}*/

void ModBusTcp_Gather::ykcommand(BYTE *buf, int &len, BYTE line, BYTE byVal)
{/*{{{*/
	buf[len++] = vec_conf[line].func;
	buf[len++] = vec_conf[line].register_addr >> 8;
	buf[len++] = vec_conf[line].register_addr;
	if(byVal == 0){
		buf[len++] = vec_conf[line].yk_instruction >> 8;
		buf[len++] = vec_conf[line].yk_instruction;
	}else if(byVal == 1){
		//printf("======== line:%d type:%d ========\n", line, type);
		buf[len++] = vec_conf[line].yk_instruction >> 24;
		buf[len++] = (vec_conf[line].yk_instruction << 8) >> 24;
	}else{
		cout<<"遥控指令错误"<<endl;
		return;
	}
	buf[4] = len >> 8;
	buf[5] = len;
	ykline = line;
}/*}}}*/

void ModBusTcp_Gather::getycframe(BYTE *buf, int &len)		//func:3 or 4	3和4功能码发送数据帧格式一样
{/*{{{*/
	buf[len++] = vec_conf[line].func;
	buf[len++] = vec_conf[line].register_addr >> 8;
	buf[len++] = vec_conf[line].register_addr;
	buf[len++] = vec_conf[line].num >> 8;
	buf[len++] = vec_conf[line].num;
}/*}}}*/

void ModBusTcp_Gather::getymframe(BYTE *buf, int &len)		//func:3 or 4
{/*{{{*/
	buf[len++] = vec_conf[line].func;
	buf[len++] = vec_conf[line].register_addr >> 8;
	buf[len++] = vec_conf[line].register_addr;
	buf[len++] = vec_conf[line].num >> 8;
	buf[len++] = vec_conf[line].num;
}/*}}}*/

void ModBusTcp_Gather::getyxframe(BYTE *buf, int &len)		//func:
{/*{{{*/
	buf[len++] = vec_conf[line].func;
	buf[len++] = vec_conf[line].register_addr >> 8;
	buf[len++] = vec_conf[line].register_addr;
	//buf[len++] = vec_conf[line].num >> 8;
	//buf[len++] = vec_conf[line].num;
	BYTE temp = 0;
	temp = vec_conf[line].num;
	buf[len++] = temp >> 8;
	buf[len++] = temp;
}/*}}}*/

void ModBusTcp_Gather::getsynframe(BYTE *buf, int &len)		//半个小时对时一次, 精度秒级!
{/*{{{*/
	time_t timep;
	struct tm *timeobj;
	time(&timep);
	timep++;
	timeobj = localtime(&timep);
	buf[len++] = 0x10;								//func:0x10
	buf[len++] = vec_conf[m_timeIndex].register_addr >> 8;
	buf[len++] = vec_conf[m_timeIndex].register_addr;
	buf[len++] = vec_conf[m_timeIndex].num >> 8;
	buf[len++] = vec_conf[m_timeIndex].num;
	buf[len++] = vec_conf[m_timeIndex].num * 2;

	buf[len++] = timeobj->tm_year - 100;
	buf[len++] = timeobj->tm_mon + 1;
	buf[len++] = timeobj->tm_mday;
	buf[len++] = timeobj->tm_hour;
	buf[len++] = timeobj->tm_min;
	buf[len++] = timeobj->tm_sec;
	buf[len++] = 0x00;
	buf[len++] = 0x00;
}/*}}}*/

void ModBusTcp_Gather::getsoeframe(BYTE *buf, int &len)				//func:0x55;		//暂未实现!
{/*{{{*/
	buf[len++] = m_wDevAddr;
	buf[len++] = vec_conf[line].func;
}/*}}}*/

BOOL ModBusTcp_Gather::verify(BYTE *buf, int len)
{/*{{{*/
	if((buf[0] != (vec_conf[lineflag].affair >> 8)) || (buf[1] != (vec_conf[lineflag].affair & 0xFF)) || (buf[2] != (vec_conf[lineflag].protype >> 8)) || (buf[3] != (vec_conf[lineflag].protype & 0xFF)))
		return FALSE;
	if((buf[4] != ((len - 6) >> 8)) || (buf[5] != ((len - 6) & 0xFF)))
		return FALSE;
	if((vec_conf[lineflag].func != buf[7]) || (m_wDevAddr != buf[6]))
		return FALSE;
	return TRUE;
}/*}}}*/

BOOL ModBusTcp_Gather::ProcessProtocolBuf(BYTE *buf, int len)
{/*{{{*/
	//switch(vec_conf[lineflag].func){
	switch(buf[7]){/*{{{*/
	case 2:				//遥信!
		if(!verify(buf, len))
			return FALSE;
		if(!resolveyxframe(buf + 8))
			return FALSE;
		break;
	case 3:				//遥脉，遥测。遥信
	case 4:				//遥脉，遥测, 遥测
		if(!verify(buf, len))
			return FALSE;
		if(buf[8] != len - 9)
			return FALSE;
		if(vec_conf[lineflag].type == 1){
			if(!resolveymframe(buf + 9, len - 9))
				return FALSE;
		}
		else if(vec_conf[lineflag].type == 0){
			if(!resolveycframe(buf + 9, len - 9))
				return FALSE;
		}
		else if(vec_conf[lineflag].type == 2){
			if(!resolveyxframe(buf + 8))
				return FALSE;
		}
		else
			return FALSE;
		break;
	case 5:				//遥控
	case 6:				//遥控 & 写寄存器!
		resolveykframe(buf + 8);
		break;
	case 0x55:
		if(!verify(buf, len))
			return FALSE;
		if(!resolvesoeframe(buf + 8, vec_conf[lineflag]))
			return FALSE;
		break;
	default:
		return FALSE;
	}/*}}}*/
	errortimes = 0;
	//	line = ++line % linesum;
	return TRUE;
}/*}}}*/
#if 0
//yuanxian
BOOL ModBusTcp_Gather::resolveyxframe(BYTE *buf)
{/*{{{*/
	UINT valuetemp = 0;
	int i = buf[0];						//字节个数!
	printf("zijie:%d:\n", i);
	int k = 0;
	WORD yx_order = vec_conf[lineflag].start_order;
	if((buf[0] > 4) || (buf[0] < 1))
		return FALSE;
	//规定一次最多采集32位。
	if(vec_conf[lineflag].func == 2){
		int ii = 0;
		do{
			valuetemp = valuetemp << 8 | buf[++ii];
		}while(ii != i);
	}else
		do{
			valuetemp = valuetemp << 8 | buf[++k];
			printf("%d\n", valuetemp);	
		}while(k < i);

	for(int m=0; m < 32; m++){
		if(vec_conf[lineflag].yx_mask & (UINT)pow(2, m))
		 cout<< "yx_order:" << yx_order << "  valuetemp:" << valuetemp << "  m:" << m << "  " << (valuetemp & (UINT)pow(2, m)) << "\n";
		 m_pMethod->SetYxData(m_SerialNo, yx_order++, (valuetemp & (UINT)pow(2, m)) ? 1 : 0);
	}
	//yx_order = yx_order % yx_sum;
	return TRUE;
}/*}}}*/
#endif
//刘清涛特别修改
BOOL ModBusTcp_Gather::resolveyxframe(BYTE *buf)
{/*{{{*/
	UINT valuetemp = 0;
	int i = buf[0];						//字节个数!
	int k = 0;
	WORD yx_order = vec_conf[lineflag].start_order;
	if((buf[0] > 4) || (buf[0] < 1))
		return FALSE;
	//规定一次最多采集32位。
	if(vec_conf[lineflag].func == 2){
		int ii = 0;
		do{
			valuetemp = valuetemp << 8 | buf[++ii];
		}while(ii != i);
	}else
		do{
			valuetemp = valuetemp << 8 | buf[++k];
			
		}while(k < i);
		//printf("%02x\n",valuetemp);
		for(int m=0; m < i*8; m++){
			if(vec_conf[lineflag].yx_mask & (UINT)pow(2, m))
			{
				//printf("order:%d value:%d\n",yx_order, (valuetemp & (UINT)pow(2, m)) ? 1 : 0);
				m_pMethod->SetYxData(m_SerialNo, yx_order++, (valuetemp & (UINT)pow(2, m)) ? 1 : 0);
			}
		    
		}
	//yx_order = yx_order % yx_sum;
	return TRUE;
}/*}}}*/


BOOL ModBusTcp_Gather::resolveymframe(BYTE *buf, int len)
{/*{{{*/
	QWORD value = 0;
	BYTE offset = 0;
	WORD ym_order = vec_conf[lineflag].start_order;

	if(vec_conf[lineflag].data_len == 0x02)
		do{
			value = (vec_conf[lineflag].datatype == 1) ? (QWORD)dealtwobyte(buf + offset) : (QWORD)(unsigned short)dealtwobyte(buf + offset);
			m_pMethod->SetYmData(m_SerialNo, ym_order++, value);
			offset += 2;
		}while(offset < len);

	else if((vec_conf[lineflag].data_len == 0x04) && ((vec_conf[lineflag].datatype == 0x00) || (vec_conf[lineflag].datatype == 0x01)))
		do{
			value = (vec_conf[lineflag].datatype == 1) ? (QWORD)dealfourbyte(buf + offset) : (QWORD)(unsigned int)dealfourbyte(buf + offset);
			m_pMethod->SetYmData(m_SerialNo, ym_order++, value);
			offset += 4;
		}while(offset < len);

	else if((vec_conf[lineflag].data_len == 0x04) && (vec_conf[lineflag].datatype == 0x02))
		do{
			value = (float)dealfloatbyte(buf + offset, len);
			m_pMethod->SetYmData(m_SerialNo, ym_order++, (QWORD)value);
			offset += 4;
		}while(offset < len);
	else
		return FALSE;
	//ym_order = ym_order % ym_sum;
	return TRUE;
}/*}}}*/

BOOL ModBusTcp_Gather::resolveycframe(BYTE *buf, int len)
{/*{{{*/
	float value = 0;
	BYTE offset = 0;
	WORD yc_order = vec_conf[lineflag].start_order;
	if(vec_conf[lineflag].data_len == 0x02)
		do{
			value = (vec_conf[lineflag].datatype == 1) ? (float)dealtwobyte(buf + offset) : (float)(unsigned short)dealtwobyte(buf + offset);
			m_pMethod->SetYcData(m_SerialNo, yc_order++, value);
			offset += 2;
		}while(offset < len);

	else if((vec_conf[lineflag].data_len == 0x04) && ((vec_conf[lineflag].datatype == 0x00) || (vec_conf[lineflag].datatype == 0x01)))
		do{
			value = (vec_conf[lineflag].datatype == 1) ? (float)dealfourbyte(buf + offset) : (float)(unsigned int)dealfourbyte(buf + offset);
			m_pMethod->SetYcData(m_SerialNo, yc_order++, value);
			offset += 4;
		}while(offset < len);

	else if((vec_conf[lineflag].data_len == 0x04) && (vec_conf[lineflag].datatype == 0x02))
		do{
			value = (float)dealfloatbyte(buf + offset, len);
			m_pMethod->SetYcData(m_SerialNo, yc_order++, value);
			offset += 4;
		}while(offset < len);

	else
		return FALSE;
	//yc_order = yc_order % yc_sum;
	return TRUE;
}/*}}}*/

void ModBusTcp_Gather::resolveykframe(BYTE *buf)
{//这样做应该不对暂时，以后再考证!/*{{{*/
	if((buf[0] != (vec_conf[ykline].register_addr >> 8)) || (buf[1] != (vec_conf[ykline].register_addr & 0xFF)))
		return;
	if(byVal == 0){					//分
		if((buf[2] != ((vec_conf[ykline].yk_instruction & 0xFFFF) >> 8)) || (buf[3] != (vec_conf[ykline].yk_instruction & 0xFF)))
			return;
	}else if(byVal == 1){			//合
		if((buf[2] != ((vec_conf[ykline].yk_instruction) >> 24)) || (buf[3] != ((vec_conf[ykline].yk_instruction << 8) >> 24)))
			return;
	}else
		return;
	if(yktype == YK_SEL)
		m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, yk_data->wPnt, yk_data->byVal);
	else if(yktype == YK_EXCT)
		m_pMethod->SetYkExeRtn(this, byBusNo, wDevNo, yk_data->wPnt, yk_data->byVal);
	else if(yktype == YK_CANCEL)
		m_pMethod->SetYkCancelRtn(this, byBusNo, wDevNo, yk_data->wPnt, yk_data->byVal);
	else
		return;
}/*}}}*/

void ModBusTcp_Gather::resolvetcpsoeframe(BYTE *buffer,frame frameobj)		//By Kai Kaiyang
{/*{{{*/
	int i = 0;
	BYTE soeflag = 0;
	TIMEDATA ptmData;

	for(i = 1; i < buffer[0]; i += 8)						//从1开始，如果无soe，报文字节数(第三个字节)为1
	{
		if( 0x0 == ( buffer[i] & 0xc0 ) )
			soeflag = 1;
		else if( 0xc0 == ( buffer[i] & 0xc0 ) )
			soeflag = 0;
		else
			continue;

		ptmData.MiSec 	= 	( (buffer[i+6] & 0xc0) << 2 ) | buffer[i+7];
		ptmData.Second 	= 	buffer[i+6] & 0x3f;
		ptmData.Minute 	= 	buffer[i+5];
		ptmData.Hour 	= 	buffer[i+4];
		ptmData.Day 	= 	buffer[i+3];
		ptmData.Month 	= 	buffer[i+2];
		ptmData.Year 	= 	buffer[i+1];

		m_pMethod->SetYxDataWithTime ( m_SerialNo, buffer[i] & 0x3f, soeflag, &ptmData );
		// char buf[100] = "";
		// sprintf(buf,"SOE m_byLineNo:%d m_wDevAddr%d num:%d val:%d time:%d.%d.%d-%d:%d:%d.%d\n",
		// m_byLineNo, m_wDevAddr , buffer[i+pos] & 0x3f , soeflag ,
		// ptmData.Year,ptmData.Month,ptmData.Day,ptmData.Hour,ptmData.Minute,ptmData.Second,ptmData.MiSec);
		// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}
}/*}}}*/

void ModBusTcp_Gather::resolveYZ202soeframe(BYTE *buffer,frame frameobj)
{/*{{{*/
	//	BYTE pos = frameobj.skew_byte;
	//WORD start_num = frameobj.start_num;
	//int i = 0;
	WORD wPnt = 0;
	BYTE soeflag = 0;
	TIMEDATA ptmData;

	if( ( buffer[2] & 0x02 ) == 0x02 )
		soeflag = 1;
	else if( ( buffer[2] & 0x02 ) == 0x00 )
		soeflag = 0;

	switch( buffer[1] )
	{
	case 4:wPnt = 0;break;
	case 8:wPnt = 1;break;
	case 9:wPnt = 2;break;
	case 16:
		   soeflag = 1;
		   switch( buffer[11] )
		   {
		   case 0x00:wPnt = 3;break;
		   case 0x01:wPnt = 4;break;
		   case 0x02:wPnt = 5;break;
		   case 0x03:wPnt = 6;break;
		   case 0x04:
					 switch( buffer[12] )
					 {
					 case 0x33:wPnt = 7;soeflag = 0;break;
					 case 0x55:wPnt = 7;break;
					 default:
							   //							   MsgErrorFlag = MSGERROR;
							   return;
					 }
					 break;
		   case 0x05:
					 switch( buffer[12] )
					 {
					 case 0x33:wPnt = 8;soeflag = 0;break;
					 case 0x55:wPnt = 8;break;
					 default:
							   //							   MsgErrorFlag = MSGERROR;
							   return;
					 }
					 break;
		   default:
					 //					 MsgErrorFlag = MSGERROR;
					 return;
		   }
		   break;
	case 17:wPnt = 9;break;
	case 18:wPnt = 10;break;
	case 19:wPnt = 11;break;
	case 21:wPnt = 12;break;
	case 22:wPnt = 13;break;
	case 23:wPnt = 14;break;
	case 24:wPnt = 15;break;
	case 25:wPnt = 16;break;
	case 26:wPnt = 17;break;
	case 27:wPnt = 18;break;
	case 28:wPnt = 19;break;
	case 31:wPnt = 20;break;
	case 35:wPnt = 21;break;
	case 37:wPnt = 22;break;
	case 38:wPnt = 23;break;
	case 39:wPnt = 24;break;
	case 42:wPnt = 25;break;
	default:
			//			MsgErrorFlag = MSGERROR;
			return;
	}



	ptmData.MiSec= buffer[9] | (buffer[10] << 8);
	ptmData.Second= buffer[8];
	ptmData.Minute= buffer[7];
	ptmData.Hour = buffer[6];
	ptmData.Day = buffer[5];
	ptmData.Month = buffer[4];
	ptmData.Year = buffer[3];

	m_pMethod->SetYxDataWithTime ( m_SerialNo, wPnt, soeflag, &ptmData );
	// char buf[100] = "";
	// sprintf(buf,"SOE m_byLineNo:%d m_wDevAddr%d num:%d val:%d time:%d.%d.%d-%d:%d:%d.%d\n",
	// m_byLineNo, m_wDevAddr , wPnt , soeflag ,
	// ptmData.Year,ptmData.Month,ptmData.Day,ptmData.Hour,ptmData.Minute,ptmData.Second,ptmData.MiSec);
	// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
}/*}}}*/

void ModBusTcp_Gather::resolveEsl_411soeframe(BYTE *buffer,frame frameobj)
{/*{{{*/
	WORD wPnt = 0;
	BYTE soeflag = 0;
	BYTE SoeAttribute = 0;
	BYTE index = 0;
	TIMEDATA ptmData;

	if( buffer[0] == 0 )
	{
		//		ESL411SOEFlag = 0;
		return;
	}
	else if( buffer[0] == 0x0f )
	{
		//		ESL411SOEFlag = 1;
	}

	ptmData.MiSec= buffer[2] | (buffer[1] << 8);
	ptmData.Second= buffer[3];
	ptmData.Minute= buffer[4];
	ptmData.Hour = buffer[5];
	ptmData.Day = buffer[6];
	ptmData.Month = buffer[7];
	ptmData.Year = buffer[8];

	SoeAttribute = buffer[9];
	index = buffer[10];
	if( index > 31 )
		return;

	switch( SoeAttribute )
	{
	case 1:
		wPnt =  index;
		break;
	case 2:
		wPnt =  index + 32;
		break;
	case 3:
		wPnt =  index + 64;
		break;
	default:
		return;
	}

	switch( buffer[11] )
	{
	case 0xff:
		soeflag = 1;
		break;
	case 0x00:
		soeflag = 0;
		break;
	default:
		return;
	}

	m_pMethod-> ( m_SerialNo, wPnt, soeflag, &ptmData );
	char buf[100] = "";
	sprintf(buf,"SOE m_byLineNo:%d m_wDevAddr%d num:%d val:%d time:%d.%d.%d-%d:%d:%d.%d\n",
			m_byLineNo, m_wDevAddr , wPnt , soeflag ,
			ptmData.Year,ptmData.Month,ptmData.Day,ptmData.Hour,ptmData.Minute,ptmData.Second,ptmData.MiSec);
	//	OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf), 3);

}/*}}}*/

//SOE处理
BOOL ModBusTcp_Gather::resolvesoeframe(BYTE *buffer,frame frameobj)
{/*{{{*/
	switch( frameobj.soeflag )
	{
	case 0:
		resolvetcpsoeframe( buffer , frameobj );
		break;
	case 1:
		resolveYZ202soeframe( buffer , frameobj );
		break;
	default:
		return FALSE;
	}
	return TRUE;
}/*}}}*/

short ModBusTcp_Gather::dealtwobyte(BYTE *buf)
{/*{{{*/
	short value = 0;
	switch(vec_conf[lineflag].dataform)
	{
	case 0:
		value = (buf[0] << 8 | buf[1]);
		break;
	case 1:
		value = (buf[0] | buf[1] << 8);
		break;
	default:
		break;
	}
	return value;
}/*}}}*/

int ModBusTcp_Gather::dealfourbyte(BYTE *buf)
{/*{{{*/
	int val = 0;
	switch(vec_conf[lineflag].dataform)
	{
	case 2:
		val = (buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
		break;
	case 3:
		val = (buf[0]<<16 | buf[1]<<24 | buf[2] | buf[3]<<8);
		break;
	case 4:
		val = (buf[0]<<8 | buf[1] | buf[2]<<24 | buf[3]<<16);
		break;
	case 5:
		val = (buf[0] | buf[1]<<8 | buf[2]<<16 | buf[3]<<24);
		break;
	default:
		break;
	}
	return val;
}/*}}}*/

float ModBusTcp_Gather::dealfloatbyte(BYTE *buf, int len)
{/*{{{*/
	float value = 0;
	BYTE float_buf[4] = "";

	switch(vec_conf[lineflag].dataform)
	{
	case 2:
		float_buf[3] = buf[0];
		float_buf[2] = buf[1];
		float_buf[1] = buf[2];
		float_buf[0] = buf[3];
		break;
	case 3:
		float_buf[2] = buf[0];
		float_buf[3] = buf[1];
		float_buf[0] = buf[2];
		float_buf[1] = buf[3];
		break;
	case 4:
		float_buf[1] = buf[0];
		float_buf[0] = buf[1];
		float_buf[3] = buf[2];
		float_buf[2] = buf[3];
		break;
	case 5:
		float_buf[0] = buf[0];
		float_buf[1] = buf[1];
		float_buf[2] = buf[2];
		float_buf[3] = buf[3];
		break;
	default:
		break;
	}
	value = *(float *)float_buf;
	return value;
}/*}}}*/

void ModBusTcp_Gather::readdefaultconfig(frame *frameobj)
{	//默认fetch遥测!/*{{{*/
	frameobj->type = 0x00;
	frameobj->affair = 0x00;
	frameobj->protype = 0x00;
	frameobj->func = 0x02;
	frameobj->register_addr = 0x1003;
	//memset(frameobj->register_addr_yk, 0, sizeof(frameobj->register_addr_yk));
	//frameobj->register_addr_yk[0] = 0;
	//frameobj->register_addr_yk[1] = 0;
	//frameobj->register_addr_yk = 0;
	frameobj->num = 0x0004;
	frameobj->start_order = 0;
	frameobj->data_len = 0x04;
	frameobj->yktype = 0x00;
	frameobj->yk_instruction = 0xFF000000;
	//frameobj->preset_value = 0x00;
	frameobj->yx_mask = 0xFFFFFFFF;
	frameobj->datatype = 0x00;
	frameobj->dataform = 0x02;
}/*}}}*/

BYTE ModBusTcp_Gather::readconf(char *filename)
{/*{{{*/
	FILE *hFile;
	char szText[160];
	char *temp;
	int num = 0;
	ULONG segvalue = 0;
	BYTE i = 0;
	BYTE conflag = 0;						//表征什么?
	frame frameobj;
	hFile = fopen(filename, "r");
	INFO yk;

	if(hFile == NULL)
		return 0;
	while( fgets(szText, sizeof(szText), hFile) != NULL ){			//将配置文件中每一行存到一个类对象中，再将这些类对象放入容器中。善哉善哉!
		rtrim(szText);				//去除字符串右侧的多余字符而不是左侧，让人费解!
		if( szText[0]=='#' || szText[0]==';' )
			continue;
		i = 0;
		conflag = 0;
		memset(&frameobj,0,sizeof(frameobj));

		temp = strtok(szText,",");
		if(temp == NULL)
			continue;
		if( ( atoi(temp) >= 0 ) && ( atoi(temp) < 5 ) )
			frameobj.type = atoi(temp);
		else
		{
			conflag = 1;
			readdefaultconfig(&frameobj);
		}
		while( ( temp = strtok(NULL,",") ) )//该while用来给modbus_conf容器中的对象的成员赋值。
		{/*{{{*/
			segvalue = (ULONG)strtoll(temp, NULL, 16);
			switch(++i)
			{
			case 1:
				if(segvalue <= 0xFFFF)
					frameobj.affair = segvalue;
				else
					conflag = 1;
				break;
			case 2:
				if(segvalue <= 0xFFFF)
					frameobj.protype = segvalue;
				else
					conflag = 1;
				break;
			case 3:
				if((segvalue >= 0x01) && (segvalue <= 0x10))
					frameobj.func = segvalue;
				else
					conflag = 1;
				break;
			case 4:
				if(segvalue <= 0xFFFF)
					frameobj.register_addr = segvalue;
				else
					conflag = 1;
				break;
			case 11 - 6:
				if(segvalue <= 0xFFFF)
					frameobj.num = segvalue;
				else
					conflag = 1;
				break;
			case 7 - 1:
				if(segvalue <=0xFFFF)
					frameobj.start_order = segvalue;				//起始序号!
				else
					conflag = 1;
				break;
			case 12 - 5:
				if((segvalue == 0x02) || (segvalue == 0x04))
					frameobj.data_len = segvalue;
				else
					conflag = 1;
				break;
			case 13 - 5:
				if((segvalue == 0x00) || (segvalue == 0x01))
					frameobj.yktype = segvalue;
				else
					conflag = 1;
				break;
			case 14 - 5:
				//if((segvalue == 0xFF000000) || (segvalue == 0xFF00FF00))
					frameobj.yk_instruction = segvalue;
				//else
				//	conflag = 1;
				break;
			case 16 - 6:
				if(segvalue <= 0xFFFFFFFF)
					frameobj.yx_mask = segvalue;
				else
					conflag = 1;
				break;
			case 17 - 6:
				if((segvalue == 0x00) || (segvalue == 0x01) || (segvalue == 0x02))
					frameobj.datatype = segvalue;
				else
					conflag = 1;
				break;
			case 18 - 6:
				if(segvalue < 0x06)
					frameobj.dataform = segvalue;
				else
					conflag = 1;
				break;
			case 19 - 6:
				if(segvalue <= 0xFF)
					frameobj.soeflag = segvalue;
				else
					conflag = 1;
				break;
			default:
				conflag = 1;
				printf("\n\n***************	i+1:%d	***************\n\n",i+1);
				break;
			}
			if( conflag == 1 )			//难道不是多此一举吗?
			{
				conflag = 1;
				printf("MBTcp config file error:\n");
				continue;
			}
		}/*}}}*/
		if( (conflag == 1) || (i < 19 - 6) )
		{
			printf("%s num is %d %d\n\n\n",filename,num+1,i+1);
			readdefaultconfig(&frameobj);
			return 0;
		}
		vec_conf.push_back(frameobj);
		num++;							//总共的行数。对象数目!
	}
	if(fclose(hFile))
		perror("fclose");

	for(i = 0; i < num; i++){
		int type = vec_conf[i].type;
		int func = vec_conf[i].func;

		if( type == 4 && ( func == 5 || func == 6 ) )
		{
			yk.pos = i;
			yk.start_order = vec_conf[i].start_order;
			yk.num = vec_conf[i].num;

			printf("line=%d index=%d start_order=%d\n", __LINE__, i, yk.start_order);
			yk_info.push_back(yk);
			++yk_pos_num;
		}
		else if (type == 4 && func == 16)
		{
			m_timeIndex = i;
		}
	}
	return num;
}/*}}}*/

void ModBusTcp_Gather::getgathernumber()
{/*{{{*/
#if 0
	for(BYTE i=0; i<linesum; i++){
		if(vec_conf[i].type == 0)		//yc
			yc_sum += vec_conf[i].num * 2 / vec_conf[i].data_len;
		else if(vec_conf[i].type == 1)	//ym
			ym_sum += vec_conf[i].num * 2 / vec_conf[i].data_len;
		else if(vec_conf[i].type == 2)	//yx
			//yx_sum += vec_conf[i].num * 16;	//不应该在配置中配置寄存器数目!
			yx_sum += vec_conf[i].num;
		else
			continue;
	}
#endif
}/*}}}*/

BOOL ModBusTcp_Gather::Init(BYTE byLineNo)
{/*{{{*/
	char szFileName[128] = "";
	vec_conf.clear();
	sprintf(szFileName, "%s%s", MODBUSTcpPREFIXNAME, m_sTemplatePath);
	linesum = readconf(szFileName);
	if(linesum <= 0){
		printf("Using Defaultconfig!\n");
		frame frameobj;
		readdefaultconfig(&frameobj);
		vec_conf.push_back(frameobj);
		linesum = 1;
	}
	//getgathernumber();
	return TRUE;
}/*}}}*/

void ModBusTcp_Gather::TimerProc()
{/*{{{*/
	portstatus = (errortimes > ERROR_CONST) ? COMSTATUS_FAULT : COMSTATUS_ONLINE;
}/*}}}*/

BOOL ModBusTcp_Gather::GetDevCommState()
{/*{{{*/
	return (portstatus == COMSTATUS_ONLINE) ? COM_DEV_NORMAL : COM_DEV_ABNORMAL;
}/*}}}*/
