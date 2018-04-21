
#include <errno.h>
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

size_t MAXSIZE = 20;

int main(int argc, char **argv)
{
	int s;
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	struct addrinfo hints, *result;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; /* IPv4 only */
	hints.ai_socktype = SOCK_STREAM;

	s = getaddrinfo("192.168.1.103", "1234", &hints, &result);
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

	while(1){
		char id[1];
		memset(id, 0, 1);
		read(sock_fd, id, 1);
		if(id[0] == 'R'){
		char filename[128];memset(filename, 0, 128);
		char read_size[10];memset(read_size, 0, 10);
		char offset[20];memset(offset, 0, 20);
		size_t idx_fn = 0;
		//size_t idx_rs = 0;
		//size_t idx_os = 0;
	        size_t read_bytes = 0;
Read_filename:
	        while(1){
	                read_bytes = read(sock_fd, filename+idx_fn, 1);
        	        if(read_bytes == -1 && errno == EINTR)
                	        continue;
	                if(read_bytes == -1)
                	        return -errno;
			if(filename[idx_fn] == '\n'){
				filename[idx_fn] = '\0';
				goto Read_readsize;
			}
			idx_fn += 1;
	        }
Read_readsize:
		while(1){
                        read_bytes = read(sock_fd, read_size, sizeof(size_t));
                        if(read_bytes == -1 && errno == EINTR)
                                continue;
                        if(read_bytes == -1)
                                return -errno;
                        goto Read_offset;
		}

Read_offset:
		while(1){
                        read_bytes = read(sock_fd, offset, sizeof(off_t));
                        if(read_bytes == -1 && errno == EINTR)
                                continue;
                        if(read_bytes == -1)
                                return -errno;
                        break;
                }
		struct stat file;
		int i = stat(filename, &file);
		if(i == -1){
			printf("error occurs in stat");
			return -1;
		}
		size_t sz = (size_t)*read_size;
		off_t ofs= (off_t)*offset;
		size_t file_size = file.st_size;
		size_t remain = file_size - ofs;
		printf("%s\n", filename);
		printf("%lu\n", sz);
		printf("%llu\n", ofs);
		if(remain < 0){
		remain = -1;
		write(sock_fd, (char*)&remain, sizeof(remain));
		printf("err occurs due to too large offset.");
		return -1;
		}
		//write(sock_fd,(char*)&remain, sizeof(remain));
		if(remain < sz){
		sz = remain;
		} 
		write(sock_fd, (char*)&sz, sizeof(sz));
		printf("Going to send %s\n", filename);
		int fd = open(filename, O_RDONLY);
		char r[sz];
		pread(fd, r, sz, ofs);
		printf("Finish reading %s\n", filename);
		printf("%lu\n", sz);
		printf("Sent %s to server\n", r);
		write(sock_fd, r, sz);
		printf("Finish writing\n");
		}
		else if(id[0] ==  'W'){
		      char filename[128];memset(filename, 0, 128);
                      char read_size[10];memset(read_size, 0, 10);
                      char offset[20];memset(offset, 0, 20);
                      size_t idx_fn = 0;
                      size_t read_bytes = 0;
      Read_filename_again:
                      while(1){
                              read_bytes = read(sock_fd, filename+idx_fn, 1);
                              if(read_bytes == -1 && errno == EINTR)
                                      continue;
                              if(read_bytes == -1)
                                      return -errno;
                              if(filename[idx_fn] == '\n'){
                                      filename[idx_fn] = '\0';
                                      goto Read_readsize_again;
                              }
                              idx_fn += 1;
                      }
      Read_readsize_again:
                      while(1){
                              read_bytes = read(sock_fd, read_size, sizeof(size_t));
                              if(read_bytes == -1 && errno == EINTR)
                                      continue;
                              if(read_bytes == -1)
                                      return -errno;
                              goto Read_offset_again;
                      }

      Read_offset_again:
                      while(1){
                              read_bytes = read(sock_fd, offset, sizeof(off_t));
                              if(read_bytes == -1 && errno == EINTR)
                                      continue;
                              if(read_bytes == -1)
                                      return -errno;
                              break;
                      }
                      int fd = open(filename, O_CREAT | O_APPEND | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
                      size_t sz = (size_t)*read_size;
                      off_t ofs= (off_t)*offset;
                      size_t remain = MAXSIZE - ofs;
                      printf("filename : %s\n", filename);
                      printf("towritesize : %lu\n", sz);
                      printf("offset : %lu\n", ofs);
			printf("remainsize: %lu\n", remain);
                      if(remain < 0){
                          remain = -1;
                          write(sock_fd, (char*)&remain, sizeof(remain));
                          printf("err occurs due to too large offset.");
                          return -1;
                      }
                      if(remain < sz){
                          sz = remain;
                      }
                      write(sock_fd, (char*)&sz, sizeof(sz));
                      printf("Going to write to %s, %lu size\n", filename, sz);
                      char r[1024];memset(r, 0,1024);
		      size_t i = sz;
			size_t bytes = 0;
		     while(i){
                       bytes = read(sock_fd + sz - i, r, i);
			if(bytes == -1 && errno == EINTR)
				continue;
			if(i < bytes)
			break;
			i -= bytes;
			}
                      printf("Finish reading %s from server.\n", filename);
                      printf("read size :%lu\n", sz);
                      printf("read offset: %llu\n", ofs);
                      pwrite(fd, r, sz, ofs);
                      printf("Finish writing %s.\n", r);
                      close(fd);
		}
		else{
			printf("error format.\n");
		}
	}
	return 0;
}
