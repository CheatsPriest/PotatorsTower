#pragma once
#include "ConnectionPool.h"
#include "Pinger.h"
#include "DateClass.h"

namespace logger {

	static void addNewLoggV1(size_t&& serverId, result&& res) {

		ConnectionPool::connectionPool.edict("INSERT INTO logs(server_id, date_time, status, delay, status_code, error) VALUES({}, '{}', '{}', {}, {}, '{}')",
			serverId, curDateInString(), to_string(res.status), res.delay.count(), res.http_status, res.error_message);

	}
	static void cleanLogsV1(const std::string& oldestDate) {

		ConnectionPool::connectionPool.edict("DELETE FROM logs WHERE date_time < '{}'", oldestDate);

	}

}