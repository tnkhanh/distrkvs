cmake_minimum_required(VERSION 3.13)

project(distrkvs VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -O2 -W -Wextra -Wall -Wsign-compare -Wshadow")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-unused-variable -Woverloaded-virtual -Wnon-virtual-dtor -Wno-missing-field-initializers -Wno-strict-aliasing")
# for debug: set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic")

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
set(gen_dir "${PROJECT_BINARY_DIR}/${PROJECT_NAME}/generated")
file(MAKE_DIRECTORY ${gen_dir})
set(proto_srcs "${gen_dir}/${proto_name}.pb.cc")
set(proto_hdrs "${gen_dir}/${proto_name}.pb.h")
set(grpc_srcs "${gen_dir}/${proto_name}.grpc.pb.cc")
set(grpc_hdrs "${gen_dir}/${proto_name}.grpc.pb.h")
add_custom_command(
      OUTPUT "${proto_srcs}" "${proto_hdrs}" "${grpc_srcs}" "${grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${gen_dir}"
      --cpp_out "${gen_dir}"
        -I "${proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${proto}"
      DEPENDS "${proto}")

# Needed libdl before, why?!

add_library(${PROJECT_NAME}_client SHARED ${proto_srcs} ${grpc_srcs} src/client.cc)
target_include_directories(${PROJECT_NAME}_client PUBLIC ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/include" ${PROJECT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}_client PUBLIC pthread gRPC::grpc++)

add_library(${PROJECT_NAME}_server SHARED ${proto_srcs} ${grpc_srcs} src/server.cc "src/store_service_impl.cc" "src/hash_id.cc")
target_include_directories(${PROJECT_NAME}_server PUBLIC ${PROJECT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/include" ${PROJECT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}_server PUBLIC pthread "${RocksDB_DIR}/../../librocksdb.so" RocksDB::rocksdb gRPC::grpc++_reflection gRPC::grpc++ protobuf::libprotobuf stdc++fs crypto)

# RocksDB::rocksdb is the lib name
# "${RocksDB_DIR}/../../librocksdb.so" is the path

add_executable(sample_client "src/sample_client.cc" "src/client.cc" ${proto_srcs} ${grpc_srcs})
target_include_directories(sample_client PUBLIC "${PROJECT_SOURCE_DIR}/include" ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(sample_client PUBLIC dl pthread gRPC::grpc++) 

add_executable(sample_server "src/sample_server.cc" "src/server.cc" "src/store_service_impl.cc" "src/hash_id.cc" ${proto_srcs} ${grpc_srcs})
target_include_directories(sample_server PUBLIC "${PROJECT_SOURCE_DIR}/include" ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(sample_server PUBLIC dl pthread "${RocksDB_DIR}/../../librocksdb.so" RocksDB::rocksdb gRPC::grpc++_reflection gRPC::grpc++ protobuf::libprotobuf stdc++fs crypto)

include(GNUInstallDirs)

message(STATUS "INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}")

#install(DIRECTORY "include/${PROJECT_NAME}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

if (DEBUG)
  add_compile_definitions(DEBUG)
endif()

if (TESTS)
  add_executable(test src/test.cc src/hash_id.cc)
  target_link_libraries(test PUBLIC crypto)
endif()
