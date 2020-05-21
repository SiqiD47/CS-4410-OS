// RPC stubs for clients to talk to fs_server

#include "fs_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

fs_client::fs_client()
{
  es = new fs_server();
}

fs_protocol::status
fs_client::create(uint32_t type, fs_protocol::fsid_t &id)
{
  fs_protocol::status ret = fs_protocol::OK;
  ret = es->create(type, id);
  return ret;
}

fs_protocol::status
fs_client::get(fs_protocol::fsid_t eid, std::string &buf)
{
  fs_protocol::status ret = fs_protocol::OK;
  ret = es->get(eid, buf);
  return ret;
}

fs_protocol::status
fs_client::getattr(fs_protocol::fsid_t eid, 
		       fs_protocol::attr &attr)
{
  fs_protocol::status ret = fs_protocol::OK;
  ret = es->getattr(eid, attr);
  return ret;
}

fs_protocol::status
fs_client::put(fs_protocol::fsid_t eid, std::string buf)
{
  fs_protocol::status ret = fs_protocol::OK;
  int r;
  ret = es->put(eid, buf, r);
  return ret;
}

fs_protocol::status
fs_client::remove(fs_protocol::fsid_t eid)
{
  fs_protocol::status ret = fs_protocol::OK;
  int r;
  ret = es->remove(eid, r);
  return ret;
}


