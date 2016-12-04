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
#define char_a 'a'
#define NULL_BYTE '\0'
#define PERMISSION 0600
#define BYTE_OFFSET 0
#define SECONDS_TO_SLEEP 2

/////// headers: ////////
 void my_signal_handler (int);

////////////////////////

// Signal handler.
// Simulate some processing and finish
void my_signal_handler (int signum) {
	int fd; // file to read from
	struct timeval t1, t2; // time measurment structure
	double elapsed_microsec; // measurment
	struct stat st;  // stat struct for file
	char * ptr_to_map; // will use during mmap
	int fileSize; // determine file size using stat
	int read_result; // read result
	char buffer[BYTE_SIZE]; /* string from input file */
	int count; // count number of 'a' occurences
	char str_a [BYTE_SIZE]; // 'a' string
	int i;
	
	// open file with permission 0600 = owner can read and write
	fd = open(FILEPATH, O_RDWR | O_CREAT);
	if (-1 == fd) {
		printf("Error opening file for writing: %s\n", strerror(errno));
		exit(errno);
	}
 
	if (chmod(FILEPATH, PERMISSION) <0 ){ //TODO maybe unnecessery?
		printf("Error while changing permissions: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	if (stat(FILEPATH, &st) < 0){
		close(fd);
		printf("Signal handle registration failed. %s\n",strerror(errno));
		exit(errno);
	}

        fileSize = st.st_size;
	
	// start time
	if (gettimeofday(&t1, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	// create mapping + set pointer to the start of file (offset == 0)
	ptr_to_map = (char*) mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BYTE_OFFSET);

	if (MAP_FAILED == ptr_to_map) { // mmap failed
		printf("Error mmapping the file: %s\n", strerror(errno));
		close(fd);
		exit(errno);
  	}
	
	// count number of 'a' bytes written
	count = 0;
	i=0;

	while (( read_result = ptr_to_map[i] ) != NULL_BYTE ){ //TODO read from map!!
		if ( ptr_to_map[i] == char_a ){
			count++;
			i++;
		}
	}
	count++; // for NULL byte - terminator
	

	// finish time
	if (gettimeofday(&t2, NULL) <0){
		printf("Error starting time: %s\n", strerror(errno));
		munmap(ptr_to_map, fileSize);
		close(fd);
		exit(errno);
	}

	// calculate elapsed time
	elapsed_microsec = (t2.tv_sec - t1.tv_sec) * 1000.0;
	elapsed_microsec += (t2.tv_usec - t1.tv_usec) * 1000.0;

	printf("READER: Time elapsed is %f, and number of 'a' bytes in file are: %d\n", elapsed_microsec, count);
	
	if (unlink(FILEPATH) <0){
		printf("Error unlinking file from memory: %s\n", strerror(errno));
		munmap(ptr_to_map, fileSize);
		close(fd);
		exit(errno);
	}
	//TODO check if the following deleted section is necessary

	if (-1 == munmap(ptr_to_map, fileSize)) {
		printf("Error un-mmapping the file: %s\n", strerror(errno));
		close(fd);
		exit(errno);
	}

	close(fd);
	signal(SIGTERM, SIG_DFL); // restore SIGTERM in cleanup
	exit(0);
}



// The flow:
// 1. register SIGUSR1 signal handler
// 2. go to infinite loop of printing a message every 2 seconds
int main (void){
	// Structure to pass to the registration syscall - 
	// used to change the action taken by a process on receipt of a specific signal
	struct sigaction new_action; 

	// Assign pointer to our handler function
	new_action.sa_handler = my_signal_handler;

	// Remove any special flag
	new_action.sa_flags = 0;

	// Register the handler
	if (0 != sigaction (SIGUSR1, &new_action, NULL)) {
		printf("Signal handle registration failed. %s\n",strerror(errno));
		exit(errno);
	}

	signal(SIGTERM, SIG_IGN); // ignore SIGTERM during operation

	// Meditate untill killed
	while(1) {
		sleep(SECONDS_TO_SLEEP);
		
	}
	printf("Exiting mmap_reader... \n");
	exit(0);
}


