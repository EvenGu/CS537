#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;

int
main(int argc, char *argv[])
{
	int a = 0;
	int b = 1;
	int c = 2;
	int d = 3;
	int f = 4;
	char buf[32] = "short string of 32 bytes\n";
	printf(stdout, "Starting small-stack.c TEST:\n");
	printf(stdout, "a @%x\n", &a);
	printf(stdout, "b @%x\n", &b);
	printf(stdout, "c @%x\n", &c);
	printf(stdout, "d @%x\n", &d);
	printf(stdout, "f @%x\n", &f);
	printf(stdout, "a @%x\n", &buf);
	// R/W variables from 1 page of the stack //
	c = a + b;
	d = f * c;
	printf(stdout, buf);

	printf(stdout, "small-stack.c test DONE\n");
	exit();
}
