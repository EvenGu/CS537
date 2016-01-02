// Do not modify this file. It will be replaced by the grading scripts
// when checking your project.

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
	printf(1, "%s", "\n********** Run ALL P3 tests ***********\n");
	char *argvv[2];
	argvv[1] = 0;
	
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "null";
		exec("null", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "null-syscall";
		exec("null-syscall", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "badaddress-syscall";
		exec("badaddress-syscall", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "small-stack";
		exec("small-stack", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "stack-growth";
		exec("stack-growth", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "fault";
		exec("fault", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "malloc";
		exec("malloc", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "overcommit-stack";
		exec("overcommit-stack", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	if(fork() == 0){
		argvv[0] = "overcommit-heap";
		exec("overcommit-heap", argvv);
	}
	wait();
	printf(1, "\n=======================================\n");
	printf(1, "\nP3 testing finished!\n");

	exit();
}
