#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iomanip>
#include <ctime>
#include <string>
#include <sstream>

std::string curDateInString(const std::string& format = "%Y-%m-%d %H:%M:%S");
std::string curDateInStringWithOffset(const long long seconds, const std::string& format = "%Y-%m-%d %H:%M:%S");

