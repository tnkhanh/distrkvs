// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_CLIENT_H_
#define DISTRKVS_CLIENT_H_

#include <string>
#include <iostream>
#include <sstream>

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"

namespace distrkvs {

using KeyString = std::string;
using ValueString = std::string;

enum DStatus{
  kOk,
  kRPCFailed,
  kNotOk
};

class DClient {
 public:
  explicit DClient(const std::string& kServerAddress);

  grpc::Status Put(const KeyString& key, const ValueString& value);
  grpc::Status Get(const KeyString& key, ValueString* value);

 private:
  std::unique_ptr<Store::Stub> stub_;
};

}  // namespace distrkvs

#endif  // DISTRKVS_CLIENT_H_
