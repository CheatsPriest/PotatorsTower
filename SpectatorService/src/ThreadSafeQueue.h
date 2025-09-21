#pragma once
#include <queue>
#include <mutex>
#include <stdexcept>

template<typename T>
class MutexQueue {
private:
	std::queue<T> q;
	std::mutex mut;

public:
	void push(T value) {
		std::lock_guard<std::mutex> locker(mut);
		q.push(value);
	}
	T pop() {
		std::lock_guard<std::mutex> locker(mut);
		if (q.empty())throw std::runtime_error("BRUUUH, queue is empty");
		T value = q.front();
		q.pop();
		return value;
	}
	T front() {
		std::lock_guard<std::mutex> locker(mut);
		if(q.empty())throw std::runtime_error("BRUUUH, queue is empty");
		return q.front();
	}
	bool empty() {
		std::lock_guard<std::mutex> locker(mut);
		return q.empty();
	}
};