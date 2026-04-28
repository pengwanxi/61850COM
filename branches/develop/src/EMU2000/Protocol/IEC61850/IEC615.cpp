// IEC615.cpp: implementation of the IEC615 class.
//
//////////////////////////////////////////////////////////////////////
#include "IEC615.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

extern "C" void GetCurrentTime(REALTIME *pRealTime);

using std::cerr;
string template_path = "/mynand/config/IEC61850/template/";  /* 61850前缀路径 */

extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
extern "C" void  OutMessageText(char *szSrc, unsigned char *pData, int nLen);
//extern "C" void report_resolv(MmsValue *, WORD, BYTE, IEC615 *);						//+2 by cyz!
//extern "C" void reportCallbackFunction(void *, ClientReport);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IEC615::IEC615()
{/*{{{*/

	m_pConnect = IedConnection_create();
	//host_name = (char *)"192.168.2.10";
	tcp_port = 102;
	//error = IED_ERROR_OK;
	vec_conf.clear();

	count_flag = 0;
	time_ym = 0;
	time_yk_sel = 0;
	yk_sel_flag = false;
	m_pCtlVal = NULL;
	connection_flag = 0;

	memset(dz_write_array, 0, sizeof(dz_write_array));
	dz_write_num = 0;
	pthread_mutex_init(&mutex, NULL);
	m_logFile = NULL;
	m_countComm = 0;
	m_bstateOldComm = COM_ABNORMAL ;
	m_bConnectIed = true ;
	m_bReadYm = false ;
}/*}}}*/

IEC615::~IEC615()
{/*{{{*/
	printf( "Delete IEC615 bus = %d , Addr = %d \n" , m_byLineNo , m_wDevAddr );

	//IedClientError error = IED_ERROR_OK;
	//IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_RPT_ENA | RCB_ELEMENT_INTG_PD, FALSE );

	//ClientReportControlBlock_setRptEna(rcb, false);
	//ClientReportControlBlock_setRptEna(rcb_yx, false);
	//ClientReportControlBlock_destroy(rcb);
	//pthread_mutex_destroy(&mutex);
	//IedConnection_uninstallReportHandler(m_pConnect)
	IedConnection_destroy(m_pConnect);
	m_pConnect =nullptr ;
}/*}}}*/

/*
 * description:	判断域掩码对应的位是否有效，有效(1)则转发，无效(0)则不转发
 * parameter:	para0:域掩码位序 para1:配置行
 * return:		true for 转发给后台, false for 不转发给后台!
 */
bool IEC615::mask_resolv(WORD mask_order, BYTE line)
{/*{{{*/
	return vec_conf[line].field_mask[mask_order] == '1' ? true : false;
}/*}}}*/

/*
 * description:	解析数据上传后台
 * parameter:
 * para0:IED上传的值，para1:域掩码位序，para2:上传给后台之序号，para3:配置行号，para4:IEC615之this指针!
 * return:		void
 */
void report_resolv(MmsValue *mmsvalue, WORD &mask_order, WORD &order, BYTE line, IEC615 *pthis)		//解析数据上传后台!
{/*{{{*/
	if(mmsvalue == NULL){
		cerr<<"mmsvalue is a null!";
		return;
	}
	/*
	 * yx:boolean,integer,bitString,utctime;
	   虽然遥信对应的逻辑节点下还包含其他数据以及其他(相对于boolean)数据类型，
	   但对我们来说有用的只有一个数据，而且该数据的类型为boolean!
	 * yx-ym:float,integer,bitString,utctime;同上，但是有效的数据也只有一个，该数据类型为float!
	 */

	//printf("line = %d configline = %d mmType = %d \n", __LINE__,line , mmsvalue->type );

	switch(mmsvalue->type){
	case MMS_BOOLEAN:			//2
		{/*{{{*/
			if(pthis->mask_resolv(mask_order, line)){
				//printf("boolean:%d order:%d mask_order = %d \n", mmsvalue->value.boolean, pthis->vec_conf[line].start_pos + order,mask_order );
				pthis->m_pMethod->SetYxData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, mmsvalue->value.boolean);
			}
			++mask_order;
			break;
		}/*}}}*/
	case MMS_BIT_STRING:		//3				//yx报告也有此类型数据!
		{/*{{{*/
			unsigned char buf[(mmsvalue->value.bitString.size + 15) / 8];		//但愿不要太大!
			memcpy(buf, mmsvalue->value.bitString.buf, (mmsvalue->value.bitString.size + 7) / 8);
#if 0
			if(pthis->vec_conf[line].type == 1)//yx
				pthis->m_pMethod->SetYxData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order++, BYTE(atoi(mmsvalue->value.bitString.buf)));
			if(pthis->vec_conf[line].type == 2)//yc
				pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order++, float(atoi(buf)));
			else if(pthis->vec_conf[line].type == 3)//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order++, float(atoi(buf)));
			else
				cerr<<"bu zhi qi ke ye!"<<endl;
#endif
			++mask_order;
			//printf("bitString:%s mask:%d mask_order:%d\n", buf, pthis->mask_resolv(mask_order - 1, line), mask_order - 1);
			break;
		}/*}}}*/
	case MMS_INTEGER:			//4
		{/*{{{*/
		if ((pthis->vec_conf[line].type == 1) && pthis->mask_resolv(mask_order, line))
		{
			pthis->m_pMethod->SetYxData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, MmsValue_toInt64(mmsvalue) == 0 ? 0 : 1);
		}
			if ((pthis->vec_conf[line].type == 2) && pthis->mask_resolv(mask_order, line))//yc
			{//pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, float(MmsValue_toInt64(mmsvalue)));	//到底是to多少，之后再做调整!
				order = pthis->vec_conf[line].start_pos + order;
				float fVal = float(MmsValue_toInt64(mmsvalue));
				pthis->m_pMethod->SetYcData(pthis->m_SerialNo, order, fVal );	//到底是to多少，之后再做调整!
				order++;
				//printf("line  =%d yc serialno = %d pos=%d fVal=%f\n", __LINE__, pthis->m_SerialNo, order, fVal);
			}
			else if((pthis->vec_conf[line].type == 3) && pthis->mask_resolv(mask_order, line))//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, (QWORD)(MmsValue_toInt64(mmsvalue)));
			++mask_order;
		//	printf("integer:%lld mask:%d mask_order:%d order:%d\n", MmsValue_toInt64(mmsvalue), pthis->mask_resolv(mask_order - 1, line), mask_order - 1, pthis->vec_conf[line].start_pos + order - 1);
			break;

		}/*}}}*/
	case MMS_UNSIGNED:			//5
		{/*{{{*/
		if ((pthis->vec_conf[line].type == 1) && pthis->mask_resolv(mask_order, line))
		{
			pthis->m_pMethod->SetYxData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, MmsValue_toUint32(mmsvalue) == 0 ? 0 : 1);
		}
			else if ((pthis->vec_conf[line].type == 2) && pthis->mask_resolv(mask_order, line))//yc
			{
			//	pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, float(MmsValue_toUint32(mmsvalue)));
				{
					order = pthis->vec_conf[line].start_pos + order;
					float fVal = float(MmsValue_toUint32(mmsvalue));
					pthis->m_pMethod->SetYcData(pthis->m_SerialNo, order, fVal);	//到底是to多少，之后再做调整!
					order++;
					printf("line  =%d yc serialno = %d pos=%d fVal=%f\n", __LINE__, pthis->m_SerialNo, order, fVal);
				}
			}
			else if((pthis->vec_conf[line].type == 3) && pthis->mask_resolv(mask_order, line))//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, (QWORD)(MmsValue_toUint32(mmsvalue)));
			else
				cerr<<"bu zhi qi ke ye1!"<<endl;
			++mask_order;

	//		printf("unsigned:%u order:%d\n", MmsValue_toUint32(mmsvalue), pthis->vec_conf[line].start_pos + order - 1);
			break;
		}/*}}}*/
	case MMS_FLOAT:				//6
		{/*{{{*/
		if ((pthis->vec_conf[line].type == 2) && pthis->mask_resolv(mask_order, line))//yc
		{
			//pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, MmsValue_toFloat(mmsvalue));
				order = pthis->vec_conf[line].start_pos + order;
				float fVal = MmsValue_toFloat(mmsvalue);
				pthis->m_pMethod->SetYcData(pthis->m_SerialNo, order, fVal);	//到底是to多少，之后再做调整!
				order++;
		}
			else if((pthis->vec_conf[line].type == 3) && pthis->mask_resolv(mask_order, line))//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, (MmsValue_toDouble(mmsvalue)));
			//else
			//cerr<<"bu zhi qi ke ye2!"<<endl;
			++mask_order;
	//		printf("float:%f mask:%d mask_order:%d order:%d\n", MmsValue_toFloat(mmsvalue), pthis->mask_resolv(mask_order - 1, line), mask_order - 1, pthis->vec_conf[line].start_pos + order - 1);
			break;
		}/*}}}*/
	case MMS_OCTET_STRING:		//7
		{/*{{{*/
			//char char_buf[mmsvalue->value.visibleString.size + 1];
			//memcpy(char_buf, MmsValue_toString(mmsvalue), mmsvalue->value.visibleString.size);
			++mask_order;		
			//printf("line = %d \n", __LINE__);
//			printf("octet:%s\n", char_buf);
			break;
		}/*}}}*/
	case MMS_VISIBLE_STRING:	//8
		{/*{{{*/
			char char_buf[mmsvalue->value.visibleString.size + 1];
			memcpy(char_buf, MmsValue_toString(mmsvalue), mmsvalue->value.visibleString.size);
			++mask_order;
			
//			printf("visibleString:%s\n", char_buf);
			break;
		}/*}}}*/
	case MMS_GENERALIZED_TIME:	//9
		{/*{{{*/
//			cout<<"generalized time:"<<endl;
			++mask_order;
			
			break;
		}/*}}}*/
	case MMS_BINARY_TIME:		//10
#if 0								//时间不需要，暂做注释!
		{/*{{{*/
			uint64_t time_of_ms = MmsValue_getBinaryTimeAsUtcMs(mmsvalue);
			if(pthis->vec_conf[line].type == 2)//yc
				pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order, float(time_of_ms));
			else if(pthis->vec_conf[line].type == 3)//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order, float(time_of_ms));
			else
				cerr<<"bu zhi qi ke ye3!"<<endl;
			mask_order++;
			//cout<<"binary:"<<time_of_ms<<endl;
//			printf("binary:%lld\n", time_of_ms);
			break;
		}/*}}}*/
#endif
	case MMS_BCD:				//11
		{/*{{{*/
//			cout<<"BCD:"<<endl;
			++mask_order;
			
			break;
		}/*}}}*/
	case MMS_OBJ_ID:			//12
		{/*{{{*/
//			cout<<"OBJID:"<<endl;
			++mask_order;
		
			break;
		}/*}}}*/
	case MMS_STRING:			//13	和MMS_OCTET_STRING什么区别?
		{/*{{{*/
//			cout<<"string:"<<endl;
			++mask_order;
			
			break;
		}/*}}}*/
	case MMS_UTC_TIME:			//14
#if 0						//时间不需要，暂做注释!
		{/*{{{*/
			uint64_t time_of_ms = MmsValue_getUtcTimeInMs(mmsvalue);
			if(pthis->vec_conf[line].type == 2)//yc
				pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order, float(time_of_ms));
			else if(pthis->vec_conf[line].type == 3)//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + mask_order, float(time_of_ms));
			else
				cerr<<"bu zhi qi ke ye4!"<<endl;
			mask_order++;
			//cout<<"utctime:"<<time_of_ms<<endl;
	//		printf("utctime:%lld\n", time_of_ms);
			break;
		}/*}}}*/
#endif
		{/*{{{*/
			uint64_t time_of_ms = MmsValue_getUtcTimeInMs(mmsvalue);
			if ((pthis->vec_conf[line].type == 2) && pthis->mask_resolv(mask_order, line))//yc
			{
				//pthis->m_pMethod->SetYcData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, (float)(time_of_ms / 1000));

				{
					order = pthis->vec_conf[line].start_pos + order;
					float fVal = (float)(time_of_ms / 1000);
					pthis->m_pMethod->SetYcData(pthis->m_SerialNo, order, fVal);
					order++;
					printf("line  =%d yc serialno = %d pos=%d fVal=%f\n", __LINE__ , pthis->m_SerialNo, order, fVal);
				}
			}
			else if((pthis->vec_conf[line].type == 3) && pthis->mask_resolv(mask_order, line))//ym
				pthis->m_pMethod->SetYmData(pthis->m_SerialNo, pthis->vec_conf[line].start_pos + order++, (QWORD)(time_of_ms / 1000));
			++mask_order;
		//	printf("utctime:%lld mask:%d mask_order:%d\n", time_of_ms, pthis->mask_resolv(mask_order - 1, line), mask_order - 1);
			break;
		}/*}}}*/
	default:
		{/*{{{*/
			if(mmsvalue->type == MMS_STRUCTURE)				//1
				for(unsigned char i = 0; i < mmsvalue->value.structure.size; i++)
				{
      //              cout << mmsvalue->value.structure.size << endl ;
					if(mmsvalue->value.structure.components[i] == NULL)
						return;
					report_resolv(mmsvalue->value.structure.components[i], mask_order, order, line, pthis);
				}
			else if(mmsvalue->type == MMS_ARRAY){			//0
				//int size = mmsvalue->value.structure.size;
				//cout << size << flush << endl ;
				for(unsigned char i = 0; i < mmsvalue->value.structure.size; i++){

					if (mmsvalue->value.structure.components[i] == NULL)
					{
						return;
					}
					
				//	printf("line = %d i = %d datamask = %llu line = %d \n", __LINE__, i, pthis->vec_conf[line].dataset_mask >> i,line);
					if((pthis->vec_conf[line].dataset_mask >> i) & 0x01)
					{
						report_resolv(mmsvalue->value.structure.components[i], mask_order, order, line, pthis);
					}
				}
				//printf("ARRAY had called, line:%d\n", __LINE__);
			}
			break;
		}/*}}}*/
	}
	return;
}/*}}}*/


 /*	获取本地时间	*/
struct tm *IEC615::GetTime()
{/*{{{*/
	time_t timep;
	timep = time(NULL);
	return localtime(&timep);
}/*}}}*/

/*
 * description:	回调函数，每次有变化数据都会被唤醒!
 * parameter:	para0:IEC615之this指针! para1:报告!
 * return:		void
 */
void reportCallbackFunction(void *parameter, ClientReport report)			//回调函数，每次有变化数据都会被唤醒!
{/*{{{*/
	IEC615 *pthis = (IEC615 *)parameter;
	MmsValue *dataset_values = ClientReport_getDataSetValues(report);
	//cout<<"received report for "<<ClientReport_getRcbReference(report)<<endl;

	if(pthis->report_line.find(ClientReport_getRcbReference(report)) == pthis->report_line.end()){
		// cerr<<"This report is not exist!"<<endl;
		return;
	}
	WORD mask_order = 0;		//掩码序号，是和filed_mask对应的概念!
	WORD order = 0;				//和后台对应的序号!
	pthread_mutex_lock(&pthis->mutex);
	if(dataset_values == NULL){
		pthread_mutex_unlock(&pthis->mutex);
		return;
	}

	int no = pthis->report_line[ClientReport_getRcbReference(report)];
	report_resolv(dataset_values, mask_order, order, pthis->report_line[ClientReport_getRcbReference(report)], pthis);
	pthread_mutex_unlock(&pthis->mutex);
	//cout<<endl<<endl<<endl<<endl;
}/*}}}*/

/*
 * description:	遥控预置
 * parameters:	para0: para1: para2:分合标志
 * return:		true for succeed,false for failed!
 */
BOOL IEC615::yk_preset(YK_DATA *yk_data, PBUSMSG pBusMsg, BYTE flag)
{/*{{{*/
	
		if (flag == 1){
			if (ControlObjectClient_selectWithValue(m_pControl, m_pCtlVal)){
				printf("\n\nControlObjectClient_selectWithValue successfully!\n\n");
				m_pMethod->SetYkSelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
				yk_sel_flag = true;
				time(&time_yk_sel);
			}
			else{
				printf("\n\nControlObjectClient_selectWithValue failed!\n\n");
				return false;
			}
		}
		else{
			if (ControlObjectClient_selectWithValue(m_pControl, m_pCtlVal)){
				printf("\n\nControlObjectClient_selectWithValue successfully!\n\n");
				m_pMethod->SetYkSelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
				yk_sel_flag = true;
				time(&time_yk_sel);
			}
			else{
				printf("\n\nControlObjectClient_selectWithValue failed!\n\n");
				return false;
			}
		}
	
	return true;
}/*}}}*/

BOOL IEC615::yk_execute(YK_DATA *yk_data, PBUSMSG pBusMsg, BYTE flag)
{/*{{{*/
	if(flag == 1){
		yk_sel_flag = false;
		if (ControlObjectClient_operate(m_pControl, m_pCtlVal, 0 /* operate now */)){
			printf("yk close operate successfully\n");
			m_pMethod->SetYkExeRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
		}else{
			printf("failed to operate yk close\n");
			MmsValue_delete(m_pCtlVal);
			ControlObjectClient_destroy(m_pControl);
			m_pControl = NULL;
			m_pCtlVal = NULL;
			return false;
		}
		MmsValue_delete(m_pCtlVal);
		ControlObjectClient_destroy(m_pControl);
		m_pControl = NULL;
		m_pCtlVal = NULL;
	}else{
		yk_sel_flag = false;
		if (ControlObjectClient_operate(m_pControl, m_pCtlVal, 0 /* operate now */)){
			printf("yk open successfully\n");
			m_pMethod->SetYkExeRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
		}else{
			printf("failed to operate yk open\n");
			MmsValue_delete(m_pCtlVal);
			ControlObjectClient_destroy(m_pControl);
			m_pControl = NULL;
			m_pCtlVal = NULL;
			return false;
		}
		MmsValue_delete(m_pCtlVal);
		ControlObjectClient_destroy(m_pControl);
		m_pControl = NULL;
		m_pCtlVal = NULL;
	}
	return true;
}/*}}}*/

BOOL IEC615::yk_cancel(YK_DATA *yk_data, PBUSMSG pBusMsg, BYTE flag)
{/*{{{*/
	if(flag == 1){
		yk_sel_flag = false;
		if(ControlObjectClient_cancel(m_pControl)){
			printf("yk close cancel successfully!\n");
			m_pMethod->SetYkCancelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
		}else{
			perror("Failed to yk close cancel!");
			MmsValue_delete(m_pCtlVal);
			ControlObjectClient_destroy(m_pControl);
			m_pControl = NULL;
			m_pCtlVal = NULL;
			return false;
		}
		MmsValue_delete(m_pCtlVal);
		ControlObjectClient_destroy(m_pControl);
		m_pControl = NULL;
		m_pCtlVal = NULL;
	}else{
		yk_sel_flag = false;
		if(ControlObjectClient_cancel(m_pControl)){
			printf("yk open cancel successfully!\n");
			m_pMethod->SetYkCancelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
		}else{
			perror("Failed to yk open cancel!");
			MmsValue_delete(m_pCtlVal);
			ControlObjectClient_destroy(m_pControl);
			m_pControl = NULL;
			m_pCtlVal = NULL;
			return false;
		}
		MmsValue_delete(m_pCtlVal);
		ControlObjectClient_destroy(m_pControl);
		m_pControl = NULL;
		m_pCtlVal = NULL;
	}
	return true;
}/*}}}*/

/*
 * description:	遥控处理:预置，执行，取消皆汇于此
 * parameters:	para0:	para1:遥控相关所有信息!
 * return:		true for successfully,false for failed
 */
BOOL IEC615::yk_deal(YK_DATA *yk_data, PBUSMSG pBusMsg)
{/*{{{*/
	
	if(!yk_sel_flag){

		//control是个指针，用完需要使用ControlObjectClient_destroy()释放!
		if (m_pControl)
		{
			ControlObjectClient_destroy(m_pControl);
			m_pControl = nullptr;
		}

		if (m_pCtlVal)
		{
			MmsValue_delete(m_pCtlVal);
			m_pCtlVal = nullptr;
		}

       // cout << vec_yk[yk_data->wPnt].report_path.c_str() << endl ;
		m_pControl = ControlObjectClient_create(vec_yk[yk_data->wPnt].report_path.c_str(), m_pConnect);
		if (!m_pControl)
		{
			cerr << "ControlObjectClient_create failed maybe yk configuration is wrong"<< endl ;
			return FALSE;
		}

		m_pCtlVal = (yk_data->byVal == 1) ? MmsValue_newBoolean(true) : MmsValue_newBoolean(false);
		if(m_pCtlVal == NULL){
			perror("Failed to MmsValue_newBoolean!\n");
			return false;
		}
		ControlObjectClient_setOrigin(m_pControl, NULL, 3);
	}
	printf("*********%d %s********group=%d  **wpnt=%d**\n", __LINE__, __FILE__, vec_yk[yk_data->wPnt].group, yk_data->wPnt);
	if (vec_yk[yk_data->wPnt].group == 1)
	{
		if (pBusMsg->dwDataType == YK_SEL)//遥控选择预置
		{
			printf("*****yuzhi****%d %s************\n", __LINE__, __FILE__);
			m_pMethod->SetYkSelRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, yk_data->wPnt, yk_data->byVal);
			yk_sel_flag = true;
			time(&time_yk_sel);
			return true;
		}
		if (pBusMsg->dwDataType == YK_EXCT)//遥控执行
		{
			printf("******exec***%d %s************\n", __LINE__, __FILE__);
			yk_sel_flag = false;
			return yk_execute(yk_data, pBusMsg, yk_data->byVal);
		}
	}
	else
	{
		printf("*********%d %s************\n", __LINE__, __FILE__);
		if (pBusMsg->dwDataType == YK_SEL)
			return yk_preset(yk_data, pBusMsg, yk_data->byVal);
		else if (pBusMsg->dwDataType == YK_EXCT)
			return yk_execute(yk_data, pBusMsg, yk_data->byVal);
		else if (pBusMsg->dwDataType == YK_CANCEL)
			return yk_cancel(yk_data, pBusMsg, yk_data->byVal);
		else
			return false;
	}

	

}/*}}}*/

#if 1
/*
 * description:将定值插入结构，以备定值写!
 * parameters:	para0:后台传入的定值指针 para1:对应para1值序号 para2:待写数据!
 * return:		false for failed, true for success!
 */
BOOL IEC615::insert_dz(DZ_DATA *dz_data, WORD order, MmsValue *stVal)
{/*{{{*/
//	printf("----FUNC = %s LINE = %d type = %d----\n", __func__, __LINE__, dz_data[order].byType);

	switch(dz_data[order].byType){
		case 0:				//BYTE	暂不予支持，实际上用不到!
			break;
		case 1:				//WORD	暂不予支持，实际上用不到!
			break;
		case 2:				//DWORD
			{
				unsigned int temp_value = 0;
				memcpy(&temp_value, dz_data[order].byVal, 4);
			//	printf("----FUNC = %s LINE = %d temp_value = %d type = %d----\n", __func__, __LINE__, temp_value, stVal->type);
				if(stVal->type == MMS_UNSIGNED)
					MmsValue_setUint32(stVal, temp_value);
				else if(stVal->type == MMS_INTEGER)
					MmsValue_setInt32(stVal, temp_value);
				break;
			}
		case 3:				//float
			{
				float temp_value = 0;
			//	printf("%X\n", dz_data[order].byVal[0]);
			//	printf("%X\n", dz_data[order].byVal[1]);
			//	printf("%X\n", dz_data[order].byVal[2]);
			//	printf("%X\n", dz_data[order].byVal[3]);
				memcpy(&temp_value, dz_data[order].byVal, 4);
			//	printf("----FUNC = %s LINE = %d temp_value = %f type = %d----\n", __func__, __LINE__, temp_value, stVal->type);
				MmsValue_setFloat(stVal, temp_value);
				break;
			}
		default:
			cerr<<"type is error line:%d"<<__LINE__<<endl;
			return false;
	}
	return true;
}/*}}}*/

/*
 * description:定值写!
 * parameters:
 */
BOOL IEC615::dz_write(DZ_DATA *dz_data, PBUSMSG pBusMsg, WORD group_no)
{/*{{{*/
	//cout<<"--------line:"<<__LINE__<<" func:"<<__func__<<"--------"<<endl;
	//DZ_DATA *p = (DZ_DATA *)pBusMsg->pData;
	//++p;			//现在指向数据部分了!
	if(pBusMsg->DataNum != 1)
	{
		memset(dz_write_array, 0, sizeof(dz_write_array));
		dz_write_num = pBusMsg->DataNum;
		memcpy(dz_write_array, dz_data + 1, sizeof(DZ_DATA) * (dz_write_num - 1));
	}
	WORD dz_num = group_to_line.count(group_no) > ((DWORD)pBusMsg->DataNum - 1) ? (pBusMsg->DataNum - 1) : group_to_line.count(group_no);				//这里不考虑实际需要写入的定值，只写入配置了的点!
//	printf("----FUNC = %s LINE = %d dz_num = %d group_no_num = %d DataNum = %d group_no = %d----\n", __func__, __LINE__, dz_num, group_to_line.count(group_no), pBusMsg->DataNum - 1, group_no);
	multimap<BYTE, BYTE>::iterator iter = group_to_line.find(group_no);
	IedClientError error = IED_ERROR_OK;
	for(BYTE i = 0; i < dz_num; ++i){/*{{{*/
		string temp_str = vec_conf[iter->second].report_path.substr(0, vec_conf[iter->second].report_path.find('/'));
		MmsValue *stVal = IedConnection_readObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.ActSG").c_str(), IEC61850_FC_SP);	//读组号!
	//	printf("----FUNC = %s LINE = %d error1 = %d----\n", __func__, __LINE__, error);
		MmsValue_setUint8(stVal, group_no);			//设置组号!
		IedConnection_writeObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.ActSG").c_str(), IEC61850_FC_SP, stVal);				//激活组号!
	//	printf("----FUNC = %s LINE = %d error2 = %d----\n", __func__, __LINE__, error);
		IedConnection_writeObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.EditSG").c_str(), IEC61850_FC_SP, stVal);				//进入指定组号!

	//	printf("----FUNC = %s LINE = %d error3 = %d----\n", __func__, __LINE__, error);

		stVal = IedConnection_readObject(m_pConnect, &error, vec_conf[iter->second].report_path.c_str(), IEC61850_FC_SG);
	//	printf("----FUNC = %s LINE = %d error4 = %d----\n", __func__, __LINE__, error);
		if(error != IED_ERROR_OK){/*{{{*/
			cerr<<"Failed to IedConnection_readObject errno is: "<<error<<endl;
			memset(dz_write_array, 0, sizeof(dz_write_array));
			dz_write_num = 0;
			return false;
		}/*}}}*/
		if(stVal == NULL){/*{{{*/
			cerr<<"stVal is NULL line: "<<__LINE__<<endl;
			memset(dz_write_array, 0, sizeof(dz_write_array));
			dz_write_num = 0;
			return false;
		}/*}}}*/
		/*
		 * 读后台转发之定值，然后设置stVal
		 */
		insert_dz(dz_write_array, i, stVal);
		IedConnection_writeObject(m_pConnect, &error, vec_conf[iter->second].report_path.c_str(), IEC61850_FC_SE, stVal);
		if(error != IED_ERROR_OK){/*{{{*/
			cerr<<"Failed to IedConnection_writeObject errno is: "<<error<<endl;
			memset(dz_write_array, 0, sizeof(dz_write_array));
			dz_write_num = 0;
			return false;
		}/*}}}*/

		stVal = IedConnection_readObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.CnfEdit").c_str(), IEC61850_FC_SP);
	//	printf("----FUNC = %s LINE = %d error6 = %d----\n", __func__, __LINE__, error);
		MmsValue_setBoolean(stVal, true);
		IedConnection_writeObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.CnfEdit").c_str(), IEC61850_FC_SP, stVal);
	//	printf("----FUNC = %s LINE = %d error7 = %d----\n", __func__, __LINE__, error);

		++iter;
	}/*}}}*/
	m_pMethod->SetDzWriteExctRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, group_no, dz_data, dz_write_num - 1);
	/*写完后将预置保存的数据清空*/
	memset(dz_write_array, 0, sizeof(dz_write_array));
	dz_write_num = 0;
	return true;
}/*}}}*/
#endif

/*
 * description:定值预置
 * parameters:	para0:定值参数与数据 para1:定值相关所有 para2:定值组号!
 * return:		true for succeed!
 */
BOOL IEC615::dz_preset(DZ_DATA *dz_data, PBUSMSG pBusMsg, WORD group_no)
{/*{{{*/
	//cout<<"********line:"<<__LINE__<<" func:"<<__func__<<"********"<<endl;
	//MmsValue stVal;
	//insert_dz(dz_data + 1, 1, &stVal);	//测试给我的数据是否能解析!
	dz_write_num = pBusMsg->DataNum;
	memcpy(dz_write_array, dz_data + 1, sizeof(DZ_DATA) * dz_write_num);
	m_pMethod->SetDzWritePresetRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, group_no, dz_data, dz_write_num);
	return true;
}/*}}}*/

/*
 * description:定值读!
 * parameters:
 * 				para0:
 * 				para1:类型，是读还是写!
 */
BOOL IEC615::dz_read(DZ_DATA *dz_data, PBUSMSG pBusMsg, WORD group_no)
{/*{{{*/
	BYTE dz_num = group_to_line.count(group_no) > ((DWORD)pBusMsg->DataNum - 1)? (pBusMsg->DataNum - 1): group_to_line.count(group_no);		//获得组号关联的value数目!
//	printf("----FUNC = %s LINE = %d DZ_READ dz_num = %d group_no_num = %d DataNum = %d group_no = %d----\n", __func__, __LINE__, dz_num, group_to_line.count(group_no), pBusMsg->DataNum - 1, group_no);
	DZ_DATA dz_data_array[dz_num];						//定值结构体数组，将读取的定值写入以传入后台!
	memset(dz_data_array, 0, sizeof(DZ_DATA) * dz_num);
	//multimap<BYTE, BYTE>::iterator iter = group_to_line.begin();
	multimap<BYTE, BYTE>::iterator iter = group_to_line.find(group_no);
	IedClientError error = IED_ERROR_OK;
	for(BYTE i = 0; i < dz_num; ++i){		//遍历组号对应的所有定值![>{{{<]
		string temp_str = vec_conf[iter->second].report_path.substr(0, vec_conf[iter->second].report_path.find('/'));
		MmsValue *stVal = IedConnection_readObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.ActSG").c_str(), IEC61850_FC_SP);	//读组号!
		MmsValue_setUint8(stVal, group_no);			//设置组号!
		IedConnection_writeObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.ActSG").c_str(), IEC61850_FC_SP, stVal);				//激活组号!
		IedConnection_writeObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.EditSG").c_str(), IEC61850_FC_SP, stVal);				//进入指定组号!
		stVal = IedConnection_readObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.CnfEdit").c_str(), IEC61850_FC_SP);
		MmsValue_setBoolean(stVal, true);
		IedConnection_writeObject(m_pConnect, &error, (temp_str + "/LLN0.SGCB.CnfEdit").c_str(), IEC61850_FC_SP, stVal);			//确定组号
		stVal = IedConnection_readObject(m_pConnect, &error, vec_conf[iter->second].report_path.c_str(), IEC61850_FC_SG);		//一次读取一个是最简单的方式!
		dz_data_array[i].wPnt = vec_conf[iter->second].start_pos;					//What are you doing?
	//	printf("----FUNC = %s LINE = %d type = %d----\n", __func__, __LINE__, stVal->type);
		if((stVal->type == MMS_UNSIGNED) || (stVal->type == MMS_INTEGER)){
			dz_data_array[i].byType = 2;
			unsigned int value = 0;
			value = MmsValue_toUint32(stVal);
			memcpy(&dz_data_array[i].byVal, &value, 4);
		}
		else if(stVal->type == MMS_FLOAT){
			dz_data_array[i].byType = 3;
			float value = 0;
			value = MmsValue_toFloat(stVal);
			memcpy(dz_data_array[i].byVal, &value, 4);
		}
		MmsValue_delete(stVal);
		/*将数据组织起来写入dz_data_array!*/
		++iter;
	}/*}}}*/
	m_pMethod->SetDzCallRtn(this, pBusMsg->SrcInfo.byBusNo, pBusMsg->SrcInfo.wDevNo, group_no, dz_data_array, dz_num);	//第四个参数应该有问题!
	return TRUE;
}/*}}}*/

/*
 * description:	定值处理
 * parameter:	para0:定值数据、说明. para1:定值所有信息!
 * para0元素类型是DZ_DATA, 第一个元素是说明/标志，之后的才是正典的数据!
 * return:		true for succeed.false for failed!
 */
BOOL IEC615::dz_deal(DZ_DATA *dz_data, PBUSMSG pBusMsg)
{/*{{{*/
	WORD group_no = dz_data[0].byVal[0];
	if(pBusMsg->dwDataType == DZ_CALL)
		return dz_read(dz_data, pBusMsg, group_no);
	else if(pBusMsg->dwDataType == DZ_WRITE_PRESET)
		return dz_preset(dz_data, pBusMsg, group_no);
	else if(pBusMsg->dwDataType == DZ_WRITE_EXCT)
		return dz_write(dz_data, pBusMsg, group_no);
	else
		return false;
}/*}}}*/

BOOL IEC615::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg)
{/*{{{*/
	if (m_bConnectIed == true) {
		iec615_inner_connection() ;
		m_bConnectIed = false;
	}

	if ( m_bReadYm == true) {
		ReadYm();
		m_bReadYm = false ;
	}

	if(pBusMsg != NULL){

		if (GetDevCommState() == COM_ABNORMAL)
		{
			printf("file=%s---FUNC = %s LINE = %d abnormal----\n", __FILE__ , __FUNCTION__, __LINE__ );
			return FALSE;
		}

		printf("file=%s---FUNC = %s LINE = %d normal----\n", __FILE__, __FUNCTION__, __LINE__);

		if(pBusMsg->byMsgType == YK_PROTO){
			YK_DATA *yk_data = (YK_DATA *)pBusMsg->pData;
			//yktype = pBusMsg->dwDataType;
			//byBusNo = pBusMsg->SrcInfo.byBusNo;
			//wDevNo = pBusMsg->SrcInfo.wDevNo;
			//wPnt = yk_data->wPnt;
			//byVal = yk_data->byVal;

			pthread_mutex_lock(&mutex);
			if(yk_deal(yk_data, pBusMsg)){
				pthread_mutex_unlock(&mutex);
				return TRUE;
			}else{
				pthread_mutex_unlock(&mutex);
				return FALSE;
			}
		}else if(pBusMsg->byMsgType == DZ_PROTO){
			DZ_DATA *dz_data = (DZ_DATA *)pBusMsg->pData;
			if(dz_deal(dz_data, pBusMsg))
				return TRUE;
			else
				return FALSE;
		}
	}
	return TRUE ;
}/*}}}*/

BOOL IEC615::ProcessProtocolBuf( BYTE * buf , int len )
{/*{{{*/
	return !GetDevCommState();
}/*}}}*/

/*
 * description:	关联回调
 * parameter:	void
 * return:		true for normal, false for error!
 */
BOOL IEC615::register_report()
{/*{{{*/
	int size = vec_conf.size();
	int i = 0;
	while(i < size){
		if((vec_conf[i].type != 2) && (vec_conf[i].type != 1)){
			++i;
			continue;
		}

		IedClientError error = IED_ERROR_OK;
		for (int tt = 0; tt < 10; tt++) {
			m_pRcb = IedConnection_getRCBValues(m_pConnect, &error, vec_conf[i].report_path.c_str(), NULL);
			if (error == IED_ERROR_OK) {
				break;
			}
			cerr << tt+1 << " failed to IedConnection_getRCBValues! (code: " << error << " host:" << host_name << " path:" << vec_conf[i].report_path.c_str() << ")" << endl;
			usleep(100 * 1000);
		}
		
		if(error != IED_ERROR_OK){
			cerr << "failed to IedConnection_getRCBValues! (code: " << error <<" host:"<< host_name << " path:"<< vec_conf[i].report_path.c_str() <<")" << endl;
			//perror("failed to IedConnection_getRCBValues!");
			return FALSE;
		}
		IedConnection_installReportHandler(m_pConnect, vec_conf[i].report_path.c_str(), ClientReportControlBlock_getRptId(m_pRcb), reportCallbackFunction, this);		//最后一个参数是默认为NULL,这里传递this指针看看能否正常使用!this指针没有问题，但是因为回调中已经使用了这里的最后参数，如果这里还传递NULL则程序无法启动!

#if 1
		ClientReportControlBlock_setTrgOps(m_pRcb, TRG_OPT_DATA_UPDATE | TRG_OPT_INTEGRITY | TRG_OPT_GI);
		ClientReportControlBlock_setRptEna(m_pRcb, true);
		ClientReportControlBlock_setIntgPd(m_pRcb, 5000);
		IedConnection_setRCBValues(m_pConnect, &error, m_pRcb, RCB_ELEMENT_RPT_ENA | RCB_ELEMENT_INTG_PD, true);
		if(error != IED_ERROR_OK){
	//		printf("report_path:%s error:%d\n", vec_conf[i].report_path.c_str(), error);
	//		perror("failed to IedConnection_setRCBValues!");
			return false;
		}
		//Thread_sleep(1000);
		ClientReportControlBlock_setGI(m_pRcb, true);
		IedConnection_setRCBValues(m_pConnect, &error, m_pRcb, RCB_ELEMENT_GI, true);
		if(error != IED_ERROR_OK)
			cerr<<"Error triggering a GI report (code: "<<error<<")"<<endl;
		++i;
#endif
	}
	cerr << "-- host:" << host_name  << "register report sucess!"<< endl;
	return true;
}/*}}}*/

/*
 * description:
 * 建立tcp连接，该连接区别于配置软件中指定的连接,是第三方库对应的连接!
 * parameter:	void
 * return:		FALSE for failed, true for succeed!
 */
BOOL IEC615::iec615_inner_connection()
{/*{{{*/

	REALTIME CurrTime;
	GetCurrentTime(&CurrTime);
	char buf[255] = { 0 };
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d: %s\n", CurrTime.wYear,
		CurrTime.wMonth, CurrTime.wDay, CurrTime.wHour, CurrTime.wMinute,
		CurrTime.wSecond, "dev re-connected");
	//writeLog(buf, strlen(buf));

	IedClientError error = IED_ERROR_OK;
	IedConnection_connect(m_pConnect, &error, host_name, tcp_port);
	if(error != IED_ERROR_OK){
		cerr<<"Failed to IedConnection_connect! errno:"<<error<<" host:"<< host_name << " port:"<< tcp_port  <<endl;

		pthread_mutex_lock(&mutex);
		IedConnection_close(m_pConnect);
		IedConnection_destroy(m_pConnect);
		m_pConnect = IedConnection_create();
		pthread_mutex_unlock(&mutex);

		return FALSE;
	}
	return register_report();
}/*}}}*/

BOOL IEC615::Init( BYTE byLineNo )
{/*{{{*/
	
	
	string _path = template_path + m_sTemplatePath;
//	cout<<_path<<endl;

	if((read_conf((template_path + m_sTemplatePath).c_str()) == -1) || (vec_conf.size() == 0))
		default_config();

#if 0
	IedClientError error = IED_ERROR_OK;
	IedConnection_connect(con, &error, host_name, tcp_port);
	if(error != IED_ERROR_OK){
		perror("failed to IedConnection_connect!");
		return FALSE;
	}
	return register_report();
#endif
	return true;
}/*}}}*/

FunctionalConstraint IEC615::get_fc(string str)
{/*{{{*/
	if(!str.compare("ST"))
		return IEC61850_FC_ST;
	else if(!str.compare("MX"))
		return IEC61850_FC_MX;
	else if(!str.compare("SP"))
		return IEC61850_FC_SP;
	else if(!str.compare("SV"))
		return IEC61850_FC_SV;
	else if(!str.compare("CF"))
		return IEC61850_FC_CF;
	else if(!str.compare("DC"))
		return IEC61850_FC_DC;
	else if(!str.compare("SG"))
		return IEC61850_FC_SG;
	else if(!str.compare("SE"))
		return IEC61850_FC_SE;
	else if(!str.compare("SR"))
		return IEC61850_FC_SR;
	else if(!str.compare("OR"))
		return IEC61850_FC_OR;
	else if(!str.compare("BL"))
		return IEC61850_FC_BL;
	else if(!str.compare("EX"))
		return IEC61850_FC_EX;
	else if(!str.compare("CO"))
		return IEC61850_FC_CO;
	else if(!str.compare("US"))
		return IEC61850_FC_US;
	else if(!str.compare("MS"))
		return IEC61850_FC_MS;
	else if(!str.compare("RP"))
		return IEC61850_FC_RP;
	else if(!str.compare("LG"))
		return IEC61850_FC_LG;
	else if(!str.compare("ALL"))
		return IEC61850_FC_ALL;
	else
		return IEC61850_FC_NONE;
}/*}}}*/

//子站状态
void IEC615::TimerProc()
{/*{{{*/
	time_t time_now;
	time(&time_now);

	//if(IedConnection_getState(con) == IED_STATE_CLOSED){		//判断连接是否已连接，没有连接则重新连接!	为什么要添加一个connection? 曰:怕创建完IEC615之后Init还没有完成就执行TimerProc，导致初始化冲突!
	
	pthread_mutex_lock(&mutex);
	IedConnectionState state = IedConnection_getState( m_pConnect );
	pthread_mutex_unlock(&mutex);
	
	if( state != IED_STATE_CONNECTED){		//判断连接是否已连接，没有连接则重新连接!	为什么要添加一个connection? 曰:怕创建完IEC615之后Init还没有完成就执行TimerProc，导致初始化冲突![>{{{<]
		if(++connection_flag >= 50){		//10s连接一次!
			count_flag = 0;
			time_ym = 0;
			yk_sel_flag = false;
			m_pCtlVal = NULL;
			m_bConnectIed = true;

			//iec615_inner_connection();

			connection_flag = 0;
		}
	}else/*}}}*/
		connection_flag = 0;
	time(&time_yk_sel);
	if(time_now - time_yk_sel >= 20)
		yk_sel_flag = false;
	if((time_now - time_ym) < 300)
		return;
	else{			//遥脉五分钟采集一次!
		m_bReadYm = true ;
		time(&time_ym);
	}
}/*}}}*/
void IEC615::ReadYm() {

	MmsValue *value = NULL;
	IedClientError error = IED_ERROR_OK;

	if (IedConnection_getState(m_pConnect) != IED_STATE_CONNECTED) {
		time(&time_ym);
		return;
	}
	for (vector<config>::iterator beg = vec_conf_ym.begin(), en = vec_conf_ym.end(); beg != en; beg++) {	//report_path:null
		value = IedConnection_readObject(m_pConnect, &error, beg->report_path.c_str(), get_fc(beg->fc_eg));
		if (value == NULL) {
			cerr << "IedConnection_readObject Failed! error is: " << error << endl;
			return;
		}
		WORD mask_order = 0;
		WORD order = 0;
		pthread_mutex_lock(&mutex);
		report_resolv(value, mask_order, order, report_line[beg->report_path], this);
		pthread_mutex_unlock(&mutex);

		MmsValue_delete(value);
	}
}
void IEC615::writeLog(char * pContent, int len)
{
	int ret = access("/mynand/log", F_OK);
	if (ret == -1)
	{
		mkdir("/mynand/log", 0755);
	}

	char fileName[255] = { 0 };
	sprintf(fileName, "/mynand/log/%d_%d.txt", m_byLineNo, m_wDevAddr);
	m_logFile = fopen(fileName, "a+");

	fseek(m_logFile, 0, SEEK_END);
	fwrite(pContent, len, 1, m_logFile);

	fclose(m_logFile);
}


//读取配置文件!
int IEC615::read_conf(const char *filename)
{/*{{{*/
	FILE *hFile;
	//char szText[128];
	char szText[1024];
	char *temp;
	BYTE i = 0;					//表征字段!
	BYTE j = 0;					//表征配置行!
#if 1
	//config config_obj;					//这一句使得启动时提示gather: symbol lookup error: ./libIEC615.so: undefined symbol: _ZN4configC1Ev.		why?	莫非是没有实现构造函数config()?

	hFile = fopen(filename, "r");

	if(hFile == NULL)
	{/*{{{*/
		perror("Failed to fopen!");
		return -1;
	}/*}}}*/

	while( fgets(szText, sizeof(szText), hFile) != NULL ){
		rtrim(szText);
		if( szText[0]=='#' || szText[0]==';' )
			continue;
		i = 0;
		config config_obj;
		//memset(&config_obj,0,sizeof(config_obj));		//memset处理类时可能存在问题!

		temp = strtok(szText,",");
		if(temp == NULL)
			continue;
		if( ( strtol(temp, NULL, 16) > 0 ) && ( strtol(temp, NULL, 16) < 10 ) )
			config_obj.type = strtol(temp, NULL, 16);
		while( ( temp = strtok(NULL, ",") ) ){
			switch(++i){/*{{{*/
			case 1 - 1:
				//类型!
				config_obj.type = strtol(temp, NULL, 16) <= 5  && strtol(temp, NULL, 16) >= 0 ? strtol(temp, NULL, 16) : 0;
				break;
#if 1
			case 2 - 1:
				//采集数量!
				config_obj.count = (strtol(temp, NULL, 16) > 0) && (strtol(temp, NULL, 16) <= 255) ? strtol(temp, NULL, 16) : 0;
				break;
#endif
			case 3 - 1:
				//start_pos:
				config_obj.start_pos = strtol(temp, NULL, 16) <= 255 ? strtol(temp, NULL, 16) : 0xFFFF;
				break;
			case 3:
				//组号!定值专供!
				config_obj.group = strtol(temp, NULL, 16) <= 255 ? strtol(temp, NULL, 16) : 0xFFFF;
				break;
			case 4:										//0-0x7fffffffffffffff
				//mask:
				config_obj.mask = (unsigned long long)strtoll(temp, NULL, 16);
				break;
			case 4 + 1:
				//数据集掩码!
				config_obj.dataset_mask = (unsigned long long)strtoll(temp, NULL, 16);
				break;
			case 5 + 1:
				//域掩码!
				config_obj.field_mask = temp;
				break;
			case 7:
				//功能约束
				config_obj.fc_eg = temp;
				break;
			case 6 + 2:
				//report_path:
				config_obj.report_path = temp;
				report_line[temp] = j++;
				break;
			default:
//				cout<<"config error!"<<endl;
				return -1;
			}/*}}}*/
		}
		if(config_obj.type == 3)		//ym
			vec_conf_ym.push_back(config_obj);

		if(config_obj.type == 4)		//yk
			vec_yk.push_back(config_obj);

		if(config_obj.type == 5)		//dz
			group_to_line.insert(make_pair(config_obj.group, j - 1));

		vec_conf.push_back(config_obj);
	}
	int freturn = fclose(hFile);//perror("fclose");
	if( freturn ){
		perror("fclose failed!");
		return -1;
	}
#endif
	return 0;
}/*}}}*/

void IEC615::default_config()
{/*{{{*/
	cerr<<"######## read default config!"<<endl;
	vec_conf.push_back(config(0, 0, 0, 0, 0xFFFF, 0xFFFF, 0, ""));
}/*}}}*/

BOOL IEC615::GetDevCommState( )
{
	if (!m_pConnect)
		return COM_ABNORMAL ;

	BOOL bstate = COM_ABNORMAL;

	if( m_countComm % 5 == 0 ){
		pthread_mutex_lock(&mutex);
		bstate = (IedConnection_getState(m_pConnect) != IED_STATE_CONNECTED) ? COM_ABNORMAL : COM_NORMAL;
		pthread_mutex_unlock(&mutex);
		m_bstateOldComm = bstate ;
	}
	else {
		bstate = m_bstateOldComm ;
	}
	m_countComm ++ ;
	
	/*BOOL bstate = COM_ABNORMAL;
	for (int i = 0; i < 3; i++)
	{
		pthread_mutex_lock(&mutex);
		bstate = (IedConnection_getState(m_pConnect) != IED_STATE_CONNECTED) ? COM_ABNORMAL : COM_NORMAL;
		pthread_mutex_unlock(&mutex);

		if (bstate == COM_NORMAL)
			break;

		usleep(10 * 1000);
	}*/
	return bstate ;
}
