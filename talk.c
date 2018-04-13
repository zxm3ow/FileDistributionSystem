#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	int fd = open("text2.txt", O_RDONLY);
	if (fd == -1) {
		perror("open failed");
		exit(1);
	}
	char buffer[1000];
	size_t len = 20;
	read(fd, buffer, len);
	buffer[len] = '\0';
	write(1, buffer, len+1);
	return 0;
}