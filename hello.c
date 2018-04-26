/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
  Copyright (C) 2011       Sebastian Pipping <sebastian@pipping.org>
  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.
  gcc -Wall fusexmp.c `pkg-config fuse --cflags --libs` -o fusexmp
*/

#define FUSE_USE_VERSION 26

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

#include <sys/types.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>

int client_fd[2];
char *filename = "text2.txt";
int total_devices = 2;
size_t each_max_size = 20;
int writers;
int writing; 
int reading;
pthread_cond_t turn = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

static void xmp_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
/*	(void) conn;
	cfg->use_ino = 1;
	cfg->entry_timeout = 0;
	cfg->attr_timeout = 0;
	cfg->negative_timeout = 0;
*/

	// connect with TCP
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
	int optval = 1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
	if (bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
		perror("bind");
		exit(1);
	}
	if (listen(sock_fd, 10) != 0) {
		perror("listen");
		exit(1);
	}
	int i = 1;
	while (i < total_devices) {
		client_fd[i] = accept(sock_fd, NULL, NULL);
		if (client_fd[i] == -1) {
			perror("accept");
			exit(1);
		}
		i++;
	}
}


static int xmp_getattr(const char *path, struct stat *stbuf)
{
	int res;

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_access(const char *path, int mask)
{
	int res;

	res = access(path, mask);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readlink(const char *path, char *buf, size_t size)
{
	int res;

	res = readlink(path, buf, size - 1);
	if (res == -1)
		return -errno;

	buf[res] = '\0';
	return 0;
}


static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	/* On Linux this could just be 'mknod(path, mode, rdev)' but this
	   is more portable */
	if (S_ISREG(mode)) {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
			res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
	int res;

	res = mkdir(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_unlink(const char *path)
{
	int res;

	res = unlink(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rmdir(const char *path)
{
	int res;

	res = rmdir(path);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_symlink(const char *from, const char *to)
{
	int res;

	res = symlink(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_rename(const char *from, const char *to)
{
	int res;

	res = rename(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_link(const char *from, const char *to)
{
	int res;

	res = link(from, to);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chmod(const char *path, mode_t mode)
{
	int res;

	res = chmod(path, mode);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;

	res = lchown(path, uid, gid);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;

	res = truncate(path, size);
	if (res == -1)
		return -errno;

	return 0;
}

#ifdef HAVE_UTIMENSAT
static int xmp_utimens(const char *path, const struct timespec ts[2])
{
	int res;

	/* don't use utime/utimes since they follow symlinks */
	res = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
	if (res == -1)
		return -errno;

	return 0;
}
#endif

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	res = open(path, fi->flags);
	if (res == -1)
		return -errno;

	close(res);

	return 0;

}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	pthread_mutex_lock(&m);
    	while (writers){
        	pthread_cond_wait(&turn, &m);
	}
    	reading++;
    	pthread_mutex_unlock(&m);
	    
	(void) fi;
	if (strcmp(path+strlen(path)-9, "text2.txt") == 0) {
		int fd2 = open("/home/pi/debug.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXG | S_IRWXU | S_IRWXO);
		dprintf(fd2, "offset:%llu\n", offset);
		dprintf(fd2, "size:%d\n", size);
		close(fd2);
		int num = offset / each_max_size;
		if (num >= total_devices) {
			return -errno;
		}
		size_t remain = size; //remaining bytes to read
		off_t local_offset = offset % each_max_size;
		char buffer[1024];
		size_t written = 0;
		
		  // while there is bytes to read and the file has not ended
		while(remain > 0 && num <= total_devices){
			if (num == 0) {
				int fd;
				int res;
			
				fd = open(path, O_RDONLY);
				if (fd == -1)
					return -errno;
				struct stat file;
				stat(path, &file);
				size_t toread = remain;
				FILE *filep = fdopen(fd, "r");
				fseek(filep, local_offset, SEEK_SET);
				if (toread > ((size_t)(file.st_size) - local_offset)) {
					toread = (size_t)(file.st_size) - local_offset;
				}
				res = read(fd, buf, toread);
				if (res == -1)
					return -errno;
				//close(fd);
				remain -= toread;
				written += toread;
			}
			else {
				write(client_fd[num], "R", 1);
				write(client_fd[num], filename, strlen(filename));
				write(client_fd[num], "\n", 1);
				write(client_fd[num], (char*)&remain, sizeof(size_t));
				write(client_fd[num], (char*)&local_offset, sizeof(off_t));
				memset(buffer, 0, 1024);
				read(client_fd[num], buffer, sizeof(size_t));
				size_t toread = (size_t)(*buffer);
				if (toread == -1) {
					perror("EOF");
					return written;
				}
				read(client_fd[num], buf+written, toread);
				remain -= toread;
				written += toread;
			}
			local_offset = 0;
			num++;
		}
		
		pthread_mutex_lock(&m);
    		reading--;
    		pthread_cond_broadcast(&turn);
    		pthread_mutex_unlock(&m);
		
		return size;
	}
	else {
		int fd;
		int res;
	
		
		fd = open(path, O_RDONLY);
		if (fd == -1)
			return -errno;
	
		res = pread(fd, buf, size, offset);
		if (res == -1)
			res = -errno;
	
		close(fd);
		
		pthread_mutex_lock(&m);
    		reading--;
    		pthread_cond_broadcast(&turn);
    		pthread_mutex_unlock(&m);
		
		return res;
	}
}

static int xmp_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	pthread_mutex_lock(&m);
    	writers++;
        while (reading || writing){ 
        	pthread_cond_wait(&turn, &m);
	}
    	writing++;
    	pthread_mutex_unlock(&m);
	
	(void) fi;
	if (strcmp(path+strlen(path)-9, "text2.txt") == 0) {
		int fd2 = open("/home/pi/Downloads/fds/debug.txt", O_CREAT | O_TRUNC | O_RDWR, S_IRWXG | S_IRWXU | S_IRWXO);
		dprintf(fd2, "offset:%llu\n", offset);
		dprintf(fd2, "size:%d\n", size);
		close(fd2);
		int num = offset / each_max_size;
		if (num >= total_devices) {
			return -errno;
		}
		size_t remain = size; //remaining bytes to read
		off_t local_offset = offset % each_max_size;
		char buffer[1024];
		size_t written = 0;
		
		  // while there is bytes to write and the file has not ended
		while(remain > 0 && num <= total_devices){
			if (num == 0) {
				int fd;
				int res;
			
				fd = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IRWXG | S_IRWXU | S_IRWXO);
				if (fd == -1)
					return -errno;
				size_t toread = remain;
				if (toread > (each_max_size - local_offset)) {
					toread = each_max_size - local_offset;
				}
				res = pwrite(fd, buf, toread, local_offset);
				if (res == -1)
					return -errno;
				//close(fd);
				remain -= toread;
				written += toread;
			}
			else {
				write(client_fd[num], "W", 1);
				write(client_fd[num], filename, strlen(filename));
				write(client_fd[num], "\n", 1);
				write(client_fd[num], (char*)&remain, sizeof(size_t));
				write(client_fd[num], (char*)&local_offset, sizeof(off_t));
				memset(buffer, 0, 1024);
				read(client_fd[num], buffer, sizeof(size_t));
				size_t toread = (size_t)(*buffer);
				if (toread == -1) {
					perror("something wrong");
					return -1;
				}
				write(client_fd[num], buf+written, toread);
				remain -= toread;
				written += toread;
			}
			local_offset = 0;
			num++;
		}
		
		pthread_mutex_lock(&m);
    		writing--;
    		writers--;
    		pthread_cond_broadcast(&turn);  
    		pthread_mutex_unlock(&m);  
		
		return written;
	}
	else {

		int fd;
		int res;
	
		(void) fi;
		fd = open(path, O_WRONLY);
		if (fd == -1)
			return -errno;
	
		res = pwrite(fd, buf, size, offset);
		if (res == -1)
			res = -errno;
	
		close(fd);
		
		pthread_mutex_lock(&m);
    		writing--;
    		writers--;
    		pthread_cond_broadcast(&turn);  
    		pthread_mutex_unlock(&m);  
		
		return res;
	}
}


static int xmp_statfs(const char *path, struct statvfs *stbuf)
{
	int res;

	res = statvfs(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) fi;
	return 0;
}

static int xmp_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */

	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

#ifdef HAVE_POSIX_FALLOCATE
static int xmp_fallocate(const char *path, int mode,
			off_t offset, off_t length, struct fuse_file_info *fi)
{
	int fd;
	int res;

	(void) fi;

	if (mode)
		return -EOPNOTSUPP;

	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -errno;

	res = -posix_fallocate(fd, offset, length);

	close(fd);
	return res;
}
#endif

#ifdef HAVE_SETXATTR
/* xattr operations are optional and can safely be left unimplemented */
static int xmp_setxattr(const char *path, const char *name, const char *value,
			size_t size, int flags)
{
	int res = lsetxattr(path, name, value, size, flags);
	if (res == -1)
		return -errno;
	return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value,
			size_t size)
{
	int res = lgetxattr(path, name, value, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_listxattr(const char *path, char *list, size_t size)
{
	int res = llistxattr(path, list, size);
	if (res == -1)
		return -errno;
	return res;
}

static int xmp_removexattr(const char *path, const char *name)
{
	int res = lremovexattr(path, name);
	if (res == -1)
		return -errno;
	return 0;
}
#endif /* HAVE_SETXATTR */

static struct fuse_operations xmp_oper = {
	.init		= xmp_init,
	.getattr	= xmp_getattr,
	.access		= xmp_access,
	.readlink	= xmp_readlink,
	.readdir	= xmp_readdir,
	.mknod		= xmp_mknod,
	.mkdir		= xmp_mkdir,
	.symlink	= xmp_symlink,
	.unlink		= xmp_unlink,
	.rmdir		= xmp_rmdir,
	.rename		= xmp_rename,
	.link		= xmp_link,
	.chmod		= xmp_chmod,
	.chown		= xmp_chown,
	.truncate	= xmp_truncate,
#ifdef HAVE_UTIMENSAT
	.utimens	= xmp_utimens,
#endif
	.open		= xmp_open,
	.read		= xmp_read,
	.write		= xmp_write,
	.statfs		= xmp_statfs,
	.release	= xmp_release,
	.fsync		= xmp_fsync,
#ifdef HAVE_POSIX_FALLOCATE
	.fallocate	= xmp_fallocate,
#endif
#ifdef HAVE_SETXATTR
	.setxattr	= xmp_setxattr,
	.getxattr	= xmp_getxattr,
	.listxattr	= xmp_listxattr,
	.removexattr	= xmp_removexattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);

	
	return fuse_main(argc, argv, &xmp_oper, NULL);
}
