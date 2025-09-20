#include <iostream>
#include <thread>
#include <chrono>
#include <grpcpp/grpcpp.h>
#include "proto/api.grpc.pb.h"
#include "proto/api.pb.h"
#include "pqxx/pqxx"

std::string server_ip = "127.0.0.1:7584", client_ip = "127.0.0.1:7585";

class CppServerImpl : public ServerChecker::Service
{
    ::grpc::Status GetServerInfo(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::ServerInfo, ::Endpoint>* stream)
    {
        std::cout << "GetServerInfo was called\n";
        Endpoint endpoint;
        while (stream->Read(&endpoint))
        {
            std::cout << "Received " << endpoint.endpoint() << " endpoint\n";
            ServerInfo info;
            info.set_endpoint(endpoint.endpoint());
            info.set_status(ServerStatus::UP);
            //info.set_allocated_time(google::protobuf::Timestamp());
            stream->Write(info);
            std::cout << "";
        }
        return grpc::Status::OK;
    }
};

void RunServer() {
    CppServerImpl service;
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_ip, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
}

int main()
{
    std::cout << "Started" << std::endl;

    // auto python_channel = grpc::CreateChannel(client_ip, grpc::InsecureChannelCredentials());
    // auto python_stub = ServerChecker::NewStub(python_channel);

    // std::thread server_thread(RunServer);

    // std::string input;
    // while (true)
    // {
    //     std::cin >> input;
    //     if (input == "exit")
    //         break;
        
    //     grpc::ClientContext context;
    //     Response response;

    //     std::unique_ptr<grpc::ClientWriter<Endpoint>> writer(
    //         python_stub->NotifyClientsAboutFall(&context, &response));

    //     Endpoint endpoint;
    //     endpoint.set_endpoint(input);
    //     writer->Write(endpoint);
        
    //     writer->WritesDone();
    //     auto status = writer->Finish();
    //     if (status.ok())
    //         std::cout << "Request has been successfully sent.\n";
    //     else
    //         std::cout << status.error_message() << '\n';
    // }

    //server_thread.join();    

    std::cout << "Ended" << std::endl;
}
