#pragma once
#include "BaseWebPage.h"
#include <stdio.h>
#include <sstream>
#include <iostream> 
#include <fstream> 
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h> 
#include <unistd.h> 
#include<sys/stat.h>
#include <assert.h>
#include <unistd.h>

#define PROJECT_PATH	"/mynand/config"  //协议工程路径
#define SAVE_JSON_FILE "/mynand/config/info_json" //存储JSON串的文件
#define MAX_LINE	200 //最大通讯行数据
#define	EMU2000_VERSION			0x01040100					/* EMU2000版本号 */
#define CONFIG_ENV_VAR     0x1001
#define CONFIG_PROC_STYLE  0x2001
#define CONFIG_PROC_PARAM  0x2002

using namespace std;

class CProJson :public CBaseWebPage
{
public:
	CProJson();

	~CProJson();

	virtual BOOL getJSONStructFromWebPage(Json::Value &root);

	virtual BOOL procCmd(BYTE byCmd);

	virtual void  Init();

	virtual void setLog(Clog *);

public:
	
	virtual int readFileList(char *basePath);

	virtual int ParseConfigItem(char *strItem, WORD *pwNum);

	 BOOL SaveJsontoFile(string pathname, Json::Value &filedata);//将打包好的JSON串存储到一个文件中去

	virtual BOOL GetfileInformation(string pathname, string filename, Json::Value &filedata);//将文件的内容转换成JSON字符串

	virtual BOOL DirfileInformation(char *path, Json::Value &dirdata);//将目录下的所有文件转变成JSON字符串 包含子目录;

	virtual BOOL GetfilerRdbInformation(string pathname, string filename, Json::Value &filedata);//rtdb.conf转JSON串

	virtual BOOL GetfilerZigBeeInformation(string pathname, string filename, Json::Value &filedata);//Zigbee文件转JSON串

	virtual BOOL GetfilerProcManInformation(string pathname, string filename, Json::Value &filedata);//procman.conf文件转JSON串

	virtual BOOL GetfilerPrBusLineInformation(string pathname, string filename, Json::Value &filedata);//BusLine.ini文件转JSON串

	virtual BOOL GetfilerPrBusInformation(string pathname, string filename, Json::Value &filedata);//bus01.ini类文件转JSON串

	virtual BOOL GetfilerTemplateInformation(string pathname, string filename, Json::Value &filedata);//Template里面的模板文件转JSON串

	virtual BOOL GetfilerStationStnInformation(string pathname, string filename, Json::Value &filedata);//Station里面文件类似stn文件相关文件格式

	virtual BOOL GetfilerRtuInformation(string pathname, string filename, Json::Value &filedata);//rtu01.txt类似文件转发协议

	virtual BOOL GetfilerMqttServerInformation(string pathname, string filename, Json::Value &filedata);//server.cfg文件
	
	virtual BOOL GetfilerMqttCacertInformation(string pathname, string filename, Json::Value &filedata);//cacert.pem文件
	
	//导入工程数据源JSON串---路径（JSON串中包含路径）
public:
	

	virtual BOOL  SaveJSonInformationBus(Json::Value &filedata);//JSON字符串内容转换成bus01.ini类文件

	virtual BOOL  SaveJSonInformationBusLine(Json::Value &filedata);//JSON字符串内容转换成BusLine.ini文件类文件

	virtual BOOL  SaveJSonInformationModuld(Json::Value &filedata);//JSON字符串内容转换成文件类文件转换成模板文件

	virtual BOOL  SaveJSonInformationZigBee(Json::Value &filedata);//JSON字符串内容转换成ZigBee文件

	virtual BOOL  SaveJSonInformationServerCfg(Json::Value &filedata);//JSON字符串内容转换成/mqtt_transmit/server.cfg文件

	virtual BOOL  SaveJSonInformationCacert(Json::Value &filedata);//JSON字符串内容转换成/mqtt_transmit/cacert.pem文件

	virtual BOOL  SaveJSonInformationRtdb(Json::Value &filedata);//JSON字符串内容转换成 rtdb.con文件

	virtual BOOL SaveJSonInformationStation(Json::Value &filedata);//JSON字符串内容转换成 /Station/stn01.conf类文件

	virtual BOOL SaveJSonInformationRtu(Json::Value &filedata);//JSON字符串内容转换成 转发协议中中的rtu02.txt类文件

	virtual BOOL SaveJSonInformationProcman(Json::Value &filedata);//JSON字符串内容转换成 procman.conf

public:
	virtual BOOL SaveMouldJSonInformationToSystem(Json::Value &filedata);//JSON 字符串中的模板 和模板库对比

	virtual BOOL FigureJsonInformationAmount(string pathname,int *yc_mount,int *yx_mount,int *ym_mount,int *yk_mount);

	virtual BOOL ComapareFileAndJson(string pathname, Json::Value &filedata);//比较模板文件中的每一行 和JSON字符串中的内容是否一样

public:
	int  substrpoint(char str[],int point);

	void ModifyLineData(char* fileName, string str, char* lineData);//修改文件中某行的内容

	size_t Get_file_size(const char *filepath);//获取文件大小

	string get_string(string res);

	string CharToStr(char * contentChar);


public:
	vector <string> pathfile;

	vector <string>filename;



	



};

