// Copyright (c) 2020 forkbase

#ifndef DISTRKVS_HASH_ID_H_
#define DISTRKVS_HASH_ID_H_

#include <string>
#include <openssl/sha.h>

namespace distrkvs {
class KeyWrapper {
  public:
    KeyWrapper(std::string* key);
    std::string* key();
  private:
    std::string* key_;
};

class HashId {
 public:
  HashId();
  explicit HashId(int n);
  explicit HashId(const std::string& digest); 
  explicit HashId(KeyWrapper key_wrapper); 
  void set(const std::string& digest); 
  bool in_range(const HashId& left, const HashId& right) const;
  HashId add_power_of_2(int p) const;
  HashId subtract_power_of_2(int p) const;
  HashId negate() const;
  static HashId power_of_2(int p);
  std::string str() const;
  std::string grpc_bytes() const;
 
 private:
  unsigned char digest_[SHA256_DIGEST_LENGTH];
  friend bool operator < (const HashId& h1, const HashId& h2); 
  friend bool operator == (const HashId& h1, const HashId& h2);
  friend HashId operator + (const HashId& h1, const HashId& h2);
  friend HashId operator - (const HashId& h1, const HashId& h2);
  friend class HashIdTest;
};

}  // namespace distrkvs

#endif  // DISTRKVS_HASH_ID_H_
