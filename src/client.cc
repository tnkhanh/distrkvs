// Copyright (c) 2020 forkbase

#include "src/client.h"

#include <filesystem>
#include <string>
#include <iostream>
#include <sstream>

#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

#include "store.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

namespace distrkvs {

DistrkvsClient::DistrkvsClient(const std::string& kServerAddress)
    : stub_(
          Store::NewStub(
              grpc::CreateChannel(
                  kServerAddress + ":50017",
                  grpc::InsecureChannelCredentials()))) {}

grpc::Status DistrkvsClient::Put(const KeyString& key, const ValueString& value) {
  PutRequest put_request;
  put_request.set_key(key);
  put_request.set_value(value);
  put_request.set_from_client(true);

  PutResponse reply;
  ClientContext context;

  return stub_->Put(&context, put_request, &reply);
}

grpc::Status DistrkvsClient::Get(const KeyString& key, ValueString* value) {
  GetRequest get_request;
  get_request.set_key(key);
  get_request.set_from_client(true);

  GetResponse reply;
  ClientContext context;

  Status status = stub_->Get(&context, get_request, &reply);
  *value = reply.value();

  return status;
}

}  // namespace distrkvs
