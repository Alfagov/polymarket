//
// Created by Lorenzo P on 4/25/26.
//
// Types for data-api.polymarket.com, mirrored from
// /Users/alfagov/RustroverProjects/rs-clob-client-v2/src/data/types.
//

#ifndef POLYMARKET_DATA_TYPES_H
#define POLYMARKET_DATA_TYPES_H
#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <boost/describe.hpp>
#include <boost/json.hpp>
#include <boost/mp11.hpp>

namespace polymarket::data {

    // ---------------------------------------------------------------------------
    // Enums (string-encoded on the wire — see Rust strum/serde rename_all).
    // ---------------------------------------------------------------------------
    enum class Side { Buy, Sell };
    inline constexpr std::string_view to_string(Side s) noexcept {
        return s == Side::Buy ? "BUY" : "SELL";
    }
    inline Side parse_side(std::string_view s) {
        if (s == "BUY"  || s == "buy")  return Side::Buy;
        if (s == "SELL" || s == "sell") return Side::Sell;
        throw std::runtime_error("invalid side: " + std::string(s));
    }

    enum class ActivityType {
        Trade, Split, Merge, Redeem, Reward, Conversion, Yield, MakerRebate,
    };
    inline constexpr std::string_view to_string(ActivityType t) noexcept {
        switch (t) {
            case ActivityType::Trade:       return "TRADE";
            case ActivityType::Split:       return "SPLIT";
            case ActivityType::Merge:       return "MERGE";
            case ActivityType::Redeem:      return "REDEEM";
            case ActivityType::Reward:      return "REWARD";
            case ActivityType::Conversion:  return "CONVERSION";
            case ActivityType::Yield:       return "YIELD";
            case ActivityType::MakerRebate: return "MAKERREBATE";
        }
        return "";
    }

    enum class PositionSortBy {
        Current, Initial, Tokens, CashPnl, PercentPnl, Title, Resolving, Price, AvgPrice,
    };
    inline constexpr std::string_view to_string(PositionSortBy s) noexcept {
        switch (s) {
            case PositionSortBy::Current:    return "CURRENT";
            case PositionSortBy::Initial:    return "INITIAL";
            case PositionSortBy::Tokens:     return "TOKENS";
            case PositionSortBy::CashPnl:    return "CASHPNL";
            case PositionSortBy::PercentPnl: return "PERCENTPNL";
            case PositionSortBy::Title:      return "TITLE";
            case PositionSortBy::Resolving:  return "RESOLVING";
            case PositionSortBy::Price:      return "PRICE";
            case PositionSortBy::AvgPrice:   return "AVGPRICE";
        }
        return "";
    }

    enum class ClosedPositionSortBy { RealizedPnl, Title, Price, AvgPrice, Timestamp };
    inline constexpr std::string_view to_string(ClosedPositionSortBy s) noexcept {
        switch (s) {
            case ClosedPositionSortBy::RealizedPnl: return "REALIZEDPNL";
            case ClosedPositionSortBy::Title:       return "TITLE";
            case ClosedPositionSortBy::Price:       return "PRICE";
            case ClosedPositionSortBy::AvgPrice:    return "AVGPRICE";
            case ClosedPositionSortBy::Timestamp:   return "TIMESTAMP";
        }
        return "";
    }

    enum class ActivitySortBy { Timestamp, Tokens, Cash };
    inline constexpr std::string_view to_string(ActivitySortBy s) noexcept {
        switch (s) {
            case ActivitySortBy::Timestamp: return "TIMESTAMP";
            case ActivitySortBy::Tokens:    return "TOKENS";
            case ActivitySortBy::Cash:      return "CASH";
        }
        return "";
    }

    enum class SortDirection { Asc, Desc };
    inline constexpr std::string_view to_string(SortDirection s) noexcept {
        return s == SortDirection::Asc ? "ASC" : "DESC";
    }

    enum class FilterType { Cash, Tokens };
    inline constexpr std::string_view to_string(FilterType t) noexcept {
        return t == FilterType::Cash ? "CASH" : "TOKENS";
    }

    enum class TimePeriod { Day, Week, Month, All };
    inline constexpr std::string_view to_string(TimePeriod t) noexcept {
        switch (t) {
            case TimePeriod::Day:   return "DAY";
            case TimePeriod::Week:  return "WEEK";
            case TimePeriod::Month: return "MONTH";
            case TimePeriod::All:   return "ALL";
        }
        return "";
    }

    enum class LeaderboardCategory {
        Overall, Politics, Sports, Crypto, Culture, Mentions, Weather,
        Economics, Tech, Finance,
    };
    inline constexpr std::string_view to_string(LeaderboardCategory c) noexcept {
        switch (c) {
            case LeaderboardCategory::Overall:   return "OVERALL";
            case LeaderboardCategory::Politics:  return "POLITICS";
            case LeaderboardCategory::Sports:    return "SPORTS";
            case LeaderboardCategory::Crypto:    return "CRYPTO";
            case LeaderboardCategory::Culture:   return "CULTURE";
            case LeaderboardCategory::Mentions:  return "MENTIONS";
            case LeaderboardCategory::Weather:   return "WEATHER";
            case LeaderboardCategory::Economics: return "ECONOMICS";
            case LeaderboardCategory::Tech:      return "TECH";
            case LeaderboardCategory::Finance:   return "FINANCE";
        }
        return "";
    }

    enum class LeaderboardOrderBy { Pnl, Vol };
    inline constexpr std::string_view to_string(LeaderboardOrderBy o) noexcept {
        return o == LeaderboardOrderBy::Pnl ? "PNL" : "VOL";
    }

    // ---------------------------------------------------------------------------
    // MarketFilter — Rust models markets XOR eventIds; we expose both as
    // optional vectors and serialize whichever is set (markets wins if both).
    // ---------------------------------------------------------------------------
    struct MarketFilter {
        std::vector<std::string> markets;     // wire: "market" (CSV of B256)
        std::vector<std::string> event_ids;   // wire: "eventId" (CSV of strings)
    };

    struct TradeFilter {
        FilterType filter_type = FilterType::Cash;   // wire: "filterType"
        double     filter_amount = 0.0;              // wire: "filterAmount"
    };

    // ---------------------------------------------------------------------------
    // Request types — pure data; the client serializes to query strings.
    // ---------------------------------------------------------------------------
    struct PositionsRequest {
        std::string                user;                      // proxy address (required)
        std::optional<MarketFilter> filter;
        std::optional<double>      size_threshold;
        std::optional<bool>        redeemable;
        std::optional<bool>        mergeable;
        std::optional<int>         limit;                     // 0..=500
        std::optional<int>         offset;                    // 0..=10000
        std::optional<PositionSortBy> sort_by;
        std::optional<SortDirection>  sort_direction;
        std::string                title;
    };

    struct TradesRequest {
        std::string                user;                      // optional address; empty = unset
        std::optional<MarketFilter> filter;
        std::optional<int>         limit;
        std::optional<int>         offset;
        std::optional<bool>        taker_only;
        std::optional<TradeFilter> trade_filter;
        std::optional<Side>        side;
    };

    struct ActivityRequest {
        std::string                user;                      // required
        std::optional<MarketFilter> filter;
        std::vector<ActivityType>  activity_types;            // serialized as CSV "type=…"
        std::optional<int>         limit;
        std::optional<int>         offset;
        std::optional<std::uint64_t> start;
        std::optional<std::uint64_t> end;
        std::optional<ActivitySortBy> sort_by;
        std::optional<SortDirection>  sort_direction;
        std::optional<Side>        side;
    };

    struct HoldersRequest {
        std::vector<std::string> markets;          // CSV of condition_ids
        std::optional<int>       limit;            // 0..=20
        std::optional<int>       min_balance;      // 0..=999_999
    };

    struct TradedRequest {
        std::string user;                          // required
    };

    struct ValueRequest {
        std::string              user;
        std::vector<std::string> markets;          // CSV of condition_ids
    };

    struct OpenInterestRequest {
        std::vector<std::string> markets;          // CSV of condition_ids
    };

    struct LiveVolumeRequest {
        std::uint64_t id = 0;                      // event id
    };

    struct ClosedPositionsRequest {
        std::string                user;                      // required
        std::optional<MarketFilter> filter;
        std::string                title;
        std::optional<int>         limit;
        std::optional<int>         offset;
        std::optional<ClosedPositionSortBy> sort_by;
        std::optional<SortDirection>        sort_direction;
    };

    struct BuilderLeaderboardRequest {
        std::optional<TimePeriod> time_period;
        std::optional<int>        limit;
        std::optional<int>        offset;
    };

    struct BuilderVolumeRequest {
        std::optional<TimePeriod> time_period;
    };

    struct TraderLeaderboardRequest {
        std::optional<LeaderboardCategory> category;
        std::optional<TimePeriod>          time_period;
        std::optional<LeaderboardOrderBy>  order_by;
        std::optional<int>                 limit;
        std::optional<int>                 offset;
        std::string                        user;
        std::string                        user_name;
    };

    // ---------------------------------------------------------------------------
    // Response types (camelCase wire keys per Rust serde rename_all).
    // ---------------------------------------------------------------------------
    struct Health {
        std::string data;
    };
    BOOST_DESCRIBE_STRUCT(Health, (), (data))

    struct ApiError {
        std::string error;
    };
    BOOST_DESCRIBE_STRUCT(ApiError, (), (error))

    struct Position {
        std::string proxyWallet;
        std::string asset;                     // U256 string
        std::string conditionId;               // B256 string
        double      size = 0.0;
        double      avgPrice = 0.0;
        double      initialValue = 0.0;
        double      currentValue = 0.0;
        double      cashPnl = 0.0;
        double      percentPnl = 0.0;
        double      totalBought = 0.0;
        double      realizedPnl = 0.0;
        double      percentRealizedPnl = 0.0;
        double      curPrice = 0.0;
        bool        redeemable = false;
        bool        mergeable = false;
        std::string title;
        std::string slug;
        std::string icon;
        std::string eventSlug;
        std::string eventId;
        std::string outcome;
        std::int32_t outcomeIndex = 0;
        std::string oppositeOutcome;
        std::string oppositeAsset;
        std::string endDate;                   // NaiveDate; empty when null
        bool        negativeRisk = false;
    };
    BOOST_DESCRIBE_STRUCT(Position, (),
        (proxyWallet, asset, conditionId, size, avgPrice, initialValue, currentValue,
         cashPnl, percentPnl, totalBought, realizedPnl, percentRealizedPnl, curPrice,
         redeemable, mergeable, title, slug, icon, eventSlug, eventId, outcome,
         outcomeIndex, oppositeOutcome, oppositeAsset, endDate, negativeRisk))

    struct ClosedPosition {
        std::string  proxyWallet;
        std::string  asset;
        std::string  conditionId;
        double       avgPrice = 0.0;
        double       totalBought = 0.0;
        double       realizedPnl = 0.0;
        double       curPrice = 0.0;
        std::int64_t timestamp = 0;
        std::string  title;
        std::string  slug;
        std::string  icon;
        std::string  eventSlug;
        std::string  outcome;
        std::int32_t outcomeIndex = 0;
        std::string  oppositeOutcome;
        std::string  oppositeAsset;
        std::string  endDate;
    };
    BOOST_DESCRIBE_STRUCT(ClosedPosition, (),
        (proxyWallet, asset, conditionId, avgPrice, totalBought, realizedPnl, curPrice,
         timestamp, title, slug, icon, eventSlug, outcome, outcomeIndex,
         oppositeOutcome, oppositeAsset, endDate))

    struct Trade {
        std::string  proxyWallet;
        std::string  side;                     // "BUY" / "SELL" (kept as string for round-trip)
        std::string  asset;
        std::string  conditionId;
        double       size = 0.0;
        double       price = 0.0;
        std::int64_t timestamp = 0;
        std::string  title;
        std::string  slug;
        std::string  icon;
        std::string  eventSlug;
        std::string  outcome;
        std::int32_t outcomeIndex = 0;
        std::string  name;
        std::string  pseudonym;
        std::string  bio;
        std::string  profileImage;
        std::string  profileImageOptimized;
        std::string  transactionHash;
    };
    BOOST_DESCRIBE_STRUCT(Trade, (),
        (proxyWallet, side, asset, conditionId, size, price, timestamp,
         title, slug, icon, eventSlug, outcome, outcomeIndex,
         name, pseudonym, bio, profileImage, profileImageOptimized, transactionHash))

    struct Activity {
        std::string  proxyWallet;
        std::int64_t timestamp = 0;
        std::string  conditionId;
        std::string  type;                     // ActivityType — kept as string
        double       size = 0.0;
        double       usdcSize = 0.0;
        std::string  transactionHash;
        std::optional<double> price;
        std::string  asset;
        std::string  side;
        std::optional<std::int32_t> outcomeIndex;
        std::string  title;
        std::string  slug;
        std::string  icon;
        std::string  eventSlug;
        std::string  outcome;
        std::string  name;
        std::string  pseudonym;
        std::string  bio;
        std::string  profileImage;
        std::string  profileImageOptimized;
    };
    BOOST_DESCRIBE_STRUCT(Activity, (),
        (proxyWallet, timestamp, conditionId, type, size, usdcSize, transactionHash,
         price, asset, side, outcomeIndex, title, slug, icon, eventSlug, outcome,
         name, pseudonym, bio, profileImage, profileImageOptimized))

    struct Holder {
        std::string  proxyWallet;
        std::string  bio;
        std::string  asset;
        std::string  pseudonym;
        double       amount = 0.0;
        std::optional<bool> displayUsernamePublic;
        std::int32_t outcomeIndex = 0;
        std::string  name;
        std::string  profileImage;
        std::string  profileImageOptimized;
        std::optional<bool> verified;
    };
    BOOST_DESCRIBE_STRUCT(Holder, (),
        (proxyWallet, bio, asset, pseudonym, amount, displayUsernamePublic,
         outcomeIndex, name, profileImage, profileImageOptimized, verified))

    struct MetaHolder {
        std::string         token;             // U256 string
        std::vector<Holder> holders;
    };
    BOOST_DESCRIBE_STRUCT(MetaHolder, (), (token, holders))

    struct Traded {
        std::string  user;
        std::int32_t traded = 0;
    };
    BOOST_DESCRIBE_STRUCT(Traded, (), (user, traded))

    struct Value {
        std::string user;
        double      value = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(Value, (), (user, value))

    // OpenInterest.market is `Market` enum on the wire — either string "GLOBAL"
    // or a B256 hex. We store it as the raw string (caller can compare).
    struct OpenInterest {
        std::string market;
        double      value = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(OpenInterest, (), (market, value))

    struct MarketVolume {
        std::string market;
        double      value = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(MarketVolume, (), (market, value))

    struct LiveVolume {
        double                    total = 0.0;
        std::vector<MarketVolume> markets;
    };
    BOOST_DESCRIBE_STRUCT(LiveVolume, (), (total, markets))

    struct BuilderLeaderboardEntry {
        std::int32_t rank = 0;                 // wire: string-encoded number
        std::string  builder;
        double       volume = 0.0;
        std::int32_t activeUsers = 0;
        bool         verified = false;
        std::string  builderLogo;
    };
    BOOST_DESCRIBE_STRUCT(BuilderLeaderboardEntry, (),
        (rank, builder, volume, activeUsers, verified, builderLogo))

    struct BuilderVolumeEntry {
        std::string  dt;                       // ISO-8601
        std::string  builder;
        std::string  builderLogo;
        bool         verified = false;
        double       volume = 0.0;
        std::int32_t activeUsers = 0;
        std::int32_t rank = 0;
    };
    BOOST_DESCRIBE_STRUCT(BuilderVolumeEntry, (),
        (dt, builder, builderLogo, verified, volume, activeUsers, rank))

    struct TraderLeaderboardEntry {
        std::int32_t rank = 0;
        std::string  proxyWallet;
        std::string  userName;
        double       vol = 0.0;
        double       pnl = 0.0;
        std::string  profileImage;
        std::string  xUsername;
        std::optional<bool> verifiedBadge;
    };
    BOOST_DESCRIBE_STRUCT(TraderLeaderboardEntry, (),
        (rank, proxyWallet, userName, vol, pnl, profileImage, xUsername, verifiedBadge))

    // ---------------------------------------------------------------------------
    // JSON glue (boost::describe + tag_invoke, with number-or-string tolerance
    // for fields the API encodes inconsistently — `rank` arrives as a string).
    // ---------------------------------------------------------------------------
    namespace detail {
        template <class T>
        T as_number(boost::json::value const& v) {
            if (v.is_double()) return static_cast<T>(v.as_double());
            if (v.is_int64())  return static_cast<T>(v.as_int64());
            if (v.is_uint64()) return static_cast<T>(v.as_uint64());
            if (v.is_string()) {
                try { return static_cast<T>(std::stod(std::string(v.as_string()))); }
                catch (...) { return T{}; }
            }
            return T{};
        }

        template <class T>
        void to_object(boost::json::object& obj, T const& t) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;
            boost::mp11::mp_for_each<Members>([&](auto D) {
                obj[D.name] = boost::json::value_from(t.*D.pointer);
            });
        }

        template <class T>
        void from_object(boost::json::object const& obj, T& t) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;
            boost::mp11::mp_for_each<Members>([&](auto D) {
                if (auto const* v = obj.if_contains(D.name)) {
                    if (v->is_null()) return;
                    using MT = std::remove_cv_t<
                        std::remove_reference_t<decltype(t.*D.pointer)>>;
                    if constexpr (std::is_same_v<MT, double>) {
                        t.*D.pointer = as_number<double>(*v);
                    } else if constexpr (std::is_integral_v<MT> && !std::is_same_v<MT, bool>) {
                        t.*D.pointer = as_number<MT>(*v);
                    } else {
                        try { t.*D.pointer = boost::json::value_to<MT>(*v); }
                        catch (...) { /* tolerate stray shape mismatches */ }
                    }
                }
            });
        }
    }

#define PDM_JSON(T)                                                              \
    inline void tag_invoke(boost::json::value_from_tag,                          \
                           boost::json::value& jv, T const& t) {                 \
        detail::to_object(jv.emplace_object(), t);                               \
    }                                                                            \
    inline T tag_invoke(boost::json::value_to_tag<T>,                            \
                        boost::json::value const& jv) {                          \
        T t{};                                                                   \
        if (auto const* o = jv.if_object()) detail::from_object(*o, t);          \
        return t;                                                                \
    }

    PDM_JSON(Health)
    PDM_JSON(ApiError)
    PDM_JSON(Position)
    PDM_JSON(ClosedPosition)
    PDM_JSON(Trade)
    PDM_JSON(Activity)
    PDM_JSON(Holder)
    PDM_JSON(MetaHolder)
    PDM_JSON(Traded)
    PDM_JSON(Value)
    PDM_JSON(OpenInterest)
    PDM_JSON(MarketVolume)
    PDM_JSON(LiveVolume)
    PDM_JSON(BuilderLeaderboardEntry)
    PDM_JSON(BuilderVolumeEntry)
    PDM_JSON(TraderLeaderboardEntry)

#undef PDM_JSON

}  // namespace polymarket::data

#endif  // POLYMARKET_DATA_TYPES_H
