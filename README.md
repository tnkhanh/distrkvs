# distrkvs
A distributed key-value store that features elastic scaling.

# Building from source
```bash
# Install pre-requisites
sudo apt-get install build-essential autoconf libtool pkg-config

# Build with cmake
mkdir -p build && cd build
cmake .. -DWITH_GFLAGS=OFF -DWITH_TESTS=0 -DWITH_BENCHMARK_TOOLS=0 -DWITH_TOOLS=0 -DFAIL_ON_WARNINGS=OFF -DROCKSDB_BUILD_SHARED=OFF -DRE2_BUILD_TESTING=OFF
cmake --build .
```
