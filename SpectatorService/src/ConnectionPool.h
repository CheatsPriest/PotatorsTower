#pragma once
#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <unordered_map>
#include <queue>
#include "DatabaseControll.h"
#include <chrono>
#include <type_traits>
#include <tuple>
#include <utility>
#include <format>
#include <condition_variable>

enum class TaskStatus {
	processing, completed
};

struct Task {
	//size_t id;
	//std::string command;
	pqxx::result response;
	TaskStatus status = TaskStatus::processing;
	//Task(std::string&& cmd, size_t _id) : command(cmd), id(_id) {}
	Task() {

	}


	pqxx::result returnResponse() && {
		return std::move(response);
	}
	
};

class ConnectionPool
{
private:

	std::atomic<size_t> requests_processed{ 0 };
	std::atomic<size_t> edicts_processed{ 0 };
	std::atomic<size_t> active_connections{ 0 };

	std::string authString;
	
	std::atomic<bool> quite{ false };
	std::atomic<size_t> freeId{ 1 };

	std::mutex queue_mtx;
	std::condition_variable queue_cv;

	std::mutex informer_mtx;
	std::condition_variable informer_cv;
	std::mutex m_q;
	
	std::mutex commitMutex;

	std::vector<std::thread> pool;

	std::queue<std::pair<size_t, std::string>> tasks;
	std::unordered_map<size_t, Task> results;


public:
	const int poolSize;
	static ConnectionPool connectionPool;

	ConnectionPool(std::string auth, int numWorkers = 4) : authString(auth), poolSize(numWorkers) {
		pool.reserve(numWorkers);
		for (int i = 0; i < numWorkers; i++) {
			pool.emplace_back(&ConnectionPool::process, this);
		}
		
	}
	template<typename ... Args>
	pqxx::result request(std::string_view command, Args&& ... arg) {

		std::string finalString = "";
		if constexpr (sizeof...(arg) > 0)finalString = std::vformat(command, std::make_format_args(arg...));
		else finalString = std::string(command);

		const size_t taskId = freeId++;

		informer_mtx.lock();
		results[taskId] = Task();
		informer_mtx.unlock();

		queue_mtx.lock();
		tasks.push({ taskId, finalString });
		queue_mtx.unlock();

		queue_cv.notify_one();

		std::unique_lock<std::mutex> locker(informer_mtx);
		informer_cv.wait(locker, [this, taskId]()->bool {
			return taskId < freeId and results[taskId].status == TaskStatus::completed;
			});

		pqxx::result res;
		res = std::move(results[taskId].response);
		results.erase(taskId);

		requests_processed++;

		return std::move(res);

	}


	pqxx::result request(std::string command) {

		const size_t taskId = freeId++;

		informer_mtx.lock();
		results[taskId] = Task();
		informer_mtx.unlock();

		queue_mtx.lock();
		tasks.push({ taskId, command });
		queue_mtx.unlock();

		queue_cv.notify_one();

		std::unique_lock<std::mutex> locker(informer_mtx);
		informer_cv.wait(locker, [this, taskId]()->bool {
			return taskId < freeId and results[taskId].status == TaskStatus::completed;
			});

		pqxx::result res;
		res = std::move(results[taskId].response);
		results.erase(taskId);

		requests_processed++;

		return std::move(res);

	}

	template<typename ... Args>
	void edict(std::string_view command, Args&& ... arg) {
		std::string finalString = "";
		if constexpr (sizeof...(arg) > 0)finalString = std::vformat(command, std::make_format_args(arg...));
		else finalString = std::string(command);

		queue_mtx.lock();
		tasks.push({ 0, finalString });
		queue_mtx.unlock();

		queue_cv.notify_one();

		edicts_processed++;
	}
	void edict(const std::string& command) {
		queue_mtx.lock();
		tasks.push({ 0, command });
		queue_mtx.unlock();

		queue_cv.notify_one();

		edicts_processed;
	}

	~ConnectionPool() {
		std::cout << "REQUESTS: " << requests_processed << std::endl;
		std::cout << "EDICTS: "<<edicts_processed << std::endl;
		quite = true;
		queue_cv.notify_all();
		informer_cv.notify_all();
		for (int i = 0; i < pool.size(); i++) {
			pool[i].join();
		}
	}

private:

	void process() {
		pqxx::connection connectionObject(authString);
		

		while (!quite) {

			std::unique_lock<std::mutex> locker(queue_mtx);
			queue_cv.wait(locker, [this]()->bool {return !tasks.empty() or quite; });

			if (!tasks.empty() and !quite) {
				pqxx::work worker(connectionObject);

				std::pair<size_t, std::string> curTask = std::move(tasks.front());
				tasks.pop();
				locker.unlock();

				

				if (curTask.first != 0) {
					pqxx::result res = worker.exec(curTask.second);
					worker.commit();
					std::lock_guard<std::mutex> informer_guard(informer_mtx);
					results[curTask.first].response = std::move(res);
					results[curTask.first].status = TaskStatus::completed;
				}
				else {
					worker.exec(curTask.second);
					//commitMutex.lock();
					worker.commit();
					//commitMutex.unlock();
				}
			}
			
			informer_cv.notify_all();

		}

	}

	

};

