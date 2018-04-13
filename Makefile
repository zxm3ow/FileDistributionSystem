all:
	gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
server:
	gcc server.c -o server