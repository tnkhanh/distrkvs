// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_HASH_ID_H_
#define DISTRKVS_HASH_ID_H_

#include <string>
#include <openssl/sha.h>

namespace distrkvs {
class HashId {
 public:
  HashId();
  HashId(const std::string& digest); 
  void hash(const std::string& input); 
  void set(const std::string& digest); 
  std::string str();
  std::string grpc_bytes();
  bool in_range(const HashId& left, const HashId& right) const;

 private:
  unsigned char digest_[SHA256_DIGEST_LENGTH];
  friend bool operator < (const HashId& h1, const HashId& h2); 
};

}  // namespace distrkvs

#endif  // DISTRKVS_HASH_ID_H_
