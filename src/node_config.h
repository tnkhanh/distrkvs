// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_NODE_CONFIG_H_
#define DISTRKVS_NODE_CONFIG_H_

#include <string>
#include <set>
#include <vector>
#include <memory>

#include "store.grpc.pb.h"

#include "src/node.h"

namespace distrkvs {

class Replica {
 public:
  Replica(NodePtr node_ptr, int replica_number);
  Replica(const std::string& key);
  void PrintHash() const;

 private:
  NodePtr node_ptr_;
  int replica_number_;
  unsigned char SHA1_digest_[20];

  friend class ReplicaCompare;
  friend class NodeConfig;
};

struct ReplicaCompare{
  bool operator()(const Replica& lhs, const Replica& rhs) const;
};

class NodeConfig {
 public:
  void AddNode(const std::string& address, int replica_count);
  NodePtr PickNode(const std::string& key);
  int NodeCount();
  NodePtr NodeAt(int pos);

 private:
  std::vector<NodePtr> node_list_;
  std::set<Replica, ReplicaCompare> replica_set_;

  friend class NodeTest;
};

}  // namespace distrkvs

#endif  // DISTRKVS_CLUSTER_CONFIG_H_
