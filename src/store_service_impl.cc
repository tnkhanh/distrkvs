// Copyright (c) 2020 forkbase

#include "store_service_impl.h"

#include <iostream>

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

using google::protobuf::Empty;
using grpc::Status;
using grpc::ServerContext;

namespace distrkvs {
StoreServiceImpl::StoreServiceImpl(const DServer* p_server) : p_server_(p_server) {}

Status StoreServiceImpl::Get(ServerContext* context, 
    const GetRequest* request, GetResponse* response) override {
  return Status::OK;
}

Status StoreServiceImpl::Put(ServerContext* context,
    const PutRequest* request, Empty* response) override {
  return Status::OK;
}

Status StoreServiceImpl::Successor(ServerContext* context, 
    const Empty* request, Address* response) override {
  return Status::OK;
}

Status StoreServiceImpl::FindSuccessor(ServerContext* context,
    const RequestWithId* request, Address* response) override {
  return Status::OK;
}

Status StoreServiceImpl::ClosestPrecedingFinger(ServerContext* context,
    const RequestWithId* request, Address* response) override {
  return Status::OK;
}

Status StoreServiceImpl::SetPredecessor(ServerContext* context,
    const Address* request, Empty* response) override {
  return Status::OK;
}

Status StoreServiceImpl::Predecessor(ServerContext* context,
    const Empty* request, Address* response) override {
  return Status::OK;
}

Status StoreServiceImpl::UpdateFingerTable(ServerContext* context,
    const UpdateFingerTableRequest* request, Empty* response) override {
  return Status::OK;
}

}  // namespace distrkvs
