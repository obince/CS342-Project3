#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "sbmem.h"

int main(int argc, char const *argv[])
{
	if(argc != 2){
		fprintf(stderr, "Enter the segment size\n");
		return -1;
	}
	int segsize = atoi(argv[1]);
	if( segsize < 32768 || segsize > 262144){
		fprintf(stderr, "Wrong segment size\n");
		return -1;
	}
	
	sbmem_init(segsize);

   return (0);
	return 0;
}

