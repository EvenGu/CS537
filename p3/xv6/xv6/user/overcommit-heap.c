#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;
void recursive_heap_grow(int num);

int
main(int argc, char *argv[])
{
	int num = 0;
	printf(stdout, "Starting overcommit-heap.c TEST:\n");
	// Overcommit heap and expect error // 
	recursive_heap_grow(num);

	printf(stdout, "overcommit-heap.c test DONE\n");
	exit();
}

void
recursive_heap_grow(int num)
{
	int i;
	int* space = malloc(PAGE);
	for(i = 0; i < PAGE/4; i++)
		space[i] = i;
	// stop infinite loop //
	if(num > 2000)
		return;
	recursive_heap_grow(num++);
}
