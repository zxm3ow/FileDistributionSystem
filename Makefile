all:
	gcc -Wall myfs.c `pkg-config fuse --cflags --libs` -o myfs
talk:
	gcc talk.c -o talk
client:
	gcc client.c -o client
