#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct Petersons_Variables {
	int turn;
	int flag[2]; 
} pvar;
int nloop = 50;

/**********************************************************
 * Function: increment a counter by some amount one by one **
 * argument: ptr (address of the counter), increment       *
 * output  : nothing                                       *
 ***********************************************************/
void add_n(int *ptr, int increment) {
	int i,j;
	for (i=0; i < increment; i++) {
		*ptr = *ptr + 1;
		for (j=0; j < 1000000;j++);
	}
}

void initialize (pvar *v) {
	v->flag[0] = 0;
	v->flag[1] = 0;
}
int main(){
	int pid;
	int *countptr;

	int fd;
	int zero = 0; 

	system("rm -f counter");

	fd = open("counter",O_RDWR | O_CREAT);
	write(fd,&zero,sizeof(int));

	countptr = (int *) mmap(NULL, sizeof(int),PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);

	if (!countptr) {
		printf("Mapping failed\n");
		exit(1);
	}
	*countptr = 0;

	close(fd);
	
	system("rm -f mfile"); 
	int mdata;
	pvar *variables;
	mdata = open("mfile", O_RDWR | O_CREAT);
	write(mdata,&zero,sizeof(pvar));

	variables  = (pvar *) mmap(NULL, sizeof(pvar), PROT_READ | PROT_WRITE, MAP_SHARED, mdata, 0);

	if(!variables) {
		printf("Unable to map variables");
		exit(1);
	}
	initialize(variables);
	close(mdata);
	pid = fork();
	if (pid == 0) {
		while(1) {
			variables->flag[0] = 1;
			variables->turn = 1;

			while(variables->flag[1] == 1 && variables->turn == 1);
			if (*countptr < nloop) {
				add_n(countptr,2);
				printf("Child process -->> counter = %d\n",*countptr);
			}
			else {
				variables->flag[0] = 0;
				close(mdata);
				close(fd);
				break;
			}
 
			variables->flag[0] = 0;
			close(mdata);
			close(fd);

		}			
	}
	else {
		while(1) {
			variables->flag[1] = 1;
			variables->turn = 0;

			while(variables->flag[0] == 1 && variables->turn == 0);

			if (*countptr < nloop) {
				add_n(countptr,20);
				printf("Parent process -->> counter = %d\n",*countptr);
			}
			else {
				variables->flag[1] = 0;
				close(mdata);
				close(fd);
				break;
			}

			variables->flag[1] = 0;
			close(mdata);
			close(fd);
		}
	}
}