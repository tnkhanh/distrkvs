cmake_minimum_required(VERSION 3.13)

project(distrkvs VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# assumes protobuf is installed
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using Protobuf ${Protobuf_VERSION}")

find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

find_package(RocksDB CONFIG REQUIRED)
message(STATUS "Using RocksDB ${RocksDB_VERSION} from ${RocksDB_DIR}")

set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
 
# Proto files
set (proto_name "store")
get_filename_component(proto "protos/${proto_name}.proto" ABSOLUTE)
get_filename_component(proto_path "${proto}" PATH)

# Generated sources
set(proto_srcs "${PROJECT_BINARY_DIR}/${proto_name}.pb.cc")
set(proto_hdrs "${PROJECT_BINARY_DIR}/${proto_name}.pb.h")
set(grpc_srcs "${PROJECT_BINARY_DIR}/${proto_name}.grpc.pb.cc")
set(grpc_hdrs "${PROJECT_BINARY_DIR}/${proto_name}.grpc.pb.h")
add_custom_command(
      OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${PROJECT_BINARY_DIR}"
      --cpp_out "${PROJECT_BINARY_DIR}"
        -I "${proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto}"
      DEPENDS "${proto}")

foreach (_target
  "${PROJECT_NAME}_server" "${PROJECT_NAME}_client")
  add_executable(${_target} "src/${_target}.cc" src/server.cc src/replica.cc ${proto_srcs} ${grpc_srcs})
  target_include_directories(${_target} PUBLIC ${PROJECT_BINARY_DIR})
  target_include_directories(${_target} PUBLIC ${PROJECT_SOURCE_DIR})
  target_link_libraries(${_target} PUBLIC pthread dl RocksDB::rocksdb gRPC::grpc++_reflection gRPC::grpc++ protobuf::libprotobuf stdc++fs)
endforeach()
