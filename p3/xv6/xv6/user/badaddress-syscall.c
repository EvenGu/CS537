#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;

int
main(int argc, char *argv[])
{
	int file = 1;
	int n = 1024;

	printf(stdout, "Starting badaddress-syscall.c TEST:\n");
	// Pass an invalid address to a syscall //
	printf(stdout, "Calling read(1,(char*)0x9000, 1024) expect no error\n");
	read(file, (char*)0x9000, n);

	printf(stdout, "badaddress-syscall.c test DONE\n");
	exit();
}
