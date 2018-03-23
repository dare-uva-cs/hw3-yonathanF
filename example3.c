#include <stdio.h>
#include <stdlib.h>

int f1(){ }

int f2(int n){ 
	if(n == 0){
		return;
	}
	f2(n-1);
}

int main(){
	int a = rand();
	if(a > 10000){
		f1();
	} else {
		f2(2);
	}
	return 0;
}