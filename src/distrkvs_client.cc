// Copyright (c) 2020 forkbase

#include <filesystem>
#include <string>
#include <iostream>
#include <sstream>

#include "grpcpp/grpcpp.h"

#include "store.grpc.pb.h"
#include "src/client.h"

using distrkvs::KeyString;
using distrkvs::ValueString;

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);

  std::cout << "Distrkvs client\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  const std::string kServerAddress(argc > 1 ? argv[1] : "localhost:50017");
  distrkvs::DClient client(kServerAddress);

  std::string comm;
  KeyString key;
  ValueString value;
  grpc::Status status;

  while (true) {
    std::cin>>comm;
    if (comm=="Put") {
      std::cout<<"Put: ";
      std::cin>>key>>value;
      status = client.Put(key, value);
      std::cout << status.error_message() <<" Done!\n"; 
    } else if (comm=="Get") {
      std::cout<<"Get: ";
      std::cin>>key;
      status = client.Get(key, &value);
      std::cout << "Value: " << value <<"\n" 
          << status.error_message() <<" Done!\n"; 
    } else if (comm=="Quit" || comm=="Exit") {
      break;
    } else {
      std::cout<<"Not a command!\n";
    }
  }

  return 0;
}
