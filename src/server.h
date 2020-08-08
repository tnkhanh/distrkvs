// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_SERVER_H_
#define DISTRKVS_SERVER_H_

#include <string>

namespace distrkvs::server {
void RunServer(const std::string& kServerAddress);
}  // namespace distrkvs::server

#endif  // DISTRKVS_SERVER_H_
