#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>

int main(int argc, char *argv[]){
	FILE *f;
	int timeout;
	struct pollfd mypollfd;

	if(argc!=3){
		printf("Usage: %s device timeout\n", argv[0]);
		return -1;
	}

	f=fopen(argv[1], "r+");
	if(!f){
		perror("fopen");
		return (errno);
	}

	timeout=atoi(argv[2]);
	mypollfd.fd=fileno(f);
	mypollfd.events=POLLRDBAND;
	mypollfd.revents=0;
	
	int result=poll(&mypollfd, 1, timeout);

	if(result){
		printf("Interrupt!\n");
	}
	if(!result){
		printf("No interrupt.\n");
	}
	
	fclose(f);


	return 0;
}
