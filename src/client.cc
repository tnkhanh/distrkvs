// Copyright (c) 2020 forkbase

#include "src/client.h"

#include <filesystem>
#include <string>
#include <iostream>
#include <sstream>

#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "store.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using distrkvs::Store;
using distrkvs::GetRequest;
using distrkvs::PutRequest;
using distrkvs::GetResponse;
using distrkvs::PutResponse;

namespace distrkvs {

DistrkvsClient::DistrkvsClient(const std::string& kServerAddress)
    : stub_(
        Store::NewStub(
            grpc::CreateChannel(
                kServerAddress + ":50017", grpc::InsecureChannelCredentials()))) {}

DStatus DistrkvsClient::Put(const KeyString& key, const ValueString& value) {
  PutRequest put_request;
  put_request.set_key(key);
  put_request.set_value(value);
  put_request.set_from_client(true);

  PutResponse reply;
  ClientContext context;

  Status status = stub_->Put(&context, put_request, &reply);
  if (status.ok()) {
    return reply.value() == "OK" ? DStatus::kOk : DStatus::kNotOk;
  } else {
    return DStatus::kRPCFailed;
  }
}

DStatus DistrkvsClient::Get(const KeyString& key, ValueString* value) {
  GetRequest get_request;
  get_request.set_key(key);
  get_request.set_from_client(true);

  GetResponse reply;
  ClientContext context;

  Status status = stub_->Get(&context, get_request, &reply);
  if (status.ok()) {
    if (reply.value().substr(0, 3) == "OK ") {
      *value = reply.value().substr(3);
      return DStatus::kOk;
    } else {
      return DStatus::kNotOk;
    }
  } else {
    return DStatus::kRPCFailed;
  }
}

}  // namespace distrkvs
