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


#include "monitor.h"
#include <iostream>
#include <cstdlib>
#include <deque>
#include <pthread.h>
#include <time.h>

#define SIZE 9
#define K 99999

using namespace std;
unsigned int IndepRand();
//implementacja kolejki FIFO wraz z obsluga przewidywanych bledow.
class Buffer : public Monitor {
  private:
	deque<char> buffer;
	char product;
	Condition sem_not_empty;
	Condition sem_not_full;
	Condition sem_let_A; ////pozwalamy czytac
	Condition sem_let_B;
	Condition sem_let_C; 


	int back() {
		return buffer.back();
	}
	void update_product(){

		product = buffer.back();
							cout << "	~~ update Product = "<< product << " ~~"<< endl;
	}
	void update_product(char product){

		buffer.back() = product;
	}


	int size() {
		return buffer.size();
	}

	void push(char prod) {
		if (buffer.size() < SIZE) {
			buffer.push_front(prod);
		} else {
        			cerr << "Blad przy operacji push_front\n";
        		exit(2);
     		}
		return;
	}

	void pop() {
		if (buffer.size() > 0) {
			buffer.pop_back();
		} else {
			cerr << "Blad przy operacji pop_back\n";
		exit(3);
		}
		return;
	}

  public:
  void produce();
  void consume_A();
  void consume_B();
  void consume_C();

};


  void Buffer::produce() {
	usleep((IndepRand() % 500000));
  	enter(); 
					cout << "Producent whodzi do SK" << endl;					
  	while (size() == SIZE) {
					cout << "	Producent zawieszony (pelna kolejka)" << endl; 
		wait(sem_not_full);
					cout << " Producer contynuacja"<< endl;
	}			
					cout << "	Producent produkuje " << 'P' << endl;
	push('P');
					cout << "	Bufor size: " << buffer.size() << endl;				

	if (size() > 0) signal(sem_not_empty);
	
					cout << "Producent wychodzi z  SK" << endl;
     	leave();						
  }

  void Buffer::consume_A() {
	usleep((IndepRand() % 500000));
  	enter();
					cout << "Cons A w SK" << endl;						
	while(size() == 0){
					cout << "Cons A zawieszony (pusta kolejka)" << endl;
		 wait(sem_not_empty);
					cout << "Cons A contynuacja"<< endl;
	}	
					cout << "	Cons A konsumuje" << endl;
	update_product();	
	while(product == 'C' || product == 'A') { 
					cout << "Cons A zawieszony (trafil A lub C)" << endl;
		wait(sem_let_A);
					cout << "Cons A contynuacja "<< endl;
		update_product(); 
	}				

	if(product == 'P') { 
		update_product('A'); 
					cout << "	Cons A czyta (ustawia flage na A)" << endl;
	}else if(product == 'B'){
					cout << "		Cons A usuwa z bufora rozmiarem: " << buffer.size() << endl;
		pop();
					cout << "	Bufor size: " << buffer.size() << endl; 

	}
		
	update_product();
	if(size() < SIZE) signal(sem_not_full);
	if(product == 'P' || product == 'B') signal(sem_let_C);
	if(product == 'P'|| product == 'A'|| product == 'C') signal(sem_let_B);
					cout << "Cons A wychodzi z  SK" << endl;				      
	leave();
  }





  void Buffer::consume_B() {
	usleep((IndepRand() % 500000));
  	enter();
					cout << "Cons B w SK" << endl;
  	while(size() == 0) {
					cout << "Cons B zawieszony (pusta kolejka)" << endl;
		wait(sem_not_empty);
					cout << "Cons B contynuacja "<< endl;
	}
	update_product();
					cout << "	Cons B konsumuje" << endl;

	while(product == 'B') {
					cout << "Cons B zawieszony (trafil na B)" << endl; 
		wait(sem_let_B); 
					cout << "Cons B contynuacja"<< endl;
		update_product();
	}

	if(product == 'P') {
		update_product('B');  
					cout << "	Cons B czyta (ustawia flage na B)" << endl; 
	}else if(product == 'A' || product == 'C'){
					cout << "		Cons B usuwa z bufora rozmiarem: " << buffer.size() << endl;
		pop();
					cout << "	Bufor size: " << buffer.size() << endl;	
	}
	
	update_product();
	if(size() < SIZE) signal(sem_not_full);
	if(product == 'P' || product == 'B') { signal(sem_let_A); signal(sem_let_C); }
					cout << "Cons B wychodzi z  SK" << endl;
	leave();
  }

  void Buffer::consume_C() {
	usleep((IndepRand() % 500000));
  	enter();
					cout << "Cons C w SK" << endl;
	while(size() == 0) {
					cout << "Cons C zawieszony (pusta kolejka)" << endl;
		wait(sem_not_empty);
					cout << "Cons C contynuacja "<< endl;
	}
	update_product();
					cout << "	Cons C konsumuje" << endl;

	while(product == 'C' || product == 'A') { 
					cout << "Cons C zawieszony (trafil na A lub C)" << endl;
		wait(sem_let_C); 
					cout << "Cons C contymuacja"<< endl;
		update_product(); 
	}	

	if(product == 'P'){
		update_product('C');
					cout << "	Cons C czyta (ustawia flage na C)" << endl;    
	}else if(product == 'B') {
					cout << "		Cons C usuwa z bufora rozmiarem: " << buffer.size() << endl;
		pop();
					cout << "	Bufor size: " << buffer.size() << endl;
	}
	
	update_product();
	if(size() < SIZE)  signal(sem_not_full);	
	if(product == 'P' || product == 'B') signal(sem_let_A);
	if(product == 'P'|| product == 'A'|| product == 'C') signal(sem_let_B);
					cout << "Cons C wychodzi z  SK" << endl;
	leave();
  }

////////////////////////////////////
////////////////////////////////////
Buffer buffer;


 void * Producer(void*) {
  int i = 0;
  while ( i!= K ) {
    i++;
    buffer.produce();
    
  }
}

void * ConsumerA(void*) {
  int i = 0;
  while( i!= K ) {
    i++;
    buffer.consume_A();
  }
}

void * ConsumerB(void*) {
  int i = 0;
  while( i!= K ) {
   i++;
    buffer.consume_B();
    }
} 

void * ConsumerC(void*) {
  int i = 0;
  while( i!= K ) {
   i++;
    buffer.consume_C();
    }
}
unsigned int IndepRand()						/*Potrzebne sa wartosci losowe*/
{
	FILE * F = fopen("/dev/urandom", "r");
	if (!F)
	{
		printf("Cannot open urandom...\n");
		abort();
	}
	unsigned int Ret;
	unsigned int X = fread((char *)&Ret, 1, sizeof(unsigned int), F);
	fclose(F);

	return Ret;
}

////////////////////////////////////
////////////////////////////////////
int main(void) {
  srand(time(NULL));
   
  // zarodek do losowania sleepa
  srand(time(NULL));
  
  // pthread_t trzyma ID wątku po utworzeniu go za pomocą pthread_create()
	pthread_t t_id[4];

	//utworz watek Producera
	pthread_create(&(t_id[0]), NULL, Producer, (void*)0 );
	

	//utworz watki konsumentow
	pthread_create(&(t_id[1]), NULL, ConsumerA, (void*)1 );
	pthread_create(&(t_id[2]), NULL, ConsumerB, (void*)2 );
	pthread_create(&(t_id[3]), NULL, ConsumerC, (void*)3 );

	//czekaj na zakonczenie watkow
	int i;
	for ( i = 0; i < 4; i++){
    pthread_join(t_id[i],(void **)NULL);}
	return 0;
}
