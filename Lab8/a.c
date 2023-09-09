#include "stdio.h"


int main(){


	int  fat[512 / sizeof(int)];
	printf("sizeof int %ld\n",sizeof(int));
	printf("len is %ld\n",sizeof(fat));


	return 0;
}
