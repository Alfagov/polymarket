//
// Created by Lorenzo P on 4/21/26.
//

#ifndef POLYMARKET_CLOB_TYPES_H
#define POLYMARKET_CLOB_TYPES_H

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <boost/describe.hpp>
#include <boost/json.hpp>
#include <boost/mp11.hpp>
#include <boost/url.hpp>

namespace polymarket::clob {
    struct PriceLevel {
        double price = 0.0;
        int size = 0;
    };
    BOOST_DESCRIBE_STRUCT(PriceLevel, (), (price, size))

    struct OrderBook {
        std::string market;
        std::string asset_id;
        std::string timestamp;
        std::string hash;
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
        int min_order_size = 0;
        double tick_size = 0.0;
        bool neg_risk = false;
        double last_traded_price = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(OrderBook, (), (market, asset_id, timestamp, hash, bids, asks, min_order_size, tick_size, neg_risk, last_traded_price))

    enum class TradeSide {
        Buy,
        Sell
    };

    inline std::string to_string(TradeSide side) {
        switch (side) {
            case TradeSide::Buy:  return "Buy";
            case TradeSide::Sell: return "Sell";
            default:              return "Unknown";
        }
    }

    inline TradeSide trade_side_from_string(std::string_view side) {
        if (side == "Buy" || side == "BUY" || side == "buy") {
            return TradeSide::Buy;
        }
        if (side == "Sell" || side == "SELL" || side == "sell") {
            return TradeSide::Sell;
        }
        throw std::invalid_argument("Invalid trade side: " + std::string(side));
    }

    struct PriceQuote {
        double price = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(PriceQuote, (), (price))

    struct SidePriceQuote : PriceQuote {
        TradeSide side = TradeSide::Buy;
    };
    BOOST_DESCRIBE_STRUCT(SidePriceQuote, (PriceQuote), (side))

    struct TokenPriceQuote : PriceQuote {
        std::string token_id;
    };
    BOOST_DESCRIBE_STRUCT(TokenPriceQuote, (PriceQuote), (token_id))

    struct TokenSidePriceQuote : SidePriceQuote {
        std::string token_id;
    };
    BOOST_DESCRIBE_STRUCT(TokenSidePriceQuote, (SidePriceQuote), (token_id))

    using TokenPrices = std::unordered_map<std::string, double>;
    using TokenSidePrices = std::unordered_map<std::string, SidePriceQuote>;

    struct PriceHistoryPoint {
        std::uint32_t timestamp = 0;
        double price = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(PriceHistoryPoint, (), (timestamp, price))

    struct PriceHistory {
        std::vector<PriceHistoryPoint> points;
    };
    BOOST_DESCRIBE_STRUCT(PriceHistory, (), (points))

    struct RewardRates {
        std::string asset_address;
        double rewards_daily_rate;
    };
    BOOST_DESCRIBE_STRUCT(RewardRates, (), (asset_address, rewards_daily_rate))

    struct MarketRewardInfo {
        std::vector<RewardRates> rates;
        double min_size;
        double max_spread;
    };
    BOOST_DESCRIBE_STRUCT(MarketRewardInfo, (), (rates, min_size, max_spread))

    struct MarketTokenInfo {
        std::string t;
        std::string o;
    };
    BOOST_DESCRIBE_STRUCT(MarketTokenInfo, (), (t, o))

    struct FeeCurve {
        std::optional<double> r;
        std::optional<double> e;
        std::optional<bool> to;
    };
    BOOST_DESCRIBE_STRUCT(FeeCurve, (), (r, e, to))

    struct ClobMarketInfo {
        std::string gst;
        MarketRewardInfo r;
        std::vector<MarketTokenInfo> t;
        double mos = 0.0;
        double mts = 0.0;
        int mbf = 0;
        int tbf = 0;
        bool rfqe = false;
        bool itode = false;
        bool ibce = false;
        FeeCurve fd;
        int oas = 0;
    };
    BOOST_DESCRIBE_STRUCT(ClobMarketInfo, (), (gst, r, t, mos, mts, mbf, tbf, rfqe, itode, ibce, fd, oas))

    struct Token {
        std::string token_id;
        std::string outcome;
        double price;
        bool winner;
    };
    BOOST_DESCRIBE_STRUCT(Token, (), (token_id, outcome, price, winner))

    struct ClobMarket {
        std::string condition_id;
        MarketRewardInfo rewards;
        std::vector<Token> tokens;
        bool enable_order_book;
        bool active;
        bool closed;
        bool archived;
        bool accepting_orders;
        std::string accepting_order_timestamp;
        double minimum_order_size;
        double minimum_tick_size;
        std::string question_id;
        std::string question;
        std::string description;
        std::string market_slug;
        std::string end_date_iso;
        std::string game_start_time;
        int seconds_delay;
        std::string fpmm;
        int maker_base_fee;
        int taker_base_fee;
        bool notifications_enabled;
        bool neg_risk;
        std::string neg_risk_market_id;
        std::string neg_risk_request_id;
        std::string icon;
        bool is_50_50_outcome;
        std::vector<std::string> tags;
    };
    BOOST_DESCRIBE_STRUCT(ClobMarket, (), (condition_id, rewards, tokens, active, closed, archived, accepting_orders, enable_order_book,
        accepting_order_timestamp, minimum_order_size, minimum_tick_size, question_id, question, description, market_slug, end_date_iso,
        game_start_time, seconds_delay, fpmm, maker_base_fee, taker_base_fee, notifications_enabled, neg_risk, neg_risk_market_id, neg_risk_request_id, icon,
        is_50_50_outcome, tags))

    struct ClobMarketsResponse {
        int limit;
        std::string next_cursor;
        int count;
        std::vector<ClobMarket> data;
    };
    BOOST_DESCRIBE_STRUCT(ClobMarketsResponse, (), (limit, next_cursor, count, data))

    struct MakerRebates {
        std::string date;
        std::string condition_id;
        std::string asset_address;
        std::string maker_address;
        std::string rebated_fees_usdc;
    };
    BOOST_DESCRIBE_STRUCT(MakerRebates, (), (date, condition_id, asset_address, maker_address, rebated_fees_usdc))

    namespace detail {
        template <class T>
        void json_value_from_described(boost::json::object& obj, T const& value) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;

            boost::mp11::mp_for_each<Members>([&](auto D) {
                obj[D.name] = boost::json::value_from(value.*D.pointer);
            });
        }

        template <class T>
        void json_value_to_described(boost::json::object const& obj, T& value) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;

            boost::mp11::mp_for_each<Members>([&](auto D) {
                if (auto const* member = obj.if_contains(D.name)) {
                    if (member->is_null()) {
                        return;
                    }

                    using MemberType = std::remove_cv_t<
                        std::remove_reference_t<decltype(value.*D.pointer)>>;
                    value.*D.pointer = boost::json::value_to<MemberType>(*member);
                }
            });
        }
    }

    inline void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, TradeSide side) {
        jv = to_string(side);
    }

    inline TradeSide tag_invoke(boost::json::value_to_tag<TradeSide>, boost::json::value const& jv) {
        return trade_side_from_string(boost::json::value_to<std::string>(jv));
    }

#define POLYMARKET_CLOB_JSON(T)                                                \
    inline void tag_invoke(boost::json::value_from_tag,                        \
                           boost::json::value& jv, T const& value) {           \
        detail::json_value_from_described(jv.emplace_object(), value);         \
    }                                                                          \
    inline T tag_invoke(boost::json::value_to_tag<T>,                          \
                        boost::json::value const& jv) {                        \
        T value{};                                                             \
        if (auto const* obj = jv.if_object()) {                                \
            detail::json_value_to_described(*obj, value);                      \
        }                                                                      \
        return value;                                                          \
    }

    POLYMARKET_CLOB_JSON(PriceLevel)
    POLYMARKET_CLOB_JSON(OrderBook)
    POLYMARKET_CLOB_JSON(PriceQuote)
    POLYMARKET_CLOB_JSON(SidePriceQuote)
    POLYMARKET_CLOB_JSON(TokenPriceQuote)
    POLYMARKET_CLOB_JSON(TokenSidePriceQuote)
    POLYMARKET_CLOB_JSON(PriceHistoryPoint)
    POLYMARKET_CLOB_JSON(PriceHistory)
    POLYMARKET_CLOB_JSON(MarketRewardInfo)
    POLYMARKET_CLOB_JSON(MarketTokenInfo)
    POLYMARKET_CLOB_JSON(FeeCurve)
    POLYMARKET_CLOB_JSON(ClobMarketInfo)
    POLYMARKET_CLOB_JSON(ClobMarket)
    POLYMARKET_CLOB_JSON(ClobMarketsResponse)

#undef POLYMARKET_CLOB_JSON
}

#endif //POLYMARKET_CLOB_TYPES_H
