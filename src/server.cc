// Copyright (c) 2020 forkbase

#include "server.h"

#include <filesystem>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <thread>

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

namespace distrkvs {

DNode::DNode() {}

DNode::DNode(const std::string& address) : address_(address), id_(KeyWrapper(&address_)) {
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
  : my_node_(kAddress) {

#ifdef DEBUG
    std::cout << "My address: " << my_node_.address() << " " << my_node_.id().str() << std::endl;
#endif

  for (int i=1; i <= kMBit; ++i) {
    finger_[i].start = my_node_.id().add_power_of_2(i - 1);
  }

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

void DServer::Run() {
  StoreServiceImpl service(this);

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.

  AddressString listening_address = "0.0.0.0" + my_node_.address().substr(my_node_.address().find(':'));
  builder.AddListeningPort(listening_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  //server->Wait();

  std::thread server_thread(&Server::Wait, (server.get()));

  std::cout<<"Server listening on " << my_node_.address() << std::endl;

  std::string comm;

  while (true) {
    std::cin>>comm;
    if (comm=="quit" || comm=="shutdown") {
      server->Shutdown();
      break;
    }
    else if (comm=="finger") {
      int index;
      std::cin>>index;

      if (index < 0 || index > kMBit) {
        std::cout << "Invalid finger index\n";
      } else {
        std::cout << "Start: " << finger_[index].start.str() << "\n";
        std::cout << "Node:  " << finger_[index].node.id().str() << " "
                  << finger_[index].node.address() << "\n";
      }
    }
  }

  server_thread.join();
}

void DServer::Join(const AddressString& known_node_address) {
  if (known_node_address.size()) {
    InitFingerTable(known_node_address);
    UpdateOthers();

    // Move keys in (predecessor, n] from successor to n: in set_predecessor
  }
  else {
    for (int i=1; i<=kMBit; ++i) {
      finger_[i].node = my_node_;
    }
    predecessor_ = my_node_;
  }
}

AddressString DServer::FindSuccessor(const AddressString& remote_node, const HashId& id) {
  if (remote_node.empty() || remote_node==my_node_.address()) {
    AddressString peer = FindPredecessor(id);
    return successor(peer);
  }
  else {
    auto stub = Store::NewStub(
        grpc::CreateChannel(
            remote_node, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "FindSuccessor; gRPCalling " << remote_node << std::endl;
#endif

    ClientContext client_context;
    RequestWithId request;
    Address response;
    request.set_id(id.grpc_bytes());
    grpc::Status status = stub->FindSuccessor(&client_context, request, &response);

#ifdef DEBUG
    if (!status.ok()) std::cout << "--gRPC failed" << std::endl;
#endif

    return response.address();
  }
}

AddressString DServer::FindPredecessor(const HashId& id) {
  AddressString peer = my_node_.address(),
                peer_succ = successor(peer);

  while (!id.in_range(HashId(KeyWrapper(&peer)), HashId(KeyWrapper(&peer_succ)))) {
    peer = ClosestPrecedingFinger(peer, id);
    peer_succ = successor(peer);
  }

  return peer;
}

AddressString DServer::ClosestPrecedingFinger(const AddressString& remote_node, const HashId& id) {
  if (remote_node.empty() || remote_node==my_node_.address()) {
    for (int i=kMBit; i>=1; --i) {
      if (finger_[i].node.id().in_range(my_node_.id(), id)) {
        return finger_[i].node.address();
      }
    }
    return my_node_.address();
  }
  else {
    auto stub = Store::NewStub(
        grpc::CreateChannel(
            remote_node, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "CPF; gRPCalling " << remote_node << std::endl;
#endif

    ClientContext client_context;
    RequestWithId request;
    Address response;
    request.set_id(id.grpc_bytes());
    grpc::Status status = stub->ClosestPrecedingFinger(&client_context, request, &response);

#ifdef DEBUG
    if (!status.ok()) std::cout << "--gRPC failed" << std::endl;
#endif

    return response.address();
  }
}

AddressString DServer::successor(const AddressString& remote_node) {
  if (remote_node.empty() || remote_node==my_node_.address()) {
    return finger_[1].node.address();
  }
  else {
    auto stub = Store::NewStub(
        grpc::CreateChannel(
            remote_node, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "successor; gRPCalling " << remote_node << std::endl;
#endif

    ClientContext client_context;
    Address response;
    grpc::Status status = stub->Successor(&client_context, Empty(), &response);

#ifdef DEBUG
    if (!status.ok()) std::cout << "--gRPC failed" << std::endl;
#endif

    return response.address();
  }
}

AddressString DServer::predecessor(const AddressString& remote_node) {
  if (remote_node.empty() || remote_node==my_node_.address()) {
    return predecessor_.address();
  }
  else {
    auto stub = Store::NewStub(
        grpc::CreateChannel(
            remote_node, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "predecessor; gRPCalling " << remote_node << std::endl;
#endif

    ClientContext client_context;
    Address response;
    grpc::Status status = stub->Predecessor(&client_context, Empty(), &response);

#ifdef DEBUG
    if (!status.ok()) std::cout << "--gRPC failed" << std::endl;
#endif

    return response.address();
  }
}

void DServer::set_predecessor(const AddressString& remote_node, const AddressString& node_address) {
  if (remote_node.empty() || remote_node==my_node_.address()) {
    predecessor_ = DNode(node_address);

    auto stub = Store::NewStub(
        grpc::CreateChannel(
            node_address, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "moving keys; gRPCalling " << node_address << std::endl;
#endif

/*    ClientContext hc_client_context; 
    HealthCheckRequest hc_request;
    HealthCheckResponse hc_response;
    grpc::Status status = stub->Check(&hc_client_context, hc_request, &hc_response);

    if (hc_response.status()!=ServingStatus::SERVING) {
#ifdef DEBUG
      std::cout << "--Failed to connect to remote node" << std::endl;
#endif
      return;
    } */

#ifdef DEBUG
    std::cout << "--Connected to new node. Key-values moving to " << node_address << std::endl;
#endif

    rocksdb::Iterator* it = db_->NewIterator(rocksdb::ReadOptions());
    PutRequest request;
    Empty response;

    for (it->SeekToFirst(); it->Valid(); it->Next()) {
      std::string key = it->key().ToString();
      if (predecessor_.id().in_range(HashId(KeyWrapper(&key)), my_node_.id())) {
        request.set_key(key);
        request.set_value(it->value().ToString());

#ifdef DEBUG
        std::cout << "  Key: " << key <<" Value: " << it->value().ToString() << std::endl;
#endif

        ClientContext client_context;
        request.set_from_client(false);
        stub->Put(&client_context, request, &response);
        db_->Delete(WriteOptions(), key);
      }

      // cout << it->key().ToString() << ": " << it->value().ToString() << endl;
    }
    assert(it->status().ok()); // Check for any errors found during the scan
    delete it; 
  }
  else {
    auto stub = Store::NewStub(
        grpc::CreateChannel(
            remote_node, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "set_predecessor; gRPCalling " << remote_node << std::endl;
#endif

    ClientContext client_context;
    Address request;
    request.set_address(node_address);
    Empty response;
    grpc::Status status = stub->SetPredecessor(&client_context, request, &response);

#ifdef DEBUG
    if (!status.ok()) std::cout << "--gRPC failed" << std::endl;
#endif

  }
}

void DServer::InitFingerTable(const AddressString& known_node_address) {
  finger_[1].node = DNode(FindSuccessor(known_node_address, finger_[1].start));
  predecessor_ = predecessor(finger_[1].node.address());
  set_predecessor(finger_[1].node.address(), my_node_.address());

  for (int i=1; i<kMBit; ++i) {
    if (finger_[i+1].start.in_range(my_node_.id(), finger_[i].node.id())) {
      finger_[i+1].node = finger_[i].node;
    } else {
      finger_[i+1].node = DNode(FindSuccessor(known_node_address, finger_[i+1].start));
    }
  }
}

void DServer::UpdateOthers() {
  for (int i=1; i<=kMBit; ++i) {
    AddressString peer = FindPredecessor(my_node_.id().subtract_power_of_2(i-1));
    UpdateFingerTable(peer, my_node_.address(), i);
  }
}

void DServer::UpdateFingerTable(const AddressString& remote_node, const AddressString& node_address, int index) {
  if (remote_node.empty() || remote_node==my_node_.address()) {
    if (DNode(node_address).in_range(my_node_, finger_[index].node)) {
      finger_[index].node = DNode(node_address);
      UpdateFingerTable(predecessor_.address(), node_address, index);
    }
  }
  else {
    auto stub = Store::NewStub(
        grpc::CreateChannel(
            remote_node, grpc::InsecureChannelCredentials()));

#ifdef DEBUG
    std::cout << "UpdateFingerTable; gRPCalling " << remote_node << std::endl;
#endif

    ClientContext client_context;

    UpdateFingerTableRequest request;
    request.set_address(node_address);
    request.set_index(index);
    Empty response;
    grpc::Status status = stub->UpdateFingerTable(&client_context, request, &response);

#ifdef DEBUG
    if (!status.ok()) std::cout << "--gRPC failed" << std::endl;
#endif

  }
}

}  // namespace distrkvs
