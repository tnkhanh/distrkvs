// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_SERVER_H_
#define DISTRKVS_SERVER_H_

#include <string>

#include "rocksdb/db.h"

using rocksdb::DB;

struct ServerConfig {
};

namespace distrkvs {

class DistrkvsServer {
 public:
  DistrkvsServer(const std::string& kDBPath, const std::string& kServerAddress);
  void run(); 

 private:
  std::string server_address_;
  ServerConfig config_;
  DB* db_;
};

}  // namespace distrkvs

#endif  // DISTRKVS_SERVER_H_
