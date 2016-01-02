#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;

int
main(int argc, char *argv[])
{
	printf(stdout, "Starting null-syscall.c TEST:\n");
	// Pass a null pointer to system call of choice //
	printf(stdout, "Running: read(0,0,0) expect no errors\n");
	read(0,0,0);

	printf(stdout, "null-syscall.c test DONE\n");
	exit();
}
