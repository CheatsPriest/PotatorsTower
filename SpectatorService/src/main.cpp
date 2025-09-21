#pragma once


#include "SQLManager.h"
#include "MediaLoadTimer.h"

void req() {
    for (int i = 0; i < 30; i++) {
        ConsoleManager::console.sendMesseage(std::to_string(rand() % 2421121));
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main()
{

    //std::string url = "https://www.catslove.com/en/10-most-popular-cat-breeds";

    //// Загружаем основную страницу
    //httplib::SSLClient cli("www.catslove.com", 443);
    //
    //cli.set_connection_timeout(1);
    //cli.set_read_timeout(1);


    //auto response = cli.Get("/en/10-most-popular-cat-breeds");

    /*if (response && response->status == 200) {
       
        

        MediaLoadTimer timer(url);

        std::cout << timer.measure_total_load_time(response->body) << std::endl;
    }
    else {
        std::cout << " Failed to load main page" << std::endl;
        if (!response) {
            std::cout << "Error: " << httplib::to_string(response.error()) << std::endl;
        }
        else {
            std::cout << "HTTP Status: " << response->status << std::endl;
        }
    }*/

    ////https://www.catslove.com/en/10-most-popular-cat-breeds
    //Pinger p;
    //request_params param;
    //
    //param.domain = "www.google.com";
    //param.expected_content_type = "text/html";
    //param.expected_content = "meta";
    //param.use_https = 1;
    //param.load_media = true;

    //auto ans = p.pingV2(param);
    //std::cout << ans.delay << " " << ans.delay << " " << ans.error_message<<" "<< to_string(ans.status) << std::endl;
   

    try
    {
        std::thread consoleThread(&ConsoleManager::consoleControll, &ConsoleManager::console);
        consoleThread.detach();

       
       
        //Нет нужды в нем сейчас DownServersNotifier и NewAddressParser вытеснили его
        //std::thread messengerThread(&Messenger::process1Thread, &Messenger::msg);
        //messengerThread.detach();

        SQLManager test;
        test.start();
        

       
        //sqladd::addManyAddressesByCin();
        
        //ConnectionPool::connectionPool.edict("UPDATE servers SET status = 'unstable', last_ping = '{}' WHERE id = {}", curDateInString(), 2);
       
        //logger::cleanLogsV1(curDateInStringWithOffset(60));
        
        /*PingDomains pinger(4);
        auto start = std::chrono::high_resolution_clock::now();
        pinger.start();
        auto end = std::chrono::high_resolution_clock::now();
        auto delay = end - start;
        std::cout << delay.count()/1000000000.f << std::endl;*/


        /* std::cout << curDateInString() << std::endl;
        std::string domain;
        std::cin >> domain;
        Pinger ping;
        result a = ping.ping(domain, 100);
        std::cout << a.delay.count() << std::endl;*/

        /*std::thread a(req), b(req);
        std::thread c(req), d(req), e(req);
        a.join();
        b.join();
        c.join();
        d.join();
        e.join();*/
        //ConnectionPool::connectionPool.edict("UPDATE servers SET status = 'up', last_ping = '2003-01-01' WHERE id = 1");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }

    

    std::cout << "All thread succesfully closed" << std::endl;
    system("pause");
    return 0;
}