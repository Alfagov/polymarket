//
// Created by Lorenzo P on 4/19/26.
//

#ifndef POLYMARKET_TYPES_H
#define POLYMARKET_TYPES_H
#include <string>
#include <vector>

namespace polymarket {

    struct APIConfig {
        std::string clob_rest_url = "https://clob.polymarket.com";
        std::string clob_ws_url = "wss://ws-subscriptions-clob.polymarket.com/ws/market";
        std::string polymarket_url = "https://polymarket.com";
        std::string gamma_api_url = "https://gamma-api.polymarket.com";
        std::string data_api_url = "https://data-api.polymarket.com";
        std::string rtds_ws_url = "wss://ws-live-data.polymarket.com";

        int ws_ping_interval_ms = 5000;
        int http_timeout_ms = 5000;
        int max_markets = 50;

        inline uint64_t now_ns() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();
        }

        inline uint64_t now_sec()
        {
            return std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                .count();
        }
    };
}

#endif //POLYMARKET_TYPES_H
