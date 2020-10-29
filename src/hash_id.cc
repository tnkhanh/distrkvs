#include "hash_id.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <openssl/sha.h>
#include <algorithm>
#include <string>
#include <vector>

#include "store.grpc.pb.h"

namespace distrkvs {
HashId::HashId() {
}

HashId::HashId(const std::string& digest) {
  if (digest.size()==SHA256_DIGEST_LENGTH)
    std::memcpy(digest_, &digest[0], SHA256_DIGEST_LENGTH);
}

void HashId::hash(const std::string& input) {
  SHA256(reinterpret_cast<const unsigned char*>(&input[0]), input.size(), digest_);
}

void HashId::set(const std::string& digest) {
  if (digest.size()==SHA256_DIGEST_LENGTH)
    std::memcpy(digest_, &digest[0], SHA256_DIGEST_LENGTH);
}

std::string HashId::str() {
  char buf[10]{};
  std::string hash;

  for (int i=0; i < SHA256_DIGEST_LENGTH; ++i) {
    sprintf(buf, "%02x ", digest_[i]);
    hash.append(buf);
  }

  return hash;
}

std::string HashId::grpc_bytes() {
  return std::string(&digest_[0], &digest_[0] + SHA256_DIGEST_LENGTH);
}

bool HashId::in_range(const HashId& left, const HashId& right) const {
  if (left < (*this)) {
    if ((*this) < right) return true;
    if (right < left) return true;
    return false;
  } else if ((*this) < left) {
    return (*this) < right && right < left;
  }

  return false;
}

bool operator < (const HashId& h1, const HashId& h2) {
  for (int i=0; i<SHA256_DIGEST_LENGTH; ++i) {
    if (h1.digest_[i] < h2.digest_[i]) return true;
    if (h1.digest_[i] > h2.digest_[i]) return false;
  }

  return false;
}

}  // namespace distrkvs
