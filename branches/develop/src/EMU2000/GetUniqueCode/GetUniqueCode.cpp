#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
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
#include <sys/stat.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <termios.h> /* POSIX terminal control definitions */
//#include<json/json.h>
#include <string>
#include<iostream>

using namespace std;

#define MAX_COMM_NUM  16
char *SERIAL_NAME[MAX_COMM_NUM] =
{
	(char *)"/dev/esdS1",
	(char *)"/dev/esdS2",
	(char *)"/dev/esdS3",
	(char *)"/dev/esdS4",
	(char *)"/dev/esdS5",
	(char *)"/dev/esdS6",
	(char *)"/dev/esdS7",
	(char *)"/dev/esdS8",
	(char *)"/dev/esdS9",
	(char *)"/dev/esdS10",
	(char *)"/dev/esdS11",
	(char *)"/dev/esdS12",
	(char *)"/dev/esdS13",
	(char *)"/dev/esdS14",
	(char *)"/dev/esdS15",
	(char *)"/dev/esdS16"
};
int set_port_attr(int fd, int  baudrate, int  databit, const char *stopbit, char parity, int vtime, int vmin);
static void set_baudrate(struct termios *opt, unsigned int baudrate);
static void set_data_bit(struct termios *opt, unsigned int databit);
static void set_stopbit(struct termios *opt, const char *stopbit);
static void set_parity(struct termios *opt, char parity);
int getUniqueCode(char * pUniqueCode);
int GetEthMac(char *eth_name, char *mac_add);
int GetEthMac(char *eth_name, char *mac_add);
void modifyCode(char * szUniqueCode);
void ReadCsvFile(char* filePath);


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
		//err(1, "/dev/atsha0");
		return -1;
	}
	res = ioctl(fd, IO_SN_READ, serno);
	if (res)
	{
		//printf("error read serno\n");
		close(fd);
		return -1;
	}
	else
	{
		for (int i = 0; i < 9; i++)
		{
			sprintf(&pUniqueCode[i * 2], "%02x", serno[i]);
		}
	}
	close(fd);
	return 0;
}

int GetEthMac(char *eth_name,char *mac_add)
{
	struct ifreq ifreq;
	int sock = 0;
	char mac[32] = "";

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		//perror("error sock");
		return 2;
	}
	strcpy(ifreq.ifr_name, eth_name);
	if (ioctl(sock, SIOCGIFHWADDR, &ifreq) < 0)
	{
		//perror("error ioctl");
		return 3;
	}
	int i = 0;
	for (i = 0; i < 6; i++){
		sprintf(mac + 3 * i, "%02X:", (unsigned char)ifreq.ifr_hwaddr.sa_data[i]);
	}
	mac[strlen(mac) - 1] = 0;
	strcpy(mac_add,mac);
	
	return 0;
}

void modifyCode(char * szUniqueCode)
{
	
	char des[30] = { 0 };

	int m = 0, index = 0, n = 0;;
	for (int i = 0; i < 18;)
	{
		if (m == 0 || m == 1)
		{
			des[index++] = szUniqueCode[i];
			m++;
			i++;
		}
		else
		{
			des[index++] = '-';
			m = 0;
		}
	}
	memcpy(szUniqueCode, des, sizeof(des));

}
void ReadCsvFile(char* filePath)
{
	char data[300] = {0};
	FILE *fp = fopen(filePath, "r");
	if (!fp)
	{
		printf("can't open file\n");
	}
	while (!feof(fp))
	{
		memset(data,'\0',sizeof(data));
		fscanf(fp, "%s", &data);
		printf("%s\n", data);	
	}
	fclose(fp);
}


//自定义终端属性设置函数具体定义如下
//设置终端属性的时候注意,有的项目是通过与&,而有的项目是通过或|. 不要混淆误解.
int  set_port_attr(int fd, int  baudrate, int  databit, const char *stopbit, char parity, int vtime, int vmin)
{
	struct termios opt;
	tcgetattr(fd, &opt);       //获取初始设置

	set_baudrate(&opt, baudrate);
	set_data_bit(&opt, databit);
	set_parity(&opt, parity);
	set_stopbit(&opt, stopbit);

	opt.c_cflag &= ~CRTSCTS;// 不使用硬件流控制
	opt.c_cflag |= CLOCAL | CREAD; //CLOCAL--忽略 modem 控制线,本地连线, 不具数据机控制功能, CREAD--使能接收标志 

	/*
	IXON--启用输出的 XON/XOFF 流控制
	IXOFF--启用输入的 XON/XOFF 流控制
	IXANY--允许任何字符来重新开始输出
	IGNCR--忽略输入中的回车
	*/
	opt.c_iflag &= ~(IXON | IXOFF | IXANY);
	opt.c_oflag &= ~OPOST; //启用输出处理
	/*
	ICANON--启用标准模式 (canonical mode)。允许使用特殊字符 EOF, EOL,
	EOL2, ERASE, KILL, LNEXT, REPRINT, STATUS, 和 WERASE，以及按行的缓冲。
	ECHO--回显输入字符
	ECHOE--如果同时设置了 ICANON，字符 ERASE 擦除前一个输入字符，WERASE 擦除前一个词
	ISIG--当接受到字符 INTR, QUIT, SUSP, 或 DSUSP 时，产生相应的信号
	*/
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	opt.c_cc[VMIN] = vmin; //设置非规范模式下的超时时长和最小字符数：
	opt.c_cc[VTIME] = vtime; //VTIME与VMIN配合使用，是指限定的传输或等待的最长时间

	tcflush(fd, TCIFLUSH);                    /* TCIFLUSH-- update the options and do it NOW */
	return (tcsetattr(fd, TCSANOW, &opt)); /* TCSANOW--改变立即发生 */
}

//自定义set_baudrate()函数
//使用set_baudrate()函数设置串口输入/输出波特率为115200的代码为：set_baudrate(&opt, B115200));
//通常来说，串口的输入和输出波特率都设置为同一个值,如果要分开设置,就要分别调用cfsetispeed , cfsetospeed
static void set_baudrate(struct termios *opt, unsigned int baudrate)
{
	cfsetispeed(opt, baudrate);
	cfsetospeed(opt, baudrate);
}

//自定义set_stopbit函数
//在set_stopbit()函数中，stopbit参数可以取值为：“1”（1位停止位）、“1.5”（1.5位停止位）和“2”（2位停止位）。
static void set_stopbit(struct termios *opt, const char *stopbit)
{
	if (0 == strcmp(stopbit, "1")) {
		opt->c_cflag &= ~CSTOPB;                                                           /* 1位停止位t             */
	}
	else if (0 == strcmp(stopbit, "1.5")) {
		opt->c_cflag &= ~CSTOPB;                                                           /* 1.5位停止位            */
	}
	else if (0 == strcmp(stopbit, "2")) {
		opt->c_cflag |= CSTOPB;                                                            /* 2 位停止位             */
	}
	else {
		opt->c_cflag &= ~CSTOPB;                                                           /* 1 位停止位             */
	}
}

//set_data_bit函数
//CSIZE--字符长度掩码。取值为 CS5, CS6, CS7, 或 CS8
static void set_data_bit(struct termios *opt, unsigned int databit)
{
	opt->c_cflag &= ~CSIZE;
	switch (databit) {
	case 8:
		opt->c_cflag |= CS8;
		break;
	case 7:
		opt->c_cflag |= CS7;
		break;
	case 6:
		opt->c_cflag |= CS6;
		break;
	case 5:
		opt->c_cflag |= CS5;
		break;
	default:
		opt->c_cflag |= CS8;
		break;
	}
}

//set_parity函数
//在set_parity函数中，parity参数可以取值为：‘N’和‘n’（无奇偶校验）、‘E’和‘e’（表示偶校验）、‘O’和‘o’（表示奇校验）。
static void set_parity(struct termios *opt, char parity)
{
	switch (parity) {
	case 'N':                                                                                   /* 无校验 */
	case 'n':
		opt->c_cflag &= ~PARENB;
		break;
	case 'E':                                                                                   /* 偶校验 */
	case 'e':
		opt->c_cflag |= PARENB;
		opt->c_cflag &= ~PARODD;
		break;
	case 'O':                                                                                   /* 奇校验 */
	case 'o':
		opt->c_cflag |= PARENB;
		opt->c_cflag |= ~PARODD;
		break;
	default:                                                                                    /* 其它选择为无校验 */
		opt->c_cflag &= ~PARENB;
		break;
	}
}

int main(int argc,char *argv[])
{
	//Json::Value root;
	//获取设备唯一码
	char szUniqueCode[100] = { 0 };
	int ret = 0;
	ret=getUniqueCode(szUniqueCode);
	modifyCode(szUniqueCode);	

	char MacAddress[100] = { 0 };

	if (argc == 2 || argc == 5)
	{
		if (strcmp(argv[1], "-code") == 0)
		{
			if (ret == 0)
			{
				printf("%s\n", szUniqueCode);
			}
			else
			{
				printf("%s\n", "00-01-02-03-04-05-06-07-08");
			}

		}
		else if (strcmp(argv[1], "-mac1") == 0)
		{
			memset(MacAddress, '\0', sizeof(MacAddress));
			ret = GetEthMac((char *)"eth0", MacAddress);
			printf("%s\n", MacAddress);
		}
		else if (strcmp(argv[1], "-mac2") == 0)
		{
			memset(MacAddress, '\0', sizeof(MacAddress));
			ret = GetEthMac((char *)"eth1", MacAddress);
			printf("%s\n", MacAddress);
		}
		else if (strcmp(argv[1], "-mac3") == 0)
		{
			memset(MacAddress, '\0', sizeof(MacAddress));
			ret = GetEthMac((char *)"eth2", MacAddress);
			printf("%s\n", MacAddress);
		}
		else if (strcmp(argv[1], "-mac4") == 0)
		{
			memset(MacAddress, '\0', sizeof(MacAddress));
			ret = GetEthMac((char *)"eth3", MacAddress);
			printf("%s\n", MacAddress);
		}
		else if (strcmp(argv[1], "-platform") == 0)
		{
			printf("%s\n", "2");
		}
		else if (strcmp(argv[1], "-myreboot") == 0)
		{
			printf("reboot success\n");
			system("reboot");
			system("sync");
		}
		else if (strcmp(argv[1], "-isregisted") == 0)
		{
			if (access("/usr/reg.reg", F_OK) == 0)
			{
				printf("true\n");
			}
			else
			{
				printf("false\n");
			}

		}
		else if (strcmp(argv[1], "-readreg") == 0)
		{
			if (access("/usr/reg.reg", F_OK) == 0)
			{
				ReadCsvFile("/usr/reg.reg");
			}
			else
			{
				printf("file does not exist\n");
			}

		}
		else if (strcmp(argv[1], "-ischangemac") == 0)
		{
			if (access("/myapp/changeMac.sh", F_OK) == 0)
			{
				printf("true\n");
			}
			else
			{
				printf("false\n");
			}
		}
		else if (strcmp(argv[1], "-writeinfo") == 0)
		{
			FILE *fpWrite;			
			if (argc == 5)
			{
				fpWrite = fopen("/usr/info_cmt.txt", "w+");
				printf("write info success\n");
				fprintf(fpWrite, "%s\n%s\n%s\n", argv[2], argv[3], argv[4]);
				fclose(fpWrite);
			}				
			else
			{
				printf("write info fail and Command parameter error please try agin\n");
				
			}

		}
		else if (strcmp(argv[1], "-readinfo") == 0)
		{
			if (access("/usr/info_cmt.txt", F_OK) == 0)
			{
				ReadCsvFile("/usr/info_cmt.txt");
				
			}
			else
			{
				printf("file does not exist\n");
			}
			
		}
		else if (strcmp(argv[1], "-serial") == 0)
		{
			int  hCom[MAX_COMM_NUM] = { 0 };
			char writebuf[20] = { 0 };
			char readbuf[20] = { 0 };
			int lenwrite = 0;
			int lenread = 0;
			int ComNum = 0;
			int i, t = 0;		 
			
			system("killall -9 pman gather rtdbserver > /dev/null 2>&1");
			sleep(1);
			for (i = 0; i < MAX_COMM_NUM; i++)//打开端口号
			{
				hCom[i] = open(SERIAL_NAME[i], O_RDWR | O_NOCTTY | O_NDELAY, 0);
				if (hCom[i] <= 0)
				{
						continue;
				}
				tcflush(hCom[i], TCIFLUSH);
				ComNum++;
			}

			for (int DetectionTimes = 0; DetectionTimes < 2; DetectionTimes++)//读写了两遍
			{
				for (i = 0; i < ComNum; i++)//读写操作
				{
					ret = set_port_attr(hCom[i], B9600, 8, "1", 'N', 10, 20);
					memset(writebuf, '\0', sizeof(writebuf));
					sprintf(writebuf, "esdtekcom%02d", i + 1);
					tcflush(hCom[i], TCOFLUSH);
					lenwrite = write(hCom[i], writebuf, strlen(writebuf));
					if (lenwrite == strlen(writebuf))
					{
						if (DetectionTimes == 1)
							printf("%d send success\n", i + 1);
					}
					else{
						if (DetectionTimes == 1)
							printf("%d send fail\n", i + 1);
					}

					if (i % 2 == 0)
						t = i + 1;
					else
						t = i - 1;

					usleep(50000);

					memset(readbuf, '\0', sizeof(readbuf));
					fd_set rfds;
					struct timeval tv;

					FD_ZERO(&rfds);
					FD_SET(hCom[t], &rfds);
					tv.tv_sec = 0;
					tv.tv_usec = 100000;
					if (select(hCom[t] + 1, &rfds, NULL, NULL, &tv) == 1)
					{
						if (FD_ISSET(hCom[t], &rfds))
						{
							lenread = read(hCom[t], readbuf, sizeof(readbuf));
							if (strcmp(readbuf, writebuf) == 0)
							{
								if (DetectionTimes == 1)
									printf("%d recv success\n", t + 1);
							}
							tcflush(hCom[t], TCIFLUSH);
						}
						else{
							if (DetectionTimes == 1)
								printf("%d recv fail\n", t + 1);
						}
					}
					else{
						if (DetectionTimes == 1)
						printf("%d recv fail\n", t + 1);
					}

				}
			}
			for (i = 0; i < ComNum; i++)//关闭端口
			{
				close(hCom[i]);
			}			
			printf("check over\n");		
			
		}
		else
		{
			printf("Command parameter error\n");
		}

	}
	else
	{
		printf("parameter error\n");
	}		

	return 0;
}
