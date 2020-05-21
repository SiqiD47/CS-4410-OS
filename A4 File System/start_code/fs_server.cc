// the fs server implementation

#include "fs_server.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

fs_server::fs_server() 
{
  im = new inode_layer();
}

int fs_server::create(uint32_t type, fs_protocol::fsid_t &id)
{
  // alloc a new inode and return inum
  printf("fs_server: create inode\n");
  id = im->alloc_inode(type);

  return fs_protocol::OK;
}

int fs_server::put(fs_protocol::fsid_t id, std::string buf, int &)
{
  id &= 0x7fffffff;
  
  const char * cbuf = buf.c_str();
  int size = buf.size();
  im->write_file(id, cbuf, size);
  
  return fs_protocol::OK;
}

int fs_server::get(fs_protocol::fsid_t id, std::string &buf)
{
  printf("fs_server: get %lld\n", id);

  id &= 0x7fffffff;

  int size = 0;
  char *cbuf = NULL;

  im->read_file(id, &cbuf, &size);
  if (size == 0)
    buf = "";
  else {
    buf.assign(cbuf, size);
    free(cbuf);
  }

  return fs_protocol::OK;
}

int fs_server::getattr(fs_protocol::fsid_t id, fs_protocol::attr &a)
{
  printf("fs_server: getattr %lld\n", id);

  id &= 0x7fffffff;
  
  fs_protocol::attr attr;
  memset(&attr, 0, sizeof(attr));
  im->getattr(id, attr);
  a = attr;

  return fs_protocol::OK;
}

int fs_server::remove(fs_protocol::fsid_t id, int &)
{
  printf("fs_server: write %lld\n", id);

  id &= 0x7fffffff;
  im->remove_file(id);
 
  return fs_protocol::OK;
}

