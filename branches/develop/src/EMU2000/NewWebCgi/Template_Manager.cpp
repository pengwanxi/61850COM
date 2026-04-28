#include "Template_Manager.h"
#include "../share/profile.h"
#include "unistd.h"

CTemplate_Manager::CTemplate_Manager()
{
	m_strRoot = "/mnt/config_system/";
}


CTemplate_Manager::~CTemplate_Manager()
{
}

BOOL CTemplate_Manager::getJSONStructFromWebPage(Json::Value &root)
{
	Json::Value protocol_list;
	Json::Value module_list;
	Json::Value fileContent;
	string strfileName;
	getProtocol_list(protocol_list);
	root["protocol_list"] = protocol_list;
	
	getModule_list(module_list);
	root["module_list"] = module_list;

	//默认获取列表中第一个元素 作为打开文件
	//strfileName = "/mnt/config_system/System/" + module_list[0];
	getModule_file_content(strfileName ,fileContent);
	root["fileContent_list"] = fileContent;

	return TRUE;
}

void CTemplate_Manager::getProtocol_list(Json::Value &protoList, WORD wCurItem )
{
	Json::Value pList;
	string strProtoiniPath;
	strProtoiniPath = m_strRoot + "System/protocol.ini";
	CProfile file( ( char *)strProtoiniPath.data() );

	
	string strRes = file.GetProfileSection("protocol", m_vProtocolKeyArray);
	if (strRes != "OK")
		return;

	int size = m_vProtocolKeyArray.size();
	for ( int i = 0 ; i < size ; i++ )
		pList.append(m_vProtocolKeyArray[i]);

	protoList["curItem"] = m_vProtocolKeyArray[wCurItem];
	protoList["plist"] = pList;

}

void CTemplate_Manager::getModule_list(Json::Value &moduleList,WORD wProtocolCurItem, WORD wTemplateCurItem)
{
	int size = m_vProtocolKeyArray.size();
	if (size == 0|| wProtocolCurItem >= size )
		return;

	string strModuleName = m_vProtocolKeyArray[wProtocolCurItem];
	string strModuleiniPath;
	strModuleiniPath = m_strRoot + "System/" + strModuleName + "/manager.ini" ;
	CProfile file((char *)strModuleiniPath.data());
	string strRes = file.GetProfileSection("Module", m_vModuleKeyArray);
	if (strRes != "OK")
		return;

	//协议模板
	Json::Value mList;
	size = m_vModuleKeyArray.size();
	for (int i = 0; i < size; i++)
		mList.append(m_vModuleKeyArray[i]);

	moduleList["curItem"] = m_vModuleKeyArray[wTemplateCurItem];
	moduleList["template_list"] = mList;

	//打开协议模板
	if (size != 0)
	{
		string strTempName = m_strRoot + "System/" + strModuleName + "/" + m_vModuleKeyArray[wTemplateCurItem] + ".txt";
		getModule_file_content(strTempName, moduleList);
	}

//	获取模板数据量
	char sztempDataNum[100] = { 0 };
	file.GetProfileString("Module", (char *)m_vModuleKeyArray[wTemplateCurItem].c_str(), "NULL", sztempDataNum, 100);
	if (strcmp("NULL", sztempDataNum) == 0 )
	{
		printf("get ycNum so on  failed\n");
	}

	char  yc[10], yx[10], ym[10], yk[10], dz[10];
	char * pToken = NULL;
	pToken = strtok(sztempDataNum, ",");
	memcpy(yc, pToken, 10 );
	WORD index = 0;
	while (pToken)
	{
		pToken = strtok(NULL, ",");
		if (index == 0)
			memcpy(yx, pToken, 10);
		else if (index == 1)
			memcpy(ym, pToken, 10);
		else if (index == 2)
			memcpy(yk, pToken, 10);
		else if (index == 3)
			memcpy(dz, pToken, 10);
		
		index++;
	}
	
	Json::Value metricsVal;
	metricsVal["yc"] = yc;
	metricsVal["yx"] = yx;
	metricsVal["ym"] = ym;
	metricsVal["yk"] = yk;
	metricsVal["dz"] = dz;

	moduleList["metrics"] = metricsVal;
}


void CTemplate_Manager::getModule_file_content(string &strfileName ,Json::Value &moduleList)
{

	if (access(strfileName.c_str(), F_OK) != 0)
	{
		printf((strfileName + " doesn't exist!").c_str());
		return;
	}


	char buf[1024 * 110];
	memset(buf, 0, sizeof(buf));


	FILE * fp = NULL;
	fp = fopen(strfileName.c_str(), "r");
	fread(buf, sizeof(buf), 1, fp);
	fclose(fp);

	moduleList["content"] = buf;
}

BOOL CTemplate_Manager::procCmd(BYTE byCmd)
{
	return FALSE;
}

void CTemplate_Manager::Init()
{

}

void CTemplate_Manager::setLog(Clog *)
{
	
}
