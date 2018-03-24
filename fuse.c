#include "fuse.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>





static int mt_open(const char* path, struct fuse_file_info* file_info){
  int fd;
  fd = open(path, file_info->flags );
  if ( fd == -1)
    return -errno;
  close (fd);
  return 0;
}

static int mt_read(const char* path, char* buffer, size_t size, size_t offset, struct fuse_file_info* file_info){
  (void) buffer;
  (void) offset;
  (void) file_info;

  int fd;
  fd = open(path, file_info->flags);
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
  if(num>=dir.total){
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
}

// basically identical to mt_read() except that we overwrite data instead of reading datd
static int mt_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info){
  (void) buffer;
  (void) offset;
  (void) file_info;

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
}
