//
// Created by Lorenzo P on 4/19/26.
//

#ifndef POLYMARKET_TYPES_H
#define POLYMARKET_TYPES_H
#include <string>
#include <vector>

namespace polymarket {
    struct PriceLevel {
        double price;
        double volume;
    };

    struct OrderBook {
        std::string asset_id;
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
        uint64_t last_updated_ns;

        // TODO: These should be already ordered or a specific DS
        double best_bid() const {
            if (bids.empty())
                return 0.0;

            double max_bid = bids[0].price;
            for (const auto &bid : bids) {
                if (bid.price > max_bid) {
                    max_bid = bid.price;
                }
            }

            return max_bid;
        }

        double best_ask() const {
            if (asks.empty())
                return 1.0;

            double min_ask = asks[0].price;
            for (const auto &ask : asks) {
                if (ask.price < min_ask) {
                    min_ask = ask.price;
                }
            }

            return min_ask;
        }
    };

    struct Token {
        std::string token_id;
        std::string outcome;
    };

    struct GammaMarket {

    };

    struct CLOBMarket {
        std::string condition_id;
        std::string question;
        std::string slug;
        std::vector<Token> tokens;
        bool neg_risk;
        bool active;
        bool closed;

        std::string token_yes() {
            for (const auto &token : tokens) {
                if (token.outcome == "yes") {
                    return token.token_id;
                }
            }

            return "";
        }

        std::string token_no() {
            for (const auto &token : tokens) {
                if (token.outcome == "no") {
                    return token.token_id;
                }
            }

            return "";
        }
    };


    struct APIConfig {
        std::string clob_rest_url = "https://clob.polymarket.com";
        std::string clob_ws_url = "wss://ws-subscriptions-clob.polymarket.com/ws/market";
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
