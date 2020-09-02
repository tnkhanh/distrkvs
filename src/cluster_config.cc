// Copyright (c) 2020 forkbase

#include "cluster_config.h"

#include <string>

#include "openssl/sha.h"

#include "store.grpc.pb.h"

#include "src/node.h"

namespace distrkvs {

bool ReplicaCompare::operator()(const Replica& lhs, const Replica& rhs) const {
  for (auto i = 0; i < 20; ++i) {
    if (lhs.SHA1_digest_[i] < rhs.SHA1_digest_[i]) return true;
    if (lhs.SHA1_digest_[i] > rhs.SHA1_digest_[i]) return false;
  }
  return false;
}

Replica::Replica(NodePtr node_ptr, int replica_number)
    : node_ptr_(node_ptr), replica_number_(replica_number) {
  AddressString hash_input(node_ptr->address());
  hash_input.append(1, '_');
  hash_input.append(std::to_string(replica_number));
  SHA1((reinterpret_cast<const unsigned char*>(&hash_input[0])), hash_input.size(), SHA1_digest_);
  #ifdef DEBUG
  PrintHash();
  #endif
}

Replica::Replica(const std::string& key) {
  SHA1((reinterpret_cast<const unsigned char*>(&key[0])), key.size(), SHA1_digest_);
  #ifdef DEBUG
  std::cout << "Key: " << key << " | " << std::endl;
  PrintHash();
  #endif
}

void Replica::PrintHash() const {
  if (node_ptr_)
    std::cout << node_ptr_ -> address() << ": " << replica_number_ << std::endl;
  else
    std::cout <<"No node!" << std::endl;
  char buf[10]{};
  std::string hash;
  for (int i=0; i < 20; ++i) {
    sprintf(buf, "%02x ", SHA1_digest_[i]);
    hash.append(buf);
  }

  std::cout << hash << std::endl;
}

void ClusterConfig::AddNode(const AddressString& address, int replica_count) {
  NodePtr ptr = std::make_shared<Node>(Node(address, replica_count));
  node_list_.push_back(ptr);

  for (int i = 0; i < replica_count; ++i) {
    replica_set_.insert(Replica(ptr, i));
  }
}

NodePtr ClusterConfig::PickNode(const std::string& key) {
  Replica dummy(key);
  auto replica_ptr = replica_set_.lower_bound(dummy);
  if (replica_ptr == replica_set_.end()) replica_ptr = replica_set_.begin();
  #ifdef DEBUG
  replica_ptr->PrintHash();
  #endif
  return replica_ptr->node_ptr_;
}

int ClusterConfig::NodeCount() {
  return node_list_.size();
}

NodePtr ClusterConfig::NodeAt(int pos) {
  return node_list_[pos];
}

}  // namespace distrkvs
