#include <stdio.h>
#include <stdlib.h>
int f(){
	return rand();
}
void lock(){}
void unlock(){}
void h1(){
	lock();
	unlock();
}
void h2(){

}
void h3(){
   	unlock();
}
void g(){
	lock();
	int j = f();

	if(j > 10){
		h1();
		unlock();
	}
	else{
		h2();
	}
	
	return;
}

int main(){
	g();

	return 0;
}
