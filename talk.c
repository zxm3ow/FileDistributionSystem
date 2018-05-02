#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main() {
	int fd = open("text1.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXG | S_IRWXU | S_IRWXO);
	if (fd == -1) {
		perror("open failed");
		exit(1);
	}

	char buffer[1024];
	memset(buffer, 0, 1024);
	sprintf(buffer, "%s", "This is file 1\n");
	size_t len = 16;
	int r = write(fd, buffer, len);
	
	int fd2 = open("text2.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXG | S_IRWXU | S_IRWXO);
	sprintf(buffer, "%s", "This is file 2\n");
	r = write(fd2, buffer, len);

	FILE *stream1 = fdopen(fd, "w+");
	fseek(stream1, 0, SEEK_SET);
	FILE *stream2 = fdopen(fd2, "w+");
	fseek(stream2, 0, SEEK_SET);
	memset(buffer, 0, 1024);
	r = read(fd, buffer, len);
	buffer[len] = 0;
	printf("first read:%s\n", buffer);
	r = read(fd2, buffer, len);
	buffer[len] = 0;
	printf("second read:%s\n", buffer);

	return 0;

}