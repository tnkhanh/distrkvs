// Copyright (c) 2020 forkbase

#include <string>
#include <iostream>

#include "src/server.h"

int main(int argc, char **argv) {
  std::ios_base::sync_with_stdio(false);
  std::cout << "Distrkvs server\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  const std::string kDBPath("/opt/database");
  const std::string kServerAddress(argc > 1 ? argv[1] : "0.0.0.0");

  distrkvs::DistrkvsServer server(kDBPath, kServerAddress);
  server.LoadConfigFromFile("config.txt");

  std::cout<<"Server listening on " << kServerAddress << ":50017" << std::endl;
  server.Run();

  return 0;
}
