// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_SERVER_H_
#define DISTRKVS_SERVER_H_

#include <string>
#include <vector>

#include "rocksdb/db.h"

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

#include "node_config.h"
#include "hash_id.h"

using rocksdb::DB;
using grpc::ClientContext;

namespace distrkvs {

const int kMBit = 256;
const std::string kDefaultPort = ":50017";

using AddressString = std::string; // this includes the port

class DNode {
 public:
  DNode(const std::string& address);
  bool in_range(const DNode& left, const DNode& right);
  AddressString address();
  HashId id();

 private:
  AddressString address_;
  HashId id_;
  friend bool operator < (const DNode& n1, const DNode& n2);
};

struct FingerEntry {
  HashId start;
  DNode node;
}

class DServer {
 public:
  DServer(const std::string& kDBPath, const std::string& kAddress);
  void Run(); 
  void Join(const AddressString& known_node_address);

 private:
  AddressString address_;
  HashId my_id_;
  NodeConfig config_;
  DB* db_;
  FingerEntry finger_[kMBit + 1];
  DNode predecessor_;
  ClientContext client_context_;

  AddressString FindSuccessor(const HashId& id); 
  AddressString FindPredecessor(const HashId& id);
  AddressString ClosestPrecedingFinger(const HashId& id);
  AddressString successor();
  AddressString predecessor();
  void set_predecessor(const AddressString& node_address);
  void InitFingerTable(const AddressString& known_node_address);
  void UpdateOthers();
  void UpdateFingerTable(const AddressString& node_address, int index);
};

}  // namespace distrkvs

#endif  // DISTRKVS_SERVER_H_
