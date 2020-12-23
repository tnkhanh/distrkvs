// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_SERVER_H_
#define DISTRKVS_SERVER_H_

#include <string>
#include <vector>

#include "rocksdb/db.h"

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

#include "hash_id.h"

using rocksdb::DB;
using grpc::ClientContext;

namespace distrkvs {

const int kMBit = 256;
const std::string kDefaultPort = ":50017";

using AddressString = std::string; // this includes the port

class DNode {
 public:
  DNode();
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
};

class DServer {
 public:
  DServer(const std::string& kDBPath, const std::string& kAddress);
  void Run(); 
  void Join(const AddressString& known_node_address);

 private:
  DNode my_node_;
  DB* db_;
  FingerEntry finger_[kMBit + 1];
  DNode predecessor_;

  //if remote_node is "", it's a local call

  AddressString FindSuccessor(const AddressString& remote_node, const HashId& id); 
  AddressString FindPredecessor(const HashId& id);
  AddressString ClosestPrecedingFinger(const AddressString& remote_node, const HashId& id);
  AddressString successor(const AddressString& remote_node);
  AddressString predecessor(const AddressString& remote_node);
  void set_predecessor(const AddressString& remote_node, const AddressString& address);
  void InitFingerTable(const AddressString& known_node_address);
  void UpdateOthers();
  void UpdateFingerTable(const AddressString& remote_node, const AddressString& node_address, int index);

  friend class StoreServiceImpl;
};

}  // namespace distrkvs

#endif  // DISTRKVS_SERVER_H_
