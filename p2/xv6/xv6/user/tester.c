// Do not modify this file. It will be replaced by the grading scripts
// when checking your project.

#include "types.h"
#include "stat.h"
#include "pstat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"

int stdout = 1;

int
main(int argc, char *argv[])
{	
	int fd;
	int i;
	int x = 0;
	fd = open("file", 0);

	if(argc < 2)
		exit();

	for(i = 0; i < atoi(argv[1]); i++)
	{
		x = i;

		if(x == 500000 && argc >= 3){
			printf(stdout, "forking\n");
			fork();
		}
	}

	close(fd);
	exit();
	return x;
}
