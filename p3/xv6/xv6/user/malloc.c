#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;

int
main(int argc, char *argv[])
{
	int i;
	int* array;
	printf(stdout, "Starting malloc.c TEST:\n");
	// Allocate memory using malloc(), ensure access //
	array = malloc(PAGE * 4);
	for(i = 0; i < PAGE; i++){
		array[i] = i;
		printf(stdout, "array[%d]=%d\t\t@%x\n", i,array[i], &array[i]);
	}
	
	printf(stdout, "malloc.c test DONE\n");
	exit();
}
