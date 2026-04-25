//
// Created by Lorenzo P on 4/21/26.
//

#include <format>
#include <stdexcept>

#include "clob_client.h"

#include <iostream>

namespace json  = boost::json;

namespace polymarket::clob {
    ClobClient::ClobClient(const APIConfig &config)
        : config_(config)
    {
        http_.set_base_url(config.clob_rest_url);
        http_.set_timeout(config_.http_timeout_ms);
    }

    OrderBook ClobClient::fetch_order_book(std::string token_id) {
        const std::string path = std::format("/book?token_id={}", token_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<OrderBook>(json::parse(response.body));
    }

    std::vector<OrderBook> ClobClient::fetch_order_books(const std::vector<std::string> &token_ids) {
        constexpr std::string path = "/books";

        json::array json_array;
        for (const auto& id : token_ids) {
            json::object obj;
            obj["token_id"] = id;
            json_array.push_back(obj);
        }

        std::string json_string = json::serialize(json_array);

        const auto response = http_.post(path, json_string);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<std::vector<OrderBook>>(json::parse(response.body));
    }

    double ClobClient::fetch_market_price(std::string token_id, const TradeSide side) {
        const std::string path = std::format("/price?token_id={}&side={}", token_id, to_string(side));

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::parse(response.body).as_object().at("price").as_double();
    }

    TokenSidePrices ClobClient::fetch_market_prices(const std::vector<std::string> &token_ids, const std::vector<TradeSide> &sides) {
        constexpr std::string path = "/prices";

        if (token_ids.size() != sides.size())
            throw std::runtime_error("Invalid number of token ids provided");

        json::array json_array;
        for (size_t i = 0; i < token_ids.size(); ++i) {
            json::object obj;
            obj["token_id"] = token_ids[i];
            obj["side"] = to_string(sides[i]);
            json_array.push_back(obj);
        }

        std::string json_string = json::serialize(json_array);

        const auto response = http_.post(path, json_string);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<TokenSidePrices>(json::parse(response.body));
    }

    double ClobClient::fetch_midpoint_price(std::string token_id) {
        const std::string path = std::format("/midpoint?token_id={}", token_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::parse(response.body).as_object().at("mid_price").as_double();
    }

    TokenPrices ClobClient::fetch_midpoint_prices(std::vector<std::string> token_ids) {
        constexpr std::string path = "/midpoints";

        json::array json_array;
        for (size_t i = 0; i < token_ids.size(); ++i) {
            json::object obj;
            obj["token_id"] = token_ids[i];
            json_array.push_back(obj);
        }

        std::string json_string = json::serialize(json_array);

        const auto response = http_.post(path, json_string);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<TokenPrices>(json::parse(response.body));
    }

    double ClobClient::fetch_spread(std::string token_id) {
        const std::string path = std::format("/spread?token_id={}", token_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        std::string spread_string = json::parse(response.body).as_object().at("spread").get_string().c_str();

        return std::stod(spread_string);
    }

    TokenPrices ClobClient::fetch_spreads(std::vector<std::string> token_ids) {
        const std::string path = "/spreads";

        json::array json_array;
        for (size_t i = 0; i < token_ids.size(); ++i) {
            json::object obj;
            obj["token_id"] = token_ids[i];
            json_array.push_back(obj);
        }

        std::string json_string = json::serialize(json_array);

        const auto response = http_.post(path, json_string);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<TokenPrices>(json::parse(response.body));
    }

    SidePriceQuote ClobClient::fetch_last_traded_price(std::string token_id) {
        const std::string path = std::format("/last-trade-price?token_id={}", token_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<SidePriceQuote>(json::parse(response.body));
    }


    std::vector<TokenSidePriceQuote> ClobClient::fetch_last_traded_prices(std::vector<std::string> token_ids) {
        const std::string path = "/last-trade-prices";

        json::array json_array;
        for (const auto& id : token_ids) {
            json::object obj;
            obj["token_id"] = id;
            json_array.push_back(obj);
        }

        std::string json_string = json::serialize(json_array);

        const auto response = http_.post(path, json_string);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<std::vector<TokenSidePriceQuote>>(json::parse(response.body));
    }

    PriceHistory ClobClient::fetch_prices_history(std::string market, double start_ts, double end_ts, std::string interval, int fidelity) {
        std::string path = std::format("/history?market={}", market);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<PriceHistory>(json::parse(response.body));
    }

    int ClobClient::fetch_fee_rate(std::string token_id) {
        const std::string path = std::format("/fee-rate?token_id={}", token_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::parse(response.body).as_object().at("base_fee").as_int64();
    }

    double ClobClient::fetch_tick_size(std::string token_id) {
        const std::string path = std::format("/tick-size?token_id={}", token_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::parse(response.body).as_object().at("minimum_tick_size").as_double();
    }

    ClobMarketInfo ClobClient::fetch_clob_market_info(std::string condition_id) {
        const std::string path = std::format("/clob-markets/{}", condition_id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<ClobMarketInfo>(json::parse(response.body));
    }

    int ClobClient::fetch_server_time() {
        constexpr std::string path = "/time";

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return std::stoi(response.body);
    }

    std::vector<ClobMarket> ClobClient::fetch_simplified_markets() {
        const std::string path = "/simplified-markets";

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<ClobMarketsResponse>(json::parse(response.body)).data;
    }

    std::vector<ClobMarket> ClobClient::fetch_markets() {
        const std::string path = "/sampling-markets";

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<ClobMarketsResponse>(json::parse(response.body)).data;
    }

    std::vector<ClobMarket> ClobClient::fetch_all_markets() {
        std::vector<ClobMarket> all_markets;
        std::string cursor = "";

        while (true) {
            std::string path = "/sampling-markets";

            if (!cursor.empty()) {
                path += "?next_cursor=" + cursor;
            }

            const auto response = http_.get(path);
            if (!response.ok())
                throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

            auto parsed = json::value_to<ClobMarketsResponse>(json::parse(response.body));
            cursor = parsed.next_cursor;

            std::cout << "Downloaded: " << parsed.count << " Total: " << all_markets.size() << " Cursor: " << cursor << std::endl;
            if (cursor.empty() || parsed.data.empty() || cursor == "LTE=")
                break;

            for (auto mkt : parsed.data) {
                all_markets.push_back(std::move(mkt));
            }
        }

        return all_markets;
    }

    std::vector<ClobMarket> ClobClient::fetch_all_simplified_markets() {
        std::vector<ClobMarket> all_markets;
        std::string cursor = "";

        while (true) {
            std::string path = "/sampling-simplified-markets";

            if (!cursor.empty()) {
                path += "?next_cursor=" + cursor;
            }

            const auto response = http_.get(path);
            if (!response.ok())
                throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

            auto parsed = json::value_to<ClobMarketsResponse>(json::parse(response.body));
            cursor = parsed.next_cursor;

            std::cout << "Downloaded: " << parsed.count << " Total: " << all_markets.size() << " Cursor: " << cursor << std::endl;
            if (cursor.empty() || parsed.data.empty() || cursor == "LTE=")
                break;

            for (auto mkt : parsed.data) {
                all_markets.push_back(std::move(mkt));
            }
        }

        return all_markets;
    }

    std::vector<MakerRebates> ClobClient::fetch_current_maker_rebates(std::string date, std::string maker_address) {
        const std::string path = std::format("/rebates/current?date={}&maker_address={}", date, maker_address);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<std::vector<MakerRebates>>(json::parse(response.body));
    }
}
