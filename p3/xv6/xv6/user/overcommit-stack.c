#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;
void recursive_stack_grow(int num);

int
main(int argc, char *argv[])
{
	int num = 0;
	printf(stdout, "Starting overcommit-stack.c TEST:\n");
	// Grow stack past the middle -> expect error //
	recursive_stack_grow(num);

	printf(stdout, "overcommit-stack.c test DONE\n");
	exit();
}

void
recursive_stack_grow(int num)
{
	int i;
	int large_array[PAGE];
	for(i = 0; i < (PAGE); i++)
		large_array[i] = i;
	// stop infinite loop //
	if(num > 2000)
		return;
	recursive_stack_grow(num++);
}
