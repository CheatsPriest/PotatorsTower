#pragma once
#include "ConnectionPool.h"
#include "PingDomains.h"
#include "Pinger.h"
#include "DateClass.h"
#include "DomainAdder.h"
#include "Controller.h"
#include "ConsoleManager.h"
#include "NewAddressesParser.h"
#include "DownServersNotifier.h"

using namespace std::chrono_literals;
class SQLManager {

private:
	void feelTheAmountOfPriorities() {
		{
			auto res1 = ConnectionPool::connectionPool.request("SELECT COUNT(*) from servers where priority = {}", (int)lowPriority);
			Controller::amountOfLowPriority = res1[0]["count"].as<size_t>();
		}
		{
			auto res1 = ConnectionPool::connectionPool.request("SELECT COUNT(*) from servers where priority = {}", (int)middlePriority);
			Controller::amountOfMiddlePriority = res1[0]["count"].as<size_t>();
		}
		{
			auto res1 = ConnectionPool::connectionPool.request("SELECT COUNT(*) from servers where priority = {}", (int)highPriority);
			Controller::amountOfHighPriority = res1[0]["count"].as<size_t>();
		}
		{
			auto res1 = ConnectionPool::connectionPool.request("SELECT COUNT(*) from servers where priority = {}", (int)extreamePriority);
			Controller::amountOfExtreamePriority = res1[0]["count"].as<size_t>();
		}


	}
	
public:

	void start() {

		
		std::thread newDomainsParserThread(&GenericAddDomain<BufTabblePolicy>::run, GenericAddDomain<BufTabblePolicy>());
		newDomainsParserThread.detach();

		std::thread downServersNotifierThread(&GenericDownServersNotifier<NotifierBufTabblePolicy>::run, GenericDownServersNotifier<NotifierBufTabblePolicy>());
		downServersNotifierThread.detach();
		size_t counter = 0;
		while (!Controller::quite) {
			//counter++;

			if (std::time(nullptr) - Controller::lastUpdate >= Controller::updatePeriod) {
				Controller::lastUpdate = std::time(nullptr);

				feelTheAmountOfPriorities();
				ConsoleManager::console.sendMesseage("Servers' status update started.");
				PingDomains pinger(Controller::threadsForPings);
				auto start = std::chrono::high_resolution_clock::now();
				pinger.start();
				auto end = std::chrono::high_resolution_clock::now();
				auto delay = end - start;
				ConsoleManager::console.sendMesseage("Last update took: " + std::to_string(delay.count() / 1000000000.f) + " seconds.");
				
			}
			

			std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		}


	}

};