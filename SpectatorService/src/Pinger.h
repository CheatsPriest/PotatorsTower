#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <system_error>
#include "httplib.h"

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#include <chrono>

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <chrono>
#endif

enum class pingStatus {
    unavailable, unstable, ok, content_mismatch
};
static std::string to_string(pingStatus a) {
    if (a == pingStatus::unavailable) {
        return "down";
    }
    else if (a == pingStatus::unstable) {
        return "unstable";
    }
    else if (a == pingStatus::ok) {
        return "up";
    }
    else if (a == pingStatus::content_mismatch) {
        return "content_mismatch";
    }
}


struct request_params {
    std::string domain;
    std::string path = "/";
    std::string method = "GET";
    std::vector<std::pair<std::string, std::string>> headers;
    std::string body;
    int timeout_sec = 1;
    bool use_https = true;
    bool verify_ssl = true; // НОВОЕ: проверять ли SSL
    std::string expected_content; // НОВОЕ: какой текст ищем в теле
    std::string expected_content_type; // НОВОЕ: какой Content-Type ожидаем
};

struct result {
    pingStatus status = pingStatus::unavailable;
    std::chrono::duration<double, std::milli> delay;
    int http_status = 0;
    std::string response_body; // НОВОЕ: сохраняем тело для последующего анализа
    std::string error_message;
};

class Pinger
{
public:
    int numAttempts = 10;
    int goodLimit = 5;
    result ping(const std::string& hostname, int timeout_ms = 1000) {

        //return ping_windows(hostname, timeout_ms);
        return check_http_availability(hostname, timeout_ms);


    }
    result pingV2(const request_params& params) {
        if(params.use_https) return sslPinger(params);
        else return noSslPinger(params);
    }

private:
    


    
    result check_http_availability(const std::string& domain, int timeout_sec = 1) {
        result res;
        //std::cout << domain << std::endl;
        try {
            httplib::Client cli(domain.c_str());
            cli.set_connection_timeout(timeout_sec);
            cli.set_read_timeout(timeout_sec);
            cli.set_write_timeout(timeout_sec);

            auto start = std::chrono::high_resolution_clock::now();
            auto response = cli.Get("/");
            auto end = std::chrono::high_resolution_clock::now();

            if (response && response->status < 500) { // 2xx, 3xx, 4xx - сервер работает
                res.delay = end - start;
                res.status = pingStatus::ok;
            }
            else {
                res.status = pingStatus::unavailable;
            }
        }
        catch (const std::exception& e) {
            res.status = pingStatus::unavailable;
        }

        return res;
    }
    httplib::Result clientGet(const request_params& params, result& res, auto& cli) {
        auto start = std::chrono::high_resolution_clock::now();
        auto response = cli.Get(params.path.c_str());
        auto end = std::chrono::high_resolution_clock::now();
       
        res.delay = end - start;

        return response;
    }
    void cheackContent(const request_params& params, result& result, httplib::Result& response) {
        if (!params.expected_content.empty() &&
            result.response_body.find(params.expected_content) == std::string::npos) {
            result.status = pingStatus::content_mismatch;
            result.error_message = "Content mismatch. Expected: [" + params.expected_content +
                "] GOT:[Something else]";
        }

        // Проверка Content-Type (если задан)
        if (!params.expected_content_type.empty()) {
            auto content_type_it = response->headers.find("Content-Type");
            if (content_type_it == response->headers.end()) {
                result.status = pingStatus::content_mismatch;
                result.error_message = "Content-Type header is missing";
            }
            else if (content_type_it->second.find(params.expected_content_type) == std::string::npos) {
                result.status = pingStatus::content_mismatch;
                result.error_message = "Content-Type mismatch. Expected: [" +
                    params.expected_content_type +
                    "], Got: [" + content_type_it->second + "]";
            }
        }

        if (result.status == pingStatus::ok && !params.expected_content.empty()) {
            if (result.response_body.find(params.expected_content) == std::string::npos) {
                result.status = pingStatus::content_mismatch;
                result.error_message = "Expected content not found: [" +
                    params.expected_content + "]";
            }
        }
    }
    result sslPinger(const request_params& params) {
        result res;
        //std::cout << domain << std::endl;
        try {
            httplib::SSLClient cli(params.domain.c_str(), 443);
            cli.set_connection_timeout(params.timeout_sec);
            cli.set_read_timeout(params.timeout_sec);
            cli.set_write_timeout(params.timeout_sec);

            cli.set_follow_location(false);
            
            if (params.use_https) {
                cli.enable_server_certificate_verification(params.verify_ssl);
                cli.enable_server_hostname_verification(params.verify_ssl);
                cli.set_ca_cert_path("./ca-bundle.crt");
            }

            
            auto response = clientGet(params, res, cli);
            

            

            if (!response) {
                auto error = response.error();
                std::string error_str = httplib::to_string(error);

                
                res.status = pingStatus::unavailable;
                res.error_message = "Connection error: " + error_str;
                return res;
            }
            res.http_status = response->status;
           

            if (response && response->status < 300) { // 2xx, 3xx, 4xx - сервер работает
                
                res.status = pingStatus::ok;
                res.http_status = response->status;
                res.response_body = response->body;

                // 6. ДОПОЛНИТЕЛЬНЫЕ ПРОВЕРКИ ПО ТЗ
                // Проверка контента (если задано)
                cheackContent(params, res, response);
            }
            else if (response->status >= 300 && response->status < 400) {
                // РЕДИРЕКТ (3xx) - можно считать "условным успехом"
                res.status = pingStatus::ok;
                res.error_message = "Redirect: " + std::to_string(response->status);
                // Можно добавить информацию о редиректе
                auto location_it = response->headers.find("Location");
                if (location_it != response->headers.end()) {
                    res.error_message += " to " + location_it->second;
                }
            }
            else if (response->status >= 400 && response->status < 500) {
                // ОШИБКА КЛИЕНТА (4xx)
                res.status = pingStatus::unavailable;
                res.error_message = "Client error: " + std::to_string(response->status);
            }
            else if (response->status >= 500) {
                // ОШИБКА СЕРВЕРА (5xx)
                res.status = pingStatus::unavailable;
                res.error_message = "Server error: " + std::to_string(response->status);
            }
        }
        catch (const std::exception& e) {
            res.status = pingStatus::unavailable;
            res.error_message = "Exception: " + std::string(e.what());
        }
        catch (...) {
            res.status = pingStatus::unavailable;
            res.error_message = "Unknown exception occurred";
        }

        return res;
    }
    result noSslPinger(const request_params& params) {
        result res;
        //std::cout << domain << std::endl;
        try {
            httplib::Client cli(params.domain.c_str());
            cli.set_connection_timeout(params.timeout_sec);
            cli.set_read_timeout(params.timeout_sec);
            cli.set_write_timeout(params.timeout_sec);

            cli.set_follow_location(false);

            

            auto response = clientGet(params, res, cli);

            

            if (!response) {
                auto error = response.error();
                std::string error_str = httplib::to_string(error);


                res.status = pingStatus::unavailable;
                res.error_message = "Connection error: " + error_str;
                return res;
            }
            res.http_status = response->status;
           

            if (response && response->status < 300) { // 2xx, 3xx, 4xx - сервер работает
                
                res.status = pingStatus::ok;
                res.http_status = response->status;
                res.response_body = response->body;

                // 6. ДОПОЛНИТЕЛЬНЫЕ ПРОВЕРКИ ПО ТЗ
                // Проверка контента (если задано)
                cheackContent(params, res, response);
            }
            else if (response->status >= 300 && response->status < 400) {
                // РЕДИРЕКТ (3xx) - можно считать "условным успехом"
                res.status = pingStatus::ok;
                res.error_message = "Redirect: " + std::to_string(response->status);
                // Можно добавить информацию о редиректе
                auto location_it = response->headers.find("Location");
                if (location_it != response->headers.end()) {
                    res.error_message += " to " + location_it->second;
                }
            }
            else if (response->status >= 400 && response->status < 500) {
                // ОШИБКА КЛИЕНТА (4xx)
                res.status = pingStatus::unavailable;
                res.error_message = "Client error: " + std::to_string(response->status);
            }
            else if (response->status >= 500) {
                // ОШИБКА СЕРВЕРА (5xx)
                res.status = pingStatus::unavailable;
                res.error_message = "Server error: " + std::to_string(response->status);
            }
        }
        catch (const std::exception& e) {
            res.status = pingStatus::unavailable;
            res.error_message = "Exception: " + std::string(e.what());
        }
        catch (...) {
            res.status = pingStatus::unavailable;
            res.error_message = "Unknown exception occurred";
        }

        return res;
    }


    result check_http_availabilityV2(const request_params& params) {
        result res;
        //std::cout << domain << std::endl;
        try {
            httplib::Client cli(params.domain.c_str(), 443);
            cli.set_connection_timeout(params.timeout_sec);
            cli.set_read_timeout(params.timeout_sec);
            cli.set_write_timeout(params.timeout_sec);

            cli.set_follow_location(false);

            if (params.use_https) {
                std::cout << params.verify_ssl << std::endl;
                
            }

            cli.enable_server_certificate_verification(true);
            cli.enable_server_hostname_verification(true);
            cli.set_ca_cert_path("./ca-bundle.crt");



            auto start = std::chrono::high_resolution_clock::now();
            auto response = cli.Get(params.path.c_str());
            auto end = std::chrono::high_resolution_clock::now();

           

            if (!response) {
                auto error = response.error();
                std::string error_str = httplib::to_string(error);

                
                res.status = pingStatus::unavailable;
                res.error_message = "Connection error: " + error_str;
                return res;
            }
            

            if (response && response->status < 300) { // 2xx, 3xx, 4xx - сервер работает
                res.delay = end - start;
                res.status = pingStatus::ok;
                res.http_status = response->status;
                res.response_body = response->body;

                // 6. ДОПОЛНИТЕЛЬНЫЕ ПРОВЕРКИ ПО ТЗ
                // Проверка контента (если задано)
                if (!params.expected_content.empty() &&
                    res.response_body.find(params.expected_content) == std::string::npos) {
                    res.status = pingStatus::content_mismatch;
                    res.error_message = "Content mismatch. Expected: '" + params.expected_content +
                        "' GOT: '" + res.response_body+"'";
                }
                    
                // Проверка Content-Type (если задан)
                if (!params.expected_content_type.empty()) {
                    auto content_type_it = response->headers.find("Content-Type");
                    if (content_type_it == response->headers.end()) {
                        res.status = pingStatus::content_mismatch;
                        res.error_message = "Content-Type header is missing";
                    }
                    else if (content_type_it->second.find(params.expected_content_type) == std::string::npos) {
                        res.status = pingStatus::content_mismatch;
                        res.error_message = "Content-Type mismatch. Expected: '" +
                            params.expected_content_type +
                            "', Got: '" + content_type_it->second + "'";
                    }
                }
                
                if (res.status == pingStatus::ok && !params.expected_content.empty()) {
                    if (res.response_body.find(params.expected_content) == std::string::npos) {
                        res.status = pingStatus::content_mismatch;
                        res.error_message = "Expected content not found: '" +
                            params.expected_content + "'";
                    }
                }
            }
            else if (response->status >= 300 && response->status < 400) {
                // РЕДИРЕКТ (3xx) - можно считать "условным успехом"
                res.status = pingStatus::unstable;
                res.error_message = "Redirect: " + std::to_string(response->status);
                // Можно добавить информацию о редиректе
                auto location_it = response->headers.find("Location");
                if (location_it != response->headers.end()) {
                    res.error_message += " to " + location_it->second;
                }
            }
            else if (response->status >= 400 && response->status < 500) {
                // ОШИБКА КЛИЕНТА (4xx)
                res.status = pingStatus::unavailable;
                res.error_message = "Client error: " + std::to_string(response->status);
            }
            else if (response->status >= 500) {
                // ОШИБКА СЕРВЕРА (5xx)
                res.status = pingStatus::unavailable;
                res.error_message = "Server error: " + std::to_string(response->status);
            }
        }
        catch (const std::exception& e) {
            res.status = pingStatus::unavailable;
            res.error_message = "Exception: " + std::string(e.what());
        }
        catch (...) {
            res.status = pingStatus::unavailable;
            res.error_message = "Unknown exception occurred";
        }

        return res;
    }
   


};

