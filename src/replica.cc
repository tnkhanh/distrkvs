// Copyright (c) 2020 forkbase

#include "replica.h"

namespace distrkvs {

Replica::Replica(std::string server_name, int replica_number)
    : server_name_(server_name), replica_number_(replica_number) {
}
}  // namespace distrkvs
