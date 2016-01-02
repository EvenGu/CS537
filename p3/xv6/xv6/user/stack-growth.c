#include "types.h"
#include "stat.h"
#include "user.h"

#define PAGE (4096)
#define MAX_PROC_MEM (640 * 1024)

int stdout = 1;
int a(int num);
int b(int num);
int c(int num);
int d(int num);

int
main(int argc, char *argv[])
{
	int first[512];
	int i;
	printf(stdout, "Starting stack-growth.c TEST:\n");
	// Nested functions each with < 1 page, sum 1 < page //
	for(i = 511; i >= 0; i--){
		first[i] = i;
		printf(stdout, "first[%d]\t @%x\n", i, &first[i]);
	}
	a(1);
	printf(stdout, "stack-growth.c test DONE\n");
	exit();
}

int
a(int num)
{
	int a[512];
	int i;
	for(i = 511; i >= 0; i--){
		a[i] = i;
		printf(stdout, "a[%d]\t @%x\n", i, &a[i]);
	}

	b(2);
	return 0;
}

int
b(int num)
{
	int b[512];
	int i;
	for(i = 511; i >= 0; i--){
		b[i] = i;
		printf(stdout, "b[%d]\t @%x\n", i, &b[i]);
	}

	c(3);
	return 0;
}

int
c(int num)
{
	int c[512];
	int i;
	for(i = 511; i >= 0; i--){
		c[i] = i;
		printf(stdout, "c[%d]\t @%x\n", i, &c[i]);
	}

	d(4);
	return 0;
}

int
d(int num)
{
	int d[512];
	int i;
	for(i = 511; i >= 0; i--){
		d[i] = i;
		printf(stdout, "d[%d]\t @%x\n", i, &d[i]);
	}

	return 0;
}
