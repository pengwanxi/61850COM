/*
 * =====================================================================================
 *
 *       Filename:  Lfp_Nsa3000.cpp
 *
 *    Description:  Lfp_Nsa3000 �汾Э��
 *
 *        Version:  1.0
 *        Created:  2017��01��13��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Shen Youzhi 
 *   Organization:  
 *
 *		  history:
 * =====================================================================================
 */
#include <stdio.h>
#include <assert.h>
#include "Lfp_Nsa3000.h"

#define	NSA3000_SYNC_INTERVAL	60*20			/* ��ʱ���  ��λs*/

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CLfp_Nsa3000
 *      Method:  CLfp_Nsa3000
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CLfp_Nsa3000::CLfp_Nsa3000 ()
{
	m_bLinkStatus = FALSE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	m_byCodeIndex = 1;
	m_bNeedConfirm = FALSE;
}  /* -----  end of method CLfp_Nsa3000::CLfp_Nsa3000  (constructor)  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CLfp_Nsa3000
 *      Method:  ~CLfp_Nsa3000
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CLfp_Nsa3000::~CLfp_Nsa3000 ()
{
}  /* -----  end of method CLfp_Nsa3000::~CLfp_Nsa3000  (destructor)  ----- */


BOOL CLfp_Nsa3000::Init( BYTE byLineNo )
{
	//�����ʼ��ģ������
	if(m_wModuleType != NSA3000)
		return FALSE;
	
	return TRUE ;
}

BOOL CLfp_Nsa3000::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg)
{
	if(!m_bNeedConfirm)
	{
		m_byCodeIndex++;
		if(m_byCodeIndex >= sizeof(byValueCode)/sizeof(BYTE))
			m_byCodeIndex = 0;
		len = PackMsg(buf, byValueCode[m_byCodeIndex]);
	}
	else
	{
		m_bNeedConfirm = FALSE;
		len = PackMsg(buf, RCS_ACK);
	}
	m_bySendCount++;
	return TRUE;
}

BOOL CLfp_Nsa3000::ProcessProtocolBuf( BYTE * buf , int len )
{
	if(buf[6] != byValueCode[m_byCodeIndex])			//0x44;
	{
		m_bNeedConfirm = TRUE;
		printf("LFP: Need Confirm, Code=0x%X\n", buf[6]);
		m_byRecvCount++;
		return FALSE;
	}

	//Ϊʲôû�м���͵��ж�?
	if(!memcmp(buf, "\xeb\x90\xed\x90\x02", 5) || buf[len-1] != 0x03 || buf[5] != m_wDevAddr)
	{
		printf("LFP: Package Check Error��Addr=%d, name=%s\n", m_wDevAddr, m_sDevName);
		m_byRecvCount++;
		return FALSE;
	}
	int iLen = buf[9]-4;
	BYTE *pData = buf+10;				//ָ�����ݵ�һ���ֽ�!
	switch(byValueCode[m_byCodeIndex])
	{
	case RCS_YC:
		for(int i = 0; i<iLen; i+=2)
		{
			WORD wValue = MAKEWORD(pData[i+1], pData[i]);
			//printf("YC %d = %d\n", i/2+1, wValue);
			float fValue = (wValue&0x07FF)/2048;
			if(wValue & 0x0800)		//12λΪ����λ
				fValue = -fValue;
			if(i/2 < 11)
				m_pMethod->SetYcData ( m_SerialNo ,i/2 , fValue);
		}
		break;
	case RCS_YX:
		for(int i = 0; i<iLen; i++)
		{
			BYTE byValue = pData[i];
			for(int j = 0; j<8; j++)
			{
				BYTE bValue = byValue & (1<<j);
				//printf("YX %d = %s\n", i*8+j, bValue?"TRUE":"FALSE");
				if(i*8+j < 48)
					m_pMethod->SetYxData ( m_SerialNo ,i*8+j , bValue?1:0);
			}
		}
		break;
	case RCS_SWITCH:
		{
//			BYTE byNum = *pData;
			pData ++;
			WORD wValue = MAKEWORD(*pData, *(pData+1));				//��--��
			for(int j = 0; j<16; j++)
			{
				WORD bValue = wValue & (1<<j);
				m_pMethod->SetYxData ( m_SerialNo ,j , bValue?1:0);
			}		
		//	printf("num = %d, wValue = 0x%X\n", byNum, wValue);
		}
		break;
	default:
		break;
	}

	m_bLinkStatus = TRUE;
	m_bySendCount = 0;
	m_byRecvCount = 0;
	return TRUE;
}
void CLfp_Nsa3000::TimerProc()
{
	if( m_bySendCount > 3 || m_byRecvCount > 3)
	{
		m_bySendCount = 0;
		m_byRecvCount = 0;
		if( m_bLinkStatus  )
		{
			m_bLinkStatus = FALSE;
			print( ( char * ) "CModBusLFP:unlink\n");
		}
	}
}

BOOL CLfp_Nsa3000::GetDevCommState( )
{
	if ( m_bLinkStatus )
	{
		return COM_DEV_NORMAL;
	}
	else
	{
		return COM_DEV_ABNORMAL;
	}
}

int CLfp_Nsa3000::PackMsg(BYTE * byBuf, BYTE byCode, BYTE * byData, int iDataLen)
{
	int iLen = 0;
	byBuf[iLen++] = 0xEB;
	byBuf[iLen++] = 0x90;
	byBuf[iLen++] = 0xEB;
	byBuf[iLen++] = 0x90;
	byBuf[iLen++] = 0x02;
	byBuf[iLen++] = m_wDevAddr;
	byBuf[iLen++] = byCode;			//������
	byBuf[iLen++] = 0x01;
	byBuf[iLen++] = 0x00;
	byBuf[iLen++] = 4+iDataLen;
	if(iDataLen && byData)
	{
		memcpy(byBuf+iLen, byData, iDataLen);
		iLen += iDataLen;
	}
	
	WORD wCrc = GetCrc(byBuf+5, iLen-5);
	byBuf[iLen++] = LOBYTE(wCrc);
	byBuf[iLen++] = HIBYTE(wCrc);
	byBuf[iLen++] = 0x03;
	
//	printf("pack:");	
//	for(int i = 0; i<iLen; i++)
//		printf("%02x ", byBuf[i]);
//	printf("\n");	
	
	return iLen;
}
