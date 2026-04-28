#include <stdio.h>
#include <math.h>
#include <vector>
#include <map>
#include <iostream>
#include "../share/rdbFun.h"
#include "../librtdb/rdbObj.h"
#include "../share/profile.h"
#include <unistd.h>
#include "../share/md5.h"
#include "../share/global.h"
/*lel*/
#include <cstdlib>
#include <stdlib.h>
/*end*/
#include <string.h>
#include <dirent.h>
#include <unistd.h>
//#include "../libdbase/dbasedatatype.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <err.h>
#include <sys/ioctl.h>

// #include <sys/sysctl.h>
#include <time.h>
#include <sys/time.h>




using namespace std;

/*lel*/
typedef struct _CurrentBus
{
	BYTE byLineNo;								//���ߺ�
	WORD wDevAddr;								//װ�õ�ַ
	BYTE byLineNum;								//���߸���
	char szDevName[30];		//װ������
}CURRENTBUS;

vector<CURRENTBUS> v_CurrentDev;
vector<int> i_CurrentBusNo;
vector<int> i_CurrentDevNo;
/*end*/

typedef struct _CurrentData
{
	char DataType[10];                          /* �������� */
	WORD wSerialNo;                             /* ��� */
	WORD wPnt;                                  /* ��� */
	char value[100];              				/* ֵ */
	char devName[30];						//װ������
}CURRENTDATA;				/* ----------  end of struct _CurrentData  ---------- */

vector<CURRENTDATA> v_CurrentData;
//extern CRTDBObj *g_pRTDBObj;
CRTDBObj *g_pRTDBObj_cgi;
map<WORD, string>g_SerialnoToDevName;
WORD g_wSerialNo = 0xFFFF;

void setVersion();
bool modifyCode(char * szUniqueCode);

const char * getDevNameFromSerianlNo(WORD wSerianlNo)
{
	int size = g_SerialnoToDevName.size();
	if (size == 0)
		return "NULL";

	map<WORD,string>::iterator  itor = g_SerialnoToDevName.find(wSerianlNo);
	if (itor == g_SerialnoToDevName.end())
		return "NULL";
	return  itor->second.c_str() ;
}

int GetCurrentDataFromShare (int &DevSum, int &BusSum)
{
	/*lel*/
	CURRENTBUS tmpBusAddr;
	STNBUS_ADDR *pStnBusAddr;
	static int BusNoFlag = 0;
	/*end*/
	CURRENTDATA tmpData;
	int i, j;
	STNPARAM *pStnUnit;
	for( i=0; i<g_pRTDBObj_cgi->m_wStnSum; i++ )
	{
		WORD wNum = 0;
		/*lel*/
		pStnBusAddr = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnBusAddr[i];

		if(NULL == pStnBusAddr)
		{
			printf ( "cgi GetCurrentDataFromShare NULL = pStnBusAddr\n" );
			break;
		}
		/*end*/
		pStnUnit = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnUnit[i];

		if(NULL == pStnUnit)
		{
			printf ( "cgi GetCurrentDataFromShare NULL = pStnUnit\n" );
			break;
		}

		/*lel*/
		tmpBusAddr.byLineNo = pStnBusAddr->byBusNo;
		tmpBusAddr.wDevAddr = pStnBusAddr->wDevAddr;
		v_CurrentDev.push_back(tmpBusAddr);

		if(BusNoFlag != tmpBusAddr.byLineNo)
		{
			BusNoFlag = tmpBusAddr.byLineNo;
			i_CurrentBusNo.push_back(tmpBusAddr.byLineNo);
		}
		/*end*/

		//ң������
		wNum = pStnUnit->wAnalogSum;

		for ( j=0; j<wNum; j++)
		{
			// printf ( "name=%s %d %d\n", pStnUnit->szStnName , pStnUnit->wStnNum, pStnUnit->dwAnalogPos);
			ANALOGITEM *pItem = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.AnalogTable[pStnUnit->dwAnalogPos+j];
			strcpy(tmpData.DataType , "YC");
			tmpData.wSerialNo = pStnUnit->wStnNum;
			tmpData.wPnt = pItem->wPntID;
			if ( tmpData.wSerialNo != g_wSerialNo)
				continue;

			if( pItem->fRealVal >= 9.99999 * pow( 10, 9 ) || pItem->fRealVal <= -9.99999 * pow( 10, 8 ))
			{
				strcpy(tmpData.value, "9999999999.999");
			}
			else
			{
				//sprintf( tmpData.value, "%.4f", (float)( pItem->fRealVal * pItem->fRatio ) );
				sprintf( tmpData.value, "%.4f", (float)( pItem->fRealVal));

			}
			tmpData.value[15 - 1] = '\0';
			v_CurrentData.push_back( tmpData);
			/*lel*/
			i_CurrentDevNo.push_back(tmpBusAddr.wDevAddr);
			/*end*/
		}

		//ң������
		wNum = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnUnit[i].wDigitalSum;

		for ( j=0; j<wNum; j++)
		{
			DIGITALITEM *pItem = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.DigitalTable[pStnUnit->dwDigitalPos+j];
			strcpy(tmpData.DataType , "YX");
			tmpData.wSerialNo = pStnUnit->wStnNum;
			tmpData.wPnt = pItem->wPntID;
			if (tmpData.wSerialNo != g_wSerialNo)
				continue;

			memset(tmpData.value, '\0', sizeof(tmpData.value));
			if (pItem->byType==7)
			sprintf(tmpData.value, "%u",pItem->wStatus);
			else
			sprintf( tmpData.value, "%u", pItem->wStatus & 0x03 );

			tmpData.value[15 - 1] = '\0';

			v_CurrentData.push_back( tmpData );
			/*lel*/
			i_CurrentDevNo.push_back(tmpBusAddr.wDevAddr);
			/*end*/
		}


		//ң������
		wNum = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnUnit[i].wPulseSum;
		int Merge_Quantity=0;
		for ( j=0; j<wNum; j++)
		{
			static string tmp;
			PULSEITEM *pItem = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.PulseTable[pStnUnit->dwPulsePos+j];
			strcpy(tmpData.DataType , "YM");
			tmpData.wSerialNo = pStnUnit->wStnNum;
			tmpData.wPnt = pItem->wPntID;
			if (tmpData.wSerialNo != g_wSerialNo)
				continue;

			QWORD tmpCalVal = ( pItem->dwRawVal );
	       /*
			if(strcmp(pItem->szName, "0_LOW") == 0)
			{
				sprintf(tmpData.value, "%08X", static_cast<DWORD>(tmpCalVal));
				tmp = tmpData.value;
				Merge_Quantity++;
				continue;

			}
			else if(strcmp(pItem->szName, "1_HIGH") == 0)
			{
				//sprintf(tmpData.value, "%08X%s", static_cast<DWORD>(tmpCalVal), tmp.c_str());
				// 合并两个32位十六进制值为64位整型，转换为十进制字符串
                uint64_t combined = (static_cast<uint64_t>(static_cast<DWORD>(tmpCalVal)) << 32)
                                  | strtoul(tmp.c_str(), NULL, 16);
                sprintf(tmpData.value, "%llu", combined);
				tmpData.wPnt = pItem->wPntID-Merge_Quantity;

			}*/
			//else
			//{
				//tmpData.wPnt = pItem->wPntID-Merge_Quantity;
				sprintf( tmpData.value, "%llu", tmpCalVal);
			//}

			/*
			if( tmpCalVal >= 9.99999 * pow( 10, 16 ) || tmpCalVal <= -9.99999 * pow( 10, 15 ) )
			{
				strcpy(tmpData.value, "9999999999.999" );
				//continue;
			}
			else
			{
				//sprintf( tmpData.value, "%.4lf", ( double )( pItem->dwRawVal * pItem->fRatio ));
				sprintf( tmpData.value, "%.4lf", tmpCalVal);
			}
			//printf("%d %d %s\n", tmpData.wSerialNo, tmpData.wPnt, tmpData.value);
           */
			v_CurrentData.push_back( tmpData );
			/*lel*/
			i_CurrentDevNo.push_back(tmpBusAddr.wDevAddr);
			/*end*/
		}
	}
	/*lel*/
	DevSum = v_CurrentDev.size();
	BusSum = i_CurrentBusNo.size();
	/*end*/

	return v_CurrentData.size(  );
}		/* -----  end of method CDBaseManager_EEM::GetInsertHistoryDataNumFromShare  ----- */

int Open_SHM_DBase_Cgi()
{
	char szText[128];
	//�������ݿ��������
	if( !g_pRTDBObj_cgi )
		g_pRTDBObj_cgi = new CRTDBObj();
	if( !g_pRTDBObj_cgi ) return -1;
	//���ӹ����ڴ����ݿ�
	if( g_pRTDBObj_cgi->OpenRTDBObj_Cgi(szText) == -1 )
	{
		printf("open rtdb error\n");
		delete g_pRTDBObj_cgi;
		g_pRTDBObj_cgi = NULL;
		return -2;
	}
	//printf("%s\n",szText);

	//printf("--------------   Open RTDB OK   ---------------\n");
	return 0;
}

int readFileList(char *basePath , char * findFileName )
 {
     DIR *dir;
     struct dirent *ptr;
     char base[1000];
	 int ret = 0;
     if ((dir = opendir(basePath)) == NULL)
     {
         perror("Open dir error...");
        // exit(1);
		 return  0;
     }

     while ((ptr = readdir(dir)) != NULL)
     {
         if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    ///current dir OR parrent dir
             continue;
		 else if (ptr->d_type == 8)    ///file
		 {
			 if (strcmp(ptr->d_name, findFileName) == 0)
			 {
				 sprintf(findFileName, "%s/%s", basePath, ptr->d_name);
				 ret = 1;
				 break;
			 }
		 }
         else if (ptr->d_type == 10)    ///link file
             printf("d_name:%s/%s\n", basePath, ptr->d_name);
         else if (ptr->d_type == 4)    ///dir
         {
             memset(base, '\0', sizeof(base));
             strcpy(base, basePath);
             strcat(base, "/");
             strcat(base, ptr->d_name);
			 if (readFileList(base, findFileName) == 1)
			 {
				 ret = 1;
				 break;
			 }
         }
     }
     closedir(dir);
     return ret ;
}

void GetCurrentDevName()
{
	int size = v_CurrentDev.size();
	int findFlag = -1;
	for ( int i = 0 ; i < size ;i++ )
	{
		CURRENTBUS &bus = v_CurrentDev[i];
		BYTE byBusNo = bus.byLineNo;
		WORD wDevAddr = bus.wDevAddr;

		static char busName[1000] = { 0 };
		int res = 0;
		if (findFlag != byBusNo)
		{
			findFlag = byBusNo;
			memset(busName, 0, sizeof(busName));
			sprintf(busName, "Bus%02d.ini", byBusNo);
			res = readFileList("/mynand/config", busName);
			//printf("res = %d , %s \n", res, busName);
		}

		CProfile file(busName);
		char devAddrBuf[10] = { 0 };
		sprintf(devAddrBuf, "DEV%03d", wDevAddr);
		int bufSize = sizeof(bus.szDevName) / sizeof(bus.szDevName[0]);
		file.GetProfileString(devAddrBuf, "name" , "NULL" , bus.szDevName , bufSize );


		int serialno = file.GetProfileInt(devAddrBuf, "serialno", -1);
		//printf("%s %d %s serianlno = %d\n", __FILE__, __LINE__, bus.szDevName , serialno );
		g_SerialnoToDevName[serialno] = string(bus.szDevName);
	}

	size = v_CurrentData.size();
	for (int m = 0; m < size; m++)
	{
		CURRENTDATA &data = v_CurrentData[m];
		strcpy( data.devName, getDevNameFromSerianlNo( data.wSerialNo ) ) ;
	}
}

void setVersion()
{
	DWORD ver = EMU2000_VERSION;

	FILE * pFile = NULL;
	pFile = fopen("/myapp/2000_version", "r");
	char szSvnVersion[10];
	fread(szSvnVersion, sizeof(szSvnVersion), 1, pFile);
	fclose(pFile);

 	printf( "&" );
	char szVersion[20] = { 0 };
	BYTE hh = ver >> 24;
	BYTE hl = ver >> 16 & 0xFF;
	BYTE lh = ver >> 8 & 0xFF;
	BYTE ll = ver & 0xFF;
	sprintf(szVersion, "%d.%d.%d.%d:%s", hh, hl, lh, ll, szSvnVersion);
	printf(szVersion);
}

int getUniqueCode(char * pUniqueCode)
{
	#define IO_SN_READ 	0x1976
	int fd, res;
	char rsp[32];
	char mac[32];
	char random[32];
	char serno[32];
	fd = open("/dev/atsha0", O_RDWR);
	if (fd < 0)
	{
		err(1, "/dev/atsha0");
		return -1;
	}
	res = ioctl(fd, IO_SN_READ, serno);
	if (res)
	{
		printf("error read serno\n");
		close(fd);
		return -1;
	}
	else
	{
		for (int i = 0; i < 9; i++)
		{
			sprintf(&pUniqueCode[ i * 2 ], "%02x", serno[i]);
		}
	}

	close(fd);
}

//У��ͨ��ΪTrue У��ûͨ��ΪFalse
bool checkRegisterFile()
{
	char filename[] = "/usr/reg.reg";
	if (access(filename, F_OK))
	{
		printf("register file does not exist\n");
		return false;
	}

	FILE * file = fopen(filename, "r");
	char szContent[100] = { 0 };
	int res = 0;
	res = fread(szContent, 32, 1, file);
	if (!res)
	{
		printf("register file reads error \n");
		fclose(file);
		return false;
	}
	fclose(file);

	//��ȡ�豸Ψһ��
	char szUniqueCode[100] = { 0 };
	getUniqueCode(szUniqueCode);
	//׼��ע����
	if (!modifyCode(szUniqueCode))
		return false;

	//����MD5��
	char md5Buf[100] = { 0 };
	MD5(md5Buf, szUniqueCode );

	if (strcmp(szContent, md5Buf) != 0)
		return false;

	return true;
}

bool modifyCode(char * szUniqueCode)
{
	if (strlen(szUniqueCode) != 18)
		return false;

	char des[30] = { 0 };
	char * pToken = { "?-+-~qw1"};
	int m = 0, index = 0, n = 0;;
	for (int i = 0; i < 18; )
	{
		if (m == 0 || m ==1 )
		{
			des[index++] = szUniqueCode[i];
			m++;
			i++;
		}
		else
		{
			des[index++] = pToken[n++];
			m = 0;
		}
	}
	memcpy(szUniqueCode, des, sizeof(des));
	return true;
}

//��ע����д���ļ���
bool writeRegisterCodeToFile(char * pMd5)
{
	char filename[] = "/usr/reg.reg";
	FILE * file = fopen(filename, "w");
	int res = 0;
	res = fwrite(pMd5, strlen(pMd5 ) , 1, file);
	if (!res)
	{
		printf("register file writes error \n");
		fclose(file);
		return false;
	}
	fflush(file);
	fclose(file);
	system("sync");
	return true;
}

bool RegisterDev(int argc, char **argv)
{
	if (argc != 2)
		return true;


	printf("Content-type:text/html\n\n");
	if (strcmp(argv[1], "register") == 0)
		{
			if (!checkRegisterFile())
			{
				printf("yes");
				char szUniqueCode[50] = { 0 };
				getUniqueCode(szUniqueCode);
				int len = strlen(szUniqueCode);
				char szUniqueCodeAddSplash[100] = { 0 };
				int index = 0;
				for ( int i = 1 ; i <= len ; i++ )
				{
					if (i % 2 == 0)
					{
						szUniqueCodeAddSplash[index++] = szUniqueCode[i - 1];
						if( i !=len )
							szUniqueCodeAddSplash[index++] = '-';
					}
					else
						szUniqueCodeAddSplash[index++] = szUniqueCode[i - 1];
				}

				char szCode[100] = { 0 };
				strcat(szCode, "&");
				strcat(szCode, szUniqueCodeAddSplash);
				printf(szCode);
				return false;
			}
			else
			{
				printf("no");
				return false;
			}
		}

	//�ӷ�������õ�registercode ��Ҫ��֤
	char argvRegisterCode[40] = { 0 };
	memcpy(argvRegisterCode, argv[1], 32);

	//���ؼ���registercode ����У��
	char szUniqueCode[50] = { 0 };
	char szMd5Code[50] = { 0 };
	getUniqueCode(szUniqueCode);
	modifyCode(szUniqueCode);
	MD5(szMd5Code, szUniqueCode);

	if (strcmp(szMd5Code, argvRegisterCode) == 0)
	{
		//��������д��ע���ļ���
		writeRegisterCodeToFile(szMd5Code);
		printf("success");
		return false;
	}
	else
	{
		printf("failed");
		return false;
	}
}

void GetSerialNoFromArgv(int argc, char **argv)
{
	if (argc != 3 )
		return ;
	if (strcmp(argv[1], "serialNo") != 0)
		return;

	g_wSerialNo = atoi(argv[2]);
}


int main(int argc, char **argv)
{
	if (!RegisterDev( argc , argv ) )
		return 0;

	//��ȡ��ҳ����ʾ��serialNo
	GetSerialNoFromArgv(argc, argv);

	int i = 0;
	g_pRTDBObj_cgi = NULL;
	if( Open_SHM_DBase_Cgi() < 0 )
	{
		printf("cgi open Dbase err!!!");
		return (-1);
	}

	v_CurrentData.clear();
	/*lel*/
	int DevSum = 0;
	int BusSum = 0;
	v_CurrentDev.clear();
	i_CurrentBusNo.clear();
	i_CurrentDevNo.clear();
	/*end*/
	int DataNum = GetCurrentDataFromShare(DevSum, BusSum);
	GetCurrentDevName();

	printf("Content-type:text/html\n\n");
	/*lel*/
#if 1
	for(i = 0; i < BusSum; i++)
	{
		//BYTE busComStatus = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus[i];

		BYTE busComStatus = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnComStatus.byBusComStatus[i_CurrentBusNo[i] - 1];

		//��ʱ�ӵĸ������ж���Ҫ���61850 ����ʾ���ߵ�״̬
		/*
		if (strcmp(g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnComStatus.byBusTypeAndProtocolName[i_CurrentBusNo[i] - 1],"TCP_CLIENT:102-IEC615")==0)

			printf("%d,%d,%s%d %s|", i_CurrentBusNo[i], 0, (char *)"Bus ", i_CurrentBusNo[i],
			g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnComStatus.byBusTypeAndProtocolName[i_CurrentBusNo[i] - 1]);
		else
		*/
		printf("%d,%d,%s%d %s,%d|", i_CurrentBusNo[i], 0, (char *)"Bus ", i_CurrentBusNo[i] ,
			g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnComStatus.byBusTypeAndProtocolName[i_CurrentBusNo[i]-1],
			busComStatus );

	}
	printf("&");

	for(i = 0; i < DevSum; i++)
	{

		BYTE devComStatus = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnComStatus.byDevComStatus[i];
		printf("%d_%d,%d,%s  %d,%d|", v_CurrentDev[i].byLineNo,
			i, v_CurrentDev[i].byLineNo, v_CurrentDev[ i].szDevName, v_CurrentDev[i].wDevAddr ,
			devComStatus
		);
	}
	printf("&");
#endif
	/*end*/

	for( i=0; i<DataNum ; i++ )
	{
		if (i != 0)
		{
			printf("|");
		}
			printf("%d,%s_%d,%s,%d,%s",
				v_CurrentData[i].wSerialNo, v_CurrentData[i].devName, i_CurrentDevNo[i], v_CurrentData[i].DataType,
				v_CurrentData[i].wPnt, v_CurrentData[i].value);
	}

	//���ñ���
	setVersion();

	return 0;
}
