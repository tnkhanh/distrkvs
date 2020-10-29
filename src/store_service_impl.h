// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_STORE_SERVICE_IMPL_H
#define DISTRKVS_STORE_SERVICE_IMPL_H

#include "store.grpc.pb.h"
#include "server.h"

using rocksdb::DB;
using grpc::ClientContext;

namespace distrkvs {

class StoreServiceImpl final : public Store::Service {
 public:
  StoreServiceImpl(const DServer* p_server);

 private:
  DServer* p_server_;
}

}  // namespace distrkvs

#endif  // DISTRKVS_STORE_SERVICE_IMPL_H
