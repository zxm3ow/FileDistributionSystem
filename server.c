#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
/*
int main(int argc, char **argv)
{
struct sockaddr_rc loc_addr = { 0 }, rem_addr = { 0 };
char buf[1024] = { 0 };
int s, client, bytes_read;
int opt = sizeof(rem_addr);
// allocate socket
s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
// bind socket to port 1 of the first available
// local bluetooth adapter
loc_addr.rc_family = AF_BLUETOOTH;
loc_addr.rc_bdaddr = *BDADDR_ANY;
loc_addr.rc_channel = (uint8_t) 1;
bind(s, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
// put socket into listening mode
listen(s, 1);
// accept one connection
client = accept(s, (struct sockaddr *)&rem_addr, &opt);
ba2str( &rem_addr.rc_bdaddr, buf );
fprintf(stderr, "accepted connection from %s\n", buf);
memset(buf, 0, sizeof(buf));
// read data from the client
bytes_read = read(client, buf, sizeof(buf));
if( bytes_read > 0 ) {
printf("received [%s]\n", buf);
}
// close connection
close(client);
close(s);
return 0;
}*/

// TCP
int main(int argc, char **argv)
{
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
int client_fd = accept(sock_fd, NULL, NULL);
if (client_fd == -1) {
	perror("accept");
	exit(1);
}
char buffer[1000];
int len = read(client_fd, buffer, sizeof(buffer)-1);
buffer[len] = '\0';
printf("%s\n", buffer);


return 0;
}