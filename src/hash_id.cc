#include "hash_id.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <openssl/sha.h>
#include <algorithm>
#include <string>
#include <vector>

namespace distrkvs {

KeyWrapper::KeyWrapper(std::string* key) : key_(key) {
}

std::string* KeyWrapper::key() {
  return key_;
}

HashId::HashId() {
  for (auto i=0; i < SHA256_DIGEST_LENGTH; ++i) digest_[i] = 0;
}

HashId::HashId(int n) {
  int i = SHA256_DIGEST_LENGTH - 1;

  while (n) {
    digest_[i] = n % 256;
    --i;
    n /= 256; 
  }
}

HashId::HashId(const std::string& digest) {
  if (digest.size()==SHA256_DIGEST_LENGTH)
    std::memcpy(digest_, &digest[0], SHA256_DIGEST_LENGTH);
}

HashId::HashId(KeyWrapper key_wrapper) {
  SHA256(reinterpret_cast<const unsigned char*>(&((*(key_wrapper.key()))[0])), key_wrapper.key()->size(), digest_);
}

void HashId::set(const std::string& digest) {
  if (digest.size()==SHA256_DIGEST_LENGTH)
    std::memcpy(digest_, &digest[0], SHA256_DIGEST_LENGTH);
}

bool HashId::in_range(const HashId& left, const HashId& right) const {
  if (left==right) return true;
  if (left < *this) {
    return (*this) < right || right < left;
  } else {
    return (*this) < right && right < left;
  }
}

HashId HashId::add_power_of_2(int p) const {
  HashId hash = *this;
  int mod = p % 8,
      offset = p / 8;

  int ind = SHA256_DIGEST_LENGTH - 1 - offset;

  hash.digest_[ind] += (1 << mod);
  if (hash.digest_[ind] < (1 << mod)) {
    while (true) {
      --ind;
      if (ind < 0) break;
      hash.digest_[ind]++;
      if (hash.digest_[ind]!=0) break;
    }
  }

  return hash;
}

HashId HashId::subtract_power_of_2(int p) const {
  HashId hash = *this;
  int mod = p % 8,
      offset = p / 8,
      ind = SHA256_DIGEST_LENGTH - 1 - offset,
      last = 256 - (1<<mod);

  hash.digest_[ind] += last;
  if (hash.digest_[ind] >= last) {
    --ind;
    while (ind>=0 && hash.digest_[ind]==0) {
      hash.digest_[ind] = 255;
      --ind;
    }

    if (ind>=0) hash.digest_[ind]--;
  }

  return hash;
}

HashId HashId::negate() const {
  HashId h1, h2;
  for (auto i=0; i<SHA256_DIGEST_LENGTH; ++i) {
    h1.digest_[i] = ~digest_[i];
  }
  h2.digest_[SHA256_DIGEST_LENGTH - 1] = 1;
  return h1 + h2;
}

HashId HashId::power_of_2(int p) {
  HashId hash;
  int mod = p % 8,
      offset = p / 8;

  hash.digest_[SHA256_DIGEST_LENGTH - 1 - offset] = 1<<mod;
  return hash;
}

std::string HashId::str() const {
  char buf[10]{};
  std::string hash;

  for (int i=0; i < SHA256_DIGEST_LENGTH; ++i) {
    sprintf(buf, "%02x ", digest_[i]);
    hash.append(buf);
  }

  return hash;
}

std::string HashId::grpc_bytes() const {
  return std::string(&digest_[0], &digest_[0] + SHA256_DIGEST_LENGTH);
}

bool operator < (const HashId& h1, const HashId& h2) {
  for (int i=0; i<SHA256_DIGEST_LENGTH; ++i) {
    if (h1.digest_[i] < h2.digest_[i]) return true;
    if (h1.digest_[i] > h2.digest_[i]) return false;
  }

  return false;
}

bool operator == (const HashId& h1, const HashId& h2) {
  for (auto i=0; i<SHA256_DIGEST_LENGTH; ++i) {
    if (h1.digest_[i] != h2.digest_[i]) return false;
  }

  return true;
}

HashId operator + (const HashId& h1, const HashId& h2) {
  HashId hash;
  unsigned char carry = 0;
  for (int i = SHA256_DIGEST_LENGTH - 1; i>=0; --i) {
    hash.digest_[i] = h1.digest_[i] + h2.digest_[i] + carry;
    if (hash.digest_[i] < h1.digest_[i] || (hash.digest_[i]==h1.digest_[i] && carry == 1)) {
      carry = 1;
    } else {
      carry = 0;
    }
  }

  return hash;
}

HashId operator - (const HashId& h1, const HashId& h2) {
  return h1 + h2.negate();
}

}  // namespace distrkvs
