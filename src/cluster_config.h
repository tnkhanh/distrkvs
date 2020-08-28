// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_CLUSTER_CONFIG_H_
#define DISTRKVS_CLUSTER_CONFIG_H_

#include <string>
#include <set>
#include <vector>
#include <memory>

#include "store.grpc.pb.h"

namespace distrkvs {

using AddressString = std::string;

struct Node {
  AddressString address;
  int replica_count;
  std::unique_ptr<Store::Stub> stub;
  Node(const AddressString& addr, int rep_count);
};

using NodePtr = std::shared_ptr<const Node>;

class Replica {
 public:
  Replica(NodePtr node_ptr, int replica_number);
  Replica(const std::string& key);
  void PrintHash();

 private:
  NodePtr node_ptr_;
  int replica_number_;
  unsigned char SHA1_digest_[20];

  friend class ReplicaCompare;
};

struct ReplicaCompare{
  bool operator()(const Replica& lhs, const Replica& rhs) const;
};

class ClusterConfig {
 public:
  void AddNode(const std::string& address, int replica_count);
  NodePtr PickNode(const std::string& key);
  int NodeCount();
  NodePtr NodeAt(int pos);
 private:
  std::vector<NodePtr> node_list_;
  std::set<Replica, ReplicaCompare> replica_set_;
};

}  // namespace distrkvs

#endif  // DISTRKVS_CLUSTER_CONFIG_H_