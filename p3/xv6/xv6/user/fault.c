#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;

int
main(int argc, char *argv[])
{
	char* x;
	x = "a";
	printf(stdout, "Starting fault.c TEST:\n");
	// Access invalid address (outside stack/heap) //
	printf(stdout, "Original x = \'%c\' @%x\n", *x, x);

	x = (char *)0x9D70C; //example bad address
	printf(stdout, "Changed x to @%x expect fault next\n", x);	
	printf(stdout, "Value of x is %c\n", *x); //should fault...
	
	printf(stdout, "fault.c test DONE\n");
	exit();
}
