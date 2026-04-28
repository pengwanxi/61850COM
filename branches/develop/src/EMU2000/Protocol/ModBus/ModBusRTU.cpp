// ModBusRTU.cpp: implementation of the CModBusRTU class.
//
//////////////////////////////////////////////////////////////////////
#include "ModBusRTU.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <vector>
#include <math.h>
#include <stdint.h>

#define MODBUSRTUPREFIXFILENAME "/mynand/config/ModBus/template/" /* modbusrtu ǰ׺·�� */
// #define MODBUSDEBUG 1
//  #ifdef MODBUSDEBUG
//  #endif
#define TIME 300
#define ERROR_CONST 5
#define COMSTATUS_ONLINE 1
#define COMSTATUS_FAULT 0

#define MASK 0xffffffff

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
extern "C" void OutMessageText(char *szSrc, unsigned char *pData, int nLen);
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModBusRTU::CModBusRTU()
{ /*{{{*/
	pos_flag = -1;
	pos = 0;
	settime_pos = 0;
	yk_pos_num = 0;
	writeval_pos_num = 0;
	yk_flag = 0;
	readval_flag = -1;
	writeval_flag = -1;
	last_settime = 0;
	timeflag = 0;
	YkNo = 0;
	MsgErrorFlag = 0;
	ESL411SOEFlag = 0;
	memset(store_buf, 0, sizeof(store_buf));
	// m_hSem.Create( 20150101 );
	m_wErrorTimer = ERROR_CONST + 1;
	m_byPortStatus = COMSTATUS_FAULT;
	DevCirFlag = FALSE;
	modbus_conf.reserve(100);
	readval_pos_num = 0;
	writeval_pos_num = 0;
	readval_info.reserve(20);
	writeval_info.reserve(20);
	m_bUnvanish = FALSE;
} /*}}}*/

CModBusRTU::~CModBusRTU()
{ /*{{{*/
	printf("Delete CModBusRTU bus = %d , Addr = %d \n", m_byLineNo, m_wDevAddr);
	yk_info.clear();
	// readval_info.clear();
	// writeval_info.clear();
	modbus_conf.clear();
	// m_hSem.Remove();
} /*}}}*/

BOOL CModBusRTU::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{ /*{{{*/

	if (modbus_conf.size() <= 0)
		return FALSE;

	if (pBusMsg != NULL)
	{
		if (pBusMsg->byMsgType == YK_PROTO)
		{
			if (TRUE == GetYKBuffer(buf, len, pBusMsg))
			{
				if (pBusMsg->dwDataType == YK_EXCT || (pBusMsg->dwDataType == YK_SEL && modbus_conf[pos_flag].YkSelFlag == 1))
				{
					m_wErrorTimer++;
					if (m_wErrorTimer > 60000)
						m_wErrorTimer = ERROR_CONST + 1;
					MsgRegisteAndData[0] = buf[2];
					MsgRegisteAndData[1] = buf[3];
					MsgRegisteAndData[2] = buf[4];
					MsgRegisteAndData[3] = buf[5];

					return TRUE;
				}
			}
			else
				return FALSE;
		}
		else if (pBusMsg->byMsgType == DZ_PROTO)
		{

			GetDzWriteProcess(buf, len, pBusMsg);
			return true;
		}
		else if (pBusMsg->byMsgType == UNVARNISH_PROTO)
		{
			printf("UNVARNISH_PROTO\n");
			ProcessUnvarnish(buf, len, pBusMsg);
			return true;
		}
	}

	if (yk_flag > 0)
	{
		yk_flag++;
		if (yk_flag >= 3)
		{
			yk_flag = 0;
		}
		memcpy(buf, YkBuf, YkLen);
		len = YkLen;
	}
	else
	{
		if (DevCirFlag == FALSE)
			return FALSE;

		while (1)
		{
			if ((modbus_conf[pos].cir_flag == 0))
			{
				pos = (pos + 1) % line;
				continue;
			}
			if ((modbus_conf[pos].type == 8)) // ��¼��ʱ�������ڵ�λ��
			{
				if (timeflag > TIME)
				{
					timeflag = 0;
					SendBuf(modbus_conf[settime_pos], buf, &len);
					pos_flag = settime_pos;
					time(&last_settime);
					pos = (pos + 1) % line;
					break;
				}
				else
				{
					pos = (pos + 1) % line;
					continue;
				}
			}
			else if ((modbus_conf[pos].type == 3)) // ��¼ң���������ڵ�λ��
			{
				pos = (pos + 1) % line;
				continue;
			}

			SendBuf(modbus_conf[pos], buf, &len); // ��ͨ������ң��ң��
			pos_flag = pos;
			pos = (pos + 1) % line;
			break;
		}
		if (ESL411SOEFlag == 1)
		{
			Esl411SoeSendBuf(buf, &len);
		}
	}
	m_wErrorTimer++;
	if (m_wErrorTimer > 60000)
		m_wErrorTimer = ERROR_CONST + 1;
	MsgRegisteAndData[0] = buf[2];
	MsgRegisteAndData[1] = buf[3];
	MsgRegisteAndData[2] = buf[4];
	MsgRegisteAndData[3] = buf[5];
	return TRUE;
} /*}}}*/

void CModBusRTU::ProcessUnvarnish(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	if (pBusMsg == NULL)
		return;
	BYTE *pData = (BYTE *)pBusMsg->pData;
	int dataLen = pBusMsg->DataLen - 11;
	memcpy(m_quest_id, &pData[dataLen], 10);

	int index = 0;
	buf[0] = pBusMsg->DstInfo.wDevNo;
	index += 1;
	memcpy(&buf[1], (BYTE *)pData, dataLen);
	index += dataLen;

	WORD wCRC = GetCrc(buf, index);

	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	len = index;

	pos_flag = pos;
	MsgRegisteAndData[0] = buf[2];
	MsgRegisteAndData[1] = buf[3];
	MsgRegisteAndData[2] = buf[4];
	MsgRegisteAndData[3] = buf[5];
	m_bUnvanish = TRUE;

	bySrcBusNo = pBusMsg->SrcInfo.byBusNo;
	wSrcDevAddr = pBusMsg->SrcInfo.wDevNo;
}

BOOL CModBusRTU::GetDzWriteProcess(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	if ((pBusMsg == NULL) && (writeval_pos_num == 0))
		return FALSE;

	DZ_DATA *dz_data;
	dz_data = (DZ_DATA *)pBusMsg->pData;
	WORD wDataNum = pBusMsg->DataNum;
	//	if (wDataNum == 2)
	//		ProcessSingleDz(buf, len, pBusMsg);
	//	else if (wDataNum > 2) //iec104д��ֵ����д����
	ProcessPatchDz(buf, len, pBusMsg);
	//	else
	//		printf("filename = %s  fuction=%d can't process DZ\n" , __FILE__ , __FUNCTION__  );

	return true;
}

void CModBusRTU::ProcessPatchDz(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	DZ_DATA *dz_data;
	dz_data = (DZ_DATA *)pBusMsg->pData;

	bySrcBusNo = pBusMsg->SrcInfo.byBusNo;
	wSrcDevAddr = pBusMsg->SrcInfo.wDevNo;
	WORD wDataNum = pBusMsg->DataNum;
	if (wDataNum < 2)
		return;

	if (pBusMsg->dwDataType == DZ_WRITE_EXCT)
	{

		ProcessDzWrite(buf, len, pBusMsg);
	}
	else if (pBusMsg->dwDataType == DZ_CALL)
	{
		ProcessDzRead(buf, len, pBusMsg);
	}
}

void CModBusRTU::ProcessDzRead(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	DZ_DATA *dz_data;
	dz_data = (DZ_DATA *)pBusMsg->pData;
	WORD wDataNum = pBusMsg->DataNum;
	// dz_data ��һ��Ϊ��ֵ������ �ڶ�����ʼΪ��ֵ�������� ��������ֵ������
	DZ_DATA *da_dz_valid = dz_data + 1;
	WORD wDzPnt = da_dz_valid->wPnt;
	float fDz[100] = {0};
	BYTE byRowNum = -1;
	BYTE byDataNum = wDataNum - 1;
	for (int i = 0; i < readval_pos_num; i++)
	{
		if (wDzPnt != readval_info[i].start_num)
			continue;
		byRowNum = i;
		break;
	}

	BYTE pos = readval_info[byRowNum].pos;
	MODBUSCONF modbusconf = modbus_conf[pos];
	BYTE index = 0;

	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = modbusconf.registe;
	buf[index++] = modbusconf.registe_num >> 8;
	buf[index++] = modbusconf.registe_num;

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	len = index;

	pos_flag = pos;
	MsgRegisteAndData[0] = buf[2];
	MsgRegisteAndData[1] = buf[3];
	MsgRegisteAndData[2] = buf[4];
	MsgRegisteAndData[3] = buf[5];
}

void CModBusRTU::ProcessDzWrite(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	DZ_DATA *dz_data;
	dz_data = (DZ_DATA *)pBusMsg->pData;
	WORD wDataNum = pBusMsg->DataNum;
	// dz_data ��һ��Ϊ��ֵ������ �ڶ�����ʼΪ��ֵ�������� ��������ֵ������
	DZ_DATA *da_dz_valid = dz_data + 1;
	WORD wDzPnt = da_dz_valid->wPnt;
	float fDz[100] = {0};
	WORD wDz[100] = {0};
	BYTE byRowNum = -1;
	BYTE byDataNum = wDataNum - 1;
	printf("*********%d %s********%d****\n", __LINE__, __FILE__, writeval_pos_num);
	// ֻ����������ֵ
	for (int i = 0; i < writeval_pos_num; i++)
	{
		if (wDzPnt != writeval_info[i].start_num)
			continue;

		byRowNum = i;
		for (int m = 0; m < byDataNum; m++)
		{
			// 10������д
			memcpy(fDz + m, da_dz_valid->byVal, sizeof(float));

			// 06������д
			//			wDz[m] = MAKEWORD(da_dz_valid->byVal[0], da_dz_valid->byVal[1]);
			//			da_dz_valid += 1;
		}
		break;
	}
	printf("line = %d\n", __LINE__);

	BYTE pos = writeval_info[byRowNum].pos;
	MODBUSCONF modbusconf = modbus_conf[pos];
	BYTE index = 0;

	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = modbusconf.registe;

	//	printf("%d %d %d\n", wDz[0] , wDz[1], wDz[2] );
	// 06������д
	//	WORD wVal = (WORD)wDz[0];
	//	buf[index++] = HIBYTE(wVal);
	//	buf[index++] = LOBYTE(wVal);

	// 10������д
	buf[index++] = modbusconf.registe_num >> 8;
	buf[index++] = modbusconf.registe_num;
	buf[index++] = byDataNum * 2; // byte count
	for (int i = 0; i < byDataNum; i++)
	{
		WORD wVal = (WORD)fDz[i];
		buf[index++] = HIBYTE(wVal);
		buf[index++] = LOBYTE(wVal);
	}

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	len = index;

	pos_flag = pos;
	MsgRegisteAndData[0] = buf[2];
	MsgRegisteAndData[1] = buf[3];
	MsgRegisteAndData[2] = buf[4];
	MsgRegisteAndData[3] = buf[5];
}

void CModBusRTU::ProcessSingleDz(BYTE *buf, int &len, PBUSMSG pBusMsg)
{
	DZ_DATA *dz_data;
	dz_data = (DZ_DATA *)pBusMsg->pData;

	bySrcBusNo = pBusMsg->SrcInfo.byBusNo;
	wSrcDevAddr = pBusMsg->SrcInfo.wDevNo;
	WORD wDataNum = pBusMsg->DataNum;
	if (wDataNum != 2)
		return;

	// dz_data ��һ��Ϊ��ֵ������ �ڶ���Ϊ��ֵ�������� ��������ֵ������
	DZ_DATA *da_dz_valid = dz_data + 1;
	if (da_dz_valid->byType != 1)
		return;

	WORD wPnt = da_dz_valid->wPnt;
	WORD wVal = 0;
	memcpy(&wVal, da_dz_valid->byVal, 2);
	if (pBusMsg->dwDataType == DZ_WRITE_EXCT)
	{
		for (int i = 0; i < writeval_pos_num; i++)
		{
			if (wPnt == writeval_info[i].start_num)
			{
				GetDzWriteBuffer(writeval_info[i].pos, buf, len);
				break;
			}
		}
	}
}

void CModBusRTU::GetDzWriteBuffer(BYTE byRowNum, BYTE *buf, int &len)
{
	// BYTE pos = writeval_info[byRowNum].pos;
	BYTE pos = byRowNum;
	SendBuf(modbus_conf[pos], buf, &len);
	pos_flag = pos;
	MsgRegisteAndData[0] = buf[2];
	MsgRegisteAndData[1] = buf[3];
	MsgRegisteAndData[2] = buf[4];
	MsgRegisteAndData[3] = buf[5];
}

BOOL CModBusRTU::ProcessProtocolBuf(BYTE *buf, int len)
{ /*{{{*/
	BYTE temp_flag = 0;
	BYTE CRC_flag = 0;
	WORD wCRC = 0;

	if (m_bUnvanish)
	{
		printf("ProcessUnvarnishRtn\n");
		ProcessUnvarnishRtn(buf, len);
		m_bUnvanish = FALSE;
		return FALSE;
	}
	/*�жϽ��ܵı����Ƿ���ȷ*/
	while (len > 4)
	{
		if (buf[0] == m_wDevAddr)
		{
			if (buf[1] == modbus_conf[pos_flag].func)
			{
				if (((buf[1] > 0) && (buf[1] < 5)) || (buf[1] == 0x55))
				{ /*{{{*/
					if ((buf[2] + 5) <= len)
					// if ((buf[2] + 5) == len)
					{
						if (modbus_conf[pos_flag].type == 1)
						{
							if ((modbus_conf[pos_flag].registe_num % 8 != 0) && (buf[2] == 1 + modbus_conf[pos_flag].registe_num / 8))
							{
								CRC_flag = 1;
							}
							else if ((modbus_conf[pos_flag].registe_num % 8 == 0) && (buf[2] == modbus_conf[pos_flag].registe_num / 8))
							{
								CRC_flag = 1;
							}
							else if (buf[2] == modbus_conf[pos_flag].registe_num * 2)
							{
								CRC_flag = 1;
							}
						}
						else
						{
							if (buf[1] == 0x55)
							{
								CRC_flag = 1;
							}
							if (buf[2] == modbus_conf[pos_flag].registe_num * 2)
							{
								CRC_flag = 1;
							}
						}
						if (CRC_flag == 1)
						{
							wCRC = GetCrc(buf, buf[2] + 3);
							if ((HIBYTE(wCRC) == buf[buf[2] + 3]) && (LOBYTE(wCRC) == buf[buf[2] + 4]))
							{
								temp_flag = 1;
								break;
							}
						}
					}
				} /*}}}*/
				else if (buf[1] == 0x5 || buf[1] == 0x6 || buf[1] == 0xf || buf[1] == 0x10)
				{ /*{{{*/
					if (MsgRegisteAndData[0] == buf[2] && MsgRegisteAndData[1] == buf[3] &&
						MsgRegisteAndData[2] == buf[4] && MsgRegisteAndData[3] == buf[5])
					{
						wCRC = GetCrc(buf, 6);
						if ((HIBYTE(wCRC) == buf[6]) && (LOBYTE(wCRC) == buf[7]))
						{
							temp_flag = 1;
							break;
						}
					}
				} /*}}}*/
			}
			else if (buf[1] == (modbus_conf[pos_flag].func | 0x80))
			{ /*{{{*/
				wCRC = GetCrc(buf, 3);
				if ((HIBYTE(wCRC) == buf[3]) && (LOBYTE(wCRC) == buf[4]))
				{
					switch (modbus_conf[pos_flag].type)
					{
					case 3:
						yk_flag = 0;
						break;
					}
					char buffer[100] = "";
					sprintf(buffer, "func %x errno num:%d\n", buf[1], buf[2]);
					OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer), 3);
					break;
				}
			} /*}}}*/
			else if (modbus_conf[pos_flag].YkSelFlag == 1 && buf[1] == 0x10 && modbus_conf[pos_flag].func == 0x05)
			{ /*{{{*/
				if (buf[2] == 0x40 && buf[3] == 0x59 && buf[4] == 0x00 && buf[5] == 0x03)
				{
					wCRC = GetCrc(buf, 6);
					if ((HIBYTE(wCRC) == buf[6]) && (LOBYTE(wCRC) == buf[7]))
					{
						m_pMethod->SetYkSelRtn(this, bySrcBusNo, wSrcDevAddr, YkNo, YkVal);
						return TRUE;
					}
				}
			} /*}}}*/
			else if (modbus_conf[pos_flag].SoeFlag == 2 && (buf[1] == 0x02 || buf[1] == 0x03 || buf[1] == 0x0c))
			{ /*{{{*/
				wCRC = GetCrc(buf, buf[2] + 3);
				if ((HIBYTE(wCRC) == buf[buf[2] + 3]) && (LOBYTE(wCRC) == buf[buf[2] + 4]))
				{
					temp_flag = 1;
					break;
				}
			} /*}}}*/
		}
		buf = buf + 1;
		char buffer[100] = "the message changed!";
		len--;
	}
	if (temp_flag == 1)
	{
		switch (modbus_conf[pos_flag].type)
		{
		case 1:
			ModBusYxDeal(buf, modbus_conf[pos_flag]);
			break;
		case 2:
			ModBusYcDeal(buf, modbus_conf[pos_flag]);
			break;
		case 3:
			ModBusYkDeal(buf, modbus_conf[pos_flag]);
			break;
		case 4:
			ModBusYmDeal(buf, modbus_conf[pos_flag]);
			break;
		case 5:
			ModBusReadVal(buf, modbus_conf[pos_flag]);
			break;
		case 6:
			ModBusWriteVal(buf, modbus_conf[pos_flag]);
			break;
		case 8:
			ModBusSetTime(buf, modbus_conf[pos_flag]);
			break;
		case 9:
			ModBusSoeDeal(buf, modbus_conf[pos_flag]);
			break;
		case 7:
			ModBusWriteDz_10(buf, modbus_conf[pos_flag]);
			break;
		default:
			return FALSE;
		}
	}
	else if (temp_flag == 0)
	{

		char buffer[100] = "the message is wrong!";
		OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer), 3);
		return FALSE;
	}
	if (MsgErrorFlag == MSGERROR)
	{

		MsgErrorFlag = MSGTRUE;
		char buffer[100] = "the config is wrong!";
		OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer), 3);
		return FALSE;
	}
	m_wErrorTimer = 0; // ���յ���Ϣ��0
	return TRUE;
} /*}}}*/

void CModBusRTU::ProcessUnvarnishRtn(BYTE *buf, int len)
{
	// �˴����ж�10������͸��У��
	char szFail[100] = {"fail"};
	char szSucceed[100] = {"succeed"};

	memcpy(&szFail[4], m_quest_id, 10);
	memcpy(&szSucceed[7], m_quest_id, 10);

	if (len != 8)
		m_pMethod->UnvarnishedRtn(this, bySrcBusNo, wSrcDevAddr, szFail, 14, VARNISH_RTN);
	else
		m_pMethod->UnvarnishedRtn(this, bySrcBusNo, wSrcDevAddr, szSucceed, 17, VARNISH_RTN);
}

void CModBusRTU::ModBusWriteDz_10(unsigned char *buffer, MODBUSCONF modbusconf)
{
	int pos = modbusconf.skew_byte;
	int get_num = modbusconf.get_num;

	printf("write Dz %d success!!\n", modbusconf.start_num);
	m_pMethod->SetDzWriteExctRtn(this, bySrcBusNo, wSrcDevAddr, 0, 0, 0);
}

BOOL CModBusRTU::Init(BYTE byLineNo)
{ /*{{{*/
	// �����ʼ��ģ������
	char szFileName[256] = "";
	sprintf(szFileName, "%s%s", MODBUSRTUPREFIXFILENAME, m_sTemplatePath);
	line = ReadConf(szFileName); // ��ȡ�����ļ�

	if (line <= 0)
	// line = read_conf("/mynand/config/ModBus/template/default.ini");			//��ȡĬ���ļ�
	{
		printf("don't found %s\n", szFileName);
		/*		MODBUSCONF mc;
				DefaultValConfig(&mc);
				modbus_conf.push_back( mc );
				line = 1;
				*/
	}
	return TRUE;
} /*}}}*/

// ��վ״̬
void CModBusRTU::TimerProc()
{ /*{{{*/
	time_t temp_time;
	time(&temp_time);
	timeflag = (int)(temp_time - last_settime);

	if (m_wErrorTimer > ERROR_CONST) // ����δ���ܱ��Ĵ������࣬������վ״̬
	{
		// if(m_byPortStatus == COMSTATUS_ONLINE)
		//  {
		m_byPortStatus = COMSTATUS_FAULT;
		// char buffer[100] = "";
		// sprintf(buffer,"*****m_byPortStatus = %d*****  COMSTATUS_FAULT\n",m_byPortStatus);
		// OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer),3);
		//  }
	}
	else
	{
		//	if(m_byPortStatus == COMSTATUS_FAULT)
		//   {
		m_byPortStatus = COMSTATUS_ONLINE;
		//	}
	}
} /*}}}*/

// ң���źŴ���
BOOL CModBusRTU::GetYKBuffer(BYTE *buf, int &len, PBUSMSG pBusMsg)
{ /*{{{*/
	int i = 0;
	YK_DATA *yk_data;
	yk_data = (YK_DATA *)pBusMsg->pData;
	// SrcwSerialNo = pBusMsg->SrcInfo.wSerialNo;

	if ((pBusMsg == NULL) && (yk_pos_num == 0))
		return FALSE;
	bySrcBusNo = pBusMsg->SrcInfo.byBusNo;
	wSrcDevAddr = pBusMsg->SrcInfo.wDevNo;

	if (pBusMsg->dwDataType == YK_SEL || pBusMsg->dwDataType == YK_CANCEL) // ң��ѡ��
	{																	   /*{{{*/
		for (i = 0; i < yk_pos_num; i++)
		{
			if (yk_data->wPnt == yk_info[i].start_num)
			//&&( yk_data->wPnt < yk_info[i].start_num + yk_info[i].get_num ) )
			{
				if ((modbus_conf[yk_info[i].pos].yk_form == 2) ||
					(modbus_conf[yk_info[i].pos].yk_form == yk_data->byVal))
				{
					if (pBusMsg->dwDataType == YK_SEL)
					{
						yk_flag = 0;
						switch (modbus_conf[yk_info[i].pos].YkSelFlag)
						{
						case 0:
							m_pMethod->SetYkSelRtn(this, bySrcBusNo, wSrcDevAddr, yk_data->wPnt, yk_data->byVal);
							break;
						case 1:
							YkNo = yk_data->wPnt;
							if (yk_data->byVal == 0 && modbus_conf[yk_info[i].pos].yk_form == 2)
								YkVal = 0;
							else
								YkVal = 1;
							YkPresetSendBuf(modbus_conf[yk_info[i].pos], yk_data, buf, &len);
							pos_flag = yk_info[i].pos;
							return TRUE;
						default:
							return FALSE;
							// break;
						}
					}
					else
					{
						yk_flag = 0;
						m_pMethod->SetYkCancelRtn(this, bySrcBusNo, wSrcDevAddr, yk_data->wPnt, yk_data->byVal);
					}
					return TRUE;
				}
			}
		}
		yk_data->byVal = YK_ERROR;
		yk_flag = 0;
		m_pMethod->SetYkSelRtn(this, bySrcBusNo, wSrcDevAddr, yk_data->wPnt, yk_data->byVal);
		return TRUE;
	} /*}}}*/
	else if (pBusMsg->dwDataType == YK_EXCT) // ң��ִ��
	{										 /*{{{*/
		// printf( "YK EXCT\n");
		for (i = 0; i < yk_pos_num; i++)
		{
			if (yk_data->wPnt == yk_info[i].start_num)
			//&&( yk_data->wPnt < yk_info[i].start_num + yk_info[i].get_num ) )
			{
				if ((modbus_conf[yk_info[i].pos].yk_form == 2) ||
					(modbus_conf[yk_info[i].pos].yk_form == yk_data->byVal))
				{
					switch (modbus_conf[yk_info[i].pos].YkExctFlag)
					{
					case 0:
						YkSendBuf(modbus_conf[yk_info[i].pos], yk_data, buf, &len);
						break;
					case 1:
						YkJ05SendBuf(modbus_conf[yk_info[i].pos], yk_data, buf, &len);
						break;
					default:
						return FALSE;
					}

					YkNo = yk_data->wPnt;
					pos_flag = yk_info[i].pos;
					return TRUE;
				}
			}
		}
		yk_data->byVal = YK_ERROR;
		yk_flag = 0;
		m_pMethod->SetYkExeRtn(this, bySrcBusNo, wSrcDevAddr, yk_data->wPnt, yk_data->byVal);
		return TRUE;
	} /*}}}*/
	else if (pBusMsg->dwDataType == YK_CANCEL)
		printf("YK_CANCEL \n");

	return TRUE;
} /*}}}*/

// �ַ���תʮ�����ƣ���ȡ�����ļ�ʱʹ��
unsigned int CModBusRTU::Atoh(char *buf)
{ /*{{{*/
	BYTE i = 0;
	UINT tempvalue = 0;
	UINT value = 0;
	BYTE len = strlen(buf);

	for (i = 0; i < len; i++)
	{
		if ((buf[i] >= 'A') && (buf[i] <= 'F'))
		{
			tempvalue = buf[i] - 'A' + 10;
		}
		if ((buf[i] >= 'a') && (buf[i] <= 'f'))
		{
			tempvalue = buf[i] - 'a' + 10;
		}
		if ((buf[i] >= '0') && (buf[i] <= '9'))
		{
			tempvalue = buf[i] - '0';
		}
		value = value * 16 + tempvalue;
	}
	return value;
} /*}}}*/

// �������ļ�ĳ�г�������ʱ������л���Ĭ�����ã���Ĭ�����ã��ɷ�����������
void CModBusRTU::DefaultValConfig(MODBUSCONF *mc)
{ /*{{{*/
	mc->type = 1;
	mc->func = 2;
	mc->registe = 0x0064;
	mc->registe_num = 0;
	mc->skew_byte = 3;
	mc->get_num = 0;
	mc->start_num = 0;
	mc->data_len = 1;
	mc->mask_code = 0xffffffff;
	mc->data_form = 0;
	mc->sign = 0;
	mc->yk_form = 2;
	mc->cir_flag = 1;
} /*}}}*/

// ��ȡ�����ļ�
int CModBusRTU::ReadConf(char *filename)
{ /*{{{*/
	FILE *hFile;
	char szText[160];
	char *temp;
	int num = 0;
	BYTE i = 0;
	BYTE conflag = 0;
	MODBUSCONF mc;
	// INFO yk,readval,writeval;
	INFO yk;
	INFO writeval; // д��ֵ�ṹ
	INFO readval;  // ����ֵ�ṹ

	// m_hSem.semTake();

	hFile = fopen(filename, "r");

	if (hFile == NULL)
	{
		perror("Failed to fopen!");
		// printf("FT:fopen conf error!\n");
		// m_hSem.semGive();
		return 0;
	}

	while (fgets(szText, sizeof(szText), hFile) != NULL)
	{
		rtrim(szText);
		if (szText[0] == '#' || szText[0] == ';')
			continue;
		i = 0;
		conflag = 0;
		memset(&mc, 0, sizeof(mc));

		temp = strtok(szText, ",");
		if (temp == NULL)
			continue;
		if ((atoi(temp) > 0) && (atoi(temp) < 10))
			mc.type = atoi(temp);
		else
		{
			conflag = 1;
			DefaultValConfig(&mc);
		}
		while ((temp = strtok(NULL, ",")))
		{
			switch (++i)
			{
			case 1: // ��Atohȫ����Ϊstrtol��strtoll.strtol��strtoll��ϵͳ����Ӧ�ø��ɿ�!
				// if( ( ( (UINT)strtol(temp, NULL, 16) > 0 ) && ( (UINT)strtol(temp, NULL, 16) <= 7 ) ) || ( (UINT)strtol(temp, NULL, 16) == 0x0f )
				//|| ( (UINT)strtol(temp, NULL, 16) == 0x10 ) || ( (UINT)strtol(temp, NULL, 16) == 0x55 ) || ( (UINT)strtol(temp, NULL, 16) == 0x0c ) )
				if ((UINT)strtol(temp, NULL, 16) > 0)
					mc.func = (UINT)strtol(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 2:
				if (((UINT)strtol(temp, NULL, 16) >= 0) && ((UINT)strtol(temp, NULL, 16) <= 0xffff))
					mc.registe = (UINT)strtol(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 3:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 0xffff))
					mc.registe_num = atoi(temp);
				else
					conflag = 1;
				break;
			case 4:
				// ƫ���ֽڴ�����С��������ֽ��� 260 - 2
				if (atoi(temp) >= 0 && atoi(temp) <= 258)
					mc.skew_byte = atoi(temp);
				else
					conflag = 1;
				break;
			case 5:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 0xffff))
					mc.get_num = atoi(temp);
				else
					conflag = 1;
				break;
			case 6:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 0xffff))
					mc.start_num = atoi(temp);
				else
					conflag = 1;
				break;
			case 7:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 8))
					mc.data_len = atoi(temp);
				else
					conflag = 1;
				break;
			case 8:
				if (((UINT)strtol(temp, NULL, 16) >= 0) && ((UINT)strtoll(temp, NULL, 16) <= 0xffffffff))
					mc.mask_code = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 9:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 25)) // 先临时调大点
					mc.data_form = atoi(temp);
				else
					conflag = 1;
				break;
			case 10:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 6))
					mc.sign = atoi(temp);
				else
					conflag = 1;
				break;
			case 11:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 2))
					mc.yk_form = atoi(temp);
				else
					conflag = 1;
				break;
			case 12:
				if ((atoi(temp) == 0) || (atoi(temp) == 1))
				{
					mc.cir_flag = atoi(temp);
					if (mc.cir_flag == 1)
						DevCirFlag = TRUE;
				}
				else
					conflag = 1;
				break;
			case 13:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 0xffff))
					mc.YkClose = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 14:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 0xffff))
					mc.YkOpen = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 15:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 2))
					mc.SetTimeFlag = atoi(temp);
				else
					conflag = 1;
				break;
			case 16:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 2))
					mc.SoeFlag = atoi(temp);
				else
					conflag = 1;
				break;
			case 17:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 2))
					mc.YkSelFlag = atoi(temp);
				else
					conflag = 1;
				break;
			case 18:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 2))
					mc.YkExctFlag = atoi(temp);
				else
					conflag = 1;
				break;
			/*lel*/
			case 19:
				if ((atoi(temp) == 0) || (atoi(temp) == 1) || (atoi(temp) == 2))
					mc.YxProcessMethod = atoi(temp);
				else
					conflag = 1;
				break;
			case 20:
				if ((atoi(temp) >= 0) && (atoi(temp) <= 4))
					mc.YxByBitValue = atoi(temp);
				else
					conflag = 1;
				break;
			case 21:
				printf("eaton flag\n");
				break;
			/*end*/
			default:
				conflag = 1;
				printf("\n\n\n%d > 18\n\n\n", i + 1);
				break;
			}
			if (conflag == 1)
			{
				conflag = 1;
				printf("ModBus config file error:\n");
				continue;
			}
		}
		if (conflag == 1)
		{
			printf("%s num is %d %d\n\n\n", filename, num + 1, i + 1);
			DefaultValConfig(&mc);
		}
		else if (i < 18)
		{
			switch (i + 1)
			{
			case 13:
				mc.YkClose = 0xff00;
			case 14:
				mc.YkOpen = 0x0000;
			case 15:
				mc.SetTimeFlag = 0;
			case 16:
				mc.SoeFlag = 0;
			case 17:
				mc.YkSelFlag = 0;
			case 18:
				mc.YkExctFlag = 0;
			/*lel*/
			case 19:
				mc.YxProcessMethod = 0;
			case 20:
				mc.YxByBitValue = 0;
				break;
				/*end*/
			}
		}
		/*   printf("%d %d %d %d %d %d %d %d %02x %d %d %d %d %d %d %d \n",mc.type,mc.func,
			 mc.registe,mc.registe_num,mc.skew_byte,mc.get_num,mc.start_num,
			 mc.data_len,mc.mask_code,mc.data_form,mc.sign,mc.yk_form,mc.cir_flag,mc.SetTimeFlag ,
			 mc.SoeFlag,mc.YkSetFlag); */
		modbus_conf.push_back(mc);
		num++;
	}
	int freturn = fclose(hFile); // perror("fclose");
	if (freturn)
		perror("fclose");

	// m_hSem.semGive();

	for (i = 0; i < num; i++)
	{
		if (modbus_conf[i].type == 3)
		{
			yk.pos = i;
			yk.start_num = modbus_conf[i].start_num;
			yk.get_num = modbus_conf[i].get_num;

			//	printf("yk_pos_num===%d %d %d %d\n",yk_pos_num,yk.pos,yk.start_num,yk.get_num);
			yk_info.push_back(yk);
			yk_pos_num++;
		}
		if (modbus_conf[i].type == 5)
		{
			readval.pos = i;
			readval.start_num = modbus_conf[i].start_num;
			readval.get_num = modbus_conf[i].get_num;

			printf("readval_pos_num===%d %d %d %d\n", readval_pos_num, readval.pos, readval.start_num, readval.get_num);
			readval_info.push_back(readval);
			readval_pos_num++;
		}
		if (modbus_conf[i].type == 6 || modbus_conf[i].type == 7)
		{
			writeval.pos = i;
			writeval.start_num = modbus_conf[i].start_num;
			writeval.get_num = modbus_conf[i].get_num;

			writeval_info.push_back(writeval);
			writeval_pos_num++;
		}
		else if (modbus_conf[i].type == 8)
		{
			settime_pos = i;
		}
	}
	return num;
} /*}}}*/

int CModBusRTU::TimePackMsecBigEdian(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec)
{ /*{{{*/
	buffer[i++] = modbusconf.registe_num * 2;
	buffer[i++] = (p->tm_year) - 100;
	buffer[i++] = 1 + (p->tm_mon);
	buffer[i++] = p->tm_mday;
	buffer[i++] = p->tm_hour;
	buffer[i++] = p->tm_min;
	buffer[i++] = p->tm_sec;
	buffer[i++] = msec >> 8;
	buffer[i++] = msec;
	return i;
} /*}}}*/

int CModBusRTU::TimePackMsecLittleEdian(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec)
{ /*{{{*/
	buffer[i++] = modbusconf.registe_num * 2;
	buffer[i++] = (p->tm_year) - 100;
	buffer[i++] = 1 + (p->tm_mon);
	buffer[i++] = p->tm_mday;
	buffer[i++] = p->tm_hour;
	buffer[i++] = p->tm_min;
	buffer[i++] = p->tm_sec;
	buffer[i++] = msec;
	buffer[i++] = msec >> 8;
	return i;
} /*}}}*/

int CModBusRTU::TimePackIEC(MODBUSCONF modbusconf, unsigned char *buffer, int i, struct tm *p, WORD msec)
{
	printf("-----%d---msec=%d--\n", p->tm_sec, msec);
	long sec_msec = 0;
	sec_msec = (p->tm_sec) * 1000 + msec;
	buffer[i++] = modbusconf.registe_num * 2;
	buffer[i++] = 0x00; // ȱʡ
	buffer[i++] = (p->tm_year) - 100;
	buffer[i++] = 1 + (p->tm_mon);
	buffer[i++] = p->tm_mday;
	buffer[i++] = p->tm_hour;
	buffer[i++] = p->tm_min;
	buffer[i++] = HIBYTE(sec_msec);
	buffer[i++] = LOBYTE(sec_msec);
	return i;
}

// ʱ�����
int CModBusRTU::SysLocalTime(MODBUSCONF modbusconf, unsigned char *buffer, int i)
{ /*{{{*/
	struct timeval tv;
	struct timezone tz;

	// struct tm *p;
	struct tm p;

	gettimeofday(&tv, &tz);

	// p = localtime( &tv.tv_sec );
	localtime_r(&tv.tv_sec, &p);

	char usec[10] = "";
	sprintf(usec, "%06ld", tv.tv_usec);
	switch (strlen(usec))
	{
	case 4:
		usec[1] = '\0';
		break;
	case 5:
		usec[2] = '\0';
		break;
	case 6:
		usec[3] = '\0';
		break;
	default:
		memset(usec, 0, sizeof(usec));
		break;
	}
	WORD msec = atoi(usec);

	switch (modbusconf.SetTimeFlag)
	{
	case 0:
		i = TimePackMsecBigEdian(modbusconf, buffer, i, &p, msec);
		break;
	case 1:
		i = TimePackMsecLittleEdian(modbusconf, buffer, i, &p, msec);
		break;
	case 2:
		i = TimePackIEC(modbusconf, buffer, i, &p, msec);
		break;

	default:
		i = TimePackMsecBigEdian(modbusconf, buffer, i, &p, msec);
		break;
	}

	return i;
} /*}}}*/

void CModBusRTU::Esl411SoeSendBuf(BYTE *buf, int *len)
{ /*{{{*/
	BYTE index = 0;

	buf[index++] = m_wDevAddr;
	buf[index++] = 0x0c;
	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
} /*}}}*/

// ң�ţ�ң�⣬��ʱ�ȷ��ͱ���
void CModBusRTU::SendBuf(MODBUSCONF modbusconf, BYTE *buf, int *len)
{ /*{{{*/
	BYTE index = 0;

	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	if (modbusconf.type == 9 && modbusconf.SoeFlag == 0)
	{
		;
	}
	else
	{
		buf[index++] = modbusconf.registe >> 8;
		buf[index++] = modbusconf.registe;
		buf[index++] = modbusconf.registe_num >> 8;
		buf[index++] = modbusconf.registe_num;
	}
	/* 	if( modbusconf.type == 6 )
		{
		buf[ index++ ] = modbusconf.registe_num*2 ;
		int i = 0;
		int data_len = modbusconf.data_len;
		int write_num = (2*modbusconf.registe_num)/modbusconf.data_len;
		while( write_num-- )
		{
		while( data_len-- )
		{
		buf[ index++ ] = val[i] >> ( 8 * data_len ) ;
		}
		data_len = modbusconf.data_len;
		i++;
		}
		} */
	if (modbusconf.type == 8)
	{
		index = SysLocalTime(modbusconf, buf, index);
	}

	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
} /*}}}*/

// ����ң��Ԥ�ñ���
void CModBusRTU::YkPresetSendBuf(MODBUSCONF modbusconf, YK_DATA *yk_data, BYTE *buf, int *len)
{ /*{{{*/
#if 0
	BYTE index = 0;
	buf[ index++ ] = m_wDevAddr;
	buf[ index++ ] = 0x10;
	buf[ index++ ] = 0x40;
	buf[ index++ ] = 0x59;
	buf[ index++ ] = 0x00;
	buf[ index++ ] = 0x03;
	buf[ index++ ] = 0x06;
	buf[ index++ ] = modbusconf.func;
	buf[ index++ ] = modbusconf.registe >> 8;
	buf[ index++ ] = ( modbusconf.registe << 8 ) >> 8;


	if( yk_data->byVal == 0 && modbusconf.yk_form ==2 )
	{
		buf[ index++ ] = 0x00;
		buf[ index++ ] = 0x00;
	}
	else
	{
		buf[ index++ ] = 0xff;
		buf[ index++ ] = 0x00;
	}
	buf[ index++ ] = 0xff;
	WORD wCRC = GetCrc( buf, index );
	buf[ index++ ] = HIBYTE(wCRC);
	buf[ index++ ] = LOBYTE(wCRC);
	*len = index;
#else
	BYTE index = 0;
	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = modbusconf.registe;
	(yk_data->byVal == 0 && modbusconf.yk_form == 2) ? buf[index++] = 0xFF : buf[index++] = 0x00;
	buf[index++] = 0x00;
	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
#endif
} /*}}}*/

// ����ң�ر���
void CModBusRTU::YkJ05SendBuf(MODBUSCONF modbusconf, YK_DATA *yk_data, BYTE *buf, int *len)
{ /*{{{*/
	BYTE index = 0;
	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = (modbusconf.registe << 8) >> 8;
	buf[index++] = modbusconf.registe_num >> 8;
	buf[index++] = modbusconf.registe_num;
	buf[index++] = modbusconf.registe_num;

	if (yk_data->byVal == 0 && modbusconf.yk_form == 2)
	{
		buf[index++] = 0x00;
	}
	else
	{
		buf[index++] = 0x01;
	}
	YkVal = buf[index - 1];
	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
} /*}}}*/

// ����ң�ر���
void CModBusRTU::YkSendBuf(MODBUSCONF modbusconf, YK_DATA *yk_data, BYTE *buf, int *len)
{ /*{{{*/
#if 0
	BYTE index = 0;
	buf[ index++ ] = m_wDevAddr;
	buf[ index++ ] = modbusconf.func;
	buf[ index++ ] = modbusconf.registe >> 8;
	buf[ index++ ] = ( modbusconf.registe << 8 ) >> 8;

	if(yk_data->byVal == 0 && modbusconf.yk_form ==2)
	{
		buf[ index++ ] = HIBYTE(modbusconf.YkOpen);
		buf[ index++ ] = LOBYTE(modbusconf.YkOpen);
	}
	else
	{
		buf[ index++ ] = HIBYTE(modbusconf.YkClose);
		buf[ index++ ] = LOBYTE(modbusconf.YkClose);
	}
	WORD wCRC = GetCrc( buf, index );
	buf[ index++ ] = HIBYTE(wCRC);
	buf[ index++ ] = LOBYTE(wCRC);
	*len = index;
#else
	BYTE index = 0;
	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = modbusconf.registe;
	if (yk_data->byVal == 0 && modbusconf.yk_form == 2)
	{
		buf[index++] = HIBYTE(modbusconf.YkOpen);
		buf[index++] = LOBYTE(modbusconf.YkOpen);
	}
	else
	{
		buf[index++] = HIBYTE(modbusconf.YkClose);
		buf[index++] = LOBYTE(modbusconf.YkClose);
	}
	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
#endif
} /*}}}*/

void CModBusRTU::ReadvalSendBuf(MODBUSCONF modbusconf, BYTE *buf, int *len)
{ /*{{{*/
	BYTE index = 0;
	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = modbusconf.registe;
	buf[index++] = modbusconf.registe_num >> 8;
	buf[index++] = modbusconf.registe_num;
	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
} /*}}}*/

void CModBusRTU::WritevalSendBuf(MODBUSCONF modbusconf, unsigned int *val, BYTE *buf, int *len)
{ /*{{{*/
	int index = 0;
	buf[index++] = m_wDevAddr;
	buf[index++] = modbusconf.func;
	buf[index++] = modbusconf.registe >> 8;
	buf[index++] = modbusconf.registe;
	buf[index++] = modbusconf.registe_num >> 8;
	buf[index++] = modbusconf.registe_num;
	buf[index++] = modbusconf.registe_num * 2;
	int i = 0;
	int data_len = modbusconf.data_len;
	int write_num = (2 * modbusconf.registe_num) / modbusconf.data_len;
	while (write_num--)
	{
		while (data_len--)
		{
			buf[index++] = val[i] >> (8 * data_len);
		}
		data_len = modbusconf.data_len;
		i++;
	}
	WORD wCRC = GetCrc(buf, index);
	buf[index++] = HIBYTE(wCRC);
	buf[index++] = LOBYTE(wCRC);
	*len = index;
} /*}}}*/

// ���ݳ��ȣ����ݸ�ʽ����
short CModBusRTU::TwoByteValue(unsigned char *buffer, int pos, MODBUSCONF modbusconf)
{ /*{{{*/
	short val = 0;
	unsigned char buf[2] = "";

	// �����봦
	// unsigned int temp_mask = modbusconf.mask_code;
	// buf[0] = buffer[a] & ((temp_mask<<16)>>24);
	// buf[1] = buffer[a+1] & ((temp_mask<<24)>>24);

	buf[0] = buffer[pos];
	buf[1] = buffer[pos + 1];
	switch (modbusconf.data_form)
	{
	case 0:
		val = (buf[0] << 8 | buf[1]);
		break;
	case 1:
		val = (buf[0] | buf[1] << 8);
		break;
	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	return val;
} /*}}}*/

// ���ݳ��ȣ����ݸ�ʽ����
unsigned short CModBusRTU::TwoByteValue_unsigned(unsigned char *buffer, int pos, MODBUSCONF modbusconf)
{ /*{{{*/
	unsigned short val = 0;
	unsigned char buf[2] = "";

	buf[0] = buffer[pos];
	buf[1] = buffer[pos + 1];
	switch (modbusconf.data_form)
	{
	case 0:
		val = (buf[0] << 8 | buf[1]);
		break;
	case 1:
		val = (buf[0] | buf[1] << 8);
		break;
	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	return val;
} /*}}}*/

// ���ݳ��ȣ����ݸ�ʽ����
int CModBusRTU::FourByteValue(unsigned char *buffer, int a, MODBUSCONF modbusconf)
{ /*{{{*/
	int val = 0;
	int i1, i2, i3, i4;

	unsigned char buf[4] = "";
	// �����봦��
	// unsigned int temp_mask = modbusconf.mask_code;
	// buf[0] = buffer[a] & (temp_mask>>24);
	// buf[1] = buffer[a+1] & ((temp_mask<<8)>>24);
	// buf[2] = buffer[a+2] & ((temp_mask<<16)>>24);
	// buf[3] = buffer[a+3] & ((temp_mask<<24)>>24);

	buf[0] = buffer[a];
	buf[1] = buffer[a + 1];
	buf[2] = buffer[a + 2];
	buf[3] = buffer[a + 3];
	switch (modbusconf.data_form)
	{
	case 2:
		val = (buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
		break;
	case 3:
		val = (buf[0] << 16 | buf[1] << 24 | buf[2] | buf[3] << 8);
		break;
	case 4:
		val = (buf[0] << 8 | buf[1] | buf[2] << 24 | buf[3] << 16);
		break;
	case 5:
		val = (buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
		break;
	case 6:
	{
		/* int i1(GetBCD(buf[0])), i2(GetBCD(buf[1])), i3(GetBCD(buf[2])), i4(GetBCD(buf[3]));*/
		i1 = GetBCD(buf[0]);
		i2 = GetBCD(buf[1]);
		i3 = GetBCD(buf[2]);
		i4 = GetBCD(buf[3]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;
	case 7:
	{
		i1 = GetBCD(buf[1]);
		i2 = GetBCD(buf[0]);
		i3 = GetBCD(buf[3]);
		i4 = GetBCD(buf[2]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;
	case 8:
	{
		i1 = GetBCD(buf[2]);
		i2 = GetBCD(buf[3]);
		i3 = GetBCD(buf[0]);
		i4 = GetBCD(buf[1]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;
	case 9:
	{
		i1 = GetBCD(buf[3]);
		i2 = GetBCD(buf[2]);
		i3 = GetBCD(buf[1]);
		i4 = GetBCD(buf[0]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;

	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	return val;
} /*}}}*/

// ���ݳ��ȣ����ݸ�ʽ����
unsigned int CModBusRTU::FourByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf)
{ /*{{{*/
	unsigned int val = 0;
	int i1, i2, i3, i4;

	unsigned char buf[4] = "";

	buf[0] = buffer[a];
	buf[1] = buffer[a + 1];
	buf[2] = buffer[a + 2];
	buf[3] = buffer[a + 3];
	switch (modbusconf.data_form)
	{
	case 2:
		val = (buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]);
		break;
	case 3:
		val = (buf[0] << 16 | buf[1] << 24 | buf[2] | buf[3] << 8);
		break;
	case 4:
		val = (buf[0] << 8 | buf[1] | buf[2] << 24 | buf[3] << 16);
		break;
	case 5:
		val = (buf[0] | buf[1] << 8 | buf[2] << 16 | buf[3] << 24);
		break;
	case 6:
	{
		/* int i1(GetBCD(buf[0])), i2(GetBCD(buf[1])), i3(GetBCD(buf[2])), i4(GetBCD(buf[3]));*/
		i1 = GetBCD(buf[0]);
		i2 = GetBCD(buf[1]);
		i3 = GetBCD(buf[2]);
		i4 = GetBCD(buf[3]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;
	case 7:
	{
		i1 = GetBCD(buf[1]);
		i2 = GetBCD(buf[0]);
		i3 = GetBCD(buf[3]);
		i4 = GetBCD(buf[2]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;
	case 8:
	{
		i1 = GetBCD(buf[2]);
		i2 = GetBCD(buf[3]);
		i3 = GetBCD(buf[0]);
		i4 = GetBCD(buf[1]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;
	case 9:
	{
		i1 = GetBCD(buf[3]);
		i2 = GetBCD(buf[2]);
		i3 = GetBCD(buf[1]);
		i4 = GetBCD(buf[0]);
		val = i3 * pow(10, 6) + i4 * pow(10, 4) + i1 * pow(10, 2) + i2;
	}
	break;

	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	return val;
} /*}}}*/

int CModBusRTU::GetBCD(BYTE byData)
{
	return (byData / 16) * 10 + byData % 16;
}

// �з���Ϊfloat�����ݸ�ʽ����
float CModBusRTU::FloatValue(unsigned char *buffer, int a, MODBUSCONF modbusconf)
{ /*{{{*/
	float val = 0;
	unsigned char float_buf[4] = "";

	unsigned char buf[4] = "";
	// �����봦��
	unsigned int temp_mask = modbusconf.mask_code;
	buf[0] = buffer[a] & (temp_mask >> 24);
	buf[1] = buffer[a + 1] & ((temp_mask << 8) >> 24);
	buf[2] = buffer[a + 2] & ((temp_mask << 16) >> 24);
	buf[3] = buffer[a + 3] & ((temp_mask << 24) >> 24);
	switch (modbusconf.data_form)
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
		MsgErrorFlag = MSGERROR;
		break;
	}
	val = *(float *)float_buf;
	return val;
} /*}}}*/
// ���ݳ��ȣ����ݸ�ʽ����
unsigned long long CModBusRTU::EightByteValue_unsigned(unsigned char *buffer, int a, MODBUSCONF modbusconf)
{ /*{{{*/
	unsigned long long val = 0;
	unsigned char buf[8] = "";
	buf[0] = buffer[a];
	buf[1] = buffer[a + 1];
	buf[2] = buffer[a + 2];
	buf[3] = buffer[a + 3];
	buf[4] = buffer[a + 4];
	buf[5] = buffer[a + 5];
	buf[6] = buffer[a + 6];
	buf[7] = buffer[a + 7];

	switch (modbusconf.data_form)
	{
	case 10: // 标准大端 (Big-Endian)  (ABCDEFGH)
		//val = (buf[0] << 56 | buf[1] << 48 | buf[2] << 40 | buf[3] << 32 | buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7]);
		val = ((uint64_t)buf[0] << 56) |
			  ((uint64_t)buf[1] << 48) |
			  ((uint64_t)buf[2] << 40) |
			  ((uint64_t)buf[3] << 32) |
			  ((uint64_t)buf[4] << 24) |
			  ((uint64_t)buf[5] << 16) |
			  ((uint64_t)buf[6] << 8)  |
			  (uint64_t)buf[7];
		break;
	case 11: // 标准小端 (Little-Endian) (HGFEDCBA)
		val = ((uint64_t)buf[7] << 56) |
			  ((uint64_t)buf[6] << 48) |
			  ((uint64_t)buf[5] << 40) |
			  ((uint64_t)buf[4] << 32) |
			  ((uint64_t)buf[3] << 24) |
			  ((uint64_t)buf[2] << 16) |
			  ((uint64_t)buf[1] << 8)  |
			  (uint64_t)buf[0];
		break;
	case 12: // 双字交换大端 (Big-Endian with word swap) （BADCFEHG）
		val = ((uint64_t)buf[1] << 56) |
			  ((uint64_t)buf[0] << 48) |
			  ((uint64_t)buf[3] << 40) |
			  ((uint64_t)buf[2] << 32) |
			  ((uint64_t)buf[5] << 24) |
			  ((uint64_t)buf[4] << 16) |
			  ((uint64_t)buf[7] << 8)  |
			  (uint64_t)buf[6];
		break;
	case 13: // 小端双字交换 (Little-Endian with DWORD swap) （GHEFCDAB）
		val = ((uint64_t)buf[6] << 56) |
			  ((uint64_t)buf[7] << 48) |
			  ((uint64_t)buf[4] << 40) |
			  ((uint64_t)buf[5] << 32) |
			  ((uint64_t)buf[2] << 24) |
			  ((uint64_t)buf[3] << 16) |
			  ((uint64_t)buf[0] << 8)  |
			  (uint64_t)buf[1];
		break;
	case 14: // 中端格式Mid-Big-Endian (CDABHGEF)
		val = ((uint64_t)buf[2] << 56) |
			  ((uint64_t)buf[3] << 48) |
			  ((uint64_t)buf[0] << 40) |
			  ((uint64_t)buf[1] << 32) |
			  ((uint64_t)buf[7] << 24) |
			  ((uint64_t)buf[6] << 16) |
			  ((uint64_t)buf[4] << 8)  |
			  (uint64_t)buf[5];
		break;
	case 15: // Mid-Big-Endian (DCBAHGFE)
		val = ((uint64_t)buf[3] << 56) |
			  ((uint64_t)buf[2] << 48) |
			  ((uint64_t)buf[1] << 40) |
			  ((uint64_t)buf[0] << 32) |
			  ((uint64_t)buf[7] << 24) |
			  ((uint64_t)buf[6] << 16) |
			  ((uint64_t)buf[5] << 8)  |
			  (uint64_t)buf[4];
		break;
	case 16: //(BAFEDCHG)
		val = ((uint64_t)buf[1] << 56) |
			  ((uint64_t)buf[0] << 48) |
			  ((uint64_t)buf[5] << 40) |
			  ((uint64_t)buf[4] << 32) |
			  ((uint64_t)buf[3] << 24) |
			  ((uint64_t)buf[2] << 16) |
			  ((uint64_t)buf[7] << 8)  |
			  (uint64_t)buf[6];
		break;
	case 17: // 电力行业B格式(ABCDHGEF)
		val = ((uint64_t)buf[0] << 56) |
			  ((uint64_t)buf[1] << 48) |
			  ((uint64_t)buf[2] << 40) |
			  ((uint64_t)buf[3] << 32) |
			  ((uint64_t)buf[7] << 24) |
			  ((uint64_t)buf[6] << 16) |
			  ((uint64_t)buf[4] << 8)  |
			  (uint64_t)buf[5];
		break;
	case 18: // 轨道交通EFGHABCD
		val = ((uint64_t)buf[4] << 56) |
			  ((uint64_t)buf[5] << 48) |
			  ((uint64_t)buf[6] << 40) |
			  ((uint64_t)buf[7] << 32) |
			  ((uint64_t)buf[0] << 24) |
			  ((uint64_t)buf[1] << 16) |
			  ((uint64_t)buf[2] << 8)  |
			  (uint64_t)buf[3];
		break;
	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	return val;
} /*}}}*/
// �з���Ϊdouble ���ݸ�ʽ���� �����յ����� 9a 99 99 99 99 99 28 40----��12.3
double CModBusRTU::DoubleValue(unsigned char *buffer, int a, MODBUSCONF modbusconf)
{ /*{{{*/
	double val = 0;
	unsigned char double_buf[8] = "";
	unsigned char buf[8] = "";
	buf[0] = buffer[a];
	buf[1] = buffer[a + 1];
	buf[2] = buffer[a + 2];
	buf[3] = buffer[a + 3];
	buf[4] = buffer[a + 4];
	buf[5] = buffer[a + 5];
	buf[6] = buffer[a + 6];
	buf[7] = buffer[a + 7];
	switch (modbusconf.data_form)
	{
	case 12: // modbus slave display对应double HGFEDCBA
		double_buf[0] = buf[0];
		double_buf[1] = buf[1];
		double_buf[2] = buf[2];
		double_buf[3] = buf[3];
		double_buf[4] = buf[4];
		double_buf[5] = buf[5];
		double_buf[6] = buf[6];
		double_buf[7] = buf[7];
		break;
	case 13: // modbus slave display 对应double ABCDEFGH
		double_buf[7] = buf[0];
		double_buf[6] = buf[1];
		double_buf[5] = buf[2];
		double_buf[4] = buf[3];
		double_buf[3] = buf[4];
		double_buf[2] = buf[5];
		double_buf[1] = buf[6];
		double_buf[0] = buf[7];
		break;
	case 14: // modbus slave display 对应double BADCFFEHG
		double_buf[7] = buf[1];
		double_buf[6] = buf[0];
		double_buf[5] = buf[3];
		double_buf[4] = buf[2];
		double_buf[3] = buf[5];
		double_buf[2] = buf[4];
		double_buf[1] = buf[7];
		double_buf[0] = buf[6];
		break;
	case 15: //  modbus slave display 对应double GHEFCDAB)
		double_buf[7] = buf[6];
		double_buf[6] = buf[7];
		double_buf[5] = buf[4];
		double_buf[4] = buf[5];
		double_buf[3] = buf[2];
		double_buf[2] = buf[3];
		double_buf[1] = buf[0];
		double_buf[0] = buf[1];
		break;
	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	val = *(double *)double_buf;
	return val;
} /*}}}*/

// ���ݸ�ʽ���з���
double CModBusRTU::ModBusValue(unsigned char *buffer, int pos, MODBUSCONF modbusconf)
{ /*{{{*/
	float value_f = 0;
	int value_i = 0;
	unsigned int value_ui = 0;
	short value_s = 0;
	unsigned short value_us = 0;
	double value_d = 0;
	unsigned long long value_ll = 0;
	switch (modbusconf.sign)
	{
		// �޷������δ���
	case 0:
		if (modbusconf.data_len == 2)
		{
			value_us = (unsigned short)TwoByteValue_unsigned(buffer, pos, modbusconf);
			return (double)value_us;
		}
		else if (modbusconf.data_len == 4)
		{
			value_ui = (unsigned int)FourByteValue_unsigned(buffer, pos, modbusconf);
			return (double)value_ui;
		}
		else if (modbusconf.data_len == 8)
		{
			value_ll = (unsigned long long)EightByteValue_unsigned(buffer, pos, modbusconf);
			return (double)value_ll;
		}
		else if (modbusconf.data_len == 1)
		{
			value_ui = buffer[pos];
			return (double)value_ui;
		}
		break;
		// �з������δ�������λΪ����λ��1 Ϊ������0 Ϊ����������Ϊ����ֵ
	case 1:
		if (modbusconf.data_len == 2)
		{
			value_s = TwoByteValue(buffer, pos, modbusconf);
			if (value_s < 0)
				value_s = ((~value_s) | 0x8000) + 1;
			return (double)value_s;
		}
		else if (modbusconf.data_len == 4)
		{
			value_i = FourByteValue(buffer, pos, modbusconf);
			if (value_i < 0)
				value_i = ((~value_i) | 0x80000000) + 1;
			return (double)value_i;
		}
		break;
		// �з������δ���������
	case 2:
		if (modbusconf.data_len == 2)
		{
			value_s = TwoByteValue(buffer, pos, modbusconf);
			return (double)value_s;
		}
		else if (modbusconf.data_len == 4)
		{
			value_i = FourByteValue(buffer, pos, modbusconf);
			return (double)value_i;
		}
		break;
		// float�����ʹ���
	case 3:
		value_f = FloatValue(buffer, pos, modbusconf);
		return (double)value_f;
		// float�����ʹ���,ȡ����ֵ
	case 4:
		value_f = FloatValue(buffer, pos, modbusconf);
		return fabs(value_f);
		break;
	case 6:
		value_d = DoubleValue(buffer, pos, modbusconf);
		return value_d;
	default:
		MsgErrorFlag = MSGERROR;
		break;
	}
	return value_f;
} /*}}}*/

// ң�Ŵ���
void CModBusRTU::ModBusYxDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	if (modbusconf.SoeFlag == 2)
	{
		ModBusRsl_411YxYcDeal(buffer, modbusconf);
		return;
	}
	/*lel*/
	switch (modbus_conf[pos_flag].YxProcessMethod)
	{
	case 0:
		ModBusYxBitDeal(buffer, modbusconf);
		break;
	case 1:
		ModBusYxByteDeal(buffer, modbusconf);
		break;
	case 2:
		ModBusYxByteDeal_new(buffer, modbusconf);
	default:
		ModBusYxBitDeal(buffer, modbusconf);
		break;
	}
	/*end*/
} /*}}}*/

UINT CModBusRTU::ModBusYXTempValue(unsigned char *buffer, int pos, MODBUSCONF modbusconf)
{ /*{{{*/
	int value_i = 0;
	unsigned int value_ui = 0;
	short value_s = 0;
	unsigned short value_us = 0;
	BYTE value_b;

	if (modbusconf.data_len == 1)
	{
		value_b = buffer[pos];
		return (UINT)value_b;
	}
	else if (modbusconf.data_len == 2)
	{
		value_s = (unsigned short)TwoByteValue(buffer, pos, modbusconf);
		memcpy(&value_us, &value_s, 2);
		return (UINT)value_us;
	}
	else if (modbusconf.data_len == 4)
	{
		value_i = (unsigned int)FourByteValue(buffer, pos, modbusconf);
		memcpy(&value_ui, &value_i, 4);
		return value_ui;
	}
	else
	{
		MsgErrorFlag = MSGERROR;
	}
	return 0;
} /*}}}*/

// ң�Ű�λ����
void CModBusRTU::ModBusYxBitDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	WORD i = 0;
	WORD j = 0;
	BYTE pos = modbusconf.skew_byte;
	UINT temp_buf = buffer[pos];
	UINT temp_mask = modbusconf.mask_code;
	WORD get_num = modbusconf.get_num;
	WORD start_num = modbusconf.start_num;
	BYTE data_len = modbusconf.data_len;
	WORD real_get_num = buffer[2] * 8;

	WORD num = 0;
	WORD wVal = 0;
	UINT real_temp_mask = 0;

	if (modbusconf.SoeFlag == 2)
	{
		get_num = 25;
	}
	// ���õĲɼ��������ܴ��ڿ��Խ���������
	if (real_get_num < get_num)
	{
		get_num = real_get_num;
	}

	for (i = 0; i < (((real_get_num - 1) / (8 * data_len)) + 1); i++)
	{
		switch (data_len)
		{
		case 1:
			real_temp_mask = ModBusYXTempValue((unsigned char *)&temp_mask, 3, modbusconf);
			break;
		case 2:
			real_temp_mask = ModBusYXTempValue((unsigned char *)&temp_mask, 2, modbusconf);
			break;
		case 4:
			real_temp_mask = ModBusYXTempValue((unsigned char *)&temp_mask, 0, modbusconf);
			break;
		default:
			MsgErrorFlag = MSGERROR;
			return;
		}

		temp_buf = ModBusYXTempValue(buffer, pos, modbusconf);
		// printf("real_temp_mask =  %x temp_buf = %04x\n",real_temp_mask,temp_buf);
		pos += data_len;

		for (j = 0; j < data_len * 8; j++)
		{
			if (num >= modbusconf.get_num)
			{
				return;
			}
			if (real_temp_mask % 2) // real_temp_mask |0x01				//�����봦��
			{
				wVal = temp_buf % 2; // wVal = temp_buf | 0x01;
				// printf("serialno:%d	order:%d	wVal:%d\n", m_SerialNo, num+start_num, wVal);
				m_pMethod->SetYxData(m_SerialNo, num + start_num, wVal);
				// char buf[100];
				// sprintf(buf,"YX1 m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, m_wDevAddr , num+start_num , wVal);
				// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
				num++;
			}
			real_temp_mask /= 2; // real_temp_mask>>1
			temp_buf /= 2;		 // temp_buf>>1
		}
	}
} /*}}}*/

void CModBusRTU::ModBusYxByteDeal_new(unsigned char *buffer, MODBUSCONF modbusconf)
{
	BYTE pos = modbusconf.skew_byte;
	WORD get_num = modbusconf.get_num;
	WORD start_num = modbusconf.start_num;
	WORD real_get_num = buffer[2] / modbusconf.data_len;

	WORD i = 0;
	BYTE wVal = 0;
	for (i = 0; i < get_num; i++)
	{
		wVal = 0;
		if (i < real_get_num)
		{
			if (modbusconf.data_len == 2)
			{
				wVal = (unsigned short)TwoByteValue_unsigned(buffer, pos, modbusconf);
			}
			else if (modbusconf.data_len == 4)
			{
				wVal = (unsigned int)FourByteValue_unsigned(buffer, pos, modbusconf);
			}
			else if (modbusconf.data_len == 8)
			{
				wVal = (unsigned long long)EightByteValue_unsigned(buffer, pos, modbusconf);
			}
			else if (modbusconf.data_len == 1)
			{
				wVal = buffer[pos];
			}
		}
		if (wVal > 255)
			return;

		if (MsgErrorFlag == MSGERROR)
		{
			return;
		}
		pos += modbusconf.data_len;

		m_pMethod->SetYxVariousData(m_SerialNo, i + start_num, (WORD)wVal);
	}
}

/*lel*/
// --------------------------------------------------------
/// \��Ҫ:	ң�Ű�ֵ����
///
/// \����:	buffer
/// \����:	modbusconf
// --------------------------------------------------------
void CModBusRTU::ModBusYxByteDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	WORD i = 0;
	WORD j = 0;
	BYTE pos = modbusconf.skew_byte;
	UINT temp_buf = buffer[pos];
	UINT temp_mask = modbusconf.mask_code;
	WORD get_num = modbusconf.get_num;
	WORD start_num = modbusconf.start_num;
	BYTE data_len = modbusconf.data_len;
	WORD real_get_num = buffer[2] * 8;

	WORD num = 0;
	WORD wVal = 0;
	UINT real_temp_mask = 0;
	WORD wByteVal = 0;
	WORD wNum = 0;

	if (modbusconf.SoeFlag == 2)
	{
		get_num = 25;
	}
	// ���õĲɼ��������ܴ��ڿ��Խ���������
	if (real_get_num < get_num)
	{
		get_num = real_get_num;
	}

	for (i = 0; i < (((real_get_num - 1) / (8 * data_len)) + 1); i++)
	{
		switch (data_len)
		{
		case 1:
			real_temp_mask = ModBusYXTempValue((unsigned char *)&temp_mask, 3, modbusconf);
			break;
		case 2:
			real_temp_mask = ModBusYXTempValue((unsigned char *)&temp_mask, 2, modbusconf);
			break;
		case 4:
			real_temp_mask = ModBusYXTempValue((unsigned char *)&temp_mask, 0, modbusconf);
			break;
		default:
			MsgErrorFlag = MSGERROR;
			return;
		}

		temp_buf = ModBusYXTempValue(buffer, pos, modbusconf);
		// printf("real_temp_mask =  %x temp_buf = %04x\n",real_temp_mask,temp_buf);
		pos += data_len;

		if (modbusconf.YxByBitValue == 2)
		{
			for (j = 0; j < data_len * 8; j++)
			{
				if (wNum >= modbusconf.get_num)
					return;
				if (real_temp_mask % 2)
				{						 // real_temp_mask |0x01				//�����봦��
					wVal = temp_buf % 2; // wVal = temp_buf | 0x01;
					// printf("serialno:%d	order:%d	wVal:%d", m_SerialNo, num+start_num, wVal);
					wByteVal |= (wVal << wNum);
					if (wNum > 1)
						continue;
					if (!((wNum + 1) % 2))
					{
						switch (wByteVal)
						{
						case 0:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 0)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 1:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 1)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 2:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 2)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 3:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 3)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						default:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 0)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						}
					}
					// char buf[100];
					// sprintf(buf,"YX1 m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, m_wDevAddr , num+start_num , wVal);
					// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
					wNum++;
				}
				real_temp_mask /= 2; // real_temp_mask>>1
				temp_buf /= 2;		 // temp_buf>>1
			}
		}
		else if (modbusconf.YxByBitValue == 3)
		{
			for (j = 0; j < data_len * 8; j++)
			{
				if (wNum >= modbusconf.get_num)
					return;
				if (real_temp_mask % 2)
				{						 // real_temp_mask |0x01				//�����봦��
					wVal = temp_buf % 2; // wVal = temp_buf | 0x01;
					// printf("serialno:%d	order:%d	wVal:%d", m_SerialNo, num+start_num, wVal);
					wByteVal |= (wVal << wNum);
					if (wNum > 2)
						continue;
					if (!((wNum + 1) % 3))
					{
						switch (wByteVal)
						{
						case 0:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 0)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 1:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 1)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 2:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 2)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 3:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 3)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 4:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 4)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 5:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 5)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 6:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 6)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 7:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 7)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						default:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 0)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						}
					}
					// char buf[100];
					// sprintf(buf,"YX1 m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, m_wDevAddr , num+start_num , wVal);
					// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
					wNum++;
				}
				real_temp_mask /= 2; // real_temp_mask>>1
				temp_buf /= 2;		 // temp_buf>>1
			}
		}
		else if (modbusconf.YxByBitValue == 4)
		{
			for (j = 0; j < data_len * 8; j++)
			{
				if (wNum >= modbusconf.get_num)
					return;
				if (real_temp_mask % 2)
				{						 // real_temp_mask |0x01				//�����봦��
					wVal = temp_buf % 2; // wVal = temp_buf | 0x01;
					// printf("serialno:%d	order:%d	wVal:%d", m_SerialNo, num+start_num, wVal);
					wByteVal |= (wVal << wNum);
					if (wNum > 3)
						continue;

					if (!((wNum + 1) % 4))
					{
						switch (wByteVal)
						{
						case 0:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 0)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 1:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 1)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 2:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 2)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 3:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 3)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 4:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 4)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 5:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 5)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 6:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 6)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 7:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 7)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 8:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 8)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 9:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 9)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 10:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 10)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 11:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 11)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 12:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 12)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 13:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 13)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 14:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 14)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						case 15:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 15)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						default:
							for (int i = 0; i < modbusconf.get_num; i++)
							{
								if (i == 0)
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 1);
								else
									m_pMethod->SetYxData(m_SerialNo, num + start_num, 0);
								num++;
							}
							break;
						}
					}
					// char buf[100];
					// sprintf(buf,"YX1 m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, m_wDevAddr , num+start_num , wVal);
					// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
					wNum++;
				}
				real_temp_mask /= 2; // real_temp_mask>>1
				temp_buf /= 2;		 // temp_buf>>1
			}
		}
	}
} /*}}}*/
/*end*/

void CModBusRTU::ModBusRsl_411YxYcDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE Fun = buffer[1];
	switch (Fun)
	{
	case 2:
		ModBusYxBitDeal(buffer, modbusconf);
		ESL411SOEFlag = (buffer[5] & 0x04) / 4;
		break;
	case 3:
		ModBusYcDeal(buffer, modbusconf);
		break;
	case 0x0c:
		ModBusEsl_411SoeDeal(buffer, modbusconf);
		break;
	default:
		return;
	}
} /*}}}*/

// ң�⴦��
void CModBusRTU::ModBusYcDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE pos = modbusconf.skew_byte;
	WORD get_num = modbusconf.get_num;
	WORD start_num = modbusconf.start_num;
	WORD real_get_num = buffer[2] / modbusconf.data_len;

	WORD i = 0;
	float wVal = 0;
	for (i = 0; i < get_num; i++)
	{
		wVal = 0;
		if (i < real_get_num)
			wVal = ModBusValue(buffer, pos, modbusconf);
		// printf("********	wVal:%f	********\n",  ModBusValue(buffer, pos, modbusconf));
		if (MsgErrorFlag == MSGERROR)
		{
			return;
		}
		pos += modbusconf.data_len;
		// printf("********	wVal:%f	********\n", wVal);
		m_pMethod->SetYcData(m_SerialNo, i + start_num, wVal);
		// printf("********	wVal:%f	********\n", wVal);

		// char buf[100];
		// sprintf(buf,"YC m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n",m_byLineNo, m_wDevAddr , i+start_num , wVal);
		// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}
} /*}}}*/

void CModBusRTU::ModBusJ05YkDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	m_pMethod->SetYkExeRtn(this, bySrcBusNo, wSrcDevAddr, YkNo, YkVal);
	// char buf[100];
	// sprintf(buf,"YK m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, wSrcDevAddr , YkNo , YkVal);
	// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
} /*}}}*/

// ң�ش���
void CModBusRTU::ModBusRtuYkDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	WORD i = 0;
	BYTE ykVal = 2;
	BYTE pos = modbusconf.skew_byte;
	UINT get_num = modbusconf.get_num;
	UINT start_num = modbusconf.start_num;
	UINT registe = modbusconf.registe;

	unsigned int recv_buf[get_num];
	memset(recv_buf, 0, get_num);

	yk_flag = 0;

	for (i = 0; i < get_num; i++)
	{
		recv_buf[i] = (unsigned int)ModBusValue(buffer, pos, modbusconf);
		if (MsgErrorFlag == MSGERROR)
		{
			return;
		}
		pos += modbusconf.data_len;
	}

	if ((registe + YkNo - start_num) == recv_buf[0])
	{
		if (modbusconf.YkClose == recv_buf[1])
			ykVal = 1;
		else if (modbusconf.YkOpen == recv_buf[1])
			ykVal = 0;
		m_pMethod->SetYkExeRtn(this, bySrcBusNo, wSrcDevAddr, YkNo, ykVal);
		// char buf[100];
		// sprintf(buf,"YK m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, m_wDevAddr , YkNo , ykVal);
		// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}
} /*}}}*/

void CModBusRTU::ModBusYkDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	switch (modbusconf.YkExctFlag)
	{
	case 0:
		ModBusRtuYkDeal(buffer, modbusconf);
		break;
	case 1:
		ModBusJ05YkDeal(buffer, modbusconf);
		break;
	default:
		MsgErrorFlag = MSGERROR;
		return;
	}
} /*}}}*/

// ң������
void CModBusRTU::ModBusYmDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE pos = modbusconf.skew_byte;
	WORD get_num = modbusconf.get_num;
	WORD start_num = modbusconf.start_num;
	WORD real_get_num = buffer[2] / modbusconf.data_len;

	WORD i = 0;
	QWORD wVal = 0;

	for (i = 0; i < get_num; i++)
	{
		wVal = 0;
		if (i < real_get_num)
		{
			if (modbusconf.data_len == 8)
			{
				if (modbusconf.sign == 0)
				{
					wVal = EightByteValue_unsigned(buffer, pos, modbusconf); //(unsigned long long)

				}
				else if (modbusconf.sign == 6)
					wVal = static_cast<QWORD>(DoubleValue(buffer, pos, modbusconf)); //(double)
			}
			else
				wVal = static_cast<QWORD>(ModBusValue(buffer, pos, modbusconf));
		}
		if (MsgErrorFlag == MSGERROR)
		{
			return;
		}
		pos += modbusconf.data_len;
		m_pMethod->SetYmData(m_SerialNo, i + start_num, wVal);
		// char buf[100];
		// sprintf(buf,"YM m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n", m_byLineNo, m_wDevAddr , i+start_num , wVal);
		// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}
} /*}}}*/

// ��ʱ����
void CModBusRTU::ModBusSetTime(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE pos = modbusconf.skew_byte;
	WORD get_num = modbusconf.get_num;
	// unsigned int registe = modbusconf.registe;
	// unsigned int registe_num = modbusconf.registe_num;

	unsigned int recv_buf[get_num];
	memset(recv_buf, 0, get_num);

	WORD i = 0;

	for (i = 0; i < get_num; i++)
	{
		recv_buf[i] = (unsigned int)ModBusValue(buffer, pos, modbusconf);
		pos += modbusconf.data_len;
	}
	/* 	if( (registe==recv_buf[0]) && (registe_num==recv_buf[1]))
		printf("settime success!!\n");
		else
		printf("settime defeat!!\n"); */
} /*}}}*/

void CModBusRTU::ModBusRtuSoeDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE pos = modbusconf.skew_byte;
	// WORD start_num = modbusconf.start_num;
	int i = 0;
	BYTE soeflag = 0;
	TIMEDATA ptmData;

	for (i = 1; i < buffer[pos - 1]; i += 8) // ��1��ʼ�������soe�������ֽ���(�������ֽ�)Ϊ1
	{
		if (0x0 == (buffer[i + pos] & 0xc0))
		{
			soeflag = 1;
		}
		else if (0xc0 == (buffer[i + pos] & 0xc0))
		{
			soeflag = 0;
		}
		else
		{
			continue;
		}

		ptmData.MiSec = ((buffer[i + pos + 6] & 0xc0) << 2) | buffer[i + pos + 7];
		ptmData.Second = buffer[i + pos + 6] & 0x3f;
		ptmData.Minute = buffer[i + pos + 5];
		ptmData.Hour = buffer[i + pos + 4];
		ptmData.Day = buffer[i + pos + 3];
		ptmData.Month = buffer[i + pos + 2];
		ptmData.Year = buffer[i + pos + 1];

		m_pMethod->SetYxDataWithTime(m_SerialNo, buffer[i + pos] & 0x3f, soeflag, &ptmData);
		// char buf[100] = "";
		// sprintf(buf,"SOE m_byLineNo:%d m_wDevAddr%d num:%d val:%d time:%d.%d.%d-%d:%d:%d.%d\n",
		// m_byLineNo, m_wDevAddr , buffer[i+pos] & 0x3f , soeflag ,
		// ptmData.Year,ptmData.Month,ptmData.Day,ptmData.Hour,ptmData.Minute,ptmData.Second,ptmData.MiSec);
		// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}
} /*}}}*/

void CModBusRTU::ModBusYZ202SoeDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE pos = modbusconf.skew_byte;
	// WORD start_num = modbusconf.start_num;
	// int i = 0;
	WORD wPnt = 0;
	BYTE soeflag = 0;
	TIMEDATA ptmData;

	if ((buffer[pos + 1] & 0x02) == 0x02)
		soeflag = 1;
	else if ((buffer[pos + 1] & 0x02) == 0x00)
		soeflag = 0;

	switch (buffer[pos])
	{
	case 4:
		wPnt = 0;
		break;
	case 8:
		wPnt = 1;
		break;
	case 9:
		wPnt = 2;
		break;
	case 16:
		soeflag = 1;
		switch (buffer[pos + 10])
		{
		case 0x00:
			wPnt = 3;
			break;
		case 0x01:
			wPnt = 4;
			break;
		case 0x02:
			wPnt = 5;
			break;
		case 0x03:
			wPnt = 6;
			break;
		case 0x04:
			switch (buffer[pos + 11])
			{
			case 0x33:
				wPnt = 7;
				soeflag = 0;
				break;
			case 0x55:
				wPnt = 7;
				break;
			default:
				MsgErrorFlag = MSGERROR;
				return;
			}
			break;
		case 0x05:
			switch (buffer[pos + 11])
			{
			case 0x33:
				wPnt = 8;
				soeflag = 0;
				break;
			case 0x55:
				wPnt = 8;
				break;
			default:
				MsgErrorFlag = MSGERROR;
				return;
			}
			break;
		default:
			MsgErrorFlag = MSGERROR;
			return;
		}
		break;
	case 17:
		wPnt = 9;
		break;
	case 18:
		wPnt = 10;
		break;
	case 19:
		wPnt = 11;
		break;
	case 21:
		wPnt = 12;
		break;
	case 22:
		wPnt = 13;
		break;
	case 23:
		wPnt = 14;
		break;
	case 24:
		wPnt = 15;
		break;
	case 25:
		wPnt = 16;
		break;
	case 26:
		wPnt = 17;
		break;
	case 27:
		wPnt = 18;
		break;
	case 28:
		wPnt = 19;
		break;
	case 31:
		wPnt = 20;
		break;
	case 35:
		wPnt = 21;
		break;
	case 37:
		wPnt = 22;
		break;
	case 38:
		wPnt = 23;
		break;
	case 39:
		wPnt = 24;
		break;
	case 42:
		wPnt = 25;
		break;
	default:
		MsgErrorFlag = MSGERROR;
		return;
	}

	ptmData.MiSec = buffer[pos + 8] | (buffer[pos + 9] << 8);
	ptmData.Second = buffer[pos + 7];
	ptmData.Minute = buffer[pos + 6];
	ptmData.Hour = buffer[pos + 5];
	ptmData.Day = buffer[pos + 4];
	ptmData.Month = buffer[pos + 3];
	ptmData.Year = buffer[pos + 2];

	m_pMethod->SetYxDataWithTime(m_SerialNo, wPnt, soeflag, &ptmData);
	// char buf[100] = "";
	// sprintf(buf,"SOE m_byLineNo:%d m_wDevAddr%d num:%d val:%d time:%d.%d.%d-%d:%d:%d.%d\n",
	// m_byLineNo, m_wDevAddr , wPnt , soeflag ,
	// ptmData.Year,ptmData.Month,ptmData.Day,ptmData.Hour,ptmData.Minute,ptmData.Second,ptmData.MiSec);
	// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);

} /*}}}*/

void CModBusRTU::ModBusEsl_411SoeDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	BYTE pos = modbusconf.skew_byte;
	WORD wPnt = 0;
	BYTE soeflag = 0;
	BYTE SoeAttribute = 0;
	BYTE index = 0;
	TIMEDATA ptmData;

	if (buffer[pos - 1] == 0)
	{
		ESL411SOEFlag = 0;
		return;
	}
	else if (buffer[pos - 1] == 0x0f)
	{
		ESL411SOEFlag = 1;
	}

	ptmData.MiSec = buffer[pos + 1] | (buffer[pos] << 8);
	ptmData.Second = buffer[pos + 2];
	ptmData.Minute = buffer[pos + 3];
	ptmData.Hour = buffer[pos + 4];
	ptmData.Day = buffer[pos + 5];
	ptmData.Month = buffer[pos + 6];
	ptmData.Year = buffer[pos + 7];

	SoeAttribute = buffer[pos + 8];
	index = buffer[pos + 9];
	if (index > 31)
		return;

	switch (SoeAttribute)
	{
	case 1:
		wPnt = index;
		break;
	case 2:
		wPnt = index + 32;
		break;
	case 3:
		wPnt = index + 64;
		break;
	default:
		return;
	}

	switch (buffer[pos + 10])
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

	m_pMethod->SetYxDataWithTime(m_SerialNo, wPnt, soeflag, &ptmData);
	char buf[100] = "";
	sprintf(buf, "SOE m_byLineNo:%d m_wDevAddr%d num:%d val:%d time:%d.%d.%d-%d:%d:%d.%d\n",
			m_byLineNo, m_wDevAddr, wPnt, soeflag,
			ptmData.Year, ptmData.Month, ptmData.Day, ptmData.Hour, ptmData.Minute, ptmData.Second, ptmData.MiSec);
	OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf), 3);

} /*}}}*/

// SOE����
void CModBusRTU::ModBusSoeDeal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	switch (modbusconf.SoeFlag)
	{
	case 0:
		ModBusRtuSoeDeal(buffer, modbusconf);
		break;
	case 1:
		ModBusYZ202SoeDeal(buffer, modbusconf);
		break;
	default:
		MsgErrorFlag = MSGERROR;
		return;
	}
} /*}}}*/

void CModBusRTU::ModBusReadVal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	int pos = modbusconf.skew_byte;
	int get_num = modbusconf.get_num;
	int i = 0;
	DZ_DATA data[100];
	if (get_num > 100)
		get_num = 100;

	for (i = 0; i < get_num; i++)
	{
		float wVal = 0;
		wVal = ModBusValue(buffer, pos, modbusconf);
		pos += modbusconf.data_len;

		data[i].wPnt = modbusconf.start_num + i;
		memcpy(data[i].byVal, &wVal, sizeof(float));
		const BYTE FLOAT_TYPE = 3;
		data[i].byType = FLOAT_TYPE;
	}

	m_pMethod->SetDzCallRtn(this, bySrcBusNo, wSrcDevAddr, 0, data, get_num);
}

void CModBusRTU::ModBusWriteVal(unsigned char *buffer, MODBUSCONF modbusconf)
{ /*{{{*/
	int pos = modbusconf.skew_byte;
	int get_num = modbusconf.get_num;

	printf("write Dz %d success!!\n", modbusconf.start_num);
	m_pMethod->SetDzWriteExctRtn(this, bySrcBusNo, wSrcDevAddr, 0, 0, 0);

} /*}}}*/

BOOL CModBusRTU::GetDevCommState()
{ /*{{{*/
	if (m_byPortStatus == COMSTATUS_ONLINE)
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
} /*}}}*/
