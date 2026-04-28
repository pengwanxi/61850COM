#ifndef CWATCHDOG_H
#define CWATCHDOG_H


class CWatchDog
{
	public:
		CWatchDog();
		virtual ~CWatchDog();
		int OpenWatchDog( ) ;
		int closeWatchDog( ) ;
		int FeedDog( ) ;
	protected:
	private:
		int fd  ;
		int SetWatchDogTimeOut( ) ;
		int SetWatchDogOpt( ) ;
		int GetWatchDogState( ) ;
};

#endif // CWATCHDOG_H

