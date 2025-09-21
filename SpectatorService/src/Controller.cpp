#include "Controller.h"

std::atomic<bool> Controller::quite{ 0 };
std::atomic<bool> Controller::updating{ 0 };
std::atomic<long long> Controller::updatePeriod = 70;
std::atomic<long long> Controller::lastUpdate = 0;
std::atomic<int> Controller::threadsForPings = 4;

std::atomic<long long> Controller::updatePeriodLowPriority=300;
std::atomic<long long> Controller::updatePeriodMiddlePriority = 120;
std::atomic<long long> Controller::updatePeriodHighPriority = 60;
std::atomic<long long> Controller::updatePeriodExtreamePriority = 30;

std::atomic<size_t> Controller::amountOfLowPriority = 0;
std::atomic<size_t> Controller::amountOfMiddlePriority = 0;
std::atomic<size_t> Controller::amountOfHighPriority = 0;
std::atomic<size_t> Controller::amountOfExtreamePriority = 0;