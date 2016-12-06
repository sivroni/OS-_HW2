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
#define END_BYTE '\0'
#define BUFF_SIZE 2048

int main(int argc, char *argv[]){
	int fd; // create pipe file
	struct timeval t1, t2; // time measurment structure
	double elapsed_microsec; // measurment
	int i; // for loop index
	int write_result;
	char buffer[BUFF_SIZE]; // string containing 'a'-s
	int iterations;
	//signal(SIGINT, SIG_IGN); // ignore SIGINT during operation

	if (argc != 2){ // check valid number of arguments
		printf("Not enough arguments eneterd. Exiting...\n");
		exit(-1);
	}

	int NUM = atoi(argv[1]); // number of bytes to transfer

	// create a size NUM buffer
/*	char buffer [NUM];
	for (i=0; i< NUM-1; i++){
		buffer[i] = char_a;
	}
		
	buffer[NUM-1] = END_BYTE;*/
	

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
	if (gettimeofday(&t1, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		unlink(FILEPATH);
		exit(errno);
	}


//TODO change to one write!!
	// now write to the file as if it were memory 'a' X (NUM)
/*	for (i = 0; i < NUM ; ++i) {
		if ( write(fd, str_a, BYTE_SIZE) < 0 ){
			printf("Error writing to pipe file: %s\n", strerror(errno));
			//TODO delete fifo?
			close(fd);
			unlink(FILEPATH);
			exit(errno);
		}
   
	}*/
	int bytesLeftToWrite = NUM;
	while ( bytesLeftToWrite > 0 ){
		
		for (i = 0; i < BUFF_SIZE; i++){ // // fill buffer with 'a'-s
			buffer[i] = char_a;		
		}

		if( (write_result = write(fd,buffer, BUFF_SIZE) ) > 0 ){
			bytesLeftToWrite = bytesLeftToWrite - write_result;
		}
		else 
			break;
	}
	buffer[write_result-1] = END_BYTE;

// len = read
//v counter +len = how much i succeed
//
/*	if ( write(fd, buffer, NUM) < 0 ){
			printf("Error writing to pipe file: %s\n", strerror(errno));
			close(fd);
			unlink(FILEPATH);
			exit(errno);
		}*/

	// finish time
	if (gettimeofday(&t2, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		unlink(FILEPATH);
		exit(errno);
	}

	// calculate elapsed time
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) * 1000.0;

	printf("%d were written in %f microseconds through FIFO\n", NUM, elapsed_microsec);

	close(fd);
	if (unlink(FILEPATH) < 0){
		printf("Error unlinking file from memory: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}
	
	//signal(SIGINT, SIG_DFL); // restore SIGINT in cleanup
	
	exit(0);

}

