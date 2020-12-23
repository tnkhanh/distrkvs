// Copyright (c) 2020 forkbase

#include "store_service_impl.h"

#include <iostream>
#include <string>

#include "rocksdb/db.h"

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

#include "hash_id.h"

using google::protobuf::Empty;
using grpc::ServerContext;
using rocksdb::ReadOptions;
using rocksdb::WriteOptions;

namespace distrkvs {
StoreServiceImpl::StoreServiceImpl(DServer* dserver) : dserver_(dserver) {}

Status StoreServiceImpl::Get(ServerContext* context, 
    const GetRequest* request, GetResponse* response) {
  if (request->from_client()) {
    std::string key = request->key();
    AddressString peer = dserver_->FindSuccessor("", HashId(KeyWrapper(&key)));

#ifdef DEBUG
    std::cout << "Get: Key: " << key << " in node: " << peer << std::endl;
#endif

    auto stub = Store::NewStub(
        grpc::CreateChannel(
            peer, grpc::InsecureChannelCredentials()));

    ClientContext client_context;
    GetRequest inner_request;
    inner_request.set_key(request->key());
    inner_request.set_from_client(false);

    GetResponse inner_response;
    grpc::Status status = stub->Get(&client_context, inner_request, &inner_response);
    response->set_value(inner_response.value());
    return status;
  } else {
    std::string value;
    rocksdb::Status s = dserver_->db_->Get(ReadOptions(), request->key(), &value);

#ifdef DEBUG
    std::cout << "Get: Key: " << request->key() << "\nValue: " << value << std::endl;
#endif

    if (s.ok()) {
      response->set_value(value);
      return grpc::Status::OK;
    } else if (s.IsNotFound()) {
      return grpc::Status(grpc::StatusCode::NOT_FOUND, s.ToString());
    } else {
      return grpc::Status(grpc::StatusCode::ABORTED, s.ToString());
    }
  }
}

Status StoreServiceImpl::Put(ServerContext* context,
    const PutRequest* request, Empty* response) {
  if (request->from_client()) {
    std::string key = request->key();
    AddressString peer = dserver_->FindSuccessor("", HashId(KeyWrapper(&key)));

#ifdef DEBUG
    std::cout << "Put: Key: " << key << " Value: " << request->value() << " in node: " << peer << std::endl;
#endif

    auto stub = Store::NewStub(
        grpc::CreateChannel(
            peer, grpc::InsecureChannelCredentials()));

    ClientContext client_context;
    PutRequest inner_request;
    inner_request.set_key(request->key());
    inner_request.set_value(request->value());
    inner_request.set_from_client(false);
    Empty inner_response;
    return stub->Put(&client_context, inner_request, &inner_response);
  } else {
    rocksdb::Status s = dserver_->db_->Put(WriteOptions(), request->key(), request->value());

#ifdef DEBUG
    std::cout << "Put: Key: " << request->key() << " Value: " << request->value() << std::endl;
#endif

    if (s.ok()) {
      return grpc::Status::OK;
    } else {
      return grpc::Status(grpc::StatusCode::ABORTED, s.ToString());
    }
  }
}

Status StoreServiceImpl::Successor(ServerContext* context, 
    const Empty* request, Address* response) {
  response->set_address(dserver_->successor(""));
  return grpc::Status::OK;
}

Status StoreServiceImpl::FindSuccessor(ServerContext* context,
    const RequestWithId* request, Address* response) {
  response->set_address(dserver_->FindSuccessor("", HashId(request->id())));
  return grpc::Status::OK;
}

Status StoreServiceImpl::ClosestPrecedingFinger(ServerContext* context,
    const RequestWithId* request, Address* response) {
  response->set_address(dserver_->ClosestPrecedingFinger("", HashId(request->id())));
  return grpc::Status::OK;
}

Status StoreServiceImpl::SetPredecessor(ServerContext* context,
    const Address* request, Empty* response) {
  dserver_->set_predecessor("", request->address());
  return grpc::Status::OK;
}

Status StoreServiceImpl::Predecessor(ServerContext* context,
    const Empty* request, Address* response) {
  response->set_address(dserver_->predecessor(""));
  return grpc::Status::OK;
}

Status StoreServiceImpl::UpdateFingerTable(ServerContext* context,
    const UpdateFingerTableRequest* request, Empty* response) {
  dserver_->UpdateFingerTable("", request->address(), request->index());
  return grpc::Status::OK;
}

}  // namespace distrkvs
