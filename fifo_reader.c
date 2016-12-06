#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>

#define FILEPATH "/tmp/osfifo" 
#define char_a 'a'
#define PERMISSION 0600
#define SECONDS_TO_WAIT 4
#define BUFF_SIZE 2048

int main(void){
	int fd; // file
	struct timeval t1_reader, t2_reader; // time measurment structure
	int count_reader; // count number of 'a' occurences
	double elapsed_microsec; // measurment
	char buffer[BUFF_SIZE]; // string containing 'a'-s
	//int count; // count number of 'a' occurences
	int read_result; // for read func - return value
	int i;
	struct sigaction sa, prev; // to handle SIGINT	
	
	// ignore SIGINT + save previous signals
	sa.sa_handler = SIG_IGN;
	if (sigaction(SIGINT, &sa, &prev) == -1) {
		printf("Error while defining SIGINT: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	
	sleep(SECONDS_TO_WAIT); // in order for fifo_writer to mkfifo!

	// open file - reading only
	fd = open(FILEPATH, O_RDONLY );
	if (-1 == fd) {
		printf("Error opening file for reading: %s\n", strerror(errno));
		exit(errno);
	}

	printf("after 'reading'\n");
	// start time
	if (gettimeofday(&t1_reader, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	count_reader = 0;
	
	while (( read_result = read(fd,buffer, BUFF_SIZE) ) > 0 ){
		for (i = 0; i < read_result; i++){ // for each byte read - if its 'a' then count_reader ++
			if ( buffer[i] == char_a ){
				count_reader++;
			}
			//if (i==3) exit(0);//TODO for creating SIGPIPE
		}
	}

	//TODO for creating SIGINT manually
	//printf("now you can SIGINT the program!!\n");
	//sleep(SECONDS_TO_WAIT); 

	if (read_result < 0){ // didnt exit loop because of EOF - indicates error
		printf("Error reading file: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	// finish time
	if (gettimeofday(&t2_reader, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		exit(errno);

	}
	
	// calculate elapsed time
	elapsed_microsec = (t2_reader.tv_sec - t1_reader.tv_sec) * 1000.0;
	elapsed_microsec += (t2_reader.tv_usec - t1_reader.tv_usec) * 1000.0;
	
	printf("%d were read in %f microseconds through FIFO\n", count_reader, elapsed_microsec);
	
	

	// set signal to prev
	if (sigaction(SIGINT, &prev, NULL) == -1) {
		printf("ERROR: implementing SIGINT, strerror: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	close(fd);
	
	exit(0);

}

