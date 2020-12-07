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
using google::protobuf::Empty;

namespace distrkvs {

DClient::DClient(const std::string& kServerAddress)
    : stub_(
          Store::NewStub(
              grpc::CreateChannel(
                  kServerAddress,
                  grpc::InsecureChannelCredentials()))) {}

grpc::Status DClient::Put(const KeyString& key, const ValueString& value) {
  PutRequest put_request;
  put_request.set_key(key);
  put_request.set_value(value);
  put_request.set_from_client(true);

  Empty response;
  ClientContext context;

  return stub_->Put(&context, put_request, &response);
}

grpc::Status DClient::Get(const KeyString& key, ValueString* value) {
  GetRequest get_request;
  get_request.set_key(key);
  get_request.set_from_client(true);

  GetResponse response;
  ClientContext context;

  Status status = stub_->Get(&context, get_request, &response);
  *value = response.value();

  return status;
}

}  // namespace distrkvs
