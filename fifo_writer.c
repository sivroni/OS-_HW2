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

#define FILEPATH "/tmp/osfifo" 
#define PERMISSION 0600
#define char_a "a"
#define BYTE_SIZE 1

int main(int argc, char *argv[]){
	int fd; // create pipe file
	struct timeval t1, t2; // time measurment structure
	double elapsed_microsec; // measurment
	int i; // for loop index
	char str_a [BYTE_SIZE]; // 'a' string

	signal(SIGINT, SIG_IGN); // ignore SIGINT during operation

	if (argc != 2){ // check valid number of arguments
		printf("Not enough arguments eneterd. Exiting...\n");
		return -1;
	}

	int NUM = atoi(argv[1]); // number of bytes to transfer

	
	//create pipe 
	if ( mkfifo(FILEPATH, PERMISSION) < 0){
		printf("Error creating fifo file: %s\n", strerror(errno));
		return -1;
	}
	
	// open file - writing only
	fd = open(FILEPATH, O_WRONLY | O_CREAT);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		//TODO delete fifo?
		return -1;
	}
	
	// start time
	if (gettimeofday(&t1, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		//TODO delete fifo?
		close(fd);
		return -1;
	}

	strcpy(str_a, char_a); 

	// now write to the file as if it were memory 'a' X (NUM)
	for (i = 0; i < NUM ; ++i) {
		if ( write(fd, str_a, BYTE_SIZE) < 0 ){
			printf("Error writing to pipe file: %s\n", strerror(errno));
			//TODO delete fifo?
			close(fd);
			return -1;
		}
   
	}
	printf("After loop\n");

	// finish time
	if (gettimeofday(&t2, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		//TODO delete fifo?
		close(fd);
		return -1;
	}

	// calculate elapsed time
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) * 1000.0;

	printf("WRITER: Time elapsed is %f, and number of bytes written are: %d\n", elapsed_microsec, NUM);

	if (unlink(FILEPATH) < 0){
		printf("Error unlinking file from memory: %s\n", strerror(errno));
		//TODO delete fifo?
		close(fd);
		return -1;
	}
	//TODO what to de during cleanup?
	close(fd);
	signal(SIGINT, SIG_DFL); // restore SIGINT in cleanup
	exit(0);
}

