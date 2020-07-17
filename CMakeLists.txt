cmake_minimum_required(VERSION 3.13)

project(distrkvs VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file("${PROJECT_NAME}Config.h.in" "${PROJECT_NAME}Config.h")

# assumes protobuf is installed
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${protobuf_VERSION}")

set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
set(_REFLECTION gRPC::grpc++_reflection)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")

set(_GRPC_GRPCPP gRPC::grpc++)
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

add_executable(${PROJECT_NAME} src/main.cc ${proto_srcs} ${grpc_srcs})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR} ~/local/include)

find_library(ROCKSDB NAMES librocksdb.a HINTS ~/work/rocksdb/buildnowith REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC pthread dl ${ROCKSDB} stdc++fs)
target_link_libraries(${PROJECT_NAME} PUBLIC ${_REFLECTION}  ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

