
#pragma once
#include "page_table.h"
#include <stdlib.h>

static struct fuse_operations mt_oper = {
  .init = mt_init,
  .open = mt_open,
  .close = mt_close,
  .read = mt_read,
  .write = mt_write,
}

static int mt_open(const char* path, struct fuse_file_info* file_info);
static int mt_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
static int mt_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* file_info);
