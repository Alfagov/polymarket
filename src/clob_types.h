//
// Created by Lorenzo P on 4/21/26.
//

#ifndef POLYMARKET_CLOB_TYPES_H
#define POLYMARKET_CLOB_TYPES_H

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/describe.hpp>
#include <boost/json.hpp>
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

    struct MarketRewardInfo {};
    BOOST_DESCRIBE_STRUCT(MarketRewardInfo, (), ())

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
}

#endif //POLYMARKET_CLOB_TYPES_H
