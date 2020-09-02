// Copyright (c) 2020 forkbase

#include "node.h"

#include <string>

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

#include "src/node.h"

using grpc::Channel;
using grpc::ClientContext;

namespace distrkvs {

Node::Node(const AddressString& address, int replica_count) 
    : address_(address), replica_count_(replica_count),
      stub_(
          Store::NewStub(
              grpc::CreateChannel(
                  address + ":50017",
                  grpc::InsecureChannelCredentials()))){}

AddressString& Node::address() {
  return address_;
}

grpc::Status Node::InternalPut(const std::string& key, const std::string& value) {
  PutRequest put_request;
  put_request.set_key(key);
  put_request.set_value(value);
  put_request.set_from_client(false);

  PutResponse reply;
  ClientContext context;

  grpc::Status status = stub_->Put(&context, put_request, &reply);
  return status;
}

grpc::Status Node::InternalGet(const std::string& key, std::string* value) {
  GetRequest get_request;
  get_request.set_key(key);
  get_request.set_from_client(false);

  GetResponse reply;
  ClientContext context;

  grpc::Status status = stub_->Get(&context, get_request, &reply);
  *value = reply.value();
  return status;
}

}  // namespace distrkvs
