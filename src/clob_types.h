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

    // ---------------------------------------------------------------------------
    // Trade side
    // ---------------------------------------------------------------------------
    enum class TradeSide {
        Buy,
        Sell
    };

    inline std::string to_string(TradeSide side) {
        switch (side) {
            case TradeSide::Buy:  return "BUY";
            case TradeSide::Sell: return "SELL";
            default:              return "BUY";
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

    // ---------------------------------------------------------------------------
    // Order book
    // ---------------------------------------------------------------------------
    struct PriceLevel {
        double price = 0.0;
        double size  = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(PriceLevel, (), (price, size))

    struct OrderBook {
        std::string             market;
        std::string             asset_id;
        std::string             timestamp;
        std::string             hash;
        std::vector<PriceLevel> bids;
        std::vector<PriceLevel> asks;
        double                  min_order_size    = 0.0;
        double                  tick_size         = 0.0;
        bool                    neg_risk          = false;
        double                  last_trade_price  = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(OrderBook, (),
        (market, asset_id, timestamp, hash, bids, asks,
         min_order_size, tick_size, neg_risk, last_trade_price))

    // ---------------------------------------------------------------------------
    // Price quote DTOs
    // ---------------------------------------------------------------------------
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

    // Single-trade reply: GET /last-trade-price
    struct LastTradePriceResponse {
        double    price = 0.0;
        TradeSide side  = TradeSide::Buy;
    };
    BOOST_DESCRIBE_STRUCT(LastTradePriceResponse, (), (price, side))

    // Bulk reply element: GET /last-trades-prices
    struct LastTradesPriceItem {
        std::string token_id;
        double      price = 0.0;
        TradeSide   side  = TradeSide::Buy;
    };
    BOOST_DESCRIBE_STRUCT(LastTradesPriceItem, (), (token_id, price, side))

    // Maps used by /midpoints, /prices, /spreads (server returns object keyed by token id)
    using TokenPrices     = std::unordered_map<std::string, double>;
    using TokenSidePrices = std::unordered_map<std::string, SidePriceQuote>;

    // ---------------------------------------------------------------------------
    // Thin response wrappers (replaces ad-hoc as_object().at("...") parsing)
    // ---------------------------------------------------------------------------
    struct MidpointResponse {
        double mid = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(MidpointResponse, (), (mid))

    struct PriceResponse {
        double price = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(PriceResponse, (), (price))

    struct SpreadResponse {
        double spread = 0.0;
    };

    struct FeeRateResponse {
        std::uint32_t base_fee = 0;
    };
    BOOST_DESCRIBE_STRUCT(FeeRateResponse, (), (base_fee))

    struct TickSizeResponse {
        double minimum_tick_size = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(TickSizeResponse, (), (minimum_tick_size))

    struct NegRiskResponse {
        bool neg_risk = false;
    };
    BOOST_DESCRIBE_STRUCT(NegRiskResponse, (), (neg_risk))

    // ---------------------------------------------------------------------------
    // Price history
    // ---------------------------------------------------------------------------
    enum class PriceHistoryInterval {
        None,
        OneHour,    // 1h
        SixHour,    // 6h
        OneDay,     // 1d
        OneWeek,    // 1w
        OneMonth,   // 1m
        Max,        // max
    };

    inline std::string to_string(PriceHistoryInterval ivl) {
        switch (ivl) {
            case PriceHistoryInterval::OneHour:  return "1h";
            case PriceHistoryInterval::SixHour:  return "6h";
            case PriceHistoryInterval::OneDay:   return "1d";
            case PriceHistoryInterval::OneWeek:  return "1w";
            case PriceHistoryInterval::OneMonth: return "1m";
            case PriceHistoryInterval::Max:      return "max";
            default:                              return "";
        }
    }

    // Mutually-exclusive timeframe selector: either an interval OR a (start_ts, end_ts) pair.
    struct PriceHistoryRequest {
        std::string                          market;             // CLOB token id
        std::optional<PriceHistoryInterval>  interval;
        std::optional<std::int64_t>          start_ts;
        std::optional<std::int64_t>          end_ts;
        std::optional<std::int32_t>          fidelity;           // resolution in minutes
    };

    // Wire shape: {"t": <ts>, "p": <price>}
    struct PricePoint {
        std::int64_t t = 0;
        double       p = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(PricePoint, (), (t, p))

    struct PriceHistoryResponse {
        std::vector<PricePoint> history;
    };
    BOOST_DESCRIBE_STRUCT(PriceHistoryResponse, (), (history))

    // ---------------------------------------------------------------------------
    // CLOB market info (compact-key wire schema, /clob-markets/{condition_id})
    // ---------------------------------------------------------------------------
    struct ClobToken {
        std::string token_id;   // wire: "t"
        std::string outcome;    // wire: "o"
    };

    struct FeeDetails {
        double        rate       = 0.0;   // wire: "r"
        std::uint32_t exponent   = 0;     // wire: "e"
        bool          taker_only = false; // wire: "to"
    };

    struct ClobMarketInfo {
        std::string                 condition_id;             // wire: "c"
        std::vector<ClobToken>      tokens;                   // wire: "t"
        double                      min_tick_size = 0.0;      // wire: "mts"
        double                      min_order_size = 0.0;     // wire: "mos"
        bool                        neg_risk = false;         // wire: "nr"
        std::optional<FeeDetails>   fee_details;              // wire: "fd"
        std::optional<double>       maker_base_fee;           // wire: "mbf"
        std::optional<double>       taker_base_fee;           // wire: "tbf"
        bool                        rfq_enabled = false;      // wire: "rfqe"
    };

    // ---------------------------------------------------------------------------
    // Market reward info (/markets, /sampling-markets)
    // ---------------------------------------------------------------------------
    struct RewardRate {
        std::string asset_address;
        double      rewards_daily_rate = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(RewardRate, (), (asset_address, rewards_daily_rate))

    struct Rewards {
        std::vector<RewardRate> rates;
        double                  min_size   = 0.0;
        double                  max_spread = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(Rewards, (), (rates, min_size, max_spread))

    // Token within a MarketResponse / SimplifiedMarket
    struct Token {
        std::string token_id;
        std::string outcome;
        double      price  = 0.0;
        bool        winner = false;
    };
    BOOST_DESCRIBE_STRUCT(Token, (), (token_id, outcome, price, winner))

    // ---------------------------------------------------------------------------
    // Markets
    // ---------------------------------------------------------------------------
    // Slim market schema returned by /simplified-markets and /sampling-simplified-markets.
    struct SimplifiedMarket {
        std::string        condition_id;
        std::vector<Token> tokens;
        Rewards            rewards;
        bool               active = false;
        bool               closed = false;
        bool               archived = false;
        bool               accepting_orders = false;
    };
    BOOST_DESCRIBE_STRUCT(SimplifiedMarket, (),
        (condition_id, tokens, rewards, active, closed, archived, accepting_orders))

    // Full market schema returned by /markets and /sampling-markets and /markets/{cid}.
    struct MarketResponse {
        bool                     enable_order_book = false;
        bool                     active = false;
        bool                     closed = false;
        bool                     archived = false;
        bool                     accepting_orders = false;
        std::string              accepting_order_timestamp;
        double                   minimum_order_size = 0.0;
        double                   minimum_tick_size = 0.0;
        std::string              condition_id;
        std::string              question_id;
        std::string              question;
        std::string              description;
        std::string              market_slug;
        std::string              end_date_iso;
        std::string              game_start_time;
        std::uint64_t            seconds_delay = 0;
        std::string              fpmm;
        double                   maker_base_fee = 0.0;
        double                   taker_base_fee = 0.0;
        bool                     notifications_enabled = false;
        bool                     neg_risk = false;
        std::string              neg_risk_market_id;
        std::string              neg_risk_request_id;
        std::string              icon;
        std::string              image;
        Rewards                  rewards;
        bool                     is_50_50_outcome = false;
        std::vector<Token>       tokens;
        std::vector<std::string> tags;
    };
    BOOST_DESCRIBE_STRUCT(MarketResponse, (),
        (enable_order_book, active, closed, archived, accepting_orders,
         accepting_order_timestamp, minimum_order_size, minimum_tick_size,
         condition_id, question_id, question, description, market_slug,
         end_date_iso, game_start_time, seconds_delay, fpmm,
         maker_base_fee, taker_base_fee, notifications_enabled,
         neg_risk, neg_risk_market_id, neg_risk_request_id,
         icon, image, rewards, is_50_50_outcome, tokens, tags))

    // Backwards-compatible alias for callers that referenced the old name.
    using ClobMarket = MarketResponse;

    // Generic paginated response. Used as Page<MarketResponse> and Page<SimplifiedMarket>.
    template <class T>
    struct Page {
        std::vector<T>      data;
        std::string         next_cursor;
        std::uint64_t       limit = 0;
        std::uint64_t       count = 0;
    };

    // ---------------------------------------------------------------------------
    // Markets-by-token
    // ---------------------------------------------------------------------------
    struct MarketByTokenResponse {
        std::string condition_id;
        std::string primary_token_id;
        std::string secondary_token_id;
    };
    BOOST_DESCRIBE_STRUCT(MarketByTokenResponse, (),
        (condition_id, primary_token_id, secondary_token_id))

    // ---------------------------------------------------------------------------
    // Geoblock
    // ---------------------------------------------------------------------------
    struct GeoblockResponse {
        bool        blocked = false;
        std::string ip;
        std::string country;
        std::string region;
    };
    BOOST_DESCRIBE_STRUCT(GeoblockResponse, (), (blocked, ip, country, region))

    // ---------------------------------------------------------------------------
    // Maker rebates (Polymarket-only endpoint, not in Rust client)
    // ---------------------------------------------------------------------------
    struct MakerRebates {
        std::string date;
        std::string condition_id;
        std::string asset_address;
        std::string maker_address;
        std::string rebated_fees_usdc;
    };
    BOOST_DESCRIBE_STRUCT(MakerRebates, (),
        (date, condition_id, asset_address, maker_address, rebated_fees_usdc))

    // ---------------------------------------------------------------------------
    // JSON glue
    // ---------------------------------------------------------------------------
    namespace detail {
        // Read a field that may arrive as either a JSON number or a quoted string.
        inline double as_number_or_string(boost::json::value const& jv) {
            if (jv.is_double())  return jv.as_double();
            if (jv.is_int64())   return static_cast<double>(jv.as_int64());
            if (jv.is_uint64())  return static_cast<double>(jv.as_uint64());
            if (jv.is_string())  return std::stod(std::string(jv.as_string()));
            return 0.0;
        }

        template <class T>
        T as_integral_or_string(boost::json::value const& jv) {
            if (jv.is_int64()) {
                return static_cast<T>(jv.as_int64());
            }
            if (jv.is_uint64()) {
                return static_cast<T>(jv.as_uint64());
            }
            if (jv.is_double()) {
                return static_cast<T>(jv.as_double());
            }
            if (jv.is_string()) {
                if constexpr (std::is_signed_v<T>) {
                    return static_cast<T>(std::stoll(std::string(jv.as_string())));
                } else {
                    return static_cast<T>(std::stoull(std::string(jv.as_string())));
                }
            }
            return T{};
        }

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

                    if constexpr (std::is_same_v<MemberType, double>) {
                        value.*D.pointer = as_number_or_string(*member);
                    } else if constexpr (std::is_integral_v<MemberType> &&
                                         !std::is_same_v<MemberType, bool>) {
                        value.*D.pointer = as_integral_or_string<MemberType>(*member);
                    } else {
                        value.*D.pointer = boost::json::value_to<MemberType>(*member);
                    }
                }
            });
        }
    }

    // TradeSide <-> JSON
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
    POLYMARKET_CLOB_JSON(LastTradePriceResponse)
    POLYMARKET_CLOB_JSON(LastTradesPriceItem)
    POLYMARKET_CLOB_JSON(MidpointResponse)
    POLYMARKET_CLOB_JSON(PriceResponse)
    POLYMARKET_CLOB_JSON(FeeRateResponse)
    POLYMARKET_CLOB_JSON(TickSizeResponse)
    POLYMARKET_CLOB_JSON(NegRiskResponse)
    POLYMARKET_CLOB_JSON(PricePoint)
    POLYMARKET_CLOB_JSON(PriceHistoryResponse)
    POLYMARKET_CLOB_JSON(RewardRate)
    POLYMARKET_CLOB_JSON(Rewards)
    POLYMARKET_CLOB_JSON(Token)
    POLYMARKET_CLOB_JSON(SimplifiedMarket)
    POLYMARKET_CLOB_JSON(MarketResponse)
    POLYMARKET_CLOB_JSON(MarketByTokenResponse)
    POLYMARKET_CLOB_JSON(GeoblockResponse)
    POLYMARKET_CLOB_JSON(MakerRebates)

#undef POLYMARKET_CLOB_JSON

    // SpreadResponse: the wire field "spread" can arrive as either number or string.
    inline void tag_invoke(boost::json::value_from_tag,
                           boost::json::value& jv, SpreadResponse const& s) {
        jv = { {"spread", s.spread} };
    }
    inline SpreadResponse tag_invoke(boost::json::value_to_tag<SpreadResponse>,
                                     boost::json::value const& jv) {
        SpreadResponse out;
        if (auto const* obj = jv.if_object()) {
            if (auto const* sp = obj->if_contains("spread")) {
                out.spread = detail::as_number_or_string(*sp);
            }
        }
        return out;
    }

    // ClobMarketInfo: compact wire keys (c, t, mts, mos, nr, fd, mbf, tbf, rfqe)
    // mapped to ergonomic C++ names.
    inline void tag_invoke(boost::json::value_from_tag,
                           boost::json::value& jv, FeeDetails const& v) {
        jv = boost::json::object{
            {"r",  v.rate},
            {"e",  v.exponent},
            {"to", v.taker_only},
        };
    }
    inline FeeDetails tag_invoke(boost::json::value_to_tag<FeeDetails>,
                                 boost::json::value const& jv) {
        FeeDetails v;
        if (auto const* obj = jv.if_object()) {
            if (auto const* p = obj->if_contains("r"))  v.rate       = detail::as_number_or_string(*p);
            if (auto const* p = obj->if_contains("e"))  v.exponent   = static_cast<std::uint32_t>(p->to_number<std::uint64_t>());
            if (auto const* p = obj->if_contains("to")) v.taker_only = p->as_bool();
        }
        return v;
    }

    inline void tag_invoke(boost::json::value_from_tag,
                           boost::json::value& jv, ClobToken const& v) {
        jv = boost::json::object{ {"t", v.token_id}, {"o", v.outcome} };
    }
    inline ClobToken tag_invoke(boost::json::value_to_tag<ClobToken>,
                                boost::json::value const& jv) {
        ClobToken v;
        if (auto const* obj = jv.if_object()) {
            if (auto const* p = obj->if_contains("t"); p && !p->is_null())
                v.token_id = boost::json::value_to<std::string>(*p);
            if (auto const* p = obj->if_contains("o"); p && !p->is_null())
                v.outcome = boost::json::value_to<std::string>(*p);
        }
        return v;
    }

    inline void tag_invoke(boost::json::value_from_tag,
                           boost::json::value& jv, ClobMarketInfo const& v) {
        boost::json::object obj;
        obj["c"]   = v.condition_id;
        obj["t"]   = boost::json::value_from(v.tokens);
        obj["mts"] = v.min_tick_size;
        obj["mos"] = v.min_order_size;
        obj["nr"]  = v.neg_risk;
        if (v.fee_details)    obj["fd"]  = boost::json::value_from(*v.fee_details);
        if (v.maker_base_fee) obj["mbf"] = *v.maker_base_fee;
        if (v.taker_base_fee) obj["tbf"] = *v.taker_base_fee;
        obj["rfqe"] = v.rfq_enabled;
        jv = std::move(obj);
    }
    inline ClobMarketInfo tag_invoke(boost::json::value_to_tag<ClobMarketInfo>,
                                     boost::json::value const& jv) {
        ClobMarketInfo v;
        auto const* obj = jv.if_object();
        if (!obj) return v;

        if (auto const* p = obj->if_contains("c"); p && !p->is_null())
            v.condition_id = boost::json::value_to<std::string>(*p);

        if (auto const* p = obj->if_contains("t"); p && p->is_array()) {
            for (auto const& el : p->as_array()) {
                if (el.is_null()) continue;
                v.tokens.push_back(boost::json::value_to<ClobToken>(el));
            }
        }

        if (auto const* p = obj->if_contains("mts"); p && !p->is_null())
            v.min_tick_size = detail::as_number_or_string(*p);

        if (auto const* p = obj->if_contains("mos"); p && !p->is_null())
            v.min_order_size = detail::as_number_or_string(*p);

        if (auto const* p = obj->if_contains("nr"); p && p->is_bool())
            v.neg_risk = p->as_bool();

        if (auto const* p = obj->if_contains("fd"); p && !p->is_null())
            v.fee_details = boost::json::value_to<FeeDetails>(*p);

        if (auto const* p = obj->if_contains("mbf"); p && !p->is_null())
            v.maker_base_fee = detail::as_number_or_string(*p);

        if (auto const* p = obj->if_contains("tbf"); p && !p->is_null())
            v.taker_base_fee = detail::as_number_or_string(*p);

        if (auto const* p = obj->if_contains("rfqe"); p && p->is_bool())
            v.rfq_enabled = p->as_bool();

        return v;
    }

    // Templated Page<T> serialization.
    template <class T>
    void tag_invoke(boost::json::value_from_tag,
                    boost::json::value& jv, Page<T> const& p) {
        boost::json::object obj;
        obj["data"]        = boost::json::value_from(p.data);
        obj["next_cursor"] = p.next_cursor;
        obj["limit"]       = p.limit;
        obj["count"]       = p.count;
        jv = std::move(obj);
    }

    template <class T>
    Page<T> tag_invoke(boost::json::value_to_tag<Page<T>>, boost::json::value const& jv) {
        Page<T> p;
        if (auto const* obj = jv.if_object()) {
            if (auto const* d = obj->if_contains("data"))
                p.data = boost::json::value_to<std::vector<T>>(*d);
            if (auto const* nc = obj->if_contains("next_cursor"); nc && !nc->is_null())
                p.next_cursor = boost::json::value_to<std::string>(*nc);
            if (auto const* l = obj->if_contains("limit"); l && !l->is_null())
                p.limit = l->to_number<std::uint64_t>();
            if (auto const* c = obj->if_contains("count"); c && !c->is_null())
                p.count = c->to_number<std::uint64_t>();
        }
        return p;
    }

}

#endif //POLYMARKET_CLOB_TYPES_H
