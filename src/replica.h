// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_REPLICA_H_
#define DISTRKVS_REPLICA_H_

#include <string>

namespace distrkvs {

class Replica {
 public:
  Replica(std::string server_name, int replica_number);
 private:
  std::string server_name_;
  int replica_number_;
  unsigned char SHA1_digest_[20];
};

}  // namespace distrkvs

#endif  // DISTRKVS_REPLICA_H_
