// Copyright (c) 2020 forkbase

#include "server.h"

#include <filesystem>
#include <string>
#include <memory>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

#include "store.grpc.pb.h"

using rocksdb::DB;
using rocksdb::Options;
using rocksdb::PinnableSlice;
using rocksdb::WriteOptions;
using rocksdb::ReadOptions;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using distrkvs::Store;
using distrkvs::GetRequest;
using distrkvs::PutRequest;
using distrkvs::GetResponse;
using distrkvs::PutResponse;

namespace distrkvs::server {
// Logic and data behind the server's behavior.
class StoreServiceImpl final : public Store::Service {
 public:
  StoreServiceImpl(const std::string& kDBPath) : Store::Service() {
    std::filesystem::create_directories(kDBPath);

    Options options;
    // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
    options.IncreaseParallelism();
    options.OptimizeLevelStyleCompaction();
    // create the DB if it's not already present
    options.create_if_missing = true;

    // open DB
    rocksdb::Status s = DB::Open(options, kDBPath, &db_);
    assert(s.ok());

    // Put key-value
    s = db_->Put(WriteOptions(), "key1", "value");
    assert(s.ok());
    std::string value;
    // get value
    s = db_->Get(ReadOptions(), "key1", &value);
    assert(s.ok());
    assert(value == "value");
  }

  ~StoreServiceImpl() {
    delete db_;
  }

 private:
  DB* db_;

  grpc::Status Get(ServerContext* context, const GetRequest* get_request,
             GetResponse* get_response) override {
    std::string found_value;
    rocksdb::Status s = db_->Get(ReadOptions(), get_request->key(), &found_value);

    if (s.ok()) {
      get_response->set_value("Value: " + found_value);
    }
    else
    if (s.IsNotFound()) {
      get_response->set_value("Key not found");
    }
    else {
      get_response->set_value("Something wrong");
    }

    return grpc::Status::OK;
  }

  grpc::Status Put(ServerContext* context, const PutRequest* put_request,
             PutResponse* put_response) override {
    rocksdb::Status s = db_->Put(WriteOptions(), 
                                put_request->key(), put_request->value());
    if (s.ok()) {
      put_response->set_value("OK!");
    }
    else {
      put_response->set_value("Not OK!");
    }
    return grpc::Status::OK;
  }

};

void RunServer(const std::string& kDBPath, const std::string& kServerAddress) {
  StoreServiceImpl service(kDBPath);

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
