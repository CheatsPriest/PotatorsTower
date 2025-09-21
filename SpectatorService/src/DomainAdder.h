#pragma once
#include "ConnectionPool.h"
#include "Pinger.h"
#include "DateClass.h"

namespace sqladd {
	static bool isExist(const std::string& domain) {

		pqxx::result ans = ConnectionPool::connectionPool.request("select count(id) from servers where endpoint = '{}'", domain);

		int num = ans[0]["count"].as<int>();

		return num != 0;
	}
	static void addInBaseDomain(const std::string& domain) {

		

		if (domain == "")return;
		Pinger ping;
		ping.numAttempts=20;
		ping.goodLimit = 10;

		auto res = ping.ping(domain, 10);

		std::string finalString = "INSERT INTO servers (endpoint, delay, status, last_ping) VALUES ('{}', '{}', '{}', '{}')";
		
		ConnectionPool::connectionPool.edict(finalString, domain, res.delay.count(), to_string(res.status), curDateInString());
	}
	static void addManyAddressesByCin() {
		std::string buf="";
		while (buf != "0") {
			std::cin >> buf;
			if (buf == "0")break;
			if (!isExist(buf)) addInBaseDomain(buf);
			else std::cout << "Duplicate" << std::endl;
		}

	}
	


}
