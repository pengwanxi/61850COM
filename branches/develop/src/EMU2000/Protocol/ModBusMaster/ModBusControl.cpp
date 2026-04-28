#include "ModBusControl.h" 
extern "C" void OutBusDebug(BYTE byBusNo, BYTE *buf, int len, int flag);
#define ERROR_CONST			5
#define COMSTATUS_ONLINE	1
#define COMSTATUS_FAULT		0
CModBusControl::CModBusControl()
{
	m_byPortStatus = 0;
	m_wErrorTimer = 0;
}

CModBusControl::~CModBusControl()
{
	
}

BOOL CModBusControl::GetProtocolBuf( BYTE * buf , int &len , PBUSMSG pBusMsg ) 
{
	m_wErrorTimer++;
	if( m_wErrorTimer > 60000 )
		m_wErrorTimer = ERROR_CONST + 1 ;
		
	len = 0;
	buf[len++] = m_wDevAddr;
	buf[len++] = 0x01;
	buf[len++] = 0x5D;
	
	WORD wCRC = GetCrc( buf, len );
    buf[ len++ ] = HIBYTE(wCRC);
    buf[ len++ ] = LOBYTE(wCRC);
	
	return TRUE;
}

BOOL CModBusControl::ProcessProtocolBuf( BYTE * buf , int len ) 
{
	
	while( 1 )
	{
		if( len >= 98 )
		{
			if( buf[0] == m_wDevAddr && buf[1] == 01 &&	buf[2] == 0x5d )
			{
				WORD wCRC = GetCrc( buf, 96 );
   				if( buf[96]== HIBYTE(wCRC) && buf[97]== LOBYTE(wCRC) )
				{
   					DealRecvMsg( buf , len );
					m_wErrorTimer = 0 ;
	   				return TRUE ;
	   			}
			}
		}
		else
		{
			OutBusDebug( m_byLineNo,(BYTE *)"control msg err", 100, 2 );
			return FALSE;
		}
		buf++;
		len--;
	}
}

BOOL CModBusControl::Init( BYTE byLineNo )
{
	return TRUE ;
}			

BOOL CModBusControl::DealRecvMsg( BYTE * buf , int len )
{
	// WORD YC[23] ;
	short wYcVal;
	float fYcVal;
	BYTE YC_harmonic[36] ;
	float YM[2] ;
	BYTE YX[24]	;
	BYTE i = 0;
	
	for( i=0 ; i< 23 ; i++ )
	{	
		// YC[i] = (buf[2*i+3] | buf[2*i+4]<<8);
		if( buf[2*i+4] & 0x80 )
		{
			wYcVal = (buf[2*i+3] | buf[2*i+4]<<8);
			fYcVal = ~wYcVal + 1;
			// fYcVal =(~(  (buf[2*i+3] | buf[2*i+4]<<8) ))  + 1;	
			fYcVal = fYcVal * -1.0;
			// printf ( "----fYcVal = %f\n", fYcVal );

		}
		else
		{
			fYcVal = (buf[2*i+3] | buf[2*i+4]<<8);	
			// printf ( "fYcVal = %f\n", fYcVal );
		}
		m_pMethod->SetYcData( m_SerialNo , i , fYcVal );
	}
	
	for( i=0 ; i< 36 ; i++ )
	{
		YC_harmonic[i] = buf[49+i];
		m_pMethod->SetYcData( m_SerialNo , i+23 , (float)YC_harmonic[i] );
	}

	for( i=0 ; i< 2 ; i++ )
	{
		memcpy( YM+i , buf+85+4*i , 4 );
		m_pMethod->SetYmData ( m_SerialNo, i, (QWORD)(YM[i]) );
	}

	for( i=0 ; i< 24 ; i++ )
	{
		BYTE bit = (i%8);
		BYTE yxbyte= buf[93+i/8];
		while( bit-- )
		{
			yxbyte /= 2;
		}
		YX[i] = yxbyte%2;
		m_pMethod->SetYxData ( m_SerialNo , i , YX[i] );
	}
	
	
	
	return TRUE;
}

void CModBusControl::TimerProc()
{
	if( m_wErrorTimer > ERROR_CONST )									//����δ���ܱ��Ĵ������࣬������վ״̬
    {
        m_byPortStatus = COMSTATUS_FAULT;
    }
	else
	{
		m_byPortStatus = COMSTATUS_ONLINE;
	}
	return;  
}

BOOL CModBusControl::GetDevCommState( )
{
	if(m_byPortStatus == COMSTATUS_ONLINE)
	{
		return COM_DEV_NORMAL ;
    }
	else
	{
		return COM_DEV_ABNORMAL ;
	}
}

