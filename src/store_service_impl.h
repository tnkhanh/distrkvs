// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_STORE_SERVICE_IMPL_H_
#define DISTRKVS_STORE_SERVICE_IMPL_H_

#include "store.grpc.pb.h"
#include "server.h"

using rocksdb::DB;
using grpc::ClientContext;
using grpc::Status;
using grpc::ServerContext;
using google::protobuf::Empty;

namespace distrkvs {

class StoreServiceImpl final : public Store::Service {
 public:
  StoreServiceImpl(DServer* dserver);

 private:
  DServer* dserver_;

  Status Get(ServerContext* context, 
      const GetRequest* request, GetResponse* response) override;

  Status Put(ServerContext* context,
      const PutRequest* request, Empty* response) override;

  Status Successor(ServerContext* context, 
      const Empty* request, Address* response) override;

  Status FindSuccessor(ServerContext* context,
      const RequestWithId* request, Address* response) override;

  Status ClosestPrecedingFinger(ServerContext* context,
      const RequestWithId* request, Address* response) override;

  Status SetPredecessor(ServerContext* context,
      const Address* request, Empty* response) override;

  Status Predecessor(ServerContext* context,
      const Empty* request, Address* response) override;

  Status UpdateFingerTable(ServerContext* context,
      const UpdateFingerTableRequest* request, Empty* response) override;

};

}  // namespace distrkvs

#endif  // DISTRKVS_STORE_SERVICE_IMPL_H_
