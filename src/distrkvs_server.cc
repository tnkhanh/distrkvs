// Copyright (c) 2020 forkbase

#include <string>
#include <iostream>

#include "src/replica.h"
#include "src/server.h"

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  std::cout << "Distrkvs server\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  const std::string kDBPath("/opt/database");
  const std::string kServerAddress("0.0.0.0:50017");
  std::cout<<"Server listening on " << kServerAddress << std::endl;
  
  distrkvs::DistrkvsServer server(kDBPath, kServerAddress);
  server.run();

  return 0;
}
