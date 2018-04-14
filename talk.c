#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
/*
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	int s = getaddrinfo(NULL, "1234", &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(s));
		exit(1);
	}
	if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
		perror("bind");
		exit(1);
	}
	if (listen(sock_fd, 10) != 0) {
		perror("listen");
		exit(1);
	}
	client_fd = accept(sock_fd, NULL, NULL);
	if (client_fd == -1) {
		perror("accept");
		exit(1);
	}
*/


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