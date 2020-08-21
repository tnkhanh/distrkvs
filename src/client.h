// Copyright (c) 2020 forkbase

#include <string>
#include <iostream>
#include <sstream>

#include "grpcpp/grpcpp.h"

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "store.grpc.pb.h"

namespace distrkvs {

using KeyString = std::string;
using ValueString = std::string;

enum DStatus{
  kOk,
  kRPCFailed,
  kNotOk
};

class DistrkvsClient {
 public:
  explicit DistrkvsClient(const std::string& kServerAddress);

  DStatus Put(const KeyString& key, const ValueString& value);
  DStatus Get(const KeyString& key, ValueString* value);

 private:
  std::unique_ptr<Store::Stub> stub_;
};

}  // namespace distrkvs
