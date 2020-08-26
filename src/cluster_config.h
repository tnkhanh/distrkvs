// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_CLUSTER_CONFIG_H_
#define DISTRKVS_CLUSTER_CONFIG_H_

#include <string>
#include <set>
#include <vector>

namespace distrkvs {

using AddressString = std::basic_string<unsigned char>;

struct Node {
  AddressString address;
  int replica_count;
  
  Node(AddressString addr, int rep_count);
};

class Replica {
 public:
  Replica(AddressString server_name, int replica_number);
 private:
  AddressString server_name_;
  int replica_number_;
  unsigned char SHA1_digest_[20];

  void PrintHash();
};

class ClusterConfig {
 public:
  void AddNode(AddressString address, int replica_count);
 private:
  std::vector<Node> nodeList_;
  std::set<Replica> replicaSet_;
};

}  // namespace distrkvs

#endif  // DISTRKVS_CLUSTER_CONFIG_H_
