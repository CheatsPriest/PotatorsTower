#pragma once
#include <atomic>

enum priorities {
	allPriority = 0,
	lowPriority = 1,
	middlePriority = 2,
	highPriority = 3,
	extreamePriority = 4
};

class Controller
{
public:
	static std::atomic<bool> quite;
	static std::atomic<bool> updating;
	static std::atomic<long long> updatePeriod;

	static std::atomic<long long> updatePeriodLowPriority;
	static std::atomic<long long> updatePeriodMiddlePriority;
	static std::atomic<long long> updatePeriodHighPriority;
	static std::atomic<long long> updatePeriodExtreamePriority;

	static std::atomic<size_t> amountOfLowPriority;
	static std::atomic<size_t> amountOfMiddlePriority;
	static std::atomic<size_t> amountOfHighPriority;
	static std::atomic<size_t> amountOfExtreamePriority;
		
	static std::atomic<long long> lastUpdate;
	static std::atomic<int> threadsForPings;
};

