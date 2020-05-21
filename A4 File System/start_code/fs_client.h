// fs client interface.

#ifndef fs_client_h
#define fs_client_h

#include <string>
#include "fs_protocol.h"
#include "fs_server.h"

class fs_client {
 private:
  fs_server *es;

 public:
  fs_client();

  fs_protocol::status create(uint32_t type, fs_protocol::fsid_t &eid);
  fs_protocol::status get(fs_protocol::fsid_t eid, 
			                        std::string &buf);
  fs_protocol::status getattr(fs_protocol::fsid_t eid, 
				                          fs_protocol::attr &a);
  fs_protocol::status put(fs_protocol::fsid_t eid, std::string buf);
  fs_protocol::status remove(fs_protocol::fsid_t eid);
};

#endif 

