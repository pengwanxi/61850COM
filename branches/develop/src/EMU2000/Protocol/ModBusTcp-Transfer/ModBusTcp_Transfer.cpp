#include "ModBusTcp_Transfer.h"
#define CONFIGPATH "/mynand/config/KunMing/template/"

ModBusTcp_KunMing::ModBusTcp_KunMing():ERROR_CONST(5),COMSTATUS_ONLINE(1),COMSTATUS_FAULT(0)
{/*{{{*/
	affair_type = 0;
	line = 0;
	linesum = 0;
	vec_yx.clear();
	vec_yc_ym.clear();
	vec_conf.clear();
	//	m_wErrorTimer = 0;
	m_byPortStatus = 0;
}/*}}}*/

ModBusTcp_KunMing::~ModBusTcp_KunMing()
{/*{{{*/
	vec_conf.clear();
	vec_yc_ym.clear();
	vec_yx.clear();
}/*}}}*/

BOOL ModBusTcp_KunMing::GetProtocolBuf(BYTE *buf, int &len, PBUSMSG pBusMsg)
{/*{{{*/
	buf[len++] = affair_type >> 8;
	buf[len++] = affair_type;
	buf[len++] = 0x00;			//协议类别!
	buf[len++] = 0x00;
	len += 2;
	buf[len++] = m_wDevAddr;
	buf[len++] = 0x10;			/*	写多个寄存器!	*/

	if((vec_conf[line].type == 0) && !FetchYcYmData())
		return FALSE;
	if((vec_conf[line].type == 1) && !FetchYxData())
		return FALSE;

	if(vec_conf[line].type == 0)
		GetYcYmFrame(buf, len);
	else
		GetYxFrame(buf, len);		/*	一次只能写一个顺序号!亦即一个设备至少一行配置!	*/

	buf[4] = (len - 6) >> 8;
	buf[5] = len - 6;
	affair_type++;
	line = ++line % linesum;
	return TRUE;
}/*}}}*/

void ModBusTcp_KunMing::ReOrder(vector<DWORD> &vec_temp)
{/*{{{*/
	vec_yc_ym.push_back(vec_temp[7]);
	vec_yc_ym.push_back(vec_temp[8]);
	vec_yc_ym.push_back(vec_temp[9]);
	vec_yc_ym.push_back(vec_temp[0]);
	vec_yc_ym.push_back(vec_temp[1]);
	vec_yc_ym.push_back(vec_temp[2]);
	vec_yc_ym.push_back(vec_temp[14]);
	vec_yc_ym.push_back(vec_temp[6]);
	vec_yc_ym.push_back(vec_temp[11]);
	vec_yc_ym.push_back(vec_temp[12]);
	vec_yc_ym.push_back(vec_temp[15]);
	vec_yc_ym.push_back(vec_temp[16]);
}/*}}}*/

BOOL ModBusTcp_KunMing::FetchYcYmData()
{/*{{{*/
	int m_SerialNo = 0;
	vector<DWORD> vec_temp;
	QWORD ym_temp = 0;
	vec_yc_ym.clear();
	for(int m = 0; m < vec_conf[line].bus_count; m++){
		for(int n = 0; n < vec_conf[line].dev_count; n++){
			if((m_SerialNo = m_pMethod->GetSerialNo(vec_conf[line].busno + m, vec_conf[line].devno + n)) == -1)
				return FALSE;
			for(int i = 1; i <= 15; i++)
				vec_temp.push_back(m_pMethod->ReadYcData(m_SerialNo, i));
//			m_pMethod->ReadYmData(m_SerialNo, 1, &ym_temp);
			m_pMethod->ReadYmData(m_SerialNo, 0, &ym_temp);			//最新的release版组件点号和点表中的对应, 而老版本组件则是加1
			vec_temp.push_back((DWORD)ym_temp);
//			m_pMethod->ReadYmData(m_SerialNo, 2, &ym_temp);
			m_pMethod->ReadYmData(m_SerialNo, 1, &ym_temp);
			vec_temp.push_back((DWORD)ym_temp);
			printf("********	line:%d	vec_temp:%d	********\n", __LINE__, vec_temp.size());
		}
	}
	ReOrder(vec_temp);
	return TRUE;
}/*}}}*/

void ModBusTcp_KunMing::GetYcYmFrame(BYTE *buf, int &len)
{/*{{{*/
	buf[len++] = vec_conf[line].register_addr >> 8;
	buf[len++] = vec_conf[line].register_addr;
	buf[len++] = vec_yc_ym.size() * 2 >> 8;
	buf[len++] = vec_yc_ym.size() * 2;
	buf[len++] = vec_yc_ym.size() * 4;

	for(vector<DWORD>::iterator beg=vec_yc_ym.begin(), en=vec_yc_ym.end(); beg != en; beg++){											//yc,ym占用两个寄存器!
		buf[len++] = *beg >> 8;
		buf[len++] = *beg;
		buf[len++] = *beg >> 24;
		buf[len++] = *beg >> 16;
	}
}/*}}}*/

BOOL ModBusTcp_KunMing::FetchYxData()																//一个设备至少拥有一条配置!
{/*{{{*/
	int m_SerialNo = 0;
	WORD yx_temp = 0;
	vector<WORD> vec_temp;
	vec_yx.clear();
	for(int j = 0; j < vec_conf[line].bus_count; j++){
		for(int i = 0; i < vec_conf[line].dev_count; i++){
			if((m_SerialNo = m_pMethod->GetSerialNo(vec_conf[line].busno + j, vec_conf[line].devno + i)) == -1)//顺序号确定了设备而不是点号!		//error take place here!
				return FALSE;
			m_pMethod->ReadYxData(m_SerialNo, 1, &yx_temp);											//某个设备第0点数据
			vec_yx.push_back(yx_temp ? 1 : 0);
			m_pMethod->ReadYxData(m_SerialNo, 2, &yx_temp);											//某个设备第1点数据
			vec_yx.push_back(yx_temp ? 1 : 0);
			vec_yx.push_back(m_pMethod->GetDevCommState(m_SerialNo) ? 0 : 1);						//这个获取状态的方式似乎不靠谱!
		}
	}
	return TRUE;
}/*}}}*/

void ModBusTcp_KunMing::GetYxFrame(BYTE *buf, int &len)
{/*{{{*/
	buf[len++] = vec_conf[line].register_addr >> 8;
	buf[len++] = vec_conf[line].register_addr;
	buf[len++] = vec_yx.size() >> 8;
	buf[len++] = vec_yx.size();
	buf[len++] = vec_yx.size() * 2;
	//	for(BYTE i = 0; i < vec_yx.size(); i++){													//一个遥信也占用一个寄存器
	//		buf[len++] = vec_yx[i] >> 8;
	//		buf[len++] = vec_yx[i];
	//	}
	for(vector<WORD>::iterator beg = vec_yx.begin(), en = vec_yx.end(); beg != en; beg++){
		buf[len++] = *beg >> 8;
		buf[len++] = *beg;
	}
}/*}}}*/

BOOL ModBusTcp_KunMing::ProcessProtocolBuf(BYTE *buf, int len)
{/*{{{*/
	return TRUE;


/*
	if((buf[0] != affair_type >> 8) || (buf[1] != affair_type & 0xFF))
		return FALSE;
	if((buf[2] != 0) || (buf[3] != 0))
		return FALSE;
	if((buf[6] != m_wDevAddr))
		return FALSE;
	if((buf[7] != 0x10))
		return FALSE;
	if((buf[8] != vec_yc_ym.size() * 4 >> 8) || (buf[8] != vec_yx.size() * 2 >> 8))
		return FALSE;
	if((buf[9] != (vec_yc_ym.size() * 4) & 0xFF) || (buf[9] != (vec_yx.size() * 2) & 0xFF))
		return FALSE;
	if((buf[10] != vec_conf[line].register_addr >> 8) || (buf[11] != (vec_conf[line].register_addr) & 0xFF))
		return FALSE;
	//	m_wErrorTimer = 0;
	//	affair_type++;							//事务类型到底怎样协定!
	//	line = ++line % linesum;
	return TRUE;
*/
}/*}}}*/

BYTE ModBusTcp_KunMing::ReadConf(BYTE *filename)
{/*{{{*/
	FILE *hFile;
	char szText[256] = {0};
	char *temp;
	UINT segvalue = 0;
	BYTE i = 0;
	BYTE conflag = 0;
	ConfigItem conf_temp;
	if((hFile = fopen((const char *)filename, "r")) == NULL)
		return 0;
	while( fgets(szText, sizeof(szText), hFile) != NULL )			//将配置文件中每一行存到一个类对象中，再将这些类对象放入容器中。善哉善哉!
	{
		rtrim(szText);				//去除字符串右侧的多余字符而不是左侧，让人费解!
		if( szText[0]=='#' || szText[0]==';' )
			continue;
		i = 0;
		conflag = 0;
		memset(&conf_temp,0,sizeof(conf_temp));

		temp = strtok(szText,",");
		if(temp == NULL)
			continue;
		if((atoi(temp) == 0) || (atoi(temp) == 1))
			conf_temp.type = atoi(temp);
		else{
			conflag = 1;
			DefaultConfig(&conf_temp);
		}
		while((temp = strtok(NULL,","))){/*{{{*/
			segvalue = (ULONG)strtoll(temp, NULL, 16);
			switch(++i){
			case 1:
				if((segvalue > 0) && (segvalue) <= 0xFF)
					conf_temp.busno = segvalue - 1;
				else
					conflag = 1;
				break;
			case 2:
				if((segvalue > 0) && (segvalue) <= 0xFF)
					conf_temp.devno = segvalue;
				else
					conflag = 1;
				break;
			case 3:
				if((segvalue > 0) && (segvalue) <= 0xFF)
					conf_temp.dev_count = segvalue;
				else
					conflag = 1;
				break;
			case 4:								//就预留在这!
				if((segvalue > 0) && (segvalue) <= 0xFF)
					conf_temp.bus_count = segvalue;
				else
					conflag = 1;
				break;
			case 5:
				if(((UINT)strtoll(temp, NULL, 16)) <= 0xFFFFFFFF)		//这个判断没有用!
					conf_temp.mask = segvalue;
				else
					conflag = 1;
				break;
			case 6:
				if(segvalue <= 0xFFFF)
					conf_temp.register_addr = segvalue;
				else
					conflag = 1;
				break;
			}
		}/*}}}*/
		if( conflag == 1 )
		{
			printf("%s num is %d %d\n\n\n",filename,linesum+1,i+1);
			DefaultConfig(&conf_temp);
			vec_conf.push_back(conf_temp);
			linesum = 1;
			return 1;
		}
		vec_conf.push_back(conf_temp);
		linesum++;
	}

	fclose(hFile);
	return linesum;
}/*}}}*/

void ModBusTcp_KunMing::DefaultConfig(ConfigItem *conf_temp)
{/*{{{*/
	conf_temp->type = 0x00;
	conf_temp->busno = 0x04;
	conf_temp->devno = 0x06;
	conf_temp->dev_count = 0x01;
	conf_temp->bus_count = 0x01;
	conf_temp->mask = 0xFFFFFFFF;
	conf_temp->register_addr = 0x7D0;
}/*}}}*/

BOOL ModBusTcp_KunMing::Init(BYTE byLineNo)
{/*{{{*/
	BYTE szFileName[128] = {0};
	sprintf((char *)szFileName,"%s%s", CONFIGPATH, "kunming_config.txt");
	ConfigItem conf_temp;
	linesum = ReadConf(szFileName);

	if(linesum == 0){
		DefaultConfig(&conf_temp);
		vec_conf.push_back(conf_temp);
		linesum = 1;
	}		/*	能够正常读取配置!	*/
	for(vector<ConfigItem>::iterator beg = vec_conf.begin(), en = vec_conf.end(); beg!=en; beg++)
		printf("\n%2X,%2X,%2X,%2X,%2X,%8X,%4X\n", (*beg).type, (*beg).busno, (*beg).devno, (*beg).bus_count, (*beg).dev_count, (*beg).mask, (*beg).register_addr);
	printf("=========================	linesum:%d	========================\n", linesum);
	return TRUE;
}/*}}}*/

void ModBusTcp_KunMing::TimerProc()
{/*{{{*/
	//	m_byPortStatus = (m_wErrorTimer > ERROR_CONST) ? COMSTATUS_FAULT : COMSTATUS_ONLINE;
}/*}}}*/

BOOL ModBusTcp_KunMing::GetDevCommState()
{/*{{{*/
//	return FALSE;
	return FALSE;
//	return (m_byPortStatus == COMSTATUS_ONLINE) ? COM_DEV_NORMAL : COM_DEV_ABNORMAL;
}/*}}}*/
