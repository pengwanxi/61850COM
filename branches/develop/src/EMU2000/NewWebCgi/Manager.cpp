#include "Manager.h"
#include "RunStatusPage.h"
#include "SysOperatePage.h"
#include"ParSetPage.h"
#include"ProjectJson.h"
#include"TransDataPage.h"
#include "DataManagementPage.h"
#include "json/json.h"
#include "Communication.h"
#include "Template_Manager.h"

CManager::CManager()
{

	m_log.setLogKey("NewWebCgi");	

	addPage(new CRunStatusPage);
	addPage(new CSysOperatePage);
	addPage(new CparSetPage);
	addPage(new CCommunication);
	addPage(new CTemplate_Manager);
	//addPage(new  CProJson);
	addPage(new  CTransDataPage);	
	addPage(new  CDataManagerPage);
}

CManager::~CManager()
{
}

void CManager::init()
{
	int size = m_vecPage.size();
	for ( int i = 0 ; i < size ; i++ )
		m_vecPage[i]->Init();
	
}
void CManager::addPage(CBaseWebPage * pPage)
{
	if (pPage == NULL)
		return;
	m_vecPage.push_back(pPage);
}

void CManager::getJSonStructString()
{
	m_JSonString.clear();
	Json::Value JVal;
	int size = m_vecPage.size();
	for ( int i = 0 ; i < size ; i++ )
	{
		m_vecPage[i]->getJSONStructFromWebPage(JVal);

	}
	Json::FastWriter fast_writer;
	m_JSonString = fast_writer.write(JVal);
}

UINT CManager::sendTo()
{
	printf("Content-type:text/html\n\n");
	//printf("Content-type:application/json\n\n");
	printf(m_JSonString.data());	

	
}
void CManager::Get_Input()
{
	char *method;

	char *lenstr;

	char *str;

	long len;

	method = getenv("REQUEST_METHOD"); //将返回结果赋予指针
	if (method == NULL)
	{
		printf("Content-type:text/html\n\n");
		printf("getenv REQUEST_METHOD ERROR\n");		
		
	}
	else
	{
		if (!strcmp(method, "POST"))  // POST方法
		{

			lenstr = getenv("CONTENT_LENGTH");

			sscanf(lenstr, "%ld", &len);

			str = new char(len+2);

			fgets(str, len + 1, stdin);

			
			printf("len=%d str=%s\n",len,str);

			////接收时boa 好像添加了data= 所以指针移动了5个---有待验证
			if (str[5] == '1')//临时的到时候看看前端发送个标记过来或者根据JSON串的格式进行判断-----此判断标志为自己测试临时接收的
			{
				printf("Content-type:text/html\n\n");
				printf("POST\n");
				//开辟一段共享内内存空间先保存一下数据
				WriteDataToMemory(str + 5, SHMPARSETBKEY);
				m_vecPage[0]->procCmd(0);//具体对应哪个网页还需调试时还需修改	
				//printf("%s\n", str + 5);					
			
			}
			else{
				printf("Content-type:text/html\n\n");
				printf("POST\n");
				printf("An error occurred when the program sent the command \n");
				//sendTo();				
			}		

			delete str;		

		}
		else if (!strcmp(method, "GET"))
		{
			
			str = new char(1024*10);
			str = getenv("QUERY_STRING");
			if (str[5] == '1')//临时的到时候看看前端发送个标记过来或者根据JSON串的格式进行判断-----此判断标志为自己测试临时接收的
			{
				printf("Content-type:text/html\n\n");
				printf("GET\n");
				//开辟一段共享内内存空间先保存一下数据
				WriteDataToMemory(str + 5, SHMPARSETBKEY);
				m_vecPage[0]->procCmd(0);//具体对应哪个网页还需调试时还需修改	
				//printf("%s\n", str + 5);

			}
			else{
				printf("Content-type:text/html\n\n");
				printf("GET\n");
				printf("An error occurred when the program sent the command \n");
				//sendTo();

			}
			delete str;

		}

	}

}
BOOL  CManager::WriteDataToMemory(char *str,key_t key)
{
	////1.创建共享内存
	int shmid = -1;
	shmid = shmget(key, BUFSZ, IPC_CREAT | 0666);//返回值为id号
	if (shmid < 0)
	{
		perror("shmget");
		exit(1);
	}
	//2.映射
	char *ptr = NULL;
	ptr = (char *)shmat(shmid, NULL, 0);// 返回值为被映射的段地址
	if (ptr == (void *)-1)
	{
		perror("shmat");
		exit(1);
	}

	//3.写数据
	strncpy(ptr, str, strlen(str));		
	
	//4.解除共享内存映射
	if (shmdt(ptr) < 0)
	{
		perror("shmdt");
		exit(1);
	}
	  return FALSE;

}


