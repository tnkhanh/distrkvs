// Copyright (c) 2020 forkbase

#include <filesystem>
#include <string>
#include <iostream>

#include "rocksdb/db.h"
#include "rocksdb/slice.h"
#include "rocksdb/options.h"

#include "src/replica.h"
#include "src/server.h"

using rocksdb::DB;
using rocksdb::Options;
using rocksdb::PinnableSlice;
using rocksdb::WriteOptions;
using rocksdb::ReadOptions;
using rocksdb::WriteBatch;

int main(int argc, char **argv) {
//  std::ios_base::sync_with_stdio(false);

  std::cout << "Distrkvs server\n";
  std::cout << "C++ Standard: " << __cplusplus << "\n";

  const std::string kDBPath = "/opt/database";
  std::filesystem::create_directories(kDBPath);

  DB* db;
  Options options;
  // Optimize RocksDB. This is the easiest way to get RocksDB to perform well
  options.IncreaseParallelism();
  options.OptimizeLevelStyleCompaction();
  // create the DB if it's not already present
  options.create_if_missing = true;

  // open DB
  rocksdb::Status s = DB::Open(options, kDBPath, &db);
  assert(s.ok());

  // Put key-value
  s = db->Put(WriteOptions(), "key1", "value");
  assert(s.ok());
  std::string value;
  // get value
  s = db->Get(ReadOptions(), "key1", &value);
  assert(s.ok());
  assert(value == "value");

  const std::string kServerAddress("0.0.0.0:50017");

  std::cout<<"Server listening on " << kServerAddress <<"\n";

  distrkvs::server::RunServer(kServerAddress);

  delete db;

  return 0;
}
