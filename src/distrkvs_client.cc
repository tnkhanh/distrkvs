// Copyright (c) 2020 forkbase

#include <filesystem>
#include <string>
#include <iostream>
#include <sstream>

#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "store.grpc.pb.h"
#include "src/client.h"

using distrkvs::KeyString;
using distrkvs::ValueString;
using distrkvs::DStatus;

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);

  std::cout << "Distrkvs client\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  const std::string kServerAddress(argc > 1 ? argv[1] : "localhost");
  distrkvs::DistrkvsClient client(kServerAddress);

  std::string comm;
  KeyString key;
  ValueString value;
  DStatus status;

  while (true) {
    std::cin>>comm;
    if (comm=="Put") {
      std::cout<<"Put: ";
      std::cin>>key>>value;
      status = client.Put(key, value);
      if (status==DStatus::kOk) {
        std::cout << "OK!\n";
      } else {
        std::cout << "Not OK!\n";
      }

    } else if (comm=="Get") {
      std::cout<<"Get: ";
      std::cin>>key;
      status = client.Get(key, &value);
      if (status==DStatus::kOk) {
        std::cout << "OK!\n";
        std::cout << "Value: " << value << "\n";
      } else {
        std::cout << "Not OK!\n";
      }

    } else if (comm=="Quit" || comm=="Exit") {
      break;
    } else {
      std::cout<<"Not a command!\n";
    }
  }

  return 0;
}
