/******************************************************************************
 *
 *  SerialPort.h: interface for the SerialPort class.
 *
 ******************************************************************************/
#ifndef _SERIALPORT_H_
#define _SERIALPORT_H_

#include "BasePort.h"

extern "C" void OutSerialSum(int iSerialSum);
/*****************************************************************************/
/* This class is the serial port */
class CSerialPort : public CBasePort
{
	/*Constructors / Destructors*/
	public:
		CSerialPort(void);
		virtual ~CSerialPort(void);
		virtual char* ClassName(){return (char *)"CSerialPort";}

		/* Attributes */
	private:
		int	m_nBaudRate;

	protected:

	public:

		/* Implementation */
	public:
		virtual BOOL   IsPortValid( void );
		virtual BOOL   OpenPort( char* lpszError=NULL );
		virtual void   ClosePort( void );
		virtual BOOL   SetQueue( DWORD dwInQueueSize, DWORD dwOutQueueSize );
		virtual int	   GetInQueue( void );
		virtual int    GetOutQueue( void );
		virtual int	   ReadPort( BYTE *pBuf, int nRead );
		virtual int	   WritePort( BYTE *pBuf, int nWrite );
		virtual int	   AsyReadData( BYTE *pBuf, int nRead );
		virtual int    AsySendData( BYTE *pBuf, int nWrite );

	private:

};
/*****************************************************************************/
#endif /*_SERIALPORT_H_*/
