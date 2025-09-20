cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(spectator_service)

find_package(libpqxx CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(protolib proto/api.proto)
target_link_libraries(protolib gRPC::grpc++)
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(
    TARGET protolib
    LANGUAGE cpp
)

protobuf_generate(
    TARGET protolib
    LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.cc .grpc.pb.h
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

add_executable(
    spectator_service
    src/main.cpp
)

target_link_libraries(
    spectator_service
    protolib
    libpqxx::pqxx
)
