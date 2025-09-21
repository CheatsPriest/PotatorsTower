#include "ConnectionPool.h"

ConnectionPool ConnectionPool::connectionPool(
    std::format("host={} port={} dbname={} user={} password={}",
            std::getenv("POSTGRES_HOST"), std::getenv("POSTGRES_PORT"),
            std::getenv("POSTGRES_DB"), std::getenv("POSTGRES_USER"),
            std::getenv("POSTGRES_PASSWORD")), 6);