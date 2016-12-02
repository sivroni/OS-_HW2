#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

#define FILEPATH "/tmp/mmapped.bin"

int main(int argc, char *argv[]) {
	char * ptr_to_map; // will use during mmap
	int fd; // write to file
	int result; // for lseek
	struct timeval t1, t2; // time measurment structure
	double elapsed_microsec; // measurment
	int i; // for loop index

	if (argc != 3){ // check valid number of arguments
		printf("Not enough arguments eneterd. Exiting...\n");
		return -1;
	}
	return 0;
	
	int NUM = atoi(argv[1]); // number of bytes to transfer
	int RPID = atoi(argv[2]); // process id thats running mmap_reader
	
	// open file with permission 0600 = owner can read and write
	fd = open(FILEPATH, O_RDWR | O_CREAT, 0600);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		return -1;
	}
	// Force the file to be of the same size as the (mmapped) array
	result = lseek(fd, NUM-1, SEEK_SET);
	if (-1 == result) {
		printf("Error calling lseek() to 'stretch' the file: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	// Something has to be written at the end of the file,
	// so the file actually has the new size. 
	result = write(fd, "", 1);
	if (1 != result) {
		printf("Error writing last byte of the file: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	// create mapping + set pointer to the start of file (offset = 0)
	ptr_to_map = (char*) mmap(NULL, NUM, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (MAP_FAILED == ptr_to_map) { // mmap failed
		printf("Error mmapping the file: %s\n", strerror(errno));
		close(fd);
		return -1;
  	}

	// start time
	if (gettimeofday(&t1, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		munmap(ptr_to_map, NUM);
		close(fd);
		return -1;
	}
	// now write to the file as if it were memory 'a' X (NUM-1)
	for (i = 0; i < NUM-1 ; ++i) {
		ptr_to_map[i] = 'a';
   
	}
	ptr_to_map[NUM-1] = '\0'; // last char is NULL
	
	if (kill(RPID, SIGUSR1) <0){
		printf("Error killing the reader process: %s\n", strerror(errno));
		munmap(ptr_to_map, NUM);
		close(fd);
		//TODO free mmap + close file (maybe delete?)
		return -1;
	}

	// finish time
	if (gettimeofday(&t2, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		munmap(ptr_to_map, NUM);
		close(fd);
		//TODO free mmap + close file (maybe delete?)
		return -1;
	}
	
	// don't forget to free the mmapped memory
	// this also ensures the changes commit to the file
	if (-1 == munmap(ptr_to_map, NUM)) {
		printf("Error un-mmapping the file: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);

	// calculate elapsed time
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) * 1000.0;

	printf("Time elapsed is %f, and number of bytes written are: %d", elapsed_microsec, NUM);

	return 0;
}

