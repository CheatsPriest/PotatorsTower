#pragma once
#include "ConnectionPool.h"
#include "Controller.h"
#include "DomainAdder.h"
#include "ConsoleManager.h"

template<typename AddDomainPolicy>
class GenericAddDomain {
private:
	AddDomainPolicy inParser;
	std::vector<std::string> returnNewDomains() {
		return inParser.returnNewDomains();
	}
public:

	void run() {

		ConsoleManager::console.sendMesseage("Add Parser Runing!");

		while (!Controller::quite) {

			std::vector<std::string> mass = returnNewDomains();

			for (auto el : mass) {
				sqladd::addInBaseDomain(el);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}

	}

};

class BufTabblePolicy {
private:

	long long addLimit = 20;
	const std::string countCommand = "SELECT COUNT(*) from new_servers";
	const std::string templateRequest = "SELECT * FROM new_servers ORDER by id LIMIT {}";
	const std::string deleteEdict = "DELETE FROM new_servers WHERE id = {}";
	long long countNumberInTabble() {
		pqxx::result ans = ConnectionPool::connectionPool.request(countCommand);
		return ans[0]["count"].as<long long>();
	}

public:
	std::vector<std::string> returnNewDomains() {

		
		long long numOfServers = countNumberInTabble();
		while (numOfServers == 0) {
			numOfServers = countNumberInTabble();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		std::vector<std::string> ans;
		numOfServers = std::min(numOfServers, addLimit);
		pqxx::result req = ConnectionPool::connectionPool.request(templateRequest, numOfServers);

		for (auto row : req) {
			ans.push_back(row["endpoint"].as<std::string>());
			ConnectionPool::connectionPool.edict(deleteEdict, row["id"].as<size_t>());
		}

		return ans;
	}
};