// Copyright (c) 2020 forkbase

#include "cluster_config.h"

#include "openssl/sha.h"

namespace distrkvs {

Node::Node(AddressString addr, int rep_count) 
    : address(addr), replica_count(rep_count) {}

Replica::Replica(AddressString server_name, int replica_number)
    : server_name_(server_name), replica_number_(replica_number) {
  AddressString hash_input(server_name);
  hash_input.append('_');
  hash_input.append(std::to_string(replica_number));
  SHA1(&hash_input[0], hash_input.size(), SHA1_digest_);
}

void ClusterConfig::AddNode(AddressString address, int replica_count) {
  nodeList_.push_back(Node(address, replica_count));

  for (int i = 0; i < replica_count; ++i) {
//    replicaSet_.insert(Replica(address, i));
  }
}

void Replica::PrintHash() {
  for (int i=0; i < 20; ++i) {
    printf("%02x ", SHA1_digest_[i]);
  }

  printf("\n");
}
}  // namespace distrkvs
