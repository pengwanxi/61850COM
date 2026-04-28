#pragma once
#include "Cjt188_2004.h"

class CCjt188_2004_TianHe :
	public CCjt188_2004
{
public:
	CCjt188_2004_TianHe();
	~CCjt188_2004_TianHe();
	virtual BOOL ProcessDataT1(const BYTE *buf, int len);

};

