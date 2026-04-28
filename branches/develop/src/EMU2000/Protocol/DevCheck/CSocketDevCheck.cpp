/*
 * =====================================================================================
 *
 *       Filename:  CSocketDevCheck.cpp
 *
 *    Description:  利用devcheck 获取所有采集装置的装置名称
 *
 *        Version:  1.0
 *        Created:  2015年09月24日 11时29分57秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */
#include"CSocketDevCheck.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

#define PORT 16000

map<int, string> filepath;
//char DevIdCheck[1024];
int readFileList(char *basePath)
{
	DIR *dir;
	struct dirent *ptr;
	char base[1000];
	char pathname[508];
	int num = 0;

	if ((dir = opendir(basePath)) == NULL)
	{
		perror("Open dir error...");
		exit(1);
	}
	while ((ptr = readdir(dir)) != NULL)
	{
		if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)    //current dir OR parrent dir
			continue;
		else if (ptr->d_type == 8)   ///file
		{
			sprintf(pathname, "%s/%s", basePath, ptr->d_name);
			string temp;
			temp = ptr->d_name;
			if (temp.size() == 9 && temp[0] == 'B'&&temp[1] == 'u'&&temp[2] == 's')
			{
				char templatefile[100] = { 0 };
				memset(templatefile, 0, sizeof(templatefile));
				sprintf(templatefile, "%s/%s", basePath, "template");
				//printf("%s\n", templatefile);
				if (access(templatefile, F_OK) == 0)
				{
					string name = ptr->d_name;
					name = name.substr(3, 2);
					//printf("---%s\n", name.c_str());
					filepath[atoi(name.c_str()) - 1] = pathname;
					//filename.push_back(ptr->d_name);
					//printf("%s\n", pathname);
				}

			}

		}
		else if (ptr->d_type == 10)   ///link file
		{
			printf("d_name:%s/%s\n", basePath, ptr->d_name);
		}
		else if (ptr->d_type == 4)   ///dir
		{
			memset(base, '\0', sizeof(base));
			strcpy(base, basePath);
			strcat(base, "/");
			strcat(base, ptr->d_name);
			readFileList(base);
		}
	}
	closedir(dir);
	return 1;
}

void GetCurrentDevName(char DevIdCheck[])
{
	char *BusName=NULL;
	int res = readFileList("/mynand/config");
	memset(DevIdCheck, '\0', sizeof(DevIdCheck));
	for (int i = 0; i < filepath.size(); i++)
	{
		if (filepath[i].empty())
			continue;

	    char busName[1000] = { 0 };
		int res = 0;
		memset(busName, 0, sizeof(busName));
		sprintf(busName, "%s", filepath[i].c_str());
		CProfile file(busName);
		int devnum = file.GetProfileInt("DEVNUM", "NUM", -1);
		for (int j = 0; j < devnum; j++)
		{
			char devAddrBuf[10] = { 0 };
			char szDevName[20] = { 0 };
			sprintf(devAddrBuf, "DEV%03d", j+1);
			file.GetProfileString(devAddrBuf, "name", "NULL", szDevName, 20);
			BusName = strtok(szDevName, "++");//部分协议中name=装置号,站址 字符串截取只提取装置号就行
			strcat(DevIdCheck, BusName);
			strcat(DevIdCheck, ",");
		}

	}

}


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketDevCheck
 *      Method:  CSocketDevCheck
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CSocketDevCheck::CSocketDevCheck()
{
	
}  /* -----  end of method CSocketDevCheck::CSocketDevCheck  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketDevCheck
 *      Method:  ~CSocketDevCheck
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CSocketDevCheck::~CSocketDevCheck()
{
	
}  /* -----  end of method CSocketDevCheck::~CSocketDevCheck  (destructor)  ----- */

void *pthread_data(void *arg)
//void ScanDevId()
{
	/* sock_fd --- socket文件描述符 创建udp套接字*/
	char  DevIdCheck[4096] = { 0 };
	GetCurrentDevName(DevIdCheck);
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
		memset(recv_buf, '\0', sizeof(recv_buf));
		recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);

		recv_buf[recv_num] = '\0';
		printf("server receive %d bytes: %s  %d\n", recv_num, recv_buf,strlen(recv_buf));
		printf("recvfrom %s \n", inet_ntoa(addr_client.sin_addr));

		if (strcmp(recv_buf, "devid") == 0)
		{
			printf("*********%d %s************\n", __LINE__, __FILE__);
			struct sockaddr_in to;
			to.sin_family = AF_INET;   //使用IPV4地址
			to.sin_port = htons(PORT);  //端口
			to.sin_addr.s_addr = inet_addr(inet_ntoa(addr_client.sin_addr));

			setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &set, sizeof(int));

			//printf("----- strlen(DevIdCheck)=%d--%s-\n", strlen(DevIdCheck) - 1, DevIdCheck);
			send_num = sendto(sock_fd, DevIdCheck, strlen(DevIdCheck) - 1, 0,(struct sockaddr *)&to, sizeof(struct sockaddr));
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
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CSocketDevCheck
 *      Method:  CreateTcpServer
 * Description:  创建服务器
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
// */
BOOL CSocketDevCheck::CreateUdpServer ( void )
{
	pthread_t DataDevCheck_id;
	int pthreadret;
	int i = 0;
	/*ScanDevId();*/
	pthreadret = pthread_create(&DataDevCheck_id, NULL, pthread_data, NULL);
	if (pthreadret != 0)
	{
		printf("pthread_DataDevcheck creat error!");
	}

	return TRUE;
}		/* -----  end of method CSocketDevCheck::CreateTcpServer  ----- */


///*
// *--------------------------------------------------------------------------------------
// *       Class:  CSocketDevCheck
// *      Method:  Init
// * Description:  创始化该协议
// *       Input:
// *		Return:
// *--------------------------------------------------------------------------------------
// */
BOOL CSocketDevCheck::Init ( void )
{
	//创建UDP服务器
	if( !CreateUdpServer(  ) )
	{
		return FALSE;	
	}	
	return TRUE;
	
}		/* -----  end of method CSocketDevCheck::Init  ----- */

