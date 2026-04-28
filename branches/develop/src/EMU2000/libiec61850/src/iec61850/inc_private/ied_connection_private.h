/*
 *  ied_connection_private.h
 *
 *  Copyright 2013 Michael Zillgith
 *
 *  This file is part of libIEC61850.
 *
 *  libIEC61850 is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libIEC61850 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with libIEC61850.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  See COPYING file for the complete license text.
 */

#ifndef IED_CONNECTION_PRIVATE_H_
#define IED_CONNECTION_PRIVATE_H_

#ifndef DEBUG_IED_CLIENT
#define DEBUG_IED_CLIENT 0
#endif

#include "hal_thread.h"

struct sIedConnection
{/*{{{*/
	MmsConnection connection;
	IedConnectionState state;
	LinkedList enabledReports;		//Report:一组大家认可的、或由客户端规定的由智能电子设备IED编排好的数据集.该数据集由智能电子设备IED定时即指定时间间隔或应要求传给客户端，报告也可由预先设定或由客户端预先规定的触发条件产生!
	LinkedList logicalDevices;
	LinkedList clientControls;
	LastApplError lastApplError;

	Semaphore stateMutex;
	Semaphore reportHandlerMutex;

	IedConnectionClosedHandler connectionCloseHandler;				//IedConnectionClosedHandler:typedef void (*IedConnectionClosedHandler)(void *, IedConnection);
	void* connectionClosedParameter;
	uint32_t connectionTimeout;							//单位是ms
};/*}}}*/

/*
 * description:报告控制块用于设定IED上数据的内容及方式等，包括告警、事件、开关、模拟量等所有IED需上送的内容，61850中除总召由client发起外，其余全部为IED主动上送数据!
 */
struct sClientReportControlBlock {/*{{{*/				//报告控制块的概念?		曰:report标示数据报文，控制块表征对report的描述、控制!
	char* objectReference;
	bool isBuffered;						//报告分为缓存和非缓存两种类型，缓存:fc=BR, 非缓存:fc=RP.缓存型report要求IED在内存中缓存，若通信中断期间发生了事件，当通信恢复后此事件报文能上送不丢失，通常告警、事件、SOE等report建模为缓存类型，非缓存者相同遭遇数据会丢失!

	MmsValue* rptId;						//report id缩写!作为不同报告间的唯一标识符
	MmsValue* rptEna;						//report enable!报告控制块使能，client使能报告后，IED就开始根据报告触发条件上送报文了.
	MmsValue* resv;
	MmsValue* datSet;						//本报告控制块对应的数据集!
	MmsValue* confRev;						//配置版本号
	MmsValue* optFlds;						//报告之选项域，决定了报文拼装中可选成员出现与否,即决定该结构体中某些字段是否存在!
	MmsValue* bufTm;						//数据集内发生第一个事件后等待的时间!
	MmsValue* sqNum;
	MmsValue* trgOps;						//trigger options!触发选项! 数据集中的数据在何种条件下通过报告上送! 一把包含四种方式,曰:数据变化上送，品质变化上送，数据更新上送，周期上送!
	MmsValue* intgPd;						//周期性上传时间!
	MmsValue* gi;							//总召!
	MmsValue* purgeBuf;
	MmsValue* entryId;						//入口标示，当OptFlds.entry为true时存在!
	MmsValue* timeOfEntry;					//入口时间，什么功能?
	MmsValue* resvTms;
	MmsValue* owner;
};/*}}}*/

IedClientError
private_IedConnection_mapMmsErrorToIedError(MmsError mmsError);

bool
private_IedConnection_doesControlObjectMatch(const char* objRef, const char* cntrlObj);

void
private_IedConnection_addControlClient(IedConnection self, ControlObjectClient control);

void
private_IedConnection_removeControlClient(IedConnection self, ControlObjectClient control);

bool
private_ClientReportControlBlock_updateValues(ClientReportControlBlock self, MmsValue* values);

void
private_IedConnection_handleReport(IedConnection self, MmsValue* value);

IedClientError
iedConnection_mapMmsErrorToIedError(MmsError mmsError);

IedClientError
iedConnection_mapDataAccessErrorToIedError(MmsDataAccessError mmsError);

ClientReport
ClientReport_create(void);

void
ClientReport_destroy(ClientReport self);

void
private_ControlObjectClient_invokeCommandTerminationHandler(ControlObjectClient self);

/* some declarations that are shared with server side ! */

char*
MmsMapping_getMmsDomainFromObjectReference(const char* objectReference, char* buffer);

char*
MmsMapping_createMmsVariableNameFromObjectReference(const char* objectReference, FunctionalConstraint fc, char* buffer);


char*
MmsMapping_varAccessSpecToObjectReference(MmsVariableAccessSpecification* varAccessSpec);

MmsVariableAccessSpecification*
MmsMapping_ObjectReferenceToVariableAccessSpec(char* objectReference);

#endif /* IED_CONNECTION_PRIVATE_H_ */
