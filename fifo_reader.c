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

#define FILEPATH "/tmp/osfifo10" 
#define char_a "a"
#define PERMISSION 0600
#define BYTE_SIZE 1

int main(void){
	int fd; // file
	struct timeval t1, t2; // time measurment structure
	double elapsed_microsec; // measurment
	char buffer[BYTE_SIZE]; /* string from input file */
	int count; // count number of 'a' occurences
	char str_a [BYTE_SIZE]; // 'a' string
	int read_result; // for read func - return value

	//signal(SIGINT, SIG_IGN); // ignore SIGTERM during operation

	// open file - reading only
	fd = open(FILEPATH, O_RDONLY );
	if (-1 == fd) {
		printf("Error opening file for reading: %s\n", strerror(errno));
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
	count = 0;
	while (( read_result = read(fd,buffer, BYTE_SIZE) ) >0 ){
		if (strcmp(buffer,char_a))
			count++;
	}

	if (read_result < 0){ // didnt exit loop because of EOF - indicates error
		printf("Error reading file: %s\n", strerror(errno));
		//TODO delete fifo?
		close(fd);
		return -1;
	}

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

	printf("READER: Time elapsed is %f, and number of 'a' bytes in file are: %d\n", elapsed_microsec, count);

	//signal(SIGINT, SIG_DFL); // restore SIGINT in cleanup

	close(fd);
	
	//return 0;
	exit(0);

}
/*

*/
