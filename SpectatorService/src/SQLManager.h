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
	size_t calculateNumThreads(size_t cur_amount) {
		return std::max(static_cast<size_t>(1),
			(cur_amount * Controller::threadsForPings) / 
			(Controller::amountOfLowPriority + Controller::amountOfMiddlePriority + 
				Controller::amountOfHighPriority + Controller::amountOfExtreamePriority)
		);
	}
public:

	void start() {

		
		std::thread newDomainsParserThread(&GenericAddDomain<BufTabblePolicy>::run, GenericAddDomain<BufTabblePolicy>());
		newDomainsParserThread.detach();

		std::thread downServersNotifierThread(&GenericDownServersNotifier<NotifierBufTabblePolicy>::run, GenericDownServersNotifier<NotifierBufTabblePolicy>());
		downServersNotifierThread.detach();
		size_t counter = 0;
		long long timesUpdates[5] = { 0 };
		while (!Controller::quite) {
			;

			if (std::time(nullptr) - Controller::lastUpdate >= Controller::updatePeriod) {
				Controller::lastUpdate = std::time(nullptr);

				feelTheAmountOfPriorities();
				/*ConsoleManager::console.sendMesseage("Servers' status update started.");
				PingDomains pinger(Controller::threadsForPings, highPriority);
				auto start = std::chrono::high_resolution_clock::now();
				pinger.start();
				auto end = std::chrono::high_resolution_clock::now();
				auto delay = end - start;
				ConsoleManager::console.sendMesseage("Last update took: " + std::to_string(delay.count() / 1000000000.f) + " seconds.");*/
				
			}
			


			if (timesUpdates[extreamePriority]< std::time(nullptr) and !Controller::updates[extreamePriority] and Controller::amountOfExtreamePriority>0) {
				std::thread pingExtreame([&]() {
						PingDomains pinger(calculateNumThreads(Controller::amountOfExtreamePriority), extreamePriority);
						pinger.start();
					});
				pingExtreame.detach();
				timesUpdates[extreamePriority] = std::time(nullptr) + Controller::updatePeriodMass[extreamePriority];
			}

			if (timesUpdates[highPriority] < std::time(nullptr) and !Controller::updates[highPriority] and Controller::amountOfHighPriority > 0) {
				std::thread pingExtreame([&]() {
					PingDomains pinger(calculateNumThreads(Controller::amountOfHighPriority), highPriority);
					pinger.start();
					});
				pingExtreame.detach();
				timesUpdates[highPriority] = std::time(nullptr) + Controller::updatePeriodMass[highPriority];

			}
			if (timesUpdates[middlePriority] < std::time(nullptr) and !Controller::updates[middlePriority] and Controller::amountOfMiddlePriority > 0) {
				std::thread pingExtreame([&]() {
					PingDomains pinger(calculateNumThreads(Controller::amountOfMiddlePriority), middlePriority);
					pinger.start();
					});
				pingExtreame.detach();
				timesUpdates[middlePriority] = std::time(nullptr) + Controller::updatePeriodMass[middlePriority];

			}
			if (timesUpdates[lowPriority] < std::time(nullptr) and !Controller::updates[lowPriority] and Controller::amountOfLowPriority > 0) {
				std::thread pingExtreame([&]() {
					PingDomains pinger(calculateNumThreads(Controller::amountOfMiddlePriority), lowPriority);
					pinger.start();
					});
				pingExtreame.detach();
				timesUpdates[lowPriority] = std::time(nullptr) + Controller::updatePeriodMass[lowPriority];

			}

			std::this_thread::sleep_for(std::chrono::milliseconds(2000));

		}
		
		for (int i = 0; i <= extreamePriority; i++) {
			while (Controller::updates[i] == true) { std::cout <<"WAITING FOR PRIRITY CHEACK: "<< i << std::endl;  std::this_thread::sleep_for(std::chrono::milliseconds(800)); };
		}
	}

};