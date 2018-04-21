#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main() {
/*	int fd = open("text2.txt", O_RDONLY);
	if (fd == -1) {
		perror("open failed");
		exit(1);
	}
	FILE *filep = fdopen(fd, "r");
	printf("current position:%lu\n", ftell(filep));
	fseek(filep, 5, SEEK_END);
	printf("current position:%lu\n", ftell(filep));
	char buffer[1024];
	memset(buffer, 0, 1024);
	int len = 10;
	int r = read(fd, buffer, (size_t)len);
	//close(fd);
	printf("read bytes!:%d\n", r);
	//buffer[len] = '\0';
	//printf("%s\n", buffer);
	write(1, buffer, 50);   
	int fd2 = open("out.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXG | S_IRWXU | S_IRWXO);
	dprintf(fd2, "%s\n", buffer); 
	close(fd2);
	return 0;*/

	int fd = open("text2.txt", O_CREAT | O_TRUNC | O_WRONLY);
	if (fd == -1) {
		perror("open failed");
		exit(1);
	}
	char *buffer = "Hi this is some contents from local ra";
	int len = 38;
	int r = write(fd, buffer, (size_t)len);
	//close(fd);
	printf("wrote bytes!:%d\n", r);
	return 0;
}