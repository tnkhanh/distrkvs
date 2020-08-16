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
//#include "src/client.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using distrkvs::Store;
using distrkvs::GetRequest;
using distrkvs::PutRequest;
using distrkvs::GetResponse;
using distrkvs::PutResponse;

class StoreClient {
 public:
  explicit StoreClient(std::shared_ptr<Channel> channel)
      : stub_(Store::NewStub(channel)) {}

  std::string Put(const std::string& key, const std::string& value) {
    PutRequest put_request;
    put_request.set_key(key);
    put_request.set_value(value);

    PutResponse reply;
    ClientContext context;

    // The actual RPC.
    Status status = stub_->Put(&context, put_request, &reply);

    if (status.ok()) {
      return reply.value();
    } else {
      std::stringstream error_stream;
      error_stream << "RPC failed: " <<  status.error_code() << ": "
                   << status.error_message();
      return error_stream.str();
    }
  }

  std::string Get(const std::string& key) {
    GetRequest get_request;
    get_request.set_key(key);
    GetResponse reply;
    ClientContext context;

    Status status = stub_->Get(&context, get_request, &reply);
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

  while (true) {
    std::string comm, key, value, reply;
    std::cin>>comm;
    if (comm=="Put") {
      std::cout<<"Put: ";
      std::cin>>key>>value;
      reply = store.Put(key, value);
      std::cout << "Server answered: " << reply << "\n";
    } else if (comm=="Get") {
      std::cout<<"Get: ";
      std::cin>>key;
      reply = store.Get(key);
      std::cout << "Server answered: " << reply << "\n";
    } else if (comm=="Quit" || comm=="Exit") {
      break;
    } else {
      std::cout<<"Not a command!\n";
    }
  }

  return 0;
}
