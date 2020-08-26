// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_SERVER_H_
#define DISTRKVS_SERVER_H_

#include <string>
#include <vector>
#include <set>

#include "rocksdb/db.h"

#include "cluster_config.h"

using rocksdb::DB;

namespace distrkvs {

class DistrkvsServer {
 public:
  DistrkvsServer(const std::string& kDBPath, const std::string& kServerAddress);
  void Run(); 

 private:
  std::string server_address_;
  ClusterConfig config_;
  DB* db_;
};

}  // namespace distrkvs

#endif  // DISTRKVS_SERVER_H_
