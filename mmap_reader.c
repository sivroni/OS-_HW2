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
#define BYTE_SIZE 1
#define char_a "a"

// Signal handler.
// Simulate some processing and finish
void my_signal_handler (int signum) {
	int fd; // file to read from
	struct timeval t1, t2; // time measurment structure
	double elapsed_microsec; // measurment
	struct stat st;  // stat struct for file
	char * ptr_to_map; // will use during mmap
	int fileSize;
	int read_result; // read result
	char buffer[BYTE_SIZE]; /* string from input file */
	int counter; // count number of 'a' occurences
	char str_a [BYTE_SIZE]; // 'a' string

	// open file with permission 0600 = owner can read and write
	fd = open(FILEPATH, O_RDWR | O_CREAT, 0600);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		return -1;
	} 

	if (stat(FILEPATH, &st) < 0){
		close(fd);
		printf("Signal handle registration failed. %s\n",strerror(errno));
		return -1;
	}
        fileSize = st.st_size;
	
	// start time
	if (gettimeofday(&t1, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	// create mapping + set pointer to the start of file (offset = 0)
	ptr_to_map = (char*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (MAP_FAILED == ptr_to_map) { // mmap failed
		printf("Error mmapping the file: %s\n", strerror(errno));
		close(fd);
		return -1;
  	}
	strcpy(str_a, char_a); //TODO check return value
	count = 0;
	while (( read_result = read(fd,buffer, BYTE_SIZE) )>0){
		if (strcmp(buffer,char_a))
			count++;
	}
	
	if (read_result < 0){ // didnt exit loop because of EOF - error
		printf("Error reading file: %s\n", strerror(errno));
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

	// calculate elapsed time
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) * 1000.0;

	printf("Time elapsed is %f, and number of 'a' bytes in file are: %d", elapsed_microsec, count);
	
	if (unlink(FILEPATH) <0){
		printf("Error unlinking file from memory: %s\n", strerror(errno));
		munmap(ptr_to_map, NUM);
		close(fd);
		//TODO free mmap + close file (maybe delete?)
		return -1;
	}
	if (-1 == munmap(ptr_to_map, NUM)) {
		printf("Error un-mmapping the file: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	close(fd);
	exit(0);
}



// The flow:
// 1. register SIGUSR1 signal handler
// 2. go to infinite loop of printing a message every 2 seconds
int main (void){
	
	struct sigaction new_action; // Structure to pass to the registration syscall -used to change the action taken by a
	//process on receipt of a specific signal

	// Assign pointer to our handler function
	new_action.sa_handler = my_signal_handler;

	// Remove any special flag
	new_action.sa_flags = 0;

	// Register the handler
	if (0 != sigaction (SIGUSR1, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n",strerror(errno));
		return -1;
	}


	// Meditate untill killed
	while(1) {
		sleep(2);
		printf("Process runs.\n");
	}
	return 0;
}


