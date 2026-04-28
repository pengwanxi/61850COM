#include "BaseWebPage.h"



CBaseWebPage::CBaseWebPage()
{
}


CBaseWebPage::~CBaseWebPage()
{
}


string CBaseWebPage::GetUptime()
{
	FILE *fp;
	string temp;
	char buf[200] = { 0 };
	memset(buf, '\0', sizeof(buf));
	string strFmt = "cat /proc/uptime |awk '{print $1}' ";//系统从启动到现在运行的时间单位秒
	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}
	while (fgets(buf, 200, fp) != NULL)
	{
		temp = buf;
	}
	pclose(fp);
	temp.erase(temp.end() - 1);//去掉最后一个换行符
	return  temp;

}
string CBaseWebPage::GetVerSion()
{
	string temp;
	DWORD ver = EMU2000_VERSION;
	FILE * pFile = NULL;
	pFile = fopen("/myapp/2000_version", "r");
	char szSvnVersion[10] = { 0 };
	memset(szSvnVersion, '\0', sizeof(szSvnVersion));
	fread(szSvnVersion, sizeof(szSvnVersion), 1, pFile);
	fclose(pFile);

	char szVersion[30] = { 0 };
	memset(szVersion, '\0', sizeof(szVersion));
	BYTE hh = ver >> 24;
	BYTE hl = ver >> 16 & 0xFF;
	BYTE lh = ver >> 8 & 0xFF;
	BYTE ll = ver & 0xFF;

	sprintf(szVersion, "EMU2000 Ver %d.%d.%d.%d:%s", hh, hl, lh, ll, szSvnVersion);
	temp = szVersion;
	temp.erase(temp.end() - 1);//去掉最后一个换行符
	return temp;

}
string CBaseWebPage::GetSysTime()
{
	FILE *fp;
	string temp;
	char buf[200] = { 0 };
	memset(buf, '\0', sizeof(buf));
	string strFmt = "date +%Y-%m-%d' '%H:%M:%M ";
	if ((fp = popen(strFmt.c_str(), "r")) == NULL)
	{
		perror("Fail to popen\n");
		exit(1);
	}
	while (fgets(buf, 200, fp) != NULL)
	{
		temp = buf;
	}
	pclose(fp);
	temp.erase(temp.end() - 1);//去掉最后一个换行符
	return  temp;
}

void CBaseWebPage::GetMemData(key_t key)
{
	int shmid;
	int ret;	
    char *shmadd;	
	//打开共享内存  
	shmid = shmget(key, BUFSZ, IPC_CREAT | 0666);
	 if (shmid < 0)
	 {
	        perror("shmget");
		       exit(-1);
	  }
	    //映射  
	  shmadd = (char *)shmat(shmid, NULL, 0);
	   if (shmadd < 0)
		{
		       perror("shmat");
		       exit(-1);
		  }
	 //读共享内存区数据  
	 printf("%s\n", shmadd);
	
	//分离共享内存和当前进程  
	 ret = shmdt(shmadd);
	  if (ret < 0)
	   {
		     perror("shmdt");
             exit(1);
	    }  
	   //删除共享内存  
	    shmctl(shmid, IPC_RMID, NULL);

}



