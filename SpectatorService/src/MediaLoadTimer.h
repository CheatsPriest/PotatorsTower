#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include <set>
#include <chrono>
#include "httplib.h"

class MediaLoadTimer {
private:
    std::string base_url;

public:
    MediaLoadTimer(const std::string& url) : base_url(url) {}

    // Парсинг всех медиа-ресурсов из HTML
    std::set<std::string> parse_media_resources(const std::string& html_content) {
        std::set<std::string> media_urls;

        std::vector<std::regex> patterns = {
            std::regex("<img[^>]+src=\"([^\"]+)\"", std::regex::icase),
            std::regex("<img[^>]+src='([^']+)'", std::regex::icase),
            std::regex("<video[^>]+src=\"([^\"]+)\"", std::regex::icase),
            std::regex("<audio[^>]+src=\"([^\"]+)\"", std::regex::icase),
            std::regex("<source[^>]+src=\"([^\"]+)\"", std::regex::icase),
            std::regex("url\\(\"([^\"]+)\"\\)", std::regex::icase),
            std::regex("<link[^>]+href=\"([^\"]+)\"[^>]*rel=\"stylesheet\"", std::regex::icase),
            std::regex("<script[^>]+src=\"([^\"]+)\"", std::regex::icase)
        };

        for (const auto& pattern : patterns) {
            std::sregex_iterator it(html_content.begin(), html_content.end(), pattern);
            std::sregex_iterator end;

            for (; it != end; ++it) {
                std::string url = clean_url(it->str(1));
                if (!url.empty() && url.find("data:") != 0) {
                    media_urls.insert(make_absolute_url(url));
                }
            }
        }

        return media_urls;
    }

    std::string clean_url(std::string url) {
        url.erase(std::remove(url.begin(), url.end(), '\''), url.end());
        url.erase(std::remove(url.begin(), url.end(), '"'), url.end());
        return url;
    }

    std::string make_absolute_url(const std::string& url) {
        if (url.find("http") == 0) return url;

        if (url[0] == '/') {
            size_t protocol_pos = base_url.find("://");
            if (protocol_pos != std::string::npos) {
                size_t domain_end = base_url.find('/', protocol_pos + 3);
                return domain_end != std::string::npos ?
                    base_url.substr(0, domain_end) + url : base_url + url;
            }
        }
        return url;
    }

    // Замер времени загрузки одного ресурса
    std::chrono::milliseconds measure_load_time(const std::string& url) {
        size_t protocol_pos = url.find("://");
        if (protocol_pos == std::string::npos) {
            return std::chrono::milliseconds(-1);
        }

        std::string domain = url.substr(protocol_pos + 3);
        size_t path_pos = domain.find('/');
        if (path_pos == std::string::npos) {
            return std::chrono::milliseconds(-1);
        }

        std::string host = domain.substr(0, path_pos);
        std::string path = domain.substr(path_pos);

        try {
            auto start = std::chrono::high_resolution_clock::now();

            if (url.find("https://") == 0) {
                httplib::SSLClient cli(host.c_str());
                cli.set_connection_timeout(1);
                cli.set_read_timeout(1);
                auto response = cli.Get(path.c_str());
                if(response)response->body.clear();
            }
            else {
                httplib::Client cli(host.c_str());
                cli.set_connection_timeout(1);
                cli.set_read_timeout(1);
                auto response = cli.Get(path.c_str());
                if (response)response->body.clear();
           }
            
            auto end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        }
        catch (const std::exception& e) {
            return std::chrono::milliseconds(-1);
        }
    }

    // Основная функция замера
    std::chrono::milliseconds measure_total_load_time(const std::string& html_content) {
        auto media_urls = parse_media_resources(html_content);

        

        std::chrono::milliseconds total_load_time(0);
        int success_count = 0;
        int failed_count = 0;

        std::vector<std::chrono::milliseconds> load_times;

        // Замеряем время для каждого ресурса
        for (const auto& url : media_urls) {
            

            auto load_time = measure_load_time(url);

            if (load_time.count() >= 0) {
                total_load_time += load_time;
                success_count++;
                load_times.push_back(load_time);

                
            }
            else {
                failed_count++;
                
            }

            // Небольшая пауза между запросами
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // Вывод результатов
        return total_load_time;
        ;
    }

    void print_results(const std::vector<std::chrono::milliseconds>& load_times,
        std::chrono::milliseconds total_time,
        int total_resources, int success_count, int failed_count) {
        if (load_times.empty()) {
            std::cout << "❌ No resources were loaded successfully" << std::endl;
            return;
        }

        // Расчет статистики
        std::chrono::milliseconds min_time = *std::min_element(load_times.begin(), load_times.end());
        std::chrono::milliseconds max_time = *std::max_element(load_times.begin(), load_times.end());

        std::chrono::milliseconds avg_time(0);
        for (const auto& time : load_times) {
            avg_time += time;
        }
        avg_time = std::chrono::milliseconds(avg_time.count() / load_times.size());

        std::cout << "\n=== MEDIA LOAD TIME RESULTS ===" << std::endl;
        std::cout << "Total resources found: " << total_resources << std::endl;
        std::cout << "Successfully measured: " << success_count << std::endl;
        std::cout << "Failed to load: " << failed_count << std::endl;
        std::cout << "Total load time: " << total_time.count() << "ms" << std::endl;
        std::cout << "Average load time: " << avg_time.count() << "ms" << std::endl;
        std::cout << "Minimum load time: " << min_time.count() << "ms" << std::endl;
        std::cout << "Maximum load time: " << max_time.count() << "ms" << std::endl;
        std::cout << "=================================" << std::endl;

        // Детальная информация по времени загрузки
        std::cout << "\n--- Detailed Load Times ---" << std::endl;
        for (size_t i = 0; i < load_times.size(); ++i) {
            std::cout << "Resource " << (i + 1) << ": " << load_times[i].count() << "ms" << std::endl;
        }
    }
};