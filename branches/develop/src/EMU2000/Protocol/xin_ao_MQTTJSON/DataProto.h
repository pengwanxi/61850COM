#pragma once
#include "../../share/typedef.h"

class CDataProto
{
public:
	CDataProto();
	virtual ~CDataProto();
	virtual bool getSendBuf( char * pDataSend, int &len) { return false; }
	virtual bool processRecvBuf( char * ptopic, char * pPayload ) { return false; }
	virtual bool subscribeMsg() { return false; }
};

