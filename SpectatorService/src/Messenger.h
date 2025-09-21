#pragma once
#include "ThreadSafeQueue.h"
#include "Controller.h"
#include "DomainAdder.h"

class Messenger
{
private:
	//Очередь индексов упавших сайтов
	MutexQueue<size_t> out;
	MutexQueue<std::string> in;

	std::condition_variable thread1_cv;
	std::mutex thread1_mut;

	std::condition_variable unpack_cv;
	std::mutex unpack_mtx;

	void processIn() {
		while (!in.empty()) {
			sqladd::addInBaseDomain(in.pop());
		}
	}
	void processOut() {
		//хз
	}

public:

	static Messenger msg;

	void pushOut(size_t id) {
		out.push(id);
		thread1_cv.notify_one();
	}
	void pushIn(const std::string& domain) {
		in.push(domain);
		thread1_cv.notify_one();
	}

	void process1Thread() {
		
		while (!Controller::quite) {

			std::unique_lock<std::mutex> lock(thread1_mut);
			thread1_cv.wait(lock, [this]() {return !out.empty() or !in.empty(); });

			if (!in.empty()) {
				processIn();
			}
			if (!out.empty()) {
				processOut();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

	}

};

