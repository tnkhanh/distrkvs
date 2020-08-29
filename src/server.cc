// Copyright (c) 2020 forkbase

#include "server.h"

#include <filesystem>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

#include "store.grpc.pb.h"

#include "cluster_config.h"

using rocksdb::DB;
using rocksdb::Options;
using rocksdb::PinnableSlice;
using rocksdb::WriteOptions;
using rocksdb::ReadOptions;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

namespace distrkvs {
// Logic and data behind the server's behavior.
class StoreServiceImpl final : public Store::Service {
 public:
  StoreServiceImpl(DB* db, ClusterConfig* config) 
    : Store::Service(), db_(db), config_(config) {}

 private:
  DB* db_;
  ClusterConfig* config_;

  grpc::Status Get(ServerContext* context, const GetRequest* get_request,
             GetResponse* get_response) override {
    if (get_request->from_client()) {
      NodePtr node = config_->PickNode(get_request->key());
      std::string value;
      node->InternalGet(get_request->key(), &value);
      if (value=="") {
        get_response->set_value("NotOK");
      }
      else {
        get_response->set_value("OK " + value);
      }
      return grpc::Status::OK;
    } else {
      std::string found_value;
      rocksdb::Status s = db_->Get(ReadOptions(), get_request->key(), &found_value);

      if (s.ok()) {
        get_response->set_value(found_value);
      } else if (s.IsNotFound()) {
        get_response->set_value("");
      } else {
        get_response->set_value("");
      }

      return grpc::Status::OK;
    }
  }

  grpc::Status Put(ServerContext* context, const PutRequest* put_request,
             PutResponse* put_response) override {
    if (put_request->from_client()) {
      NodePtr node = config_->PickNode(put_request->key());
      node->InternalPut(put_request->key(), put_request->value());
      put_response->set_value("OK");
    } else {
      rocksdb::Status s = db_->Put(WriteOptions(), 
                                  put_request->key(), put_request->value());
      if (s.ok()) {
        put_response->set_value("OK");
      } else {
        put_response->set_value("NotOK");
      }
    }

    return grpc::Status::OK;
  }

};

DistrkvsServer::DistrkvsServer(const std::string& kDBPath, const std::string& kServerAddress) 
  : server_address_(kServerAddress) {
  std::filesystem::create_directories(kDBPath);

  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB on construction
  rocksdb::Status s = DB::Open(options, kDBPath, &db_);
  assert(s.ok());
}

void DistrkvsServer::LoadConfigFromFile(const std::string& file_name) {
  std::ifstream istream(file_name, std::ifstream::in);

  if (!istream.is_open()) {
    std::cout << "Failed to  open " << file_name << "\n";
  } else {
    int node_count, replica_count;
    std::string node_name;

    istream >> node_count;
    for (int i=0; i < node_count; ++i) {
      std::cin >> node_name >> replica_count;
      config_.AddNode(node_name, replica_count);
    }
  }
}

void DistrkvsServer::Run() {
  StoreServiceImpl service(db_, &config_);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

}  // namespace distrkvs
