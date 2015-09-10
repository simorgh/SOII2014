#include <unistd.h>
#include <stdio.h>

int  main(){
	int nproc = sysconf(_SC_NPROCESSORS_ONLN);
	printf("\tNCORES\t %d\n", nproc);

	return 0;
}
