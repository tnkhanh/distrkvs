// Copyright (c) 2020 forkbase

#include <signal.h>
#include <execinfo.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include "src/server.h"

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

int main(int argc, char **argv) {
// signal(SIGABRT, handler);
  std::ios_base::sync_with_stdio(false);
  std::cout << "Distrkvs server\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  if (argc <= 1) {
    std::cout << "Usage: ";
    std::cout << argv[0] << " your_address : start a new ring\n";
    std::cout << argv[0] << " your_address remote_address : join an existing ring\n";
    return 0;
  }

  const std::string kDBPath = std::string("/opt/database/") + std::string(argv[1]);
  const std::string kAddress(argv[1]);
  const std::string kRemoteNode(argc > 2 ? argv[2] : "");

  distrkvs::DServer server(kDBPath, kAddress);

  server.Join(kRemoteNode);

  if (kRemoteNode.empty()) {
    std::cout << "Server created a new ring" << std::endl;
  } else {
    std::cout << "Server joined remote node " << kRemoteNode << std::endl;
  }
  server.Run();

  return 0;
}
