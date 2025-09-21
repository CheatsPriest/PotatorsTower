cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(spectator_service)

include(FetchContent)
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.12.1  # Use a specific version
)
FetchContent_MakeAvailable(httplib)

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
    # src/a.cpp
    src/main.cpp
    src/ConnectionPool.cpp
    src/ConsoleManager.cpp
    src/Controller.cpp
    src/DatabaseControll.cpp
    src/DateClass.cpp
    src/DomainAdder.cpp
    src/DownServersNotifier.cpp
    src/Messenger.cpp
    src/PingDomains.cpp
    src/Pinger.cpp
)

target_link_libraries(
    spectator_service
    protolib
    libpqxx::pqxx
    httplib
)

target_include_directories(spectator_service PRIVATE ${OPENSSL_INCLUDE_DIR})
