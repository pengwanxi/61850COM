#pragma once
#include "Dlt645_2007.h"
class Dlt645_DINGXING_2007:public CDlt645_2007
{
public:
	Dlt645_DINGXING_2007();
	~Dlt645_DINGXING_2007();
	virtual BOOL  ProcessYxData(const BYTE *buf, int len);
};

