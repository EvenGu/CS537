#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

int
main(int argc, char *argv[])
{
	int fd;
	struct stat st; // since it 'may' not be initialized
	
	if(argc < 2){
		printf(1, "Please specify a pathname\n");
		exit();
	}

	fd = open((char*)argv[1], 0);

	if(fstat(fd, &st) < 0) {
		printf(1, "Something went wrong with retrieving file status\n");
		close(fd);
		exit();
	}

	if(st.type == T_CHECKED)
		printf(1, "Type: %d\tSize: %d\tChecksum: %d\n", st.type, st.size, st.checksum);
	else
		printf(1, "Type: %d\tSize: %d\tChecksum: %d\n", st.type, st.size, 0);

	close(fd);
  exit();
}
