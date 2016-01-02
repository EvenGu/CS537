// include //
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>
// static variables //

// usage //
void usage(char *prog)
{
	fprintf(stderr, "Usage: %s -i inputfile -o outputfile -l lowvalue -h highvalue\n", prog);
	exit(1);
}

// main //
int main(int argc, char *argv[])
{
	// variables //
	int check_arg;
	char *inFile = NULL;
	char *outFile = NULL;
	unsigned int low_value = 0;
	unsigned int high_value = 0;

	// check parameters //
	if(argc < 9)
		usage(argv[0]);

	// I am assuming correct argument types have been passed in...
	// C isn't very type check friendly
	while((check_arg = getopt(argc, argv, "i:o:l:h:")) != -1) 
	{
		switch(check_arg)
		{
			case 'i':
				inFile = strdup(optarg);
				break;
			case 'o':
				outFile	= strdup(optarg);
				break;
			case 'l':
				low_value = atoi(optarg);
				break;
			case 'h':
				high_value = atoi(optarg);
				break;
		}
	}
	// create output file //
	int f_out = open(outFile, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);
	if(f_out < 0)
	{
		fprintf(stderr, "*** ERROR *** Failed to create/open output file!\n");
		exit(1);
	}

	// read in file //
	int f_in = open(inFile, O_RDONLY);
	if(f_in < 0)
	{
		fprintf(stderr, "*** ERROR *** %i Failed to open input file!\n", f_in);
		exit(1);
	}
	// get size of data //
	struct stat fileStat;
	if(fstat(f_in,&fileStat) < 0)
		exit(1);
	int fileSize;
	fileSize = (int)fileStat.st_size;
	rec_t *origin;
	origin = (rec_t*) malloc(fileSize);

	int index = 0;
	rec_t record; // Record struct from sort.h
	while(1)
	{
		int read_c;
		read_c = read(f_in, &record, sizeof(rec_t));
		if(read_c == 0)
			break; // EOF reached
		if(read_c < 0)
		{
			fprintf(stderr, "*** ERROR *** Read error occured\n");
			exit(1);
		}
		
		// exclude keys out of our range from the get go //
		if(record.key < low_value || record.key > high_value)
		{
			continue;
		}
		else
		{			
			memcpy((origin + index), &record, sizeof(rec_t));
			index++;
		}
	}

	// sort //
	int k, j;
	for(k = 0; k < index; k++)
	{
		rec_t r;
		r = origin[k];
		for(j = 0; j < index-1; j++)
		{
			if(origin[j].key > origin[j+1].key)
			{
				rec_t temp = origin[j+1];
				origin[j+1] = origin[j];
				origin[j] = temp;
			}
		}
	}

	// print sorted //
	for(k = 0; k < index; k++)
	{
		rec_t r;
		r = origin[k];
		int write_c = write(f_out, &r, sizeof(rec_t));
		if(write_c != sizeof(rec_t))
			exit(1);
	}
	
	(void) close(f_in);
	(void) close(f_out);
	free(origin);
	return 0;	
}
