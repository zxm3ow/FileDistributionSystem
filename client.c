#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int main(int argc, char **argv)
{
	int s;
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; /* IPv4 only */
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo("192.168.1.101", "1234", &hints, &result);
	if (s != 0) {
	        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        	exit(1);
	}
/*
	struct sockaddr_in localaddr;
	localaddr.ai_family = AF_INET;
	localaddr.ai_addr.s_addr = inet_addr("192.168.1.101");
	localaddr.ai_port = 1234;
*/
	if(connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1){
                perror("connect");
                exit(2);
        }

	//char *buffer = "This is Catherine talking. Hey Phoebe r u there?";

        // For this trivial demo just assume write() sends all bytes in one go and is not interrupted
	char filename[10];
	read(sock_fd, filename,10);
	int fd = open(filename, O_RDONLY);
	char r[20];
	read(fd, r, 19);
	r[20] = '\0'; 
	write(sock_fd, r, strlen(r));


    return 0;
}
