#pragma once
#include <pqxx/pqxx>
#include <string>
#include <iostream>

class DatabaseControll
{
private:
	

public:
	pqxx::connection connectionObject;
	pqxx::work worker;
	DatabaseControll(std::string connectionString) : connectionObject(connectionString.c_str()), worker(connectionObject) {};

};

