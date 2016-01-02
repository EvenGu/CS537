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
	x = "f";
	printf(stdout, "Starting null.c TEST:\n");
	// Dereference a null pointer //
	char *c = 0;
	printf(stdout, "Non-null character %c at 0x%x\n", *x, x);
	printf(stdout, "Expect an error after DONE for SUCCESS\n");
	printf(stdout, "null.c test DONE\n");
	printf(stdout, "Null character %c at 0x%x\n", *c, c);

	printf(stdout, "null.c test FAILED\n");
	exit();
}
