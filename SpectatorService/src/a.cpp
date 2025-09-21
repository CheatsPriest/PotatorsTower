#include <pqxx/pqxx>
#include <cstdlib>
#include <iostream>

int main() {
    try {
        std::string host = std::getenv("POSTGRES_HOST");
        std::string port = std::getenv("POSTGRES_PORT");
        std::string db_name = std::getenv("POSTGRES_DB");
        std::string user = std::getenv("POSTGRES_USER");
        std::string password = std::getenv("POSTGRES_PASSWORD");
        
        std::string connection_string =
            std::format("host={} port={} dbname={} user={} password={}",
            std::getenv("POSTGRES_HOST"), std::getenv("POSTGRES_PORT"),
            std::getenv("POSTGRES_DB"), std::getenv("POSTGRES_USER"),
            std::getenv("POSTGRES_PASSWORD"));
        
        std::cout << "Connecting with: " << connection_string << std::endl;
        
        pqxx::connection connectionObject(connection_string.c_str());
        
        if (connectionObject.is_open()) {
            std::cout << "Connected successfully to " << connection_string << std::endl;
        } else {
            std::cout << "Connection failed!" << std::endl;
        }
        
    } catch (const std::exception &e) {
        std::cerr << "Connection error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}