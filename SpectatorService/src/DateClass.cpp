#include "DateClass.h"
#include <ctime>
#include <iomanip>
#include <sstream>

std::string curDateInString(const std::string& format) {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::stringstream buf;
    buf << std::put_time(&tm, format.c_str());
    return buf.str();
}

std::string curDateInStringWithOffset(const long long seconds, const std::string& format) {
    // Преобразуем long long в time_t
    time_t raw_time = std::time(nullptr);
    raw_time -= static_cast<time_t>(seconds); // Явное преобразование типа
    
    auto tm = *std::localtime(&raw_time); // Передаем указатель на time_t
    std::stringstream buf;
    buf << std::put_time(&tm, format.c_str());
    return buf.str();
}