/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
*/

/** @file
 *
 * minimal example filesystem using high-level API
 *
 * Compile with:
 *
 *     gcc -Wall hello.c `pkg-config fuse3 --cflags --libs` -o hello
 *
 * ## Source code ##
 * \include hello.c
 */


#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void *mt_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	(void) conn;
//	cfg->kernel_cache = 1;
	return NULL;
}

static int mt_open(const char* path, struct fuse_file_info* file_info){
/*  int fd;
  fd = open(path, file_info->flags );
  if ( fd == -1)
    return -errno;
  close (fd);
*/
  return 0;
}

static int mt_read(const char* path, char* buffer, size_t size, size_t offset, struct fuse_file_info* file_info){
  (void) buffer;
  (void) offset;
  (void) file_info;
/*
  int fd = open(path, file_info->flags);
  if ( fd == -1)
    return -errno;
  struct stat mt_stat;
  stat(path,&mt_stat);
  //calculate which device we need to connect to based on what proportion of the file the offset is in
  //we assume that the input parameter offset indicates the byte that the user wishes
  //to start reading
  int num = offset/mt_stat->st_size;
  //if the offset is greater than the total number of devices we have, there is a problem:
  //the user requested to read at a place where the file has ended
  if(num>=total_devices){
    return -errno;
  }

  size_t remain = size; //remaining bytes to read
  // while there is bytes to read and the file has not ended
  while(remain>0 && num<= dir.total){
    uint32_t mt_addr = *((uint32_t*)dictionary_get(&dir.addr_map, &num)); //global variable directory
    //TODO: bluetooth connection
    //TODO: assume we have accessed to the local table (mt_table_num)
    size_t local_size = mt_table.size;
    if(local_offset!=0){ // the first device we read
      size_t local_offset = offset - (num-1)*local_size;
    }
    // remaining bytes that could be read on this device
    size_t bytes_read = remain - (local_size - local_offset);
    if(bytes_read<0){ // this is the last device that we need to read from
      memcpy(buffer, mt_table.addr_start+local_offset, remain);
      break;
    }
    else{ // if user requested a reading size that's larger than the size we could read from this device,
          //we still need to access and read from the next device
      memcpy(buffer, mt_table.addr_start+local_offset, bytes_read);
    }
    remain = bytes_read;
    // for all devices after the first one, start reading at the start of the local memory for the device
    local_offset = 0;
    num++;
  }
  close (fd);
  return size;
*/

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

write(client_fd, "text2.txt", 10);

int len = read(client_fd, buffer, size);
return len;
}

// basically identical to mt_read() except that we overwrite data instead of reading datd
static int mt_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info){
  (void) buffer;
  (void) offset;
  (void) file_info;
/*
  int fd;
  fd = open(path, file_info->flags);
  if ( fd == -1)
    return -errno;
  struct stat mt_stat;
  stat(path,&mt_stat);
  int num = offset/mt_stat->st_size;
  if(num>=dir.total){
    return -errno;
  }

  size_t remain = size;
  while(remain>0 && num<= dir.total){
    uint32_t mt_addr = *((uint32_t*)dictionary_get(&dir.addr_map, &num));//global variable directory
    //bluetooth connection
    //assume we have accessed to the local table (mt_table_num)
    size_t local_size = mt_table.size;
    if(local_offset!=0){
      size_t local_offset = offset - (num-1)*local_size;
    }
    size_t bytes_read = remain - (local_size - local_offset);
    if(bytes_read<0){
      memcpy(mt_table.addr_start+local_offset, buffer, remain);
      break;
    }
    else{
      memcpy(mt_table.addr_start+local_offset, buffer, bytes_read);
    }
    remain = bytes_read;
    local_offset = 0;
    num++;
  }
  close (fd);
  return size;
*/
	return 0;
}

static struct fuse_operations mt_oper = {
  .init = mt_init,
  .open = mt_open,
//  .close = mt_close,
  .read = mt_read,
 .write = mt_write,
};


static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("hello");
	options.contents = strdup("Hello World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0] = (char*) "";
	}

	return fuse_main(args.argc, args.argv, &mt_oper, NULL);
}