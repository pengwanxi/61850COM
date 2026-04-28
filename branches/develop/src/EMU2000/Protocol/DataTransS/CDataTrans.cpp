/*
 * =====================================================================================
 *
 *       Filename:  CDataTrans.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2015��06��09�� 18ʱ28��18��
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  mengqp 
 *   Organization:  
 *
 *		  history:	Time								Author			version			Description
 *					2015��06��09�� 18ʱ29��13��         mengqp			1.0				created
 * =====================================================================================
 */

#include "CDataTrans.h"
#include <stdio.h>
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  CDataTrans
 * Description:  constructor
 *--------------------------------------------------------------------------------------
 */
CDataTrans::CDataTrans ()
{
	memset( m_fYcBuf, 0, sizeof( m_fYcBuf ) );
	memset( m_byYxBuf, 0, sizeof( m_byYxBuf ) );
	memset( m_dwYmBuf, 0, sizeof( m_dwYmBuf ));

	//Ĭ����15s
	m_wAllDataInterval = 15;
	m_LocalHeartbeatTime = 60 * 1000;

	//��ʼ������״̬����
	InitProtocolState(  );
	printf ( "CDataTrans construtor\n" );


}  /* -----  end of method CDataTrans::CDataTrans  (constructor)  ----- */



/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ~CDataTrans
 * Description:  destructor
 *--------------------------------------------------------------------------------------
 */
CDataTrans::~CDataTrans ()
{
	printf ( "CDataTrans destrutor\n" );
	/* �������� */	
}  /* -----  end of method CDataTrans::~CDataTrans  (destructor)  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  TimerProc
 * Description:  ʱ�䴦��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::TimerProc ( void )
{
	/* 	���±仯���� */
	ReadChangData();	
	
	/* ʱ��Э�鴦�� */
	TimeToProtocol(  );

	/* 	Э�鳬ʱ�򳬴������� */
	ProtocolErrorProc(  );
}		/* -----  end of method CDataTrans::TimerProc  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ProcessProtocolBuf
 * Description:  ����Э�鱨��
 *       Input:	 pBuf������ָ��
 *				 len:����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::ProcessProtocolBuf ( BYTE *pBuf, int len )
{
	int pos =0;
	/* �жϱ��ĺ�����  ���ҳ�һ֡�������� */	
	if( !WhetherBufValid( pBuf, len, pos ) )
	{
		print( ( char * )"CDataTrans can't find right recv buf" );
		SetState( DATATRANS_RESEND_STATE );
		return FALSE;
	}

	/* �������� */
	if( !ProcessRecvBuf( &pBuf[pos], len ) )
	{
		return FALSE;
	}
	
	/* ����״̬�л� */
	SetRecvParam(  );
	return TRUE;
}		/* -----  end of method CDataTrans::ProcessProtocolBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetProtocolBuf
 * Description:  ��ȡЭ�鱨�� 
 *       Input:  buf:��֯���Ļ�����
 *				 len:���������� 
 *				 pBusMsg:��Ϣָ��  �ڴ�Э��������
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetProtocolBuf ( BYTE *buf,
		int &len,
		PBUSMSG pBusMsg)
{
	BOOL bRtn = FALSE;
/* 	�ж����������Ч�� */
	//�ж�buf
	if( NULL == buf  )
	{
		print( ( char * )" CDataTrans GetProtocolBuf buf = NULL" );
		return FALSE;	
	}
	memset( buf, 0, 256 );
	//���ж�pBusMsg

/* 	��ȡ���ͱ��� */
	bRtn = GetSendBuf( buf, len );

/* 	����״̬�����л� */
	SetSendParam( bRtn );

	return bRtn;
}		/* -----  end of method CDataTrans::GetProtocolBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  Init
 * Description:  ��ʼ��Э��  
 *       Input:  byLineNo:���ߺ�
 *		Return:  BOOL
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::Init ( BYTE byLineNo )
{
	//�������ļ�
	if( !ReadCfgInfo(  ) )
	{
		return FALSE;
	}
	
	//��ʼ������
	InitProtocol(  );
	
	// CloseLink(  );
	UnsetState( DATATRANS_LINK_STATE );
	return TRUE;
}		/* -----  end of method CDataTrans::Init  ----- */


/* #####   time ʱ�䲿��   ################################################### */
/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  TimeToProtocol
 * Description:  ʱ��Э�鴦��
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::TimeToProtocol ( void )
{
	//�Ƿ�ʱ�䷢��ȫ���� ������û�з��� �������˴�
	if( TimeToAll(  ) )
	{
		print( (char *)"CDataTrans timetoall" );
		DWORD dwAll = DATATRANS_YC_STATE | DATATRANS_YX_STATE | DATATRANS_YM_STATE ;
		SetState( dwAll );
		// OpenLink(  );
		SetState( DATATRANS_LINK_STATE );
	}

	if( TimeToHeartbeat(  ) )
	{
		print( (char *)"CDataTrans timetoheart" );
		SetState( DATATRANS_HEARTBEAT_STATE );
		// OpenLink(  );
		SetState( DATATRANS_LINK_STATE );
	}

}		/* -----  end of method CDataTrans::TimeToProtocol  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  TimeToAll
 * Description:  �Ƿ�ʱ�䷢��ȫ������ 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::TimeToAll ( void )
{
	m_LocalAddTime += 200;
	if( m_LocalAddTime  >= m_LocalSumTime )
	{
		//��������������� 
		if( m_ProtocolState )
		{
			return FALSE;
		}

		//��������
		m_LocalHeartbeatAddTime = 0;
		//����ʱ��
		m_LocalAddTime = 0;
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CDataTrans::TimeToAll  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  TimeToHeartbeat
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::TimeToHeartbeat ( void )
{
	m_LocalHeartbeatAddTime += 200;
	if( m_LocalHeartbeatAddTime  >= m_LocalHeartbeatTime )
	{
		//��������������� 
		if( m_ProtocolState )
		{
			return FALSE;
		}

		//��������
		m_LocalHeartbeatAddTime = 0;
		return TRUE;
	}

	return FALSE;
}		/* -----  end of method CDataTrans::TimeToHeartbeat  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ProtocolErrorProc
 * Description:  Э������� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::ProtocolErrorProc ( void )
{
	m_byTimerCount ++;

	if( m_byTimerCount > 20 )
	{
		m_byTimerCount = 0;
		if( ! IsHaveState(DATATRANS_LINK_STATE) )
		print( "CDataTrans is runing please wait" );
	}

	if( m_bySendCount > DATATRANS_MAX_SEND_COUNT )
	{
		sprintf( m_szPrintBuf, "sendcount=%d > %d init protocol",m_bySendCount,  DATATRANS_MAX_SEND_COUNT );
		print(m_szPrintBuf  );
		InitProtocolState(  );
	}
}		/* -----  end of method CDataTrans::ProtocolErrorProc  ----- */

/* #####   recv ���ղ���   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ProcessRecvBuf
 * Description:  �������ձ��� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::ProcessRecvBuf ( BYTE *pBuf, int len )
{
	if( !m_bSending )
	{
		return FALSE;
	}

	switch ( pBuf[1] )
	{
		case 0xF1:	
			if( !IsHaveState( DATATRANS_YC_STATE ) )
			{
				return FALSE;	
			}
			if( IsHaveState( DATATRANS_YC_OVER_STATE ) )
			{
				UnsetState( DATATRANS_YC_OVER_STATE );
				UnsetState( DATATRANS_YC_STATE );
			}
			print( (char *)"CDataTrans pocess recv yc" );
			break;

		case 0xF3:	
			if( !IsHaveState( DATATRANS_YX_STATE ) 
					&& !IsHaveState( DATATRANS_CHANGE_YX_STATE ) )
			{
				return FALSE;	
			}

			if( IsHaveState( DATATRANS_CHANGE_YX_STATE ) )
			{
				print( (char *)"CDataTrans pocess recv changeyx" );
				UnsetState( DATATRANS_CHANGE_YX_STATE );
			}

			if( IsHaveState( DATATRANS_YX_OVER_STATE ) )
			{
				UnsetState( DATATRANS_YX_OVER_STATE );
				UnsetState( DATATRANS_YX_STATE );
			}
			print( (char *)"CDataTrans pocess recv yx" );
			break;

		case 0xF5:	
			if( !IsHaveState( DATATRANS_YM_STATE ) )
			{
				return FALSE;	
			}
			if( IsHaveState( DATATRANS_YM_OVER_STATE ) )
			{
				UnsetState( DATATRANS_YM_OVER_STATE );
				UnsetState( DATATRANS_YM_STATE );
			}
			print( (char *)"CDataTrans pocess recv ym" );
			break;

		case 0xF7:	
			if( !IsHaveHeart(  ) )
			{
				return FALSE;
			}

			UnsetState( DATATRANS_HEARTBEAT_STATE );
			print( (char *)"CDataTrans pocess recv heart" );

			break;


		default:	
			return FALSE;
			break;
	}				/* -----  end switch  ----- */

	return TRUE;
}		/* -----  end of method CDataTrans::ProcessRecvBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  SetRecvParam
 * Description:  ���ý��ղ��� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::SetRecvParam ( void  )
{
	m_bSending = FALSE;
	m_bySendCount = 0;
	UnsetState( DATATRANS_RESEND_STATE );
}		/* -----  end of method CDataTrans::SetRecvParam  ----- */


/* #####   send ���Ͳ���   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetSendBuf
 * Description:  ��ȡ���ͱ��� 
 *       Input:  buf ������
 *				 len ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetSendBuf ( BYTE *buf, int &len )
{
	BOOL bRtn = TRUE;
	//��ȡ����״̬
	if( !GetProtocolState(  ) )	
	{
		return FALSE;
	}
	
	//���ݸ�ʽ��֯��Ӧ���ͱ���
	bRtn = GetSendTypeBuf( buf, len );
	
	//�����ط�����
	SaveResendBuf( buf, len, bRtn );

	return bRtn;
}		/* -----  end of method CDataTrans::GetSendBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetProtocolState
 * Description:  ��������õ�Э��״̬
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetProtocolState ( void )
{
	//�ط���ʽ�ж�
	if( IsResend(  ) )
	{
		// print( (char *)"CDataTrans resend" );
		SetState( DATATRANS_RESEND_STATE );	
		// OpenLink(  );
		SetState( DATATRANS_LINK_STATE );
		return TRUE;
	}

	//�仯Yx����
	if( IsHaveChangeYX(  ) )
	{
		print( (char *)"CDataTrans changeyx" );
		SetState( DATATRANS_CHANGE_YX_STATE );
		// OpenLink(  );
		SetState( DATATRANS_LINK_STATE );
		return TRUE;
	}
	
	//ȫ����
	if( IsHaveAll(  ) )
	{
		print( (char *)"CDataTrans alldata" );
		// OpenLink(  );
		SetState( DATATRANS_LINK_STATE );
		return TRUE;
	}

	//����
	if( IsHaveHeart(  ) )
	{
		print( (char *)"CDataTrans heart" );
		// OpenLink(  );
		SetState( DATATRANS_LINK_STATE );
		return TRUE;
	}

	// CloseLink(  );
	UnsetState( DATATRANS_LINK_STATE );
	return FALSE;
}		/* -----  end of method CDataTrans::GetProtocolState  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  IsResend
 * Description:  �Ƿ���Ҫ�ط� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::IsResend ( void ) const
{
	return IsHaveState( DATATRANS_RESEND_STATE ); 
}		/* -----  end of method CDataTrans::IsResend  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  IsHaveChangeYX
 * Description:  �Ƿ��б仯YX 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::IsHaveChangeYX ( void  ) const
{
	if ( m_dwDIEQueue.size( ) > 0 )
	{
		return TRUE;
	}

	return FALSE;
}		/*  -----  end of method CDataTrans::IsHaveChangeYXData  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  IsHaveAll
 * Description:  �Ƿ���ȫ���������� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::IsHaveAll ( void  ) const
{
	DWORD dwAll = DATATRANS_YC_STATE | DATATRANS_YX_STATE | DATATRANS_YM_STATE 
		| DATATRANS_YC_OVER_STATE | DATATRANS_YX_OVER_STATE | DATATRANS_YM_OVER_STATE;
	if( dwAll & m_ProtocolState )
	{
		return TRUE;
	}

	return FALSE;
}		/*  -----  end of method CDataTrans::IsHaveAll  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  IsHaveHeart
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::IsHaveHeart ( void ) const
{
	return IsHaveState( DATATRANS_HEARTBEAT_STATE );
}		/* -----  end of method CDataTrans::IsHaveHeart  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetSendTypeBuf
 * Description:  ��ȡ��Ӧ���͵�����
 *       Input:  buf ������
 *				 len ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetSendTypeBuf ( BYTE *buf, int &len)
{
	/* ��ȡ�ط����� */
	if( IsHaveState( DATATRANS_RESEND_STATE ) )
	{
		GetResendBuf( buf, len );	
		return TRUE;
	}

	/* ��ȡ�仯ң������ */
	if( IsHaveState( DATATRANS_CHANGE_YX_STATE ) )
	{
		return GetChangeYXBuf( buf, len  );
	}

	/* ��ȡȫ������ */
	if( GetAllDataBuf( buf, len ) )
	{
		return TRUE;
	}

	/*  ��ȡ��������*/
	if( IsHaveHeart(  ) )
	{
		return GetHeartBuf( buf, len );
	}

	return FALSE;
}		/* -----  end of method CDataTrans::GetSendTypeBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetChangeYXBuf
 * Description:  ��ȡ�仯YX���� 
 *       Input:  buf ������
 *				 len ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetChangeYXBuf ( BYTE *buf, int &len )
{
	/* ��֯�仯YX���� */
	BOOL bRtn =  PackChangeYXBuf( buf, len );
	//״̬����
	// UnsetState( DATATRANS_CHANGE_YX_STATE );
	print( (char *)"CDataTrans get changeyx" );

	return bRtn;
}		/* -----  end of method CDataTrans::GetChangeYXBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  PackChangeYXBuf
 * Description:  ��֯YX���� 
 *       Input:  buf ������
 *				 len ����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::PackChangeYXBuf ( BYTE *buf, int &len )
{
	WORD wSerialNo;
	WORD wPnt;
	WORD wVal;
	BOOL bDevState;
	BYTE byVal;

	//��ȡ�仯ң�ŵ���š���ź�ֵ
	if( !GetDigitalEvt( wSerialNo, wPnt, wVal ) )
	{
		return FALSE;
	}
	//��ȡװ��״̬
	bDevState = m_pMethod->GetDevCommState( wSerialNo );	
	//bDevState:0���� 1������ wVal:0�� 1�� 10 11��Ч 00 01��Ч
	byVal = ( bDevState << 1 ) | ( wVal & 0x01 );

    len = 0;	
	//����ͷ
	buf[len++] = 0x68;
	//��ַ
	buf[len++] = HIBYTE( m_wDevAddr );
	buf[len++] = LOBYTE( m_wDevAddr );
	//������
	buf[len++] = 0xF2;
	//��ʼ���
	buf[len++] = HIBYTE( wPnt );
	buf[len++] = LOBYTE( wPnt );
	//����
	buf[len++] = 0x01;
	//ֵ
	buf[len++] = byVal;

	return TRUE;
}		/* -----  end of method CDataTrans::PackChangeYXBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetAllDataBuf
 * Description:  ��ȡȫ������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetAllDataBuf ( BYTE *buf, int &len )
{
	/* YC���� */
	if( IsHaveState( DATATRANS_YC_STATE ) )
	{
		print( (char *)"CDataTrans get yc" );
		return GetYCDataBuf( buf, len );
	}

	/* YX���� */
	if( IsHaveState( DATATRANS_YX_STATE ) )
	{
		print( (char *)"CDataTrans get yx" );
		return GetYXDataBuf( buf, len );
	}

	/* YM���� */
	if ( IsHaveState( DATATRANS_YM_STATE ) )
	{
		print( (char *)"CDataTrans get ym" );
		return GetYMDataBuf( buf, len );
	}

	return FALSE;
}		/* -----  end of method CDataTrans::GetAllDataBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetYcDataBuf
 * Description:  ��ȡyc���ݰ� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetYCDataBuf ( BYTE *buf, int &len )
{
	/* ��֯YC���� */
	BOOL bRtn = PackYCBuf( buf, len );
	//״̬����
	if( m_wAllDataPos >= m_wAISum )
	{
		// UnsetState( DATATRANS_YC_STATE );
		SetState ( DATATRANS_YC_OVER_STATE );
		m_wAllDataPos = 0;
	}
	return bRtn;
}		/* -----  end of method CDataTrans::GetYcDataBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  PackYCBuf
 * Description:  ��֯YX���� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::PackYCBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
    len = 0;	
	//����ͷ
	buf[len++] = 0x68;
	//��ַ
	buf[len++] = HIBYTE( m_wDevAddr );
	buf[len++] = LOBYTE( m_wDevAddr );
	//������
	buf[len++] = 0xF0;
	//��ʼ���
	buf[len++] = HIBYTE( m_wAllDataPos );
	buf[len++] = LOBYTE( m_wAllDataPos );
	//����
	buf[len++] = 0x00;

	for( int i=m_wAllDataPos; i<m_wAISum; i++ )
	{
		float fVal = m_fYcBuf[i];
		BYTE szTmp[4];
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( YC_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		//��Ч��
		buf[len++] = (BYTE)bDevState;

		//ֵ
		memcpy( szTmp, &fVal, 4 );
		buf[len++] = szTmp[3];
		buf[len++] = szTmp[2];
		buf[len++] = szTmp[1];
		buf[len++] = szTmp[0];

		// char szBuf[256];
		// sprintf( szBuf, "yc wSerialNo:%d wPnt:%d, State:%d, val=%f", wSerialNo, i, bDevState, fVal );
		// print( szBuf );
		
		//���Χ50
		byCount ++;
		if( byCount >= 45 )
		{
			break;
		}
	}

	//����
	buf[6] = byCount;
	m_wAllDataPos += byCount;

	return TRUE;
}		/* -----  end of method CDataTrans::PackYCBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetYcDataBuf
 * Description:  ��ȡyc���ݰ� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetYXDataBuf ( BYTE *buf, int &len )
{
	/* ��֯YX���� */
	BOOL bRtn = PackYXBuf( buf, len );
	//״̬����
	if( m_wAllDataPos >= m_wDISum )
	{
		// UnsetState( DATATRANS_YX_STATE );
		SetState ( DATATRANS_YX_OVER_STATE );
		m_wAllDataPos = 0;
	}
	return bRtn;
}		/* -----  end of method CDataTrans::GetYcDataBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  PackYXBuf
 * Description:  ��֯YX���� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::PackYXBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
    len = 0;	
	//����ͷ
	buf[len++] = 0x68;
	//��ַ
	buf[len++] = HIBYTE( m_wDevAddr );
	buf[len++] = LOBYTE( m_wDevAddr );
	//������
	buf[len++] = 0xF2;
	//��ʼ���
	buf[len++] = HIBYTE( m_wAllDataPos );
	buf[len++] = LOBYTE( m_wAllDataPos );
	//����
	buf[len++] = 0x00;

	for( int i=m_wAllDataPos; i<m_wDISum; i += 4 )
	{

		for ( int j=0; j<4; j++)
		{
			if( i + j >= m_wDISum )
			{
				break;
			}
			WORD wVal = m_byYxBuf[i+j];
			//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
			WORD wSerialNo = GetSerialNoFromTrans( YX_TRANSTOSERIALNO , i+j ) ;
			BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;

			//ֵ
			BYTE byVal = ( bDevState << 1 ) | ( wVal & 0x01 );
			buf[len]  = ( buf[len] | ( byVal << ( 2 * j ) ) ) ;

			// char szBuf[256];
			// sprintf( szBuf, "yx wSerialNo:%d wPnt:%d, State:%d, val=%d buflen=%x len=%d", wSerialNo, i +j, bDevState, byVal, buf[len], len );
			// print( szBuf );
			
		}

		len ++;	
		//���Χ240
		byCount += 4;
		if( byCount >= 240 )
		{
			break;
		}
	}

	//����
	buf[6] = byCount;
	m_wAllDataPos += byCount;
	return TRUE;
}		/* -----  end of method CDataTrans::PackYXBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetYMDataBuf
 * Description:  ��ȡyc���ݰ� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetYMDataBuf ( BYTE *buf, int &len )
{
	/* ��֯YM���� */
	BOOL bRtn = PackYMBuf( buf, len );
	//״̬����
	if( m_wAllDataPos >= m_wPISum )
	{
		// UnsetState( DATATRANS_YM_STATE );
		SetState ( DATATRANS_YM_OVER_STATE );
		m_wAllDataPos = 0;
	}
	return bRtn;
}		/* -----  end of method CDataTrans::GetYMDataBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  PackYMBuf
 * Description:  ��֯YX���� 
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::PackYMBuf ( BYTE *buf, int &len )
{
	BYTE byCount = 0;
    len = 0;	

	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	//����ͷ
	buf[len++] = 0x68;
	//��ַ
	buf[len++] = HIBYTE( m_wDevAddr );
	buf[len++] = LOBYTE( m_wDevAddr );
	//������
	buf[len++] = 0xF4;
	//��ʼ���
	buf[len++] = HIBYTE( m_wAllDataPos );
	buf[len++] = LOBYTE( m_wAllDataPos );
	//����
	buf[len++] = 0x00;

	for( int i=m_wAllDataPos; i<m_wPISum; i++ )
	{
		char szTmp[4];
		DWORD dwVal = (DWORD)(m_dwYmBuf[i]);
		//��ȡװ��״̬�� ���װ�ò�ͨ�� ����Ϊ��Ч���ݷǵ�ǰֵ
		WORD wSerialNo = GetSerialNoFromTrans( DD_TRANSTOSERIALNO , i ) ;
		BOOL bDevState = m_pMethod->GetDevCommState( wSerialNo ) ;
		//��Ч��
		buf[len++] = (BYTE)bDevState;
		float fVal = (float)dwVal;

		memcpy( szTmp, &fVal, 4 );
		buf[len++] = szTmp[3];
		buf[len++] = szTmp[2];
		buf[len++] = szTmp[1];
		buf[len++] = szTmp[0];

		//���Χ50
		byCount ++;
		if( byCount >= 45 )
		{
			break;
		}
	}

	//����
	buf[6] = byCount;
	m_wAllDataPos += byCount;
	return TRUE;
}		/* -----  end of method CDataTrans::PackYMBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetHeartBuf
 * Description:  
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::GetHeartBuf ( BYTE *buf, int &len )
{
	len = 0;
	buf[len++] = 0x68;
	buf[len++] = 0xF6;

	return TRUE;
}		/* -----  end of method CDataTrans::GetHeartBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  GetResendBuf
 * Description:  ��ȡ�ط����� 
 *       Input:  buf:������
 *				 len:����
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::GetResendBuf ( BYTE *buf, int &len )
{
	len = m_iResendLen;	
	memcpy( buf, m_byResendBuf, len );

	m_byResendCount ++;
}		/* -----  end of method CDataTrans::GetResendBuf  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  SaveResendBuf
 * Description:  �����ط����� 
 *       Input:  buf:������
 *				 len:����
 *				 byValid:�Ƿ񱣴�
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::SaveResendBuf ( BYTE *buf, int len, BOOL byValid )
{
	if( byValid )
	{
		m_iResendLen = len;	
		memcpy( m_byResendBuf, buf, m_iResendLen );
	}
}		/* -----  end of method CDataTrans::SaveResendBuf  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  SetSendParam
 * Description:  ���÷��Ͳ���
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::SetSendParam ( BOOL bIsSendValid )
{
	if( bIsSendValid  )
	{
		m_bSending = TRUE;
		m_bySendCount ++;
		SetState( DATATRANS_RESEND_STATE );
	}
}		/* -----  end of method CDataTrans::SetSendParam  ----- */

/* #####   Init ��ʼ������   ################################################### */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ReadCfgInfo
 * Description:  ��ȡ����ģ����Ϣ
 *       Input:  void
 *		Return:  
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::ReadCfgInfo ( void )
{
	char szPath[256] = "";
	sprintf( szPath, "%s%s" ,DATATRANSSPREFIXFILENAME, m_sTemplatePath );
	print( szPath );
	
	//������ģ��ĵ����Ϣ
    ReadCfgMapInfo ( szPath ); 

	//������ģ�������������Ϣ
	//sprintf( szPath, "%sBus%.2dOtherCfg.txt",DATATRANSSPREFIXFILENAME,  m_byLineNo+1  );
	if( !ReadCfgOtherInfo( szPath ) )
	{
		//return FALSE;
	}
	
	print( (char *)"CDataTrans ReadCfgInfo OK" );
	return TRUE;
}		/* -----  end of method CDataTrans::ReadCfgInfo  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ReadCfgMapInfo
 * Description:  ��ȡ�����Ϣ 
 *       Input:  szPath ģ��·��
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::ReadCfgMapInfo ( char *szPath )
{
	//����Rtu.cpp��ȡ
	ReadMapConfig( szPath );
}		/* -----  end of method CDataTrans::ReadCfgMapInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  ReadCfgOtherInfo
 * Description:  ������ģ�������������Ϣ
 *       Input:  szPath ģ��·��
 *		Return:
 *--------------------------------------------------------------------------------------
 */
BOOL CDataTrans::ReadCfgOtherInfo ( char *szPath )
{
	FILE *fp = NULL;
	char szLineBuf[256];
	int iLineNum = 0;
	int iOtherInfoNum = 0;
	int iOtherInfoAllNum = 2;

	fp = fopen( szPath , "r");
	if ( NULL == fp )
	{
		printf ( "open file %s err!!!\n", szPath );
		return FALSE;
	}

	while ( NULL != fgets(szLineBuf, sizeof(szLineBuf), fp) 
			&& 30 > iLineNum )
	{
		iLineNum ++; 
		if( 0 == strncmp ( szLineBuf, "SENDINTERVAL=", 13 ) )
		{
			WORD wCfgVal = (WORD)( atoi( &szLineBuf[13] ) );
			if( wCfgVal >= 1 )
			{
				m_wAllDataInterval = wCfgVal;		
			}
			else
			{
				printf ( "CDataTrans SENDINTERVAL=%d error!!! default is used\n", wCfgVal );
				m_wAllDataInterval = 15;
			}

			m_LocalSumTime = m_wAllDataInterval *1000 ;
			iOtherInfoNum ++;
		}
		if( 0 ==strncmp( szLineBuf, "HEARTTIME=", 10 ) )
		{
			WORD wCfgVal = (WORD)( atoi( &szLineBuf[10] ) );
			if( wCfgVal >= 1 )
			{
				m_LocalHeartbeatTime = wCfgVal * 1000;		
			}
			else
			{
				printf ( "CDataTrans HEARTTIME=%d error!!! default is used\n", wCfgVal );
				m_LocalHeartbeatTime = 60 * 1000;
			}

			iOtherInfoNum ++;
		}

		if( iOtherInfoAllNum <= iOtherInfoNum )
		{
			printf ( "CDataTrans alldata interval=%lums, heat interval=%lums\n", m_LocalSumTime, m_LocalHeartbeatTime );
			break;
		}
	}

	fclose( fp );
	return TRUE;
}		/* -----  end of method CDataTrans::ReadCfgOtherInfo  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  InitProtocol
 * Description:  ��ʼ��Э��״̬
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::InitProtocol ( void )
{
	//��ʼ������״̬����
	InitProtocolState(  );

	//��ʼ�������Ϣ
	InitProtocolTransTab(  ); 

	//��ʼ��Э������
	InitProtocolData(  ); 

	print( (char *)"CDataTrans InitProtocol OK" );
}		/* -----  end of method CDataTrans::InitProtocol  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  InitProtocolState
 * Description:  ��ʼ��Э�����
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::InitProtocolState ( void )
{
	/* ��ʼ����Ϣ */
	//��ʼ��Э��״̬
	if( IsHaveState( DATATRANS_LINK_STATE ) )
	{
		// CloseLink(  );
		UnsetState( DATATRANS_LINK_STATE );
	}
	m_ProtocolState = 0;
	//��ʼ���ط�״̬
	m_byResendCount = 0;
	m_iResendLen = 0;
	memset( m_byResendBuf, 0, sizeof( m_byResendBuf ) );
	//ȫ����λ��
	m_wAllDataPos = 0;
	//״̬���
	m_bSending=FALSE;;
	m_bySendCount = 0;

	//ʱ�����
	m_LocalAddTime = 0;
	m_LocalHeartbeatAddTime = 0;
	m_byTimerCount = 0;



	print( (char *)"CDataTrans InitProtocolState" );

}		/* -----  end of method CDataTrans::InitProtocolState  ----- */


/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  InitProtocolTransTab
 * Description:  ��ʼ��ת����Ϣ
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::InitProtocolTransTab ( void )
{
	/* ��ȡ�����Ϣ��ת����� */
    CreateTransTab();
}		/* -----  end of method CDataTrans::InitProtocolTransTab  ----- */

/*
 *--------------------------------------------------------------------------------------
 *       Class:  CDataTrans
 *      Method:  InitProtocolData
 * Description:  ��ʼ��Э������
 *       Input:
 *		Return:
 *--------------------------------------------------------------------------------------
 */
void CDataTrans::InitProtocolData ( void )
{
	/* ���ڴ����ݿ���--��ȡת����Ĭ������ */
	m_pMethod->ReadAllYcData(&m_fYcBuf[0]);
	m_pMethod->ReadAllYmData(&m_dwYmBuf[0]);
	m_pMethod->ReadAllYxData( &m_byYxBuf[ 0 ] ) ;
}		/* -----  end of method CDataTrans::InitProtocolData  ----- */

/* #####   other ��������   ################################################### */
int CDataTrans::GetRealVal(BYTE byType, WORD wPnt, void *v)
{
    WORD  wValue = 0 ;
    switch(byType)
    {
    case 0:
        if(wPnt>=DATATRANS_MAX_YC_NUM) return -2;
        memcpy(v, &m_fYcBuf[wPnt], sizeof(WORD));
        break;
    case 1:
        {
			if(wPnt>=DATATRANS_MAX_YX_NUM)
				return -2;

			if( m_byYxBuf[ wPnt ] ==0 )
				wValue = 0;
			else
				wValue = 1;

			memcpy(v, &wValue, sizeof(WORD));
		}
        break;
    case 2:
        if(wPnt>=DATATRANS_MAX_YM_NUM) return -2;
        memcpy(v, &m_dwYmBuf[wPnt], sizeof(QWORD));
        break;
    default:
        return -1;
    }
    return 0;
}

BOOL CDataTrans::WriteAIVal(WORD wSerialNo ,WORD wPnt, float fVal)
{
    if(m_pwAITrans==NULL) return FALSE;
    WORD wNum = m_pwAITrans[wPnt];
    if(wNum>m_wAISum) return FALSE;
    if(wNum<DATATRANS_MAX_YC_NUM)//mengqp ��<=��Ϊ< ����m_wAIBuf[4096]Խ��
    {
        float fDelt = fVal - m_fYcBuf[wNum];
        if(abs((int)fDelt)>=m_wDeadVal)
        {
            m_fYcBuf[wNum] = fVal;
			// if(m_bDataInit)
			// {
                AddAnalogEvt( wSerialNo , wNum, fVal);
			// }
        }
    }
    return TRUE ;
}

BOOL CDataTrans::WriteDIVal(WORD wSerialNo ,WORD wPnt, WORD wVal)
{
    if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>m_wDISum) return FALSE;
    if( wNum<DATATRANS_MAX_YX_NUM)//mengqp ��<= ��Ϊ<
    {
        if( m_byYxBuf[ wNum ] != wVal )
        {
            m_byYxBuf[ wNum ] = wVal ;
            // if(m_bDataInit)
			// {
                AddDigitalEvt( wSerialNo , wNum, wVal);
			// }
        }
    }
    return TRUE ;
}
BOOL CDataTrans::WritePIVal(WORD wSerialNo ,WORD wPnt, QWORD dwVal)
{
    if(m_pwPITrans==NULL) return FALSE;
    WORD wNum = m_pwPITrans[wPnt];
    if(wNum>m_wPISum) return FALSE;
    if(wNum<DATATRANS_MAX_YM_NUM)//mengqp ��<= ��Ϊ<
    {
        m_dwYmBuf[wNum] = dwVal;
    }
    return TRUE ;
}

BOOL CDataTrans::WriteSOEInfo( WORD wSerialNo ,WORD wPnt, WORD wVal, LONG lTime, WORD wMiSecond)
{
    if(m_pwDITrans==NULL) return FALSE;
    WORD wNum = m_pwDITrans[wPnt] & 0x7fff;
    if(wNum>=m_wDISum) return FALSE;
    if(wNum<DATATRANS_MAX_YX_NUM)
    {
        AddSOEInfo(wSerialNo , wNum, wVal, lTime, wMiSecond);
    }
    return TRUE ;
}
/* ====================  OtherEnd    ======================================= */

