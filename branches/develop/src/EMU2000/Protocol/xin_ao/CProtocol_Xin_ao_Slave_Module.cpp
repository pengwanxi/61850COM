#include "CProtocol_Xin_ao_Slave_Module.h"
#include <time.h>
#include <iostream>
using namespace std ;

extern "C" int SetCurrentTime(REALTIME *pRealTime);

CProtocol_Xin_ao_Slave_Module::CProtocol_Xin_ao_Slave_Module()
{
	m_bySendSeq = ecertificate;
	m_startTime = m_endTime = 0;
}

CProtocol_Xin_ao_Slave_Module::~CProtocol_Xin_ao_Slave_Module()
{
    //dtor
}

BOOL CProtocol_Xin_ao_Slave_Module::GetProtocolBuf(BYTE * buf, int &len, PBUSMSG pBusMsg /*= NULL */)
{
	if( !buf )
		return FALSE;

	BOOL bRes = FALSE;
	switch (m_bySendSeq)
	{
	case ecertificate:
		bRes = GetCertificateBuf(buf, len, pBusMsg);
		m_bConfirm = TRUE;
		m_bySendSeq = estandby;
		break;
	case econfirm:
		bRes = GetConfirmBuf(buf, len, pBusMsg);
		m_bySendSeq = etransmit;
		break;
	case etransmit:
		if (IscanSend())
			bRes = GetTransmitData(buf, len, pBusMsg);
			m_bySendSeq = etransmit;
		break;
	default:
		break;
	}

	if (!bRes)
		return bRes;

	//crc
	buf[ len - 2] = GetCs(buf + 6, len - 8);
	//len
	WORD lenth = len - 8;
	lenth = (lenth << 2) | 0x0003;
	buf[1] = LOBYTE(lenth);
	buf[2] = HIBYTE(lenth);
	buf[3] = LOBYTE(lenth);
	buf[4] = HIBYTE(lenth);

	return bRes;
}


BOOL CProtocol_Xin_ao_Slave_Module::IscanSend()
{
	if (m_startTime == 0)
	{
		time(&m_startTime);
		return TRUE;
	}
	else
	{
		time(&m_endTime);
		if (difftime(m_endTime, m_startTime) > 1 * 60)
		{
			time(&m_startTime);
			return TRUE;
		}
		else
			return FALSE;
	}

}

BOOL CProtocol_Xin_ao_Slave_Module::GetTransmitData(BYTE * pbuf, int &len, PBUSMSG pBusMsg)
{
	if (m_bConfirm == TRUE)
		return FALSE;

	int i = 0;
	pbuf[i++] = 0x68;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0x68;

	pbuf[i++] = 0xC0;
	pbuf[i++] = m_byRegin[2];
	pbuf[i++] = m_byRegin[1];
	pbuf[i++] = m_byRegin[0];

	pbuf[i++] = LOBYTE(m_wDevAddr);
	pbuf[i++] = HIBYTE(m_wDevAddr);
	pbuf[i++] = 0x00;

	//AFN ������
	pbuf[i++] = 0x07;
	pbuf[i++] = 0xF1;

	i = GetDataFromMap(pbuf , i);

	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;

	time_t timeFlag;
	struct tm *localtm;
	time(&timeFlag);
	localtm = localtime(&timeFlag);

	pbuf[i++] = ToBCD( localtm->tm_sec );
	pbuf[i++] = ToBCD( localtm->tm_min );
	pbuf[i++] = ToBCD(localtm->tm_hour);
	pbuf[i++] = ToBCD( localtm->tm_mday );
	pbuf[i++] = 01;

	pbuf[i++] = 0;
	pbuf[i++] = 0x16;

	len = i;

	return TRUE ;
}

int CProtocol_Xin_ao_Slave_Module::GetDataFromMap( BYTE * pbuf , int index )
{
	int size = m_protocol_DT.size();
	if (size == 0)
		return index;

    if( size > 400 )
        size = 400 ;
	static int serialNo = 1;
	for (int i = 1; i <= size; i++ )
	{
		BYTE dtindex = m_protocol_DT[i].byDtIndex;
        BYTE bySerialconf = m_protocol_DT[i].bySerialNo;
		int pnt = m_protocol_DT[i].byPnt;
		int dataType = m_protocol_DT[i].dataType;

		DWORD dwVal = 0;
		float fVal = 0;
		double dVal = 0;
		WORD wYx = 0;

		pbuf[index++] = 0x01;		//�豸����
		pbuf[index++] = bySerialconf + 1;  //�豸���
		pbuf[index++] = dtindex;
		pbuf[index++] = 0x00;

		if (dataType == eyc)
		{
			dwVal = m_pMethod->ReadYcData(bySerialconf, pnt - 1 );
			fVal = CalcAIRipeVal(bySerialconf + 1 , pnt, ( float )dwVal);
			memcpy(pbuf + index, &fVal, sizeof(fVal));
			index += 4;
		}
		else if (dataType == eyx)
			m_pMethod->ReadYxData(bySerialconf, pnt - 1 , &wYx);
		else if (dataType == eym)
		{
			double tempval= 0;
			m_pMethod->ReadYmData(bySerialconf, pnt - 1, &tempval);
			dwVal=(double)tempval;
			dVal = CalcPulseRipeVal(bySerialconf + 1, pnt, dwVal);
			memcpy(pbuf + index, &dVal, sizeof(dVal));
			index += 8;
		}

		serialNo++ ;
	}
	return index;
}

BOOL CProtocol_Xin_ao_Slave_Module::GetConfirmBuf(BYTE * pbuf, int &len, PBUSMSG pBusMsg)
{
	if (m_bConfirm == TRUE)
		return FALSE;

	int i = 0;
	pbuf[i++] = 0x68;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0x68;

	pbuf[i++] = 0x80;
	pbuf[i++] = m_byRegin[2];
	pbuf[i++] = m_byRegin[1];
	pbuf[i++] = m_byRegin[0];

	pbuf[i++] = LOBYTE(m_wDevAddr);
	pbuf[i++] = HIBYTE(m_wDevAddr);
	pbuf[i++] = m_MSA ;

	//AFN ������
	pbuf[i++] = 0;
	pbuf[i++] = 0xE0;

	pbuf[i++] = 0x01;
	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;

	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;

	time_t timeFlag;
	struct tm *localtm;
	time(&timeFlag);
	localtm = localtime(&timeFlag);

	pbuf[i++] = ToBCD( localtm->tm_sec );
	pbuf[i++] = ToBCD( localtm->tm_min ) ;
	pbuf[i++] = ToBCD ( localtm->tm_hour ) ;
	pbuf[i++] = ToBCD(  localtm->tm_mday );
	pbuf[i++] = 01;

	pbuf[i++] = 0;
	pbuf[i++] = 0x16;

	len = i;
	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave_Module::GetCertificateBuf(BYTE * pbuf, int &len, PBUSMSG pBusMsg)
{
	if (m_bConfirm == TRUE)
		return FALSE ;

	int i = 0;
	pbuf[i++] = 0x68;
	pbuf[i++] = 0;
	pbuf[i++] = 0;
	pbuf[i++] = 0 ;
	pbuf[i++] = 0 ;
	pbuf[i++] = 0x68;

	pbuf[i++] = 0x80;
	pbuf[i++] = m_byRegin[2];
	pbuf[i++] = m_byRegin[1];
	pbuf[i++] = m_byRegin[0];

	pbuf[i++] = LOBYTE(m_wDevAddr);
	pbuf[i++] = HIBYTE(m_wDevAddr);
	pbuf[i++] = 0;

	pbuf[i++] = 0x06;
	pbuf[i++] = 0xF0;

	pbuf[i++] = 0x01;
	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;
	pbuf[i++] = 0x00;

	DWORD dwToken = atoi(m_szToken);
	WORD lw = LOWORD(dwToken);
	WORD hw = HIWORD(dwToken);
	pbuf[i++] = LOBYTE(lw);
	pbuf[i++] = HIBYTE(lw);
	pbuf[i++] = LOBYTE(hw);
	pbuf[i++] = HIBYTE(hw);
	for (int m = 0; m < 12; m++)
		pbuf[i++] = 0;

	for (int m = 0; m < 16; m++)
		pbuf[i++] = 0;

	pbuf[i++] = 0;

	time_t timeFlag;
	struct tm *localtm;
	time(&timeFlag);
	localtm = localtime(&timeFlag);

	pbuf[i++] = ToBCD( localtm->tm_sec );
	pbuf[i++] = ToBCD( localtm->tm_min );
	pbuf[i++] = ToBCD( localtm->tm_hour );
	pbuf[i++] = ToBCD( localtm->tm_mday );
	pbuf[i++] = 01;
	pbuf[i++] = 0 ;
	pbuf[i++] = 0x16;

    len = i ;
	return TRUE ;
}

WORD CProtocol_Xin_ao_Slave_Module::GetLength(int len)
{
	if (!len )
		return FALSE;

	BYTE byl = 0 , byh = 0;
	WORD resLen = len - 8;
	resLen = (resLen << 2) | 0x0003;
	byl = LOBYTE(resLen);
	byh = HIBYTE(resLen);

	return MAKEWORD(byl, byh);
}

BOOL CProtocol_Xin_ao_Slave_Module::ProcessProtocolBuf(BYTE * pbuf, int len)
{
	if (!len || !pbuf)
		return FALSE;

	WORD wLen = GetLength(len);
	if (pbuf[0] != 0x68 || pbuf[1] != LOBYTE(wLen) || pbuf[2] != HIBYTE(wLen) ||
		pbuf[3] != LOBYTE(wLen) || pbuf[4] != HIBYTE(wLen) || pbuf[5] != 0x68)
		return FALSE;

	BYTE dir = pbuf[6] >> 7;
	BYTE prm = (pbuf[6] >> 6) & 0x01;
	const BOOL fromMaster = 0;
	if (dir !=  fromMaster )
		return FALSE;

	if (pbuf[7] != m_byRegin[2] || pbuf[8] != m_byRegin[1] || pbuf[9] != m_byRegin[0])
		return FALSE;

	//��Ҫ���ø���  �ն˱�����ַ�ж�
	if (pbuf[10] != LOBYTE(m_wDevAddr) || pbuf[11] != HIBYTE( m_wDevAddr ) )
		return FALSE;

	//�Ƿ�Ϊ����ַ
	BYTE A3 = pbuf[12];
	m_MSA = pbuf[12];

	//���������뷶Χ
	processAFN(pbuf + 13 , len - 15 );

	return FALSE;
}

BOOL CProtocol_Xin_ao_Slave_Module::processAFN(BYTE * pbuf , int len )
{
	if (!pbuf || !len)
		return FALSE;

	BYTE byFunc = pbuf[0];
	BYTE bySeq = pbuf[1];
	BYTE byTpv = bySeq >> 7;
	BYTE byFir = (bySeq >> 6) & 0x01;
	BYTE byFin = (bySeq >> 5) & 0x01;
	BYTE byCon = (bySeq >> 4) & 0x01;
	BYTE bypSeq = bySeq & 0x0F;

	BOOL bres = FALSE;
	switch (byFunc)
	{
	case 6:
	{
		//��֡
		if (byFir == 1 && byFin == 1 && byCon == 0)
			bres = processAFN_06(pbuf, len);

		break;
	}
	case 5:
	{
		if (byFin == 1 && byFir && byCon == 1)
		{
			bres = processAFN_05(pbuf, len);
		}
	}
	default:
		break;
	}

	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave_Module::processAFN_05(BYTE * pbuf, int  len)
{
	BYTE byFunc = pbuf[0];
	if (byFunc != 0x05)
		return FALSE;

	WORD wda = MAKEWORD(pbuf[2], pbuf[3]);
	WORD wdt = MAKEWORD(pbuf[4], pbuf[5]);
	if (wda != 0x01 || wdt != 0)
		return FALSE;

	int index = 6;
	BYTE bySec = GetBCD( pbuf[ index++ ] );
	BYTE bymin = GetBCD( pbuf[ index++ ] );
	BYTE byhour = GetBCD(pbuf[index++] );
	BYTE byday = GetBCD( pbuf[ index++] );

	BYTE bytmp = pbuf[ index++] ;
	BYTE byweek = ( bytmp >> 5 ) & 0x07;
	BYTE byMon = bytmp & 0x1F;
	BYTE byYear = GetBCD( pbuf[ index++] );

	REALTIME  tm;
	tm.wYear = byYear + 2000;
	tm.wMonth = byMon;
	tm.wDay = byday;
	tm.wHour = byhour;
	tm.wMinute = bymin;
	tm.wSecond = bySec;
	tm.wMilliSec = 0;
	SetCurrentTime(&tm);

	m_bySendSeq = econfirm;
	return TRUE;
}

BYTE CProtocol_Xin_ao_Slave_Module::GetBCD(BYTE byData)
{
	return  ( byData / 16 ) * 10  + byData % 16;
}

BYTE CProtocol_Xin_ao_Slave_Module::ToBCD(BYTE bVal)
{
	return (bVal / 10) * 16 + bVal % 10;
}

BOOL CProtocol_Xin_ao_Slave_Module::processAFN_06(BYTE * pbuf, int  len)
{
	BYTE byFunc = pbuf[0];
	if (byFunc != 0x06)
		return FALSE;

	WORD wda = MAKEWORD(pbuf[2], pbuf[3]);
	WORD wdt = MAKEWORD(pbuf[4], pbuf[5]);
	if (wda != 0x02 || wdt != 0)
		return FALSE;

	WORD wtokenlow = MAKEWORD(pbuf[6], pbuf[7]);
	WORD wtokenhi = MAKEWORD(pbuf[8], pbuf[9]);
	DWORD wtoken = MAKELONG(wtokenlow, wtokenhi);
	if (wtoken != (DWORD)atoi(m_szToken))
		return FALSE;

	m_bConfirm = FALSE;
	return TRUE;
}

BOOL CProtocol_Xin_ao_Slave_Module::Init(BYTE byLineNo)
{
	if (!strlen( m_sTemplatePath))
	{
		cout << "templatePath is empty" << "file:"<< __FILE__ <<"line:" << __LINE__ <<endl;
		return FALSE;
	}

	return 	readServerCfg();
}

BOOL CProtocol_Xin_ao_Slave_Module::readServerCfg()
{
	char bufData[100] = { 100 };
	char bufSec[10][20] = { "serverID" , "dev-category" };
	char bufkey[10][20] = { "gatewayID" , "token" , "regionID" , "DA" };
	char * ptemp[10] = { m_szGatewayID , m_szToken , m_szReginID , m_szProtocol_DA };

	const BYTE keyNum = 3;

	for (int m = 0; m < keyNum; m++)
		if (GetProfileData(bufSec[0], bufkey[m], bufData, sizeof(bufData)))
			strcpy(ptemp[ m ], bufData);
		else
			return FALSE;

	if (GetProfileData("dev-category", "DA", bufData, sizeof(bufData)))
		strcpy(ptemp[3], bufData);
	else
		return FALSE;

	int len = strlen( m_szReginID ) / sizeof(m_szReginID[0]);
	int index = 0;
	for (int i = 0; i < len; i += 2)
	{
		char tmp[2]={ 0 } ;
		memcpy(tmp, m_szReginID + i , 2);
		BYTE bVal = atoi(tmp);
		m_byRegin[index++] = ( bVal / 10 )* 16 + bVal % 10;
	}

	return readDtData();
}

BOOL CProtocol_Xin_ao_Slave_Module::readDtData()
{
	char szPath[100] = { "/mynand/config/xinaoSlave/template/" };
	strcat(szPath, m_sTemplatePath);
	CProfile pfile(szPath);
	if (!pfile.IsValid())
		return FALSE;

	char szSec[20] = { "data-category" };
	char szKey[20] = { 0 };
	int index = 0 ;
	char szDefault[ 10 ] = "NULL";
	BOOL bres = FALSE ;
	do
	{
		sprintf(szKey, "%d", ++index );
		char outputBuf[20] = { 0 };
		pfile.GetProfileString(szSec, szKey, szDefault, outputBuf, sizeof(outputBuf) );
		bres = !strcmp(outputBuf, szDefault);
		if ( bres )
		{
			cout << "file:"<< __FILE__ << "line:"<<__LINE__ << "read dt configure" << index - 1 << "Data finish"<< endl;
		}
		else
		{
			XINAO_DT dt{ 0,0,0,0 };
			if (AddDataToMap(outputBuf, &dt))
				m_protocol_DT[index] = dt;
		}

	} while (!bres);

		return TRUE;
}


BOOL CProtocol_Xin_ao_Slave_Module::AddDataToMap(char * pData, XINAO_DT * pdt )
{
	if (!pData)
		return FALSE ;
	char * pstrip = pData;
	char pskip[10] = ",";
	char * token = NULL;
	token = strtok( pstrip, pskip );
	int index = 0;
	while (token)
	{
		if (index == eserialno)
			pdt->bySerialNo = atoi(token);
		else if (index == edt)
			pdt->byDtIndex = atoi(token);
		else if (index == epnt)
			pdt->byPnt = atoi(token);
		else if (index == edata )
			pdt->dataType = atoi(token);

		token = strtok(NULL, pskip); // C4996
		index++ ;
	}

	return TRUE ;
}

BOOL CProtocol_Xin_ao_Slave_Module::GetProfileData( char * pSec , char * pkey , char * poutPut , int outputsize )
{
	if ( !pSec || !pkey || !poutPut )
		return FALSE;
	char szPath[100] = { "/mynand/config/xinaoSlave/template/" };
	strcat(szPath, m_sTemplatePath);
	CProfile pfile(szPath);
	if (!pfile.IsValid())
		return FALSE;

	int len = pfile.GetProfileString(pSec, pkey, "NULL", poutPut, outputsize);
	if (len <= 0)
	{
		cout << __FILE__ << __LINE__ << "profile error" << endl;
		return FALSE;
	}

	return TRUE;
}


BYTE CProtocol_Xin_ao_Slave_Module::GetCs(const BYTE * pBuf, int len)
{
	BYTE cs = 0;
	for (int i = 0; i < len; i++)
		cs += pBuf[i];

	return cs;
}

void CProtocol_Xin_ao_Slave_Module::TimerProc()
{

}
