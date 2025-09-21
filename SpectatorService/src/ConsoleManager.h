#pragma once
#include <iostream>
#include <mutex>
#include "ThreadSafeQueue.h"
#include <string>
#include <atomic>
#include "Controller.h"
#include "Messenger.h"

class ConsoleManager
{
private:
	MutexQueue<std::string> messeages;

	std::condition_variable in_cv;
	std::mutex in_mut;

	std::ostream& out = std::cout;
	std::istream& in = std::cin;

	std::mutex consoleControl_mut;
	std::atomic<bool> outBlocked{ 0 };

public:
	static ConsoleManager console;
	void sendMesseage(const std::string& msg) {
		
		messeages.push(msg);
		in_cv.notify_one();
	}
	void displayMesseages() {
		while (!Controller::quite) {
			std::unique_lock<std::mutex> locker(in_mut);

			in_cv.wait_for(locker, std::chrono::milliseconds(100), [this]() {
				return !messeages.empty() || !Controller::quite;
				});
			if (Controller::quite)break;

			while (!messeages.empty()) {

				if (!outBlocked)out << messeages.pop() << std::endl;
				else std::this_thread::sleep_for(std::chrono::milliseconds(200));

			}
		}
	}
	void commands() {
		std::string command = "";
		while (!Controller::quite) {
			command.clear();
			//std::getline(in, command);
			in >> command;
			outBlocked = true;
			if (command == "0" or command == "exit") {
				Controller::quite = true;
				in_cv.notify_all();
				return;
			}
			else if (command == "push") {
				outBlocked = true;
				in >> command;
				sendMesseage(command);
				outBlocked = false;
			}
			else if (command == "set") {
				outBlocked = true;
				in >> command;


				outBlocked = false;
			}
			else if (command == "add") {
				out << "Enter domain: ";
				in >> command;
				Messenger::msg.pushIn(command);
			}
			outBlocked = false;

		}
	}
	void consoleControll() {
		std::thread outputThread(&ConsoleManager::displayMesseages, this);
		
		commands();

		
		if (outputThread.joinable()) {
			outputThread.join();
		}
	}

};

