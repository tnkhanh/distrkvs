// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_NODE_H_
#define DISTRKVS_NODE_H_

#include <string>
#include <set>
#include <vector>
#include <memory>

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

namespace distrkvs {

using AddressString = std::string;

class Node {
 public:
  Node(const AddressString& address, int replica_count);
  AddressString& address();
  grpc::Status InternalPut(const std::string& key, const std::string& value);
  grpc::Status InternalGet(const std::string& key, std::string* value);

 private:
  AddressString address_;
  int replica_count_;
  std::unique_ptr<Store::Stub> stub_;
};

using NodePtr = std::shared_ptr<Node>;

}  // namespace distrkvs

#endif  // DISTRKVS_NODE_H_
