// Copyright (c) 2020 forkbase

#include <string>

#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

#include "store.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using distrkvs::Store;
using distrkvs::Key;
using distrkvs::KeyValue;
using distrkvs::Value;

namespace distrkvs::server {
// Logic and data behind the server's behavior.
class StoreServiceImpl final : public Store::Service {
  grpc::Status Get(ServerContext* context, const Key* key,
             Value* value) override {
    std::string prefix("Key: ");
    value->set_value(prefix + key->key());
    return grpc::Status::OK;
  }

  grpc::Status Put(ServerContext* context, const KeyValue* key_value,
             Value* value) override {
    value->set_value("Key: " + key_value->key() + "\nValue: " +
                     key_value->value());
    return grpc::Status::OK;
  }
};

void RunServer(const std::string& kServerAddress) {
  StoreServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(kServerAddress, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}
}  // namespace distrkvs::server
