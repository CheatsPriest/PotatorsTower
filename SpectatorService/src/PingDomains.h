#pragma once
#include "Pinger.h"
#include "ConnectionPool.h"
#include "DateClass.h"
#include "Logger.h"
#include "DownServersNotifier.h"

class PingDomains
{
private:
	int numThreads;
	size_t tabbleSize = 0;
	std::atomic<int> completed{ 0 };
	std::vector<std::thread> pool;
	priorities priorityType;
	const std::string requestAll = "SELECT * FROM servers ORDER BY id OFFSET {} LIMIT 1";
	const std::string requestByType = "SELECT * FROM servers where priority = {} ORDER BY id OFFSET {} LIMIT 1";

public:
	std::atomic<int> counter{ 0 };


	PingDomains(int numWorkers = 4, priorities p_t= allPriority) : numThreads(numWorkers), priorityType(p_t){

	}
	void start() {
		Controller::updates[priorityType] = true;

		pqxx::result ans = ConnectionPool::connectionPool.request("select count(*) from servers");
		tabbleSize = ans[0][0].as<size_t>();
		completed = 0;
		pool.clear();
		pool.reserve(numThreads);
		for (int i = 0; i < numThreads; i++) {
			pool.emplace_back(&PingDomains::cheackDomains, this, i, numThreads);

		}

		auto start = std::chrono::high_resolution_clock::now();
		while (completed != numThreads);

		for (int i = 0; i < pool.size(); i++) {
			pool[i].join();
		}
		//std::cout << counter << std::endl;
		auto end = std::chrono::high_resolution_clock::now();
		ConsoleManager::console.sendMesseage("Last update priority " + std::to_string(priorityType) + ": " + std::to_string((end-start).count() / 1000000000.f) + " seconds.");
		Controller::updates[priorityType] = false;
		

	}

private:
	void cheackDomains(size_t thread_id, size_t total_threads) {
		size_t chunk_size = tabbleSize / total_threads;
		size_t start = thread_id * chunk_size;
		size_t end = (thread_id == total_threads - 1) ? tabbleSize : start + chunk_size;

		for (size_t i = start; i < end; i++) {
			pqxx::result ans;
			if(priorityType!=allPriority){
				ans = ConnectionPool::connectionPool.request(requestByType, (int)priorityType, i);
			}
			else {
				ans = ConnectionPool::connectionPool.request(requestAll, i);
			}
			

			
			std::string domain = "";
			std::string id = "";
			std::string prevState = "";
			bool cheack_ssl = false;
			bool is_https = false;
			bool load_media = false;
			std::string content;
			std::string content_type;
			std::string path = "/";
			if (!ans.empty()) {
				auto row = ans[0]; // Берем первую строку

				// Проверяем что поля существуют и не NULL
				if (row["id"].is_null()) {
					id = "N/A";
				}
				else {
					id = row["id"].as<std::string>();
				}

				if (row["endpoint"].is_null()) {
					domain = "N/A";
				}
				else {
					domain = row["endpoint"].as<std::string>();
				}

				if (row["cheack_ssl"].is_null()) {
					cheack_ssl = false;
				}
				else {
					cheack_ssl = row["cheack_ssl"].as<bool>();
				}

				if (row["is_https"].is_null()) {
					is_https = false;
				}
				else {
					is_https = row["is_https"].as<bool>();
				}

				if (row["load_media"].is_null()) {
					load_media = false;
				}
				else {
					load_media = row["load_media"].as<bool>();
				}

				if (row["content"].is_null()) {
					content = "";
				}
				else {
					content = row["content"].as<std::string>();
				}

				if (row["content_type"].is_null()) {
					content_type = "";
				}
				else {
					content_type = row["content_type"].as<std::string>();
				}

				if (row["path"].is_null()) {
					path = "/";
				}
				else {
					path = row["path"].as<std::string>();
				}


			}

			//Pinging
			if (domain == "") {
				
				continue;
			}
			Pinger ping;
			


			request_params params;
			params.use_https = is_https;
			params.verify_ssl = cheack_ssl;
			params.load_media = load_media;

			params.expected_content = std::move(content);
			params.expected_content_type = std::move(content_type);
			params.domain = "www."+std::move(domain);
			params.path = path;
			

			result res = ping.pingV2(params);

			
			
			ConnectionPool::connectionPool.edict("UPDATE servers SET status = '{}', last_ping = '{}' WHERE id = {}", 
				to_string(res.status), curDateInString(), id);
			
			if (res.status != pingStatus::ok) {
				auto req = ConnectionPool::connectionPool.request("SELECT * FROM logs where server_id = {} and date_time = '{}'",
					id, ans[0]["last_ping"].as<std::string>());
				if(req.size()>0)
				if(req[0]["status"].as<std::string>()=="up") downServersQueue::queue.push(downInfo(std::stoi(id), domain));
			}


			logger::addNewLoggV1(std::stoi(id), std::move(res));
			

		}
		completed++;

	}
};

