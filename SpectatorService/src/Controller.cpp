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

std::atomic<bool> Controller::updatingLow{ 0 };
std::atomic<bool> Controller::updatingMiddle{ 0 };
std::atomic<bool> Controller::updatingHigh{ 0 };
std::atomic<bool> Controller::updatingExtreame{ 0 };

std::atomic<bool> Controller::updates[5] = { false, false, false, false, false };
std::atomic<long long> Controller::updatePeriodMass[5] = { 300, 300, 120, 60, 30 };

