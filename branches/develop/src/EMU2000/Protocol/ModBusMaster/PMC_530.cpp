#include "PMC_530.h"
#include <math.h>
#include <unistd.h>
#include <time.h>

PMC_530::PMC_530():TEST(1<<31),TEST0(1<<15),ERROR_CONST(5),COMSTATUS_ONLINE(1),COMSTATUS_FAULT(0)
{/*{{{*/
	m_wErrorTimer 	= 0;
	loopflag = 0;
	m_byPortStatus 	= 0;
	yktype = 0;
	yk_data = NULL;
	byBusNo = 0;
	wDevNo = 0;
	wPnt = 0;
	byVal = 0;
}/*}}}*/

PMC_530::~PMC_530()
{}

BOOL PMC_530::GetYKBuffer(BYTE *buf, int &len, PBUSMSG pBusMsg)
{/*{{{*/
	//YK_DATA *yk_data;
	//yk_data = (YK_DATA *) pBusMsg->pData;

	buf[len++] = m_wDevAddr;
	buf[len++] = 0x05;
	buf[len++] = 0xEA;

	if(wPnt == 0x00){
		if((pBusMsg->dwDataType == YK_SEL) && (byVal == 0x01))		//至于这里和下面的byVal值是不是翻了以后在讨论!
			buf[len++] = 0xA0;										//预置合
		else if((pBusMsg->dwDataType == YK_SEL) && (byVal == 0x00))
			buf[len++] = 0xA4;										//预置分
		else if((pBusMsg->dwDataType == YK_EXCT) && (byVal == 0x01))
			buf[len++] = 0xA1;										//执行合
		else if((pBusMsg->dwDataType == YK_EXCT) && (byVal == 0x00))
			buf[len++] = 0xA5;										//执行分
		else
			return FALSE;
	}
	else
		return FALSE;
	buf[len++] = 0xFF;
	buf[len++] = 0x00;

	WORD wCRC = GetCrc(buf, len);
	buf[len++] = HIBYTE(wCRC);
	buf[len++] = LOBYTE(wCRC);

	return TRUE;
}/*}}}*/

BOOL PMC_530::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{/*{{{*/

	len = 0;
	if(pBusMsg != NULL){
		if(pBusMsg->byMsgType == YK_PROTO){
			yk_data = (YK_DATA *)pBusMsg->pData;
			yktype 	= pBusMsg->dwDataType;
			byBusNo = pBusMsg->SrcInfo.byBusNo;
			wDevNo 	= pBusMsg->SrcInfo.wDevNo;
			wPnt 	= yk_data->wPnt;
			byVal 	= yk_data->byVal;
			if(GetYKBuffer(buf, len, pBusMsg)){
				if( m_wErrorTimer > 60000 )							//时延!
					m_wErrorTimer = ERROR_CONST + 1;
				m_wErrorTimer++;
				return TRUE;
			}
		}
	}

	buf[len++] = m_wDevAddr;
	buf[len++] = 0x03;
	if(loopflag == 0){					//遥脉
		buf[len++] = 0x9D;
		buf[len++] = 0x00;
		buf[len++] = 0x00;
		buf[len++] = 0x08;
	}
	else if(loopflag == 1){				//遥测
		buf[len++] = 0x9C;
		buf[len++] = 0x40;
		buf[len++] = 0x00;
		buf[len++] = 0x35;
	}
	else if(loopflag == 2){				//遥信
		buf[len++] = 0x9C;
		buf[len++] = 0xA1;
		buf[len++] = 0x00;
		buf[len++] = 0x01;
	}
	else
		return FALSE;

	WORD wCRC = GetCrc(buf, len);
	buf[len++] = HIBYTE(wCRC);
	buf[len++] = LOBYTE(wCRC);
	loopflag++;
	loopflag = (loopflag % 3);

	m_wErrorTimer++;
	if(m_wErrorTimer > 60000)
		m_wErrorTimer = ERROR_CONST + 1;
	return TRUE;
}/*}}}*/

BOOL PMC_530::ResolvYkFrame(BYTE *buf, int &len)
{/*{{{*/
	WORD value = 0;
	WORD wCRC = GetCrc(buf, len - 2);
	if(buf[1] == 0x05){/*{{{*/
		if((buf[0] == m_wDevAddr) && (buf[6] == HIBYTE(wCRC)) && (buf[7] == LOBYTE(wCRC))){
			if((buf[2] == 0xEA) && (buf[3] == 0xA0) && (yktype == YK_SEL)){				//遥控预置!
				value = buf[4];
				value = value << 8 | buf[5];
				if(value == 0xFF00)					//m_pMethod:头文件包含会出问题!
					m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, wPnt, byVal);
				else
					return FALSE;
			}
			else if((buf[2] == 0xEA) && (buf[3] == 0xA4) && (yktype == YK_SEL)){	//应该可以和上面的if合并!
				value = buf[4];
				value = buf[5] | value << 8;
				if(value == 0xFF00)
					m_pMethod->SetYkSelRtn(this, byBusNo, wDevNo, wPnt, byVal);
				else
					return FALSE;
			}
			else if((buf[2] == 0xEA) && (buf[3] == 0xA1) && (yktype) == YK_EXCT){
				value = buf[4];
				value = value << 8 | buf[5];
				if(value == 0xFF00)
					m_pMethod->SetYkExeRtn(this, byBusNo, wDevNo, wPnt, byVal);
				else
					return FALSE;
			}
			else if((buf[2] == 0xEA) && (buf[3] == 0xA5) && (yktype) == YK_EXCT){
				value = buf[4];
				value = value << 8 | buf[5];
				if(value == 0xFF00)
					m_pMethod->SetYkExeRtn(this, byBusNo, wDevNo, wPnt, byVal);
				else
					return FALSE;
			}
			else{
				m_pMethod->SetYkExeRtn(this, byBusNo, wDevNo, wPnt, YK_ERROR);
				return FALSE;
			}
		}
	}/*}}}*/
	else{/*{{{*/
		m_pMethod->SetYkExeRtn(this, byBusNo, wDevNo, wPnt, YK_ERROR);
		return FALSE;
	}/*}}}*/
	m_wErrorTimer = 0;
	return TRUE;
}/*}}}*/

void PMC_530::ResolvYcFrame(BYTE *buf)				//处理遥测数据帧!
{/*{{{*/
#if 0
	unsigned int value;
	unsigned int valuetemp[32] = {0};
	float valuebridge[20] = {0};

	for(int i=0; i<24; i++){						//前24个数据!
		value = buf[3 + 4*i];
		for(int j=4*(i+1); j<(7+4*i); j++){
			value = (value << 8) | buf[j];
		}
		valuetemp[i] = value;
	}
	for(int i=0; i<5; i++){							//后5个数据!
		value = buf[99 + i*2];
		value = (value << 8) | buf[100 + 2*i];
		valuetemp[24 + i] = value;
	}

	for(int i = 0; i < 11; i++)
		valuebridge[i] = (float)valuetemp[i];
	if(valuetemp[15] & TEST)
		valuebridge[11] = -(float)((~(valuetemp[15] - TEST - 1)) - TEST);
	else
		valuebridge[11] = (float)valuetemp[15];
	if(valuetemp[19] & TEST)
		valuebridge[12] = -(float)((~(valuetemp[19] - TEST - 1)) - TEST);
	else
		valuebridge[12] = (float)valuetemp[19];
	if(valuetemp[23] & TEST)
		valuebridge[13] = -(float)((~(valuetemp[23] - TEST - 1)) - TEST);
	else
		valuebridge[13] = (float)valuetemp[23];
	if(valuetemp[27] & TEST0)			//之前这里是错的!
		valuebridge[14] = -(float)((~(valuetemp[27] - TEST0 - 1)) - TEST0);
	else
		valuebridge[14] = (float)valuetemp[27];
	valuebridge[15] = (float)valuetemp[28];
	valuebridge[16] = (float)valuetemp[11];
	valuebridge[17] = (float)valuetemp[12];
	valuebridge[18] = (float)valuetemp[13];
	valuebridge[19] = (float)valuetemp[14];
#endif
	//time_t Time = time(NULL);
	float valuebridge[25] = {0};
	unsigned int valuetemp[32] = {0};

	for(int i = 0; i < 24; i++)
		valuetemp[i] = (buf[3 + 4*i] << 24) | (buf[4 + 4*i] << 16) | (buf[5 + 4*i] << 8) | buf[6 + 4*i];

	for(int i=0; i < 5; i++)
		valuetemp[i + 24] = (buf[99 + 2*i] << 8) | buf[100 + 2*i];
	for(int i = 0; i < 11; i++)
		valuebridge[i] = valuetemp[i];

	valuebridge[11] = ( int )valuetemp[15];
	valuebridge[12] = (valuetemp[19] & TEST) ? -(float)(~(valuetemp[19] - 1)) : valuetemp[19];
	valuebridge[13] = (valuetemp[23] & TEST) ? -(float)(~(valuetemp[19] - 1)) : valuetemp[23];
	valuebridge[14] = (valuetemp[27] & 0x8000) ? -(float)((~(valuetemp[27] - 1)) & 0xFFFF) : valuetemp[27];
	valuebridge[15] = valuetemp[28];
	valuebridge[16] = valuetemp[11];
	valuebridge[17] = valuetemp[12];
	valuebridge[18] = valuetemp[13];
	valuebridge[19] = valuetemp[14];
	for(int i=0; i<20; i++)
		m_pMethod->SetYcData(m_SerialNo, i, valuebridge[i]);
		//m_pMethod->SetYcData(m_SerialNo, i, i * 400 + Time % 400);
}/*}}}*/

void PMC_530::ResolvYxFrame(BYTE *buf)
{/*{{{*/
	WORD value = buf[3];
	value = value << 8 | buf[4];
	char valuebridge[4] = {0};
	valuebridge[0] = value & 0x01;
	if(value & 0x02)
		valuebridge[1] = 0x01;
	if(value & 0x04)
		valuebridge[2] = 0x01;
	for(int i=0; i < 3; i++)
		m_pMethod->SetYxData(m_SerialNo, i, valuebridge[i]);
}/*}}}*/

void PMC_530::ResolvYmFrame(BYTE *buf)
{/*{{{*/
	unsigned int value;
	float valuebridge[4];
	for(int i = 0; i < 4; i++){
		value = buf[3 + i*4];
		for(int j = 4*(i + 1); j < (7 + i*4); j++)
			value = (value << 8) | buf[j];
		valuebridge[i] = (float)value;
	}
	m_pMethod->SetYmData(m_SerialNo, 0, (QWORD)(valuebridge[0]));
	m_pMethod->SetYmData(m_SerialNo, 1, (QWORD)(valuebridge[1]));
	m_pMethod->SetYmData(m_SerialNo, 2, (QWORD)(valuebridge[3]));
	m_pMethod->SetYmData(m_SerialNo, 3, (QWORD)(valuebridge[2]));
}/*}}}*/

BOOL PMC_530::ProcessProtocolBuf(BYTE *buf, int len)
{/*{{{*/

	if((buf[1] == 0x05) || (buf[1] == 0x85)){
		if(ResolvYkFrame(buf, len))
			return TRUE;
		else
			return FALSE;
	}
	else{
		WORD wCRC = GetCrc(buf, len - 2);
		if((buf[len-2] != HIBYTE(wCRC)) || (buf[len-1] != LOBYTE(wCRC)))
			return FALSE;
		switch(loopflag){		//1:遥脉	2:遥测	0:遥信
		case 0:
			if(len == 7)
				ResolvYxFrame(buf);
			else
				return FALSE;
			break;

		case 1:
			if(len == 21)
				ResolvYmFrame(buf);
			else
				return FALSE;
			break;

		case 2:
			if(len == 111)
				ResolvYcFrame(buf);
			else
				return FALSE;
			break;

		default:
			return FALSE;
		}
		m_wErrorTimer = 0;
	}
	return TRUE;
}/*}}}*/

BOOL PMC_530::Init(BYTE byLineNo)
{/*{{{*/
	return TRUE;
}/*}}}*/

void PMC_530::TimerProc()
{/*{{{*/
	if(m_wErrorTimer > ERROR_CONST)
		m_byPortStatus = COMSTATUS_FAULT;
	else
		m_byPortStatus = COMSTATUS_ONLINE;
	return;
}/*}}}*/

BOOL PMC_530::GetDevCommState()
{/*{{{*/
	if(m_byPortStatus == COMSTATUS_ONLINE)
		return COM_DEV_NORMAL;
	else
		return COM_DEV_ABNORMAL;
}/*}}}*/
