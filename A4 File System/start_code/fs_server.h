// this is the fs server

#ifndef fs_server_h
#define fs_server_h

#include <string>
#include <map>
#include "fs_protocol.h"
#include "inode_layer.h"

class fs_server {
 protected:
#if 0
  typedef struct fs {
    std::string data;
    struct fs_protocol::attr attr;
  } fs_t;
  std::map <fs_protocol::fsid_t, fs_t> fss;
#endif
  inode_layer *im;

 public:
  fs_server();

  int create(uint32_t type, fs_protocol::fsid_t &id);
  int put(fs_protocol::fsid_t id, std::string, int &);
  int get(fs_protocol::fsid_t id, std::string &);
  int getattr(fs_protocol::fsid_t id, fs_protocol::attr &);
  int remove(fs_protocol::fsid_t id, int &);
};

#endif 







