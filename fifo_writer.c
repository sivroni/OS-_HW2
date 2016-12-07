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
#define char_a 'a'
#define END_BYTE "\0"
#define BUFF_SIZE 2048

// Global variables:
int fd; // create pipe file
struct timeval t1_writer, t2_writer; // time measurment structure
int counter_writer; // how many bytes did we red until now

// headers:
void pipe_handler(int);

// signal handler for sigpipe - exit gracefully + cleanup !
void pipe_handler(int signal) {
	if (signal == SIGPIPE) {
		double elapsed_microsec_r;
		double elapsed_microsec_w;
		printf("Handling SIGPIPE and exiting program: %s\n", strerror(errno));
		// PRINTING:	
		// calculate writer:
		if (gettimeofday(&t2_writer, NULL) <0){
			printf("Error starting time: %s\n", strerror(errno));
		}
		elapsed_microsec_w = (t2_writer.tv_sec - t1_writer.tv_sec) * 1000.0;
		elapsed_microsec_w += (t2_writer.tv_usec - t1_writer.tv_usec) * 1000.0;
		printf("%d were written in %f miliseconds through FIFO\n", counter_writer, elapsed_microsec_w);

		// cleanup
		close(fd);
		if (unlink(FILEPATH) < 0)
			printf("Error unlinking file from memory: %s\n", strerror(errno));
		exit(0);
	}
}

int main(int argc, char *argv[]){
	double elapsed_microsec; // measurment
	int i; // for loop index
	int write_result; // result from write function
	char buffer[BUFF_SIZE]; // string to write in fd
	char last[1] = END_BYTE; // last byte in fd
	struct sigaction sa; // to handle SIGINT, SIGPIPE
	int bytesLeftToWrite;
	int toWrite; // how much to write in current iteration
	printf("enters writer...\n");
	sa.sa_handler = pipe_handler;
	// Signals - first entered here
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		printf("Error while defining SIGINT: %s\n", strerror(errno));
		exit(errno);
	}
	// SIGPIPE = the file has no reader
	if (sigaction(SIGPIPE, &sa, NULL) == -1) {
		printf("Error while defining SIGPIPE: %s\n", strerror(errno));
		exit(errno);
	}

	if (argc != 2){ // check valid number of arguments
		printf("Not enough arguments eneterd. Exiting...\n");
		exit(-1);
	}

	int NUM = atoi(argv[1]); // number of bytes to transfer

	//create pipe 
	if ( mkfifo(FILEPATH, PERMISSION) < 0){
		printf("Error creating fifo file: %s\n", strerror(errno));
		exit(errno);
	}
	
	// open file - writing only
	fd = open(FILEPATH, O_WRONLY);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		unlink(FILEPATH);
		exit(errno);
	}
	
	// start time
	if (gettimeofday(&t1_writer, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		unlink(FILEPATH);
		exit(errno);
	}


	bytesLeftToWrite = NUM;
	toWrite = 0;
	counter_writer = 0;
	while ( bytesLeftToWrite > 0 ){
		
		if (bytesLeftToWrite<BUFF_SIZE){ // last buffer to write
			toWrite = bytesLeftToWrite;
			
		}
		else{
			toWrite = BUFF_SIZE;
		}

		for (i = 0; i < toWrite; i++){ // fill buffer with 'a'-s
			buffer[i] = char_a;		
		}

		if( (write_result = write(fd,buffer, toWrite) ) > 0 ){
			bytesLeftToWrite = bytesLeftToWrite - write_result;
		}
		if (bytesLeftToWrite<BUFF_SIZE){ // last buffer to write
			write_result = write(fd,last, 1);
		}
		counter_writer = counter_writer + toWrite;
		
	}
		
	// check for errors in writing
	if (write_result < 0){
		printf("Error while writing: %s\n", strerror(errno));
		close(fd);
		unlink(FILEPATH);
		exit(errno);
	}


	// finish time
	if (gettimeofday(&t2_writer, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		unlink(FILEPATH);
		exit(errno);
	}

	// calculate elapsed time
	elapsed_microsec = (t2_writer.tv_sec - t1_writer.tv_sec) * 1000.0;
	elapsed_microsec += (t2_writer.tv_usec - t1_writer.tv_usec) * 1000.0;

	printf("%d were written in %f miliseconds through FIFO\n", counter_writer, elapsed_microsec);

	close(fd);
	if (unlink(FILEPATH) < 0){ 
		printf("Error unlinking file from memory: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}
	
	
	exit(0);

}

