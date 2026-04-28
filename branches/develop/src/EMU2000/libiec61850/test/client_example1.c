/*
 * client_example1.c
 *
 * This example is intended to be used with server_example3 or server_example_goose.
 */

#include <stdlib.h>
#include <stdio.h>

#include "iec61850_client.h"
#include "hal_thread.h"

#include "mms_value.h"
#include "mms_value_internal.h"

void report_resolv(MmsValue *MmsValue)
{/*{{{*/
	unsigned char buf[4096] = {0};
	char char_buf[4096] = {0};
	uint64_t time_of_ms = 0;
	switch(MmsValue->type){
	case MMS_BOOLEAN:
		printf("boolean:%d\n", MmsValue->value.boolean);
		break;
	case MMS_BIT_STRING:
		memcpy(buf, MmsValue->value.bitString.buf, (MmsValue->value.bitString.size + 7) / 8);
		printf("bitString:%s\n", buf);
		break;
	case MMS_INTEGER:				//是个结构体，integer是个结构体!
		/*printf("%d\n", *(MmsValue->value->integer));*/
		/*printf("%d\n", MmsValue_toInt32(MmsValue));			//前者or后者?*/
		printf("integer:%d\n", MmsValue_toInt64(MmsValue));
		break;
	case MMS_UNSIGNED:
		printf("unsigned:%u\n", MmsValue_toUint32(MmsValue));
		//找不到似乎!
		break;
	case MMS_FLOAT:
		printf("float:%f\n", MmsValue_toFloat(MmsValue));
		break;
	case MMS_OCTET_STRING:
		memcpy(buf, MmsValue->value.bitString.buf, MmsValue->value.bitString.size);
		break;
	case MMS_VISIBLE_STRING:
		/*char buf[MmsValue->value.visibleString.size + 1] = {0};*/
		/*memcpy(buf, MmsValue->value.visibleString.buf, sizeof(buf) - 1);*/
		memcpy(char_buf, MmsValue_toString(MmsValue), MmsValue->value.visibleString.size);
		printf("visibleString:%s\n", char_buf);
		break;
	case MMS_GENERALIZED_TIME:
		//不知何往!
		break;
	case MMS_BINARY_TIME:
		time_of_ms = MmsValue_getBinaryTimeAsUtcMs(MmsValue);
		printf("binarytime:%lu\n", time_of_ms);
		break;
	case MMS_UTC_TIME:
		time_of_ms = MmsValue_getUtcTimeInMs(MmsValue);
		printf("utctime:%lu\n", time_of_ms);
		break;
		//
	case MMS_STRING:
		break;
	default:
		if(MmsValue->type == MMS_STRUCTURE)
			for(unsigned char i = 0; i < MmsValue->value.structure.size; i++)
				report_resolv(MmsValue->value.structure.components[i]);
		else if(MmsValue->type == MMS_ARRAY)
			/*for(unsigned char i = 0; i < MmsValue->value.structure.size; i++)*/	//这种方式会取得数据集所有数据!
				/*report_resolv(MmsValue->value.structure.components[i]);				//i如何确定?*/
			report_resolv(MmsValue->value.structure.components[0]);
		break;
	}
	return;
}/*}}}*/

/*
 * callback function when a report is received.
 * 只能得到数据集! 每一个报告要注册一次!
 * **report->dataSetValues.value.structure.components 显示 dataAccessError为DATA_ACCESS_ERROR_OBJECT_ACCESS_DENIED!
 */
	void
reportCallbackFunction(void* parameter, ClientReport report)
{/*{{{*/
	MmsValue* dataSetValues = ClientReport_getDataSetValues(report);

	printf("received report for %s\n", ClientReport_getRcbReference(report));
	/*printf("dataSetName:%s\n", report->dataSetName);*/
	report_resolv(dataSetValues);			//+ by cyz!

	int i;
	for (i = 0; i < 4; i++) {							//i的数目即数据集之子项数目!现在是14!
		ReasonForInclusion reason = ClientReport_getReasonForInclusion(report, i);

		if (reason != IEC61850_REASON_NOT_INCLUDED) {
			printf("  GGIO1.SPCSO%i.stVal: %i (included for reason %i)\n", i,
					MmsValue_getBoolean(MmsValue_getElement(dataSetValues, i)), reason);		//reason:5 总召者也!
			printf("-------- time:%lu --------\n", MmsValue_getUtcTimeInMs((*(*dataSetValues->value.structure.components)->value.structure.components)->value.structure.components[4]));	//得到时间!
			printf("-------- float:%f --------\n", MmsValue_toFloat(*(*(*(*dataSetValues->value.structure.components)->value.structure.components[0]->value.structure.components)->value.structure.components)->value.structure.components));
			/*
			 * MmsValue_getElement
			 */
		}
	}
}/*}}}*/

int main(int argc, char** argv)
{/*{{{*/

	char* hostname;
	int tcpPort = 102;

	if (argc > 1)
		hostname = argv[1];
	else
		/*hostname = "localhost";*/
		hostname = "192.168.2.10";

	if (argc > 2)
		tcpPort = atoi(argv[2]);

	IedClientError error;

	IedConnection con = IedConnection_create();				//创建连接!

	IedConnection_connect(con, &error, hostname, tcpPort);	//建立连接!	这里有对recv的调用! 读取的内容存在:con->connection->isoClient->CotpConnection->readBuffer->buffer中!
	//连接状态:con->connection->connectionState
	if (error == IED_ERROR_OK) {			//正常连接!

		IedConnection_getServerDirectory(con, &error, false);

		/* read an analog measurement value from server */	//没有看到recv的调用，如何获取?
		/*MmsValue* value = IedConnection_readObject(con, &error, "simpleIOGenericIO/GGIO1.AnIn1.mag.f", IEC61850_FC_MX);	//con->connection->lastResponse!*/
		/*MmsValue* value = IedConnection_readObject(con, &error, "SampleIEDDevice1/DGEN1.MX.TotWh.mag.f", IEC61850_FC_MX);	//con->connection->lastResponse!*/
		/*MmsValue* value = IedConnection_readObject(con, &error, "BAY1LD0/CMMXU1.A.phsB.cVal.mag.f", IEC61850_FC_MX);	//其名曰：读对象，然而读的却是一个普通数字，“对象”者，何谓也?	王曰：对象既可以是具体的数字，也可以是结构体，甚至是数组,就看BAY1LD0/后面路径的等级!即如果phsA后面省略则value的值就是一个phsA结构体!			//正常读取!*/
		/*MmsValue* value = IedConnection_readObject(con, &error, "BAY1LD0/CMMXU1.A.phsB", IEC61850_FC_MX);	//其名曰：读对象，然而读的却是一个普通数字，“对象”者，何谓也?	王曰：对象既可以是具体的数字，也可以是结构体，甚至是数组,就看BAY1LD0/后面路径的等级!即如果phsA后面省略则value的值就是一个phsA结构体!*/
		MmsValue* value = IedConnection_readObject(con, &error, "BAY1LD0/CMMXU1", IEC61850_FC_NONE);	//其名曰：读对象，然而读的却是一个普通数字，“对象”者，何谓也?	王曰：对象既可以是具体的数字，也可以是结构体，甚至是数组,就看BAY1LD0/后面路径的等级!即如果phsA后面省略则value的值就是一个phsA结构体!
		/*MmsValue* value = IedConnection_readObject(con, &error, "SampleIEDDevice1/MMXU2.TotW.mag.f", IEC61850_FC_MX);	//con->connection->lastResponse!*/

		if (value != NULL) {
			/*float fval = MmsValue_toFloat(value);*/
			/*uint32_t fval = MmsValue_toUnixTimestamp((MmsValue *)&(*(MmsValue **)(value->value.structure.components)[4]));*/
			/*uint32_t fval = MmsValue_toUnixTimestamp((MmsValue *)(*(MmsValue **)(value->value.structure.components) + 4));*/		//不对!
			/*printf("read unsigned int value: %ld\n", fval);*/
			MmsValue_delete(value);
		}

		/* write a variable to the server */	//每次都被成功拒绝!
		/*value = MmsValue_newVisibleString("libiec61850.com");		//将para写入value.visibleString.buf中并将para长度存入value.visibleString.size*/
		value = MmsValue_newFloat(50);		//将para写入value.visibleString.buf中并将para长度存入value.visibleString.size
		/*value = MmsValue_newInteger(10);			//+ by cyz!*/
		/*IedConnection_writeObject(con, &error, "simpleIOGenericIO/GGIO1.NamPlt.vendor", IEC61850_FC_DC, value);*/
		/*IedConnection_writeObject(con, &error, "SampleIEDDevice1/DGEN1.DC.NamPlt.vendor", IEC61850_FC_DC, value);			//what are you doing?*/
		//一些FC是禁止写的，除非再服务端运行IedServer_setWriteAccessPolicy(IedServer,
		//IEC61850_FC_DC, ACCESS_POLICY_ALLOW);
		IedConnection_writeObject(con, &error, "BAY1LD0/PHIPTOC1.StrVal.setMag.f", IEC61850_FC_SE, value);			//what are you doing?每次都被成功的拒绝，我到底写什么?

		if (error != IED_ERROR_OK){
			printf("failed to write Object error:%d\n", error);
			return -1;
		}

		MmsValue_delete(value);

		//start
		/*LinkedList new_dataset_entries = LinkedList_create();*/
		/*LinkedList_add(new_dataset_entries, "BAY1LD0/LLN0.A[MX]");*/
		/*IedConnection_createDataSet(con, &error, "BAY1LD0/LLN0.A.Wahaha", new_dataset_entries);*/
		/*if(error != IED_ERROR_OK)*/
		/*printf("line:%d error:%d\n", __LINE__, error);*/
		/*IedConnection_deleteDataSet(con, &error, "BAY1LD0/LLN0.MeasFlt");*/
		//end

		error = IED_ERROR_OK;			//+ by cyz!
		/* read data set */
		ClientDataSet clientDataSet = IedConnection_readDataSetValues(con, &error, "BAY1LD0/LLN0.MeasFlt", NULL);

		if (clientDataSet == NULL)
			printf("failed to read dataset\n");

		/* Read RCB values */
		//rcb的13(一般)个成员都是用MmsValue结构体指针来描述的，每一个成员都是一个属性!
		ClientReportControlBlock rcb =
			IedConnection_getRCBValues(con, &error, "BAY1LD0/LLN0.BR.rcbMeasFlt01", NULL);	//LLN0:LN name. rcbMeasFlt01:RCB name.
		/*IedConnection_getRCBValues(con, &error, "BAY1LD0/LLN0.BR.rcbStatIO01", NULL);	//LLN0:LN name. rcbMeasFlt01:RCB name.*/


		bool rptEna = ClientReportControlBlock_getRptEna(rcb);

		printf("RptEna = %i\n", rptEna);

		/* Install handler for reports */
		IedConnection_installReportHandler(con, "BAY1LD0/LLN0.BR.rcbMeasFlt01",						//para1无论是什么都不影响报告之访问，那么其意义何在!
				/*IedConnection_installReportHandler(con, "BAY1LD0/LLN0.BR.rcbStatIO01",						//para1无论是什么都不影响报告之访问，那么其意义何在!*/
				ClientReportControlBlock_getRptId(rcb), reportCallbackFunction, NULL);				//函数指针的使用是实现回调的基本方式，也可以实现多态特性!con->enabledReports->data->callback == reportCallbackFunction!
				/*IedConnection_installReportHandler(con, "BAY1LD0/CMMXU1.MX.A",*/
				/*ClientReportControlBlock_getRptId(rcb), reportCallbackFunction, NULL);				//函数指针的使用是实现回调的基本方式，也可以实现多态特性!con->enabledReports->data->callback == reportCallbackFunction!*/

				/* Set trigger options and enable report */
				ClientReportControlBlock_setTrgOps(rcb, TRG_OPT_DATA_UPDATE | TRG_OPT_INTEGRITY | TRG_OPT_GI);
				ClientReportControlBlock_setRptEna(rcb, true);
				/*ClientReportControlBlock_setIntgPd(rcb, 5000);*/
				ClientReportControlBlock_setIntgPd(rcb, 4096);
				/*IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_RPT_ENA | RCB_ELEMENT_TRG_OPS | RCB_ELEMENT_INTG_PD, true);*/	//why error?
				IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_RPT_ENA | RCB_ELEMENT_TRG_OPS | RCB_ELEMENT_INTG_PD, true);			//订阅了报告，然后呢?

				if (error != IED_ERROR_OK)
				printf("report activation failed (code: %i)\n", error);

				Thread_sleep(1000);

				/* trigger GI report */
				ClientReportControlBlock_setGI(rcb, true);
				IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_GI, true);

				if (error != IED_ERROR_OK)
					printf("Error triggering a GI report (code: %i)\n", error);

				Thread_sleep(60000);

				//start
				while(1);
		//end
		/* disable reporting */
		ClientReportControlBlock_setRptEna(rcb, false);
		IedConnection_setRCBValues(con, &error, rcb, RCB_ELEMENT_RPT_ENA, true);

		if (error != IED_ERROR_OK)
			printf("disable reporting failed (code: %i)\n", error);

		ClientDataSet_destroy(clientDataSet);				//为什么NULL!

		ClientReportControlBlock_destroy(rcb);

close_connection:

		IedConnection_close(con);
	}
	else {
		printf("Failed to connect to %s:%i\n", hostname, tcpPort);
	}

	IedConnection_destroy(con);
}/*}}}*/
