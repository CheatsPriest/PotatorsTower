#pragma once

#include "ConnectionPool.h"
#include "Controller.h"
#include "DomainAdder.h"
#include "ConsoleManager.h"
#include "Messenger.h"
#include "ThreadSafeQueue.h"
#include "ConsoleManager.h"

struct downInfo {
	size_t id;
	std::string endpoint;
	downInfo(size_t i, std::string e) : id(i), endpoint(e) {

	}
};
struct downServersQueue {
	static MutexQueue<downInfo> queue;
};

template<typename NotificationPolicy>
class GenericDownServersNotifier {
private:

	NotificationPolicy notificator;

public:

	void run() {
		ConsoleManager::console.sendMesseage("Down Servers Manager Runing!");
		while (!Controller::quite) {

			if (downServersQueue::queue.empty()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			else {
				notificator.notify(downServersQueue::queue.front());
				downServersQueue::queue.pop();
			}

		}
		//notificator.run();

	}

};

class NotifierBufTabblePolicy {
private:
	const std::string notifyCommand = "INSERT INTO down_servers (server_id, endpoint) VALUES ({}, '{}')";

public:

	void notify(downInfo in) {
		
		ConnectionPool::connectionPool.edict(notifyCommand, in.id, std::move(in.endpoint));

	}

};