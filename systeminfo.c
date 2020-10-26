#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

#define BUFFER_SIZE 800
#define SYSTEMINFO 335

int main(int argc, char **argv){

	char buffer[BUFFER_SIZE];
	long int return_val = syscall(SYSTEMINFO, buffer, argv[1], argv[2], argv[3]);
	printf("%s", buffer);
	return return_val;

}
