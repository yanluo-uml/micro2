// ioperm skeleton code

#include <stdio.h>
#include <unistd.h> // For ioperm
#include <sys/io.h> // For inb and outb

int main(int argc, char *argv[]){
	int result;

	/* Parallel I/O Ports:
	 *  Baseport = 0x378
	 * 	Data    = Base + 0
	 * 	Status  = Base + 1
	 * 	Control = Base + 2
	 */
	int baseport=0x378;
	int numports=3;

	// Request I/O resources
	result=ioperm(baseport, numports, 1);

	// Check for an error
	if(result){
		printf("Could not obtain ports.\n");
		perror("ioperm");
		return -1;
	}

	printf("Got the ports!\n");

	// Perform I/O here
	unsigned char value=0;
	outb(value,baseport+0); // Write byte to data register
	value=inb(baseport+1);  // Read byte from status register

	// Release Ports
	ioperm(baseport, numports, 0);
	return 0;
}


