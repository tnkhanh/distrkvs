// Copyright (c) 2020 forkbase

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
#include "src/replica.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using distrkvs::Key;
using distrkvs::KeyValue;
using distrkvs::Value;
using distrkvs::Store;

class StoreClient {
 public:
  explicit StoreClient(std::shared_ptr<Channel> channel)
      : stub_(Store::NewStub(channel)) {}

  std::string Put(const std::string& key, const std::string& value) {
    KeyValue kv;
    kv.set_key(key);
    kv.set_value(value);

    Value reply;
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Put(&context, kv, &reply);

    if (status.ok()) {
      return reply.value();
    } else {
      std::stringstream error_stream;
      error_stream << "RPC failed: " <<  status.error_code() << ": "
                   << status.error_message();
      return error_stream.str();
    }
  }

  std::string Get(const std::string& key_string) {
    Key key;
    key.set_key(key_string);
    Value reply;
    ClientContext context;

    Status status = stub_->Get(&context, key, &reply);
    if (status.ok()) {
      return reply.value();
    } else {
      std::stringstream error_stream;
      error_stream << "RPC failed: " <<  status.error_code() << ": "
                   << status.error_message();
      return error_stream.str();
    }
  }

 private:
  std::unique_ptr<Store::Stub> stub_;
};

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);

  std::cout << "Distrkvs client\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  std::string server_address(argc > 1 ? argv[1] : "localhost");

  StoreClient store(grpc::CreateChannel(
      server_address + ":50017", grpc::InsecureChannelCredentials()));

  std::string reply = store.Put("some key", "some value");
  std::cout << "Server answered: " << reply << "\n";

  reply = store.Get("Key to get");
  std::cout << "Server answered: " << reply << "\n";

  return 0;
}
