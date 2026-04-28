// ModBusRTU.cpp: implementation of the CModBusQuickly class.
//
//////////////////////////////////////////////////////////////////////

#include "ModBusQuickly.h"



#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <vector>
#include <math.h>


#define MODBUSRTUPREFIXFILENAME	"/mynand/config/ModBus/template/"  /* modbusrtu 前缀路径 */
//#define MODBUSDEBUG 1
// #ifdef MODBUSDEBUG
// #endif
#define TIME				300
#define ERROR_CONST			5
#define COMSTATUS_ONLINE	1
#define COMSTATUS_FAULT		0

#define MASK				0xffffffff



extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
extern "C" void  OutMessageText(char *szSrc, unsigned char *pData, int nLen);
#include <iostream>
using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CModBusQuickly::CModBusQuickly()
{/*{{{*/
	pos_flag = -1;
	pos = 0;
	settime_pos = 0;
	yk_pos_num = 0;
	yk_flag = 0;
	readval_flag = -1;
	writeval_flag = -1;
	last_settime = 0;
	timeflag = 0;
	YkNo = 0;
	MsgErrorFlag = 0;
	ESL411SOEFlag = 0;
	memset(store_buf,0,sizeof(store_buf));
	//m_hSem.Create( 20150101 );
	m_wErrorTimer = ERROR_CONST+1;
	m_byPortStatus = COMSTATUS_FAULT;
	DevCirFlag = FALSE;
	modbus_conf.reserve(100);

	//start
	memset(&yc_conf, 0, sizeof(MODBUSCONF));
	memset(&ym_conf, 0, sizeof(MODBUSCONF));
	memset(&yx_conf, 0, sizeof(MODBUSCONF));
	//end
}/*}}}*/

CModBusQuickly::~CModBusQuickly()
{/*{{{*/
	printf( "Delete CModBusQuickly bus = %d , Addr = %d \n" , m_byLineNo , m_wDevAddr );
	yk_info.clear();
	//readval_info.clear();
	//writeval_info.clear();
	modbus_conf.clear() ;
	//m_hSem.Remove();
}/*}}}*/

BOOL CModBusQuickly::ProcessProtocolBuf( BYTE * buf , int len )
{/*{{{*/
	// printf( "CModBusQuickly::ProcessProtocolBuf \n" );

	BYTE temp_flag = 0;
	BYTE CRC_flag = 0;
	WORD wCRC = 0 ;

	//printf("\n----------------------------------------\n");
	//for(int i = 0; i < len; i++)
		//printf("%4X", buf[i]);
	//printf("\n----------------------------------------\n");
	/*判断接受的报文是否正确*/
	while( len > 4 )
	{
		if( buf[0] == m_wDevAddr )
		{
			if( buf[1] == modbus_conf[pos_flag].func )
			{
				if( ( buf[1] > 0 ) && ( buf[1] < 5 ) || buf[1] == 0x55  )
				{
					if( ( buf[2] + 5 ) <= len )
					{
						/*if( ( buf[1] == 3 || buf[1] == 4 ) && buf[2] == modbus_conf[pos_flag].registe_num*2 )
						  {
						  CRC_flag = 1;
						  }
						  else if( buf[1] == 2 )
						  {
						  if( ( modbus_conf[pos_flag].registe_num%8 != 0) && ( buf[2] == 1 + modbus_conf[pos_flag].registe_num/8 ) )
						  {
						  CRC_flag = 1;
						  }
						  else if( ( modbus_conf[pos_flag].registe_num%8 == 0) && ( buf[2] == modbus_conf[pos_flag].registe_num/8 ) )
						  {
						  CRC_flag = 1;
						  }
						  }
						  else if( buf[1] == 0x55 )
						  {
						  CRC_flag = 1;
						  }*/

						if( modbus_conf[pos_flag].type == 1 )
						{
							if( ( modbus_conf[pos_flag].registe_num%8 != 0) && ( buf[2] == 1 + modbus_conf[pos_flag].registe_num/8 ) )	//何为其然也!
							{
								CRC_flag = 1;
							}
							else if( ( modbus_conf[pos_flag].registe_num%8 == 0) && ( buf[2] == modbus_conf[pos_flag].registe_num/8 ) )	//何为其然也!
							{
								CRC_flag = 1;
							}
							else if ( buf[2] == modbus_conf[pos_flag].registe_num*2 )
							//else if ( buf[2] == modbus_conf[pos_flag].registe_num * 2 + modbus_conf[pos_flag].registenumsec * 2 + modbus_conf[pos_flag].registenumthi * 2 )
							{
								CRC_flag = 1;
							}
						}
						else if(modbus_conf[pos_flag].type == 2 || modbus_conf[pos_flag].type == 4){
							if(buf[2] == modbus_conf[pos_flag].registe_num * 2)
								CRC_flag = 1;
						}else{
							if( buf[1] == 0x55 )
							{
								CRC_flag = 1;
							}
							if ( buf[2] == modbus_conf[pos_flag].registe_num*2 + modbus_conf[pos_flag].registenumsec * 2 + modbus_conf[pos_flag].registenumthi * 2)
							{
								CRC_flag = 1;
							}

						}

						//printf("CRC_flag:%d temp_flag:%d line:%d\n", CRC_flag, temp_flag, __LINE__);

						if( CRC_flag == 1 )
						{
							wCRC = GetCrc( buf, buf[2] + 3 );
							if( ( HIBYTE(wCRC) == buf[ buf[2] + 3 ] ) && ( LOBYTE(wCRC) == buf[ buf[2] + 4 ] ) )
							{
								temp_flag = 1;
								break;
							}
						//printf("CRC_flag:%d temp_flag:%d\n", CRC_flag, temp_flag);
						}
					}
				}
				else if ( buf[1] == 0x5 || buf[1] == 0x6 || buf[1] == 0xf || buf[1] == 0x10 )
				{
					if( MsgRegisteAndData[0] == buf[2] && MsgRegisteAndData[1] == buf[3] &&
							MsgRegisteAndData[2] == buf[4] && MsgRegisteAndData[3] == buf[5] )
					{
						wCRC = GetCrc( buf, 6 );
						if( ( HIBYTE(wCRC) == buf[ 6 ] ) && ( LOBYTE(wCRC) == buf[ 7 ] ) )
						{
							temp_flag = 1;
							break;
						}
					}
				}
			}
			else if( buf[1] == ( modbus_conf[pos_flag].func | 0x80 ) )
			{
				wCRC = GetCrc( buf, 3 );
				if( ( HIBYTE(wCRC) == buf[ 3 ] ) && ( LOBYTE(wCRC) == buf[ 4 ] ) )
				{
					switch(modbus_conf[pos_flag].type)
					{
					case 3:
						yk_flag = 0;
						break;
					}
					char buffer[100] = "";
					sprintf(buffer,"func %x errno num:%d\n",buf[1],buf[2]);
					//OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer),3);
					break;
				}
			}
			else if( modbus_conf[pos_flag].YkSelFlag == 1 && buf[1] == 0x10 && modbus_conf[pos_flag].func == 0x05 )
			{
				if( buf[2] == 0x40 && buf[3] == 0x59 && buf[4] == 0x00 && buf[5] == 0x03 )
				{
					wCRC = GetCrc( buf, 6 );
					if( ( HIBYTE(wCRC) == buf[ 6 ] ) && ( LOBYTE(wCRC) == buf[ 7 ] ) )
					{
						m_pMethod->SetYkSelRtn ( this , bySrcBusNo, wSrcDevAddr , YkNo , YkVal );
						return TRUE ;
					}
				}
			}
			else if( modbus_conf[pos_flag].SoeFlag == 2 && ( buf[1] == 0x02 || buf[1] == 0x03 || buf[1] == 0x0c) )
			{
				wCRC = GetCrc( buf, buf[2] + 3 );
				if( ( HIBYTE(wCRC) == buf[ buf[2] + 3 ] ) && ( LOBYTE(wCRC) == buf[ buf[2] + 4 ] ) )
				{
					temp_flag = 1;
					break;
				}
			}
		}

		buf = buf + 1 ;
		char buffer[100] = "the message changed!";
		//OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer),3);
		len--;
	}
	//printf("temp_flag:%d type:%d\n", temp_flag, modbus_conf[pos_flag].type);
	if( temp_flag == 1 )
	{
		switch(modbus_conf[pos_flag].type)
		{
		case 1:
			ModBusYxDeal(buf,modbus_conf[pos_flag]);
			break;
		case 2:
			ModBusYcDeal(buf,modbus_conf[pos_flag]);
			break;
		case 3:
			ModBusYkDeal(buf,modbus_conf[pos_flag] );
			break;
		case 4:
			ModBusYmDeal(buf,modbus_conf[pos_flag]);
			break;
		case 5:
			ModBusReadVal(buf,modbus_conf[pos_flag]);
			break;
		case 6:
			ModBusWriteVal(buf,modbus_conf[pos_flag]);
			break;
		case 8:
			ModBusSetTime(buf,modbus_conf[pos_flag]);
			break;
		case 9:
			ModBusSoeDeal(buf,modbus_conf[pos_flag]);
			break;
		default:return FALSE;
		}
	}
	else if( temp_flag == 0 )
	{
		char buffer[100] = "the message is wrong!";
		//OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer),3);
		return FALSE;
	}
	if( MsgErrorFlag == MSGERROR )
	{
		MsgErrorFlag = MSGTRUE;
		char buffer[100] = "the config is wrong!";
		//OutBusDebug(m_byLineNo, (BYTE *)buffer, strlen(buffer),3);
		return FALSE; 
	}
	m_wErrorTimer = 0;								//接收到消息置0
	return TRUE ;
}/*}}}*/

//当配置文件某行出现问题时，想该行换成默认配置，该默认配置，可发非正常报文
void CModBusQuickly::DefaultValConfig(MODBUSCONF * mc)
{/*{{{*/
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
	mc->cir_flag = 0;
	mc->typesec = 0;
	mc->registenumsec = 0;
	mc->getnumsec = 0;
	mc->startpossec = 0;
	mc->typethi = 0;
	mc->registenumthi = 0;
	mc->getnumthi = 0;
	mc->startposthi = 0;
}/*}}}*/

//读取配置文件
int CModBusQuickly::ReadConf(char *filename)
{/*{{{*/
	FILE *hFile;
	char szText[160];
	char *temp;
	int num = 0;
	BYTE i = 0;
	BYTE conflag = 0;
	MODBUSCONF mc;
	INFO yk;

	//m_hSem.semTake();

	hFile = fopen(filename, "r");

	if(hFile == NULL)
	{
		//printf("FT:fopen conf error!\n");
		//m_hSem.semGive();
		return 0;
	}

	while( fgets(szText, sizeof(szText), hFile) != NULL )
	{
		rtrim(szText);
		if( szText[0]=='#' || szText[0]==';' )
			continue;
		i = 0;
		conflag = 0;
		memset(&mc,0,sizeof(mc));

		temp = strtok(szText,",");
		if(temp == NULL)
			continue;
		if( ( atoi(temp) > 0 ) && ( atoi(temp) < 10 ) )
			mc.type = atoi(temp);
		else
		{
			conflag = 1;
			printf("config error line:%d\n", __LINE__);
			DefaultValConfig(&mc);
		}
		while( ( temp = strtok(NULL,",") ) )
		{
			switch(++i)
			{
			case 1:
				if( ( ( (UINT)strtoll(temp, NULL, 16) > 0 ) && ( (UINT)strtoll(temp, NULL, 16) <= 7 ) ) || ( (UINT)strtoll(temp, NULL, 16) == 0x0f )
						|| ( (UINT)strtoll(temp, NULL, 16) == 0x10 ) || ( (UINT)strtoll(temp, NULL, 16) == 0x55 ) || ( (UINT)strtoll(temp, NULL, 16) == 0x0c ) )
					mc.func = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 2:
				if( ( (UINT)strtoll(temp, NULL, 16) >= 0 ) && ( (UINT)strtoll(temp, NULL, 16) <= 0xffff ) )
					mc.registe = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 3:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 0xffff ) )
					mc.registe_num = atoi(temp);
				else
					conflag = 1;
				break;
			case 4:
				//偏移字节大于零小于最大传输字节数 260 - 2
				if(  atoi(temp) >= 0 && atoi( temp ) <= 258 )
					mc.skew_byte = atoi(temp);
				else
					conflag = 1;
				break;
			case 5:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 0xffff ) )
					mc.get_num = atoi(temp);
				else
					conflag = 1;
				break;
			case 6:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 0xffff ) )
					mc.start_num = atoi(temp);
				else
					conflag = 1;
				break;
			case 7:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 4 ) )
					mc.data_len = atoi(temp);
				else
					conflag = 1;
				break;
			case 8:
				if( ( (UINT)strtoll(temp, NULL, 16) >= 0 ) && ( (UINT)strtoll(temp, NULL, 16) <= 0xffffffff ) )
					mc.mask_code = (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 9:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 9 ) )
					mc.data_form = atoi(temp);
				else
					conflag = 1;
				break;
			case 10:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 4 ) )
					mc.sign = atoi(temp);
				else
					conflag = 1;
				break;
			case 11:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 2 ) )
					mc.yk_form = atoi(temp);
				else
					conflag = 1;
				break;
			case 12:
				if( ( atoi(temp) == 0 ) || ( atoi(temp) == 1 ) )
				{
					mc.cir_flag = atoi(temp);
					if( mc.cir_flag == 1 )
						DevCirFlag = TRUE;
				}
				else
					conflag = 1;
				break;

				//start
			case 13:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFF))
					mc.typesec = atoi(temp);
				else
					conflag = 1;
				break;
			case 14:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFF))
					mc.registenumsec = atoi(temp);
				else
					conflag = 1;
				break;
			case 15:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFFFF))
					mc.getnumsec = atoi(temp);
				else
					conflag = 1;
				break;
			case 16:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFFFF))
					mc.startpossec = atoi(temp);
				else
					conflag = 1;
				break;
			case 17:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFF))
					mc.typethi = atoi(temp);
				else
					conflag = 1;
				break;
			case 18:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFF))
					mc.registenumthi = atoi(temp);
				else
					conflag = 1;
				break;
			case 19:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFFFF))
					mc.getnumthi = atoi(temp);
				else
					conflag = 1;
				break;
			case 20:
				if((atoi(temp) >= 0) && (atoi(temp) <= 0xFFFF))
					mc.startposthi = atoi(temp);
				else
					conflag = 1;
				break;
				//end

			case 21:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 0xffff ) )
					mc.YkClose= (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 22:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 0xffff ) )
					mc.YkOpen= (UINT)strtoll(temp, NULL, 16);
				else
					conflag = 1;
				break;
			case 23:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 2 ) )
					mc.SetTimeFlag = atoi(temp);
				else 
					conflag = 1;
				break;
			case 24:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 2 ) )
					mc.SoeFlag = atoi(temp);
				else
					conflag = 1;
				break;
			case 25:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 2 ) )
					mc.YkSelFlag = atoi(temp);
				else
					conflag = 1;
				break;
			case 26:
				if( ( atoi(temp) >= 0 ) && ( atoi(temp) <= 2 ) )
					mc.YkExctFlag = atoi(temp);
				else
					conflag = 1;
				break;
			default:
				conflag = 1;
				printf("\n\n\n%d > 18\n\n\n",i+1);
				break;
			}
			if( conflag == 1 )
			{
				printf("ModBus config file error:%d\n", i);
				//continue;
				break;
			}

		}
		if( conflag == 1 )
		{
			printf("%s num is %d %d\n\n\n",filename,num+1,i+1);
			DefaultValConfig(&mc);
		}
		else if( i < 26)
		{
			switch( i + 1 )
			{
			//case 13:mc.typesec = 0;			//配置软件中预置
			//case 14:mc.registenumsec = 0;
			//case 15:mc.getnumsec = 0;
			//case 16:mc.startpossec = 0;
			//case 17:mc.typethi = 0;
			//case 18:mc.registenumthi = 0;
			//case 19:mc.getnumthi = 0;
			//case 20:mc.startposthi = 0;
			case 21:mc.YkClose= 0xff00;
			case 22:mc.YkOpen= 0x0000;
			case 23:mc.SetTimeFlag = 0;
			case 24:mc.SoeFlag = 0;
			case 25:mc.YkSelFlag = 0;
			case 26:mc.YkExctFlag = 0;break;
			}
		}
		/*   printf("%d %d %d %d %d %d %d %d %02x %d %d %d %d %d %d %d \n",mc.type,mc.func,
			 mc.registe,mc.registe_num,mc.skew_byte,mc.get_num,mc.start_num,
			 mc.data_len,mc.mask_code,mc.data_form,mc.sign,mc.yk_form,mc.cir_flag,mc.SetTimeFlag ,
			 mc.SoeFlag,mc.YkSetFlag); */
		if((yx_conf.type != 1) && (mc.type == 1))
			yx_conf = mc;
		if((yc_conf.type != 2) && (mc.type == 2))
			yc_conf = mc;
		if((ym_conf.type != 4) && (mc.type == 4))
			ym_conf = mc;
		modbus_conf.push_back( mc );
		num++;
	}
	int freturn = fclose(hFile);//perror("fclose");
	if( freturn )
		perror("fclose");

	//m_hSem.semGive();

	for(i=0;i<num;i++)
	{
		if( modbus_conf[i].type == 3 )
		{
			yk.pos =  i;
			yk.start_num =  modbus_conf[i].start_num;
			yk.get_num =  modbus_conf[i].get_num;

			//	printf("yk_pos_num===%d %d %d %d\n",yk_pos_num,yk.pos,yk.start_num,yk.get_num);
			yk_info.push_back( yk );
			yk_pos_num++;
		}
		if( modbus_conf[i].type == 5 )
		{
			//readval.pos =  i;
			//readval.start_num =  modbus_conf[i].start_num;
			//readval.get_num =  modbus_conf[i].get_num;

			// printf("readval_pos_num===%d %d %d %d\n",readval_pos_num,readval.pos,readval.start_num,readval.get_num);
			//	readval_info.push_back( readval );
			readval_pos_num++;
		}
		if( modbus_conf[i].type == 6 )
		{
			//writeval.pos =  i;
			//writeval.start_num =  modbus_conf[i].start_num;
			//writeval.get_num =  modbus_conf[i].get_num;

			// printf("writeval_pos_num===%d %d %d %d\n",writeval_pos_num,writeval.pos,writeval.start_num,writeval.get_num);
			//	writeval_info.push_back( writeval );
			writeval_pos_num++;
		}
		else if( modbus_conf[i].type == 8 )
		{
			settime_pos = i;
		}
	}
	return num;
}/*}}}*/

BOOL CModBusQuickly::Init( BYTE byLineNo )
{/*{{{*/
	//这里初始化模板数据
	char szFileName[256] = "";
	sprintf( szFileName, "%s%s", MODBUSRTUPREFIXFILENAME, m_sTemplatePath);
	line = ReadConf(szFileName);				//读取配置文件

	if( line <= 0 )
		//line = read_conf("/mynand/config/ModBus/template/default.ini");			//读取默认文件
	{
		printf("don't found %s\n\n\n\n",szFileName);
		MODBUSCONF mc;
		DefaultValConfig(&mc);
		modbus_conf.push_back( mc );
		line = 1;
	}
	return TRUE ;
}/*}}}*/

//遥信，遥测，对时等发送报文
void CModBusQuickly::SendBuf( MODBUSCONF modbusconf, BYTE * buf ,int *len )
{ /*{{{*/
	BYTE index = 0;

	buf[ index++ ] = m_wDevAddr;
	buf[ index++ ] = modbusconf.func;
	if( modbusconf.type == 9 && modbusconf.SoeFlag == 0 )
	{
		;
	}
	else
	{
		buf[ index++ ] = modbusconf.registe >> 8;
		buf[ index++ ] = modbusconf.registe ;
		buf[ index++ ] = (modbusconf.registe_num >> 8) + (modbusconf.registenumsec >> 8) + (modbusconf.registenumthi >> 8) + (modbusconf.registe_num + modbusconf.registenumsec + modbusconf.registenumthi) / 256;
		buf[ index++ ] = modbusconf.registe_num + modbusconf.registenumsec + modbusconf.registenumthi;

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
	if( modbusconf.type == 8 )
	{
		index = SysLocalTime(modbusconf,buf,index);
	}

	WORD wCRC = GetCrc( buf, index );
	buf[ index++ ] = HIBYTE(wCRC);
	buf[ index++ ] = LOBYTE(wCRC);
	*len = index;
}/*}}}*/

void CModBusQuickly::ModBusYxDeal(unsigned char *buffer,MODBUSCONF modbusconf)
{/*{{{*/
	if( modbusconf.SoeFlag == 2 )
	{
		ModBusRsl_411YxYcDeal( buffer , modbusconf );
		return ;
	}

	//switch(modbus_conf[pos_flag].func)
	{
		//case 2:
		ModBusYxBitDeal(buffer,modbusconf);
		//break;
		//case 3:
		//ModBusYxByteDeal(buffer,modbusconf);
		//break;
	}
}/*}}}*/

//遥信安位处理
void CModBusQuickly::ModBusYxBitDeal(unsigned char *buffer,MODBUSCONF modbusconf)
{/*{{{*/
	WORD i = 0;
	WORD j = 0;
	BYTE pos = 0;						//摆放顺序影响内存使用量!
	BYTE data_len = 0;
	WORD get_num = 0;
	WORD start_num = 0;
	WORD real_get_num = 0;
	UINT temp_buf = 0;
	UINT temp_mask = 0;
	UINT first_reg_num = modbusconf.registe_num - modbusconf.registenumsec - modbusconf.registenumthi;
	if(modbusconf.type == 1){
		pos = modbusconf.skew_byte;
		temp_buf = buffer[pos];
		temp_mask = modbusconf.mask_code;
		get_num = modbusconf.get_num;
		start_num = modbusconf.start_num;
		data_len = modbusconf.data_len;
		real_get_num = first_reg_num * 16;
	}else if(modbusconf.typesec == 1){
		pos = 0;
		temp_buf = buffer[pos];
		temp_mask = modbusconf.mask_code;
		get_num = modbusconf.getnumsec;
		start_num = modbusconf.startpossec;
		data_len = yx_conf.data_len;
		real_get_num = modbusconf.registenumsec * 16;
	}else if(modbusconf.typethi == 1){
		pos = 0;
		temp_buf = buffer[pos];
		temp_mask = modbusconf.mask_code;
		get_num = modbusconf.getnumthi;
		start_num = modbusconf.startposthi;
		data_len = yx_conf.data_len;
		real_get_num = modbusconf.registenumthi * 16;
	}

	WORD num = 0;
	WORD wVal = 0;
	UINT real_temp_mask =0;

	if( modbusconf.SoeFlag == 2 )
	{	
		get_num = 25;
	}
	//配置的采集数量不能大于可以解析的数量
	if( real_get_num < get_num )
	{
		get_num = real_get_num;
	}

	for(i = 0; i < (( (real_get_num-1) / (8*data_len) )+1); i++)					//i表示一个字节!
	{
		switch( data_len )
		{
		case 1:
			real_temp_mask = ModBusYXTempValue( (unsigned char*)&temp_mask , 3 , (modbusconf.type == 1) ? modbusconf : yx_conf);
			break;
		case 2:
			real_temp_mask = ModBusYXTempValue( (unsigned char*)&temp_mask , 2 , (modbusconf.type == 1) ? modbusconf : yx_conf);
			break;
		case 4:
			real_temp_mask = ModBusYXTempValue( (unsigned char*)&temp_mask , 0 , (modbusconf.type == 1) ? modbusconf : yx_conf);
			break;
		default:
			MsgErrorFlag = MSGERROR;
			return;
		}

		temp_buf = ModBusYXTempValue( buffer , pos , (modbusconf.type == 1) ? modbusconf : yx_conf);
		pos += data_len;

		for(j=0;j<data_len*8;j++)
		{
			if( num >= ((modbusconf.type == 1) ? modbusconf.get_num : ((modbusconf.typesec == 1) ? modbusconf.getnumsec : modbusconf.getnumthi)))
			{
				if(modbusconf.type == 1)
					break;
				else
					return;
			}
			if(real_temp_mask%2)//real_temp_mask |0x01				//屏蔽码处理
			{
				wVal = temp_buf%2;//wVal = temp_buf | 0x01;
				m_pMethod->SetYxData ( m_SerialNo , num+start_num , wVal );
				//char buf[100];
				//sprintf(buf,"YX1 m_byLineNo:%d m_wDevAddr%d num:%d val:%d\n", m_byLineNo, m_wDevAddr , num+start_num , wVal);
				//OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
				num++;
			}
			real_temp_mask /= 2; //real_temp_mask>>1 
			temp_buf /= 2;//temp_buf>>1 
		}
	}
	if(modbusconf.type == 1){
		if(modbusconf.typesec == 2)
			ModBusYcDeal(buffer + modbusconf.skew_byte + first_reg_num * 2, modbusconf);
		else if(modbusconf.typesec == 4)
			ModBusYmDeal(buffer + modbusconf.skew_byte + first_reg_num * 2, modbusconf);
		if(modbusconf.typethi == 2)
			ModBusYcDeal(buffer + modbusconf.skew_byte + first_reg_num * 2 + modbusconf.registenumsec * 2, modbusconf);
		else if(modbusconf.typethi == 4)
			ModBusYmDeal(buffer + modbusconf.skew_byte + first_reg_num * 2 + modbusconf.registenumsec * 2, modbusconf);
	}
}/*}}}*/

//遥测处理
void CModBusQuickly::ModBusYcDeal(unsigned char *buffer,MODBUSCONF modbusconf)
{/*{{{*/
	BYTE pos = 0;
	WORD get_num = 0;
	WORD start_num = 0;
	WORD real_get_num = 0;
	UINT first_reg_num = modbusconf.registe_num - modbusconf.registenumsec - modbusconf.registenumthi;
	if(modbusconf.type == 2){
		pos = modbusconf.skew_byte;
		get_num = modbusconf.get_num;
		start_num = modbusconf.start_num;
		real_get_num = first_reg_num * 2 / modbusconf.data_len;
	}else if(modbusconf.typesec == 2){
		pos = 0;
		get_num = modbusconf.getnumsec;
		start_num = modbusconf.startpossec;
		real_get_num = modbusconf.registenumsec * 2 / yc_conf.data_len;
	}else if(modbusconf.typethi == 2){
		pos = 0;
		get_num = modbusconf.getnumthi;
		start_num = modbusconf.startposthi;
		real_get_num = modbusconf.registenumsec * 2 / yc_conf.data_len;
	}

	WORD i = 0;
	float wVal = 0 ;
	for(i = 0; i < get_num; i++)
	{
		wVal = 0 ;
		if( i < real_get_num )
			wVal = ModBusValue(buffer,pos,(modbusconf.type == 2) ? modbusconf : yc_conf);
		if( MsgErrorFlag == MSGERROR )
		{
			return;
		}
		//pos += (modbusconf.type == 1) ? modbusconf.data_len : yc_conf.data_len;
		pos += (modbusconf.type == 2) ? modbusconf.data_len : yc_conf.data_len;
		m_pMethod->SetYcData( m_SerialNo , i+start_num , wVal );
		//char buf[100];
		//sprintf(buf,"YC m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n",m_byLineNo, m_wDevAddr , i+start_num , wVal);
		//OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}

	if(modbusconf.type == 2){
		if(modbusconf.typesec == 1)
			ModBusYxDeal(buffer + modbusconf.skew_byte + first_reg_num * 2, modbusconf);
		else if(modbusconf.typesec == 4)
			ModBusYmDeal(buffer + modbusconf.skew_byte + first_reg_num * 2, modbusconf);
		if(modbusconf.typethi == 1)
			ModBusYxDeal(buffer + modbusconf.skew_byte + first_reg_num * 2 + modbusconf.registenumsec * 2, modbusconf);
		else if(modbusconf.typethi == 4)
			ModBusYmDeal(buffer + modbusconf.skew_byte + first_reg_num * 2 + modbusconf.registenumsec * 2, modbusconf);
	}
}/*}}}*/

//遥脉处理
void CModBusQuickly::ModBusYmDeal(unsigned char *buffer,MODBUSCONF modbusconf)
{/*{{{*/
	BYTE pos = 0;
	WORD get_num = 0;
	WORD start_num = 0;
	WORD real_get_num = 0;
	UINT first_reg_num = modbusconf.registe_num - modbusconf.registenumsec - modbusconf.registenumthi;

	if(modbusconf.type == 4){
		pos = modbusconf.skew_byte;
		get_num = modbusconf.get_num;
		start_num = modbusconf.start_num;
//		real_get_num = buffer[2] / modbusconf.data_len;
		real_get_num = first_reg_num * 2 / modbusconf.data_len;
	}else if(modbusconf.typesec == 4){
		pos = 0;
		get_num = modbusconf.getnumsec;
		start_num = modbusconf.startpossec;
		real_get_num = modbusconf.registenumsec * 2 / ym_conf.data_len;
	}else if(modbusconf.typethi == 4){
		pos = 0;
		get_num = modbusconf.getnumthi;
		start_num = modbusconf.startposthi;
		real_get_num = modbusconf.registenumthi * 2 / ym_conf.data_len;
	}

	WORD i = 0;
	float wVal = 0 ;

	for(i=0;i<get_num;i++)
	{
		wVal = 0;
		if( i < real_get_num )
			wVal = ModBusValue(buffer,pos,(modbusconf.type == 4) ? modbusconf : ym_conf);
		if( MsgErrorFlag == MSGERROR )
		{
			return;
		}
		pos += (modbusconf.type == 4) ? modbusconf.data_len : ym_conf.data_len;
		m_pMethod->SetYmData ( m_SerialNo, i+start_num, wVal );
		// char buf[100];
		// sprintf(buf,"YM m_byLineNo:%d m_wDevAddr%d num:%d val:%f\n", m_byLineNo, m_wDevAddr , i+start_num , wVal);
		// OutBusDebug(m_byLineNo, (BYTE *)buf, strlen(buf),3);
	}

	if(modbusconf.type == 4){
		if(modbusconf.typesec == 1)
			ModBusYxDeal(buffer + modbusconf.skew_byte + first_reg_num * 2, modbusconf);
		else if(modbusconf.typesec == 2)
			ModBusYcDeal(buffer + modbusconf.skew_byte + first_reg_num * 2, modbusconf);
		if(modbusconf.typethi == 1)
			ModBusYxDeal(buffer + modbusconf.skew_byte + first_reg_num * 2 + modbusconf.registenumsec * 2, modbusconf);
		else if(modbusconf.typethi == 2)
			ModBusYcDeal(buffer + modbusconf.skew_byte + first_reg_num * 2 + modbusconf.registenumsec * 2, modbusconf);
	}
}/*}}}*/
