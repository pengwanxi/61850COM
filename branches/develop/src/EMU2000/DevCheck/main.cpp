#include <iostream>
#include <vector> 
#include <map>
#include <stdio.h>
#include <math.h>
#include "../share/UdpPort.h"
#include "../share/global.h"
#include "../librtdb/rdbObj.h"
#include "../share/profile.h"
#include <pthread.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <dirent.h>

using namespace std;

#define PORT 16000

/*lel*/
typedef struct CurrentBus
{
	BYTE byLineNo;								//总线号
	WORD wDevAddr;								//装置地址
	BYTE byLineNum;								//总线个数
	char szDevName[20];		//装置名称
}CURRENTBUS;

vector<CURRENTBUS> v_CurrentDev;
vector<int> i_CurrentBusNo;
vector<int> i_CurrentDevNo;
/*end*/
typedef struct _CurrentData
{
	char DataType[10];                          /* 数据类型 */
	WORD wSerialNo;                             /* 序号 */
	WORD wPnt;                                  /* 点号 */
	char value[15];              				/* 值 */
	char devName[20];						//装置名称
}CURRENTDATA;				/* ----------  end of struct _CurrentData  ---------- */

vector<CURRENTDATA> v_CurrentData;
CRTDBObj *g_pRTDBObj_cgi;
map<WORD, string>g_SerialnoToDevName;
char DevIdCheck[1024];

int Open_SHM_DBase_Cgi()
{
	char szText[128];
	//构造数据库管理对象
	if (!g_pRTDBObj_cgi)
		g_pRTDBObj_cgi = new CRTDBObj();
	if (!g_pRTDBObj_cgi) return -1;
	//连接共享内存数据库
	if (g_pRTDBObj_cgi->OpenRTDBObj_Cgi(szText) == -1)
	{
		printf("open rtdb error\n");
		delete g_pRTDBObj_cgi;
		g_pRTDBObj_cgi = NULL;
		return -2;
	}

	return 0;
}

const char * getDevNameFromSerianlNo(WORD wSerianlNo)
{
	int size = g_SerialnoToDevName.size();
	if (size == 0)
		return "NULL";

	map<WORD, string>::iterator  itor = g_SerialnoToDevName.find(wSerianlNo);
	if (itor == g_SerialnoToDevName.end())
		return "NULL";
	return  itor->second.c_str();
}

int readFileList(char *basePath, char * findFileName)
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
	return ret;
}
int GetCurrentDataFromShare(int &DevSum, int &BusSum)
{
	/*lel*/
	CURRENTBUS tmpBusAddr;
	STNBUS_ADDR *pStnBusAddr;
	static int BusNoFlag = 0;
	/*end*/
	CURRENTDATA tmpData;
	int i, j;
	STNPARAM *pStnUnit;
	for (i = 0; i<g_pRTDBObj_cgi->m_wStnSum; i++)
	{
		WORD wNum = 0;
		/*lel*/
		pStnBusAddr = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnBusAddr[i];

		if (NULL == pStnBusAddr)
		{
			printf("cgi GetCurrentDataFromShare NULL = pStnBusAddr\n");
			break;
		}
		/*end*/
		pStnUnit = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnUnit[i];
		if (NULL == pStnUnit)
		{
			printf("cgi GetCurrentDataFromShare NULL = pStnUnit\n");
			break;
		}
		/*lel*/
		tmpBusAddr.byLineNo = pStnBusAddr->byBusNo;
		tmpBusAddr.wDevAddr = pStnBusAddr->wDevAddr;
		v_CurrentDev.push_back(tmpBusAddr);

		if (BusNoFlag != tmpBusAddr.byLineNo)
		{
			BusNoFlag = tmpBusAddr.byLineNo;
			i_CurrentBusNo.push_back(tmpBusAddr.byLineNo);
		}

		/*end*/

		//遥测数据
		wNum = pStnUnit->wAnalogSum;

		for (j = 0; j<wNum; j++)
		{
			// printf ( "name=%s %d %d\n", pStnUnit->szStnName , pStnUnit->wStnNum, pStnUnit->dwAnalogPos);
			ANALOGITEM *pItem = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.AnalogTable[pStnUnit->dwAnalogPos + j];
			strcpy(tmpData.DataType, "YC");
			tmpData.wSerialNo = pStnUnit->wStnNum;
			tmpData.wPnt = pItem->wPntID;

			if (pItem->fRealVal >= 9.99999 * pow(10, 9) || pItem->fRealVal <= -9.99999 * pow(10, 8))
			{
				strcpy(tmpData.value, "9999999999.999");
			}
			else
			{
				sprintf(tmpData.value, "%.4f", (float)(pItem->fRealVal * pItem->fRatio));
				//printf("yc v*x=xv %f*%f=%.3f\n",pItem->fRealVal,pItem->fRatio,(float)( pItem->fRealVal * pItem->fRatio ));
			}
			tmpData.value[15 - 1] = '\0';
			v_CurrentData.push_back(tmpData);
			/*lel*/
			i_CurrentDevNo.push_back(tmpBusAddr.wDevAddr);
			/*end*/
		}

		//遥信数据
		wNum = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnUnit[i].wDigitalSum;
		for (j = 0; j<wNum; j++)
		{
			DIGITALITEM *pItem = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.DigitalTable[pStnUnit->dwDigitalPos + j];
			strcpy(tmpData.DataType, "YX");
			tmpData.wSerialNo = pStnUnit->wStnNum;
			tmpData.wPnt = pItem->wPntID;
			sprintf(tmpData.value, "%u", pItem->wStatus & 0x03);

			v_CurrentData.push_back(tmpData);
			/*lel*/
			i_CurrentDevNo.push_back(tmpBusAddr.wDevAddr);
			/*end*/
		}

		//遥脉数据
		wNum = g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.StnUnit[i].wPulseSum;
		for (j = 0; j<wNum; j++)
		{
			PULSEITEM *pItem = &g_pRTDBObj_cgi->m_pRTDBSpace->RTDBase.PulseTable[pStnUnit->dwPulsePos + j];
			strcpy(tmpData.DataType, "YM");
			tmpData.wSerialNo = pStnUnit->wStnNum;
			tmpData.wPnt = pItem->wPntID;

			double tmpCalVal = (double)(pItem->dwRawVal * pItem->fRatio);
			if (tmpCalVal >= 9.99999 * pow(10, 9) || tmpCalVal <= -9.99999 * pow(10, 8))
			{
				strcpy(tmpData.value, "9999999999.999");
				continue;
			}
			else
			{
				sprintf(tmpData.value, "%.4lf", (double)(pItem->dwRawVal * pItem->fRatio));
			}
			v_CurrentData.push_back(tmpData);
			/*lel*/
			i_CurrentDevNo.push_back(tmpBusAddr.wDevAddr);
			/*end*/
		}
	}
	/*lel*/
	DevSum = v_CurrentDev.size();
	BusSum = i_CurrentBusNo.size();
	/*end*/
	return v_CurrentData.size();
}		/* -----  end of method CDBaseManager_EEM::GetInsertHistoryDataNumFromShare  ----- */
void GetCurrentDevName()
{
	char *BusName;
	int size = v_CurrentDev.size();
	memset(DevIdCheck,'\0',sizeof(DevIdCheck));
	int findFlag = -1;
	for (int i = 0; i < size; i++)
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
			
		}
		CProfile file(busName);
		char devAddrBuf[10] = { 0 };
		sprintf(devAddrBuf, "DEV%03d", wDevAddr);
		int bufSize = sizeof(bus.szDevName) / sizeof(bus.szDevName[0]);
		file.GetProfileString(devAddrBuf, "name", "NULL", bus.szDevName, bufSize);

		
		BusName = strtok(bus.szDevName, "++");//部分协议中name=装置号,站址 字符串截取只提取装置号就行

		int serialno = file.GetProfileInt(devAddrBuf, "serialno", -1);
		printf("%s %d %s serianlno = %d\n", __FILE__, __LINE__, bus.szDevName, serialno);
		//strcat(DevIdCheck, bus.szDevName);
		strcat(DevIdCheck, BusName);
		strcat(DevIdCheck, ",");
		
		//g_SerialnoToDevName[serialno] = string(bus.szDevName);
		g_SerialnoToDevName[serialno] = string(BusName);
		BusName = NULL;
		
	}
	size = v_CurrentData.size();
	for (int m = 0; m < size; m++)
	{
		CURRENTDATA &data = v_CurrentData[m];
		strcpy(data.devName, getDevNameFromSerianlNo(data.wSerialNo));
		
	}

}

void *pthread_data(void *arg)
//void ScanDevId()
{
	/* sock_fd --- socket文件描述符 创建udp套接字*/
	printf("--------------------------\n");
	int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd < 0)
	{
		perror("socket");
		exit(1);
	}
	int set = 1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));

	/* 将套接字和IP、端口绑定 */
	struct sockaddr_in addr_serv;
	int len;
	memset(&addr_serv, 0, sizeof(struct sockaddr_in));  //每个字节都用0填充
	addr_serv.sin_family = AF_INET;   //使用IPV4地址
	addr_serv.sin_port = htons(PORT);  //端口
	/* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是PORT，就会被该应用程序接收到 */
	addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);  //自动获取IP地址
	len = sizeof(addr_serv);

	/* 绑定socket */
	if (bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0)
	{
		perror("bind error:");
		exit(1);
	}
	int recv_num;
	int send_num;
	char recv_buf[20];
	int i;
	struct sockaddr_in addr_client;
	while (1)
	{
		printf("server wait:\n");
		memset(recv_buf,'\0',sizeof(recv_buf));
		recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);

		recv_buf[recv_num] = '\0';
		printf("server receive %d bytes: %s\n", recv_num, recv_buf);
		printf("recvfrom %s \n", inet_ntoa(addr_client.sin_addr));
		if (strcmp(recv_buf, "devid")==0)
		{
			struct sockaddr_in to;
			to.sin_family = AF_INET;   //使用IPV4地址
			to.sin_port = htons(PORT);  //端口
			to.sin_addr.s_addr = inet_addr(inet_ntoa(addr_client.sin_addr));

			setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));
			printf("----- strlen(DevIdCheck)=%d--%s-\n", strlen(DevIdCheck) - 1, DevIdCheck);

			send_num = sendto(sock_fd, DevIdCheck, strlen(DevIdCheck) - 1, 0,
				(struct sockaddr *)&to, sizeof(struct sockaddr));

			printf("send_num=%d**** %s\n", send_num, DevIdCheck);
			if (send_num < 0)
			{
				perror("sendto error:");
				//exit(1);
			}
		}
	}
	close(sock_fd);
	pthread_exit(0);
}
int main()
{
	pthread_t DataDevCheck_id;
	int pthreadret;
	int i = 0;
	g_pRTDBObj_cgi = NULL;
	if (Open_SHM_DBase_Cgi() < 0)
	{
		printf("devcheck open Dbase err!!!");
		return (-1);
	}
	v_CurrentData.clear();
	/*lel*/
	int DevSum = 0;
	int BusSum = 0;
	v_CurrentDev.clear();
	i_CurrentBusNo.clear();
	i_CurrentDevNo.clear();

	int DataNum = GetCurrentDataFromShare(DevSum, BusSum);
	GetCurrentDevName();
	/*ScanDevId();*/
	pthreadret = pthread_create(&DataDevCheck_id, NULL, pthread_data, NULL);
	if (pthreadret != 0)
		printf("pthread_DataDevcheck creat error!");
	pthread_join(DataDevCheck_id, NULL);
	return 0;
}

