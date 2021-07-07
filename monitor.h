/***********************************************************************************************************************
* SOI lab 4 "Monitory"                                                                                                 *
* autor: Halyna Polekha 294866                                                                                         *
*                                                                                                                      *
* -------------------------------------------------Tresc---------------------------------------------------------------*
* Bufor 9-elementowy FIFO. Jest jeden producent i trzech konsumentow (A, B, C). Producent produkuje jeden element,     *
* jezeli jest miejsce w buforze. Element jest usuwany z bufora, jeżeli zostanie przeczytany przez albo obu konsumentow *
* A i B, albo przez obu konsumentow B i C. Konsument A nie może przeczytac elementu, jezeli zostal on juz przez niego  *
* wczesniej przeczytany, albo zostal przeczytany przez konsumenta C i na odwrot.                                       *
***********************************************************************************************************************/

#ifndef __monitor_h
#define __monitor_h

#include <stdio.h> 
#include <stdlib.h> 
#include <iostream> // io
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>

#endif

class Semaphore
{
public:

  Semaphore( int value )
  {
#ifdef _WIN32
	sem = CreateSemaphore( NULL, value, 1, NULL );
#else
     if( sem_init( & sem, 0, value ) != 0 )
       throw "sem_init: failed";
#endif
  }
  ~Semaphore()
  { 
#ifdef _WIN32
	CloseHandle( sem );
#else
	  sem_destroy( & sem ); 
#endif
  }

  void p()
  {
#ifdef _WIN32
	  WaitForSingleObject( sem, INFINITE );
#else
     if( sem_wait( & sem ) != 0 )
       throw "sem_wait: failed";
#endif
	
  }

  void v()
  {
#ifdef _WIN32
	  ReleaseSemaphore( sem, 1, NULL );
#else
     if( sem_post( & sem ) != 0 )
       throw "sem_post: failed";
#endif
  }
  
  int gv(){
	int r;
	sem_getvalue(&sem, &r);
	return r;
}
private:

#ifdef _WIN32
	HANDLE sem;
#else
	sem_t sem;
#endif
};

class Condition
{
  friend class Monitor;

public:
	Condition() : w( 0 )
	{
		waitingCount = 0;
	}

	void wait()
	{
		w.p();
	}

	bool signal()
	{
		if( waitingCount )
		{
			-- waitingCount;
			w.v();
			return true;
		}//if
		else
			return false;
	}

private:
	Semaphore w;
	int waitingCount; //liczba oczekujacych watkow
};


class Monitor
{
public:
	Monitor() : s( 1 ) {}	
	void enter()
	{		
	    s.p();	
	}

	void leave()
	{
		s.v();
	}

	void wait( Condition & cond )
	{
		++ cond.waitingCount;
		leave();
		cond.wait();
		enter();
	}

	void signal( Condition & cond )
	{
		cond.signal();
	}


private:
	Semaphore s;
};

#endif

