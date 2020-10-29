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

#include "store_service_impl.h"
#include "hash_id.h"

using rocksdb::DB;
using rocksdb::Options;
using rocksdb::PinnableSlice;
using rocksdb::WriteOptions;
using rocksdb::ReadOptions;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;

namespace distrkvs {

DNode::DNode(const std::string& address) : address_(address) {
  id_.hash(address_);
}

bool DNode::in_range(const DNode& left, const DNode& right) {
  if (left < *this) {
    return (*this) < right || right < left;
  } else {
    return (*this) < right && right < left;
  }
}

AddressString DNode::address() {
  return address_;
}

HashId DNode::id() {
  return id_;
}

bool operator < (const DNode& n1, const DNode& n2) {
  return n1.id_ < n2.id_;
}

DServer::DServer(const std::string& kDBPath, const std::string& kAddress) 
  : address_(kAddress) {
  std::filesystem::create_directories(kDBPath);

  my_id_.resize(SHA256_DIGEST_LENGTH);

  SHA256(reinterpret_cast<const unsigned char*>(&address_[0]),
         address_.size(), &my_id_[0]);

  for (int i=1; i <= kMBit; ++i) {
    // TODO: maybe put this in a function 
    finger_[i].start = my_id_;
    int mod = (i - 1) % (sizeof unsigned char),
        offset = (i - 1) / (sizeof unsigned char);

    int ind = SHA256_DIGEST_LENGTH - 1 - offset;

    finger_[i].start[ind] += (1 << mod);
    if (finger_[i].start[ind] < (1 << mod)) {
      while (true) {
        --ind;
        if (ind < 0) break;
        finger_[i].start[ind]++;
        if (finger_[i].start[ind]!=0) break;
      }
    }
  }

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

void DServer::Run() {
  StoreServiceImpl service(db_, &config_);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(address_ + ":50017", grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

void DServer::Join(const AddressString& known_node_address) {
  if (known_node_address.size()) {
    InitFingerTable(known_node_address);
    UpdateOthers();

    // TODO: move keys in (predecessor, n] from successor to n
  }
  else {
    for (int i=1; i<=kMBit; ++i) {
      finger_[i].node.address = address_;
      finger_[i].node.id = my_id_;
    }

    predecessor_.address = address_;
    predecessor_.id = my_id_;
/*
      stub_(
          Store::NewStub(
              grpc::CreateChannel(
                  address + kDefaultPort,
                  grpc::InsecureChannelCredentials()))){
                  */
  }
}

AddressString DServer::FindSuccessor(const HashId& id) {
  AddressString peer = FindPredecessor(id);
  std::unique_ptr<Store::Stub> stub = Store::NewStub(
      grpc::CreateChannel(peer, grpc::InsecureChannelCredentials()));

  RequestWithId request;
  request.set_id(id.grpc_bytes());
  AddressResponse response;
  grpc::Status status = stub->Successor(&client_context_, request, &response);

  return response.address();
}

AddressString DServer::FindPredecessor(const HashId& id) {
  AddressString peer = address_;
  while (true) {
  }

  return peer;
}

AddressString DServer::ClosestPrecedingFinger(const HashId& id) {
}

AddressString DServer::successor() {
  return finger_[1].node.address;
}

AddressString DServer::predecessor() {
  return predecessor_.address;
}

void DServer::set_predecessor(const AddressString& node_address) {
}

void DServer::InitFingerTable(const AddressString& known_node_address) {
}

void DServer::UpdateOthers() {
}

void DServer::UpdateFingerTable(const AddressString& node_address, int index) {
}

}  // namespace distrkvs
