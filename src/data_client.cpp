//
// Created by Lorenzo P on 4/25/26.
//

#include "data_client.h"

#include <stdexcept>
#include <string>
#include <string_view>

#include <boost/json.hpp>
#include <boost/url.hpp>

#include "url_utils.h"

namespace json = boost::json;
namespace urls = boost::urls;

namespace polymarket::data {
    namespace {
        [[noreturn]] void throw_http(const HttpResponse &r, std::string_view path) {
            throw std::runtime_error(
                "data-api request failed (" + std::string(path) + "): " +
                std::to_string(r.status) + " - " + r.error);
        }

        std::string with_query(std::string base, const urls::url &u) {
            auto q = static_cast<std::string>(u.encoded_query());
            if (q.empty()) return base;
            base.push_back('?');
            base.append(q);
            return base;
        }

        void append_str(urls::url &u, std::string_view k, const std::string &v) {
            if (!v.empty()) u.params().append({std::string(k), v});
        }
        void append_opt_int(urls::url &u, std::string_view k, const std::optional<int> &v) {
            if (v) u.params().append({std::string(k), std::to_string(*v)});
        }
        void append_opt_u64(urls::url &u, std::string_view k, const std::optional<std::uint64_t> &v) {
            if (v) u.params().append({std::string(k), std::to_string(*v)});
        }
        void append_opt_bool(urls::url &u, std::string_view k, const std::optional<bool> &v) {
            if (v) u.params().append({std::string(k), *v ? "true" : "false"});
        }
        void append_opt_double(urls::url &u, std::string_view k, const std::optional<double> &v) {
            if (v) u.params().append({std::string(k), std::to_string(*v)});
        }

        template <class E>
        void append_opt_enum(urls::url &u, std::string_view k, const std::optional<E> &v) {
            if (v) u.params().append({std::string(k), std::string(to_string(*v))});
        }

        void append_market_filter(urls::url &u, const std::optional<MarketFilter> &f) {
            if (!f) return;
            if (!f->markets.empty()) {
                u.params().append({"market", join_vector(f->markets)});
            } else if (!f->event_ids.empty()) {
                u.params().append({"eventId", join_vector(f->event_ids)});
            }
        }

        void append_trade_filter(urls::url &u, const std::optional<TradeFilter> &f) {
            if (!f) return;
            u.params().append({"filterType",   std::string(to_string(f->filter_type))});
            u.params().append({"filterAmount", std::to_string(f->filter_amount)});
        }

        void append_activity_types(urls::url &u, const std::vector<ActivityType> &types) {
            if (types.empty()) return;
            std::string csv;
            for (size_t i = 0; i < types.size(); ++i) {
                if (i) csv.push_back(',');
                csv.append(to_string(types[i]));
            }
            u.params().append({"type", csv});
        }
    }

    DataClient::DataClient(const APIConfig &config)
        : config_(config)
    {
        if (config_.log_level.has_value()) {
            http_.set_log_level(*config_.log_level);
        }
        http_.set_base_url(config_.data_api_url);
        http_.set_timeout(config_.http_timeout_ms);
    }

    Health DataClient::health() {
        const auto r = http_.get("/");
        if (!r.ok()) throw_http(r, "/");
        return json::value_to<Health>(json::parse(r.body));
    }

    std::vector<Position> DataClient::positions(const PositionsRequest &req) {
        urls::url u;
        append_str       (u, "user",          req.user);
        append_market_filter(u, req.filter);
        append_opt_double(u, "sizeThreshold", req.size_threshold);
        append_opt_bool  (u, "redeemable",    req.redeemable);
        append_opt_bool  (u, "mergeable",     req.mergeable);
        append_opt_int   (u, "limit",         req.limit);
        append_opt_int   (u, "offset",        req.offset);
        append_opt_enum  (u, "sortBy",        req.sort_by);
        append_opt_enum  (u, "sortDirection", req.sort_direction);
        append_str       (u, "title",         req.title);
        const auto r = http_.get(with_query("/positions", u));
        if (!r.ok()) throw_http(r, "/positions");
        return json::value_to<std::vector<Position>>(json::parse(r.body));
    }

    std::vector<Trade> DataClient::trades(const TradesRequest &req) {
        urls::url u;
        append_str       (u, "user",      req.user);
        append_market_filter(u, req.filter);
        append_opt_int   (u, "limit",     req.limit);
        append_opt_int   (u, "offset",    req.offset);
        append_opt_bool  (u, "takerOnly", req.taker_only);
        append_trade_filter(u, req.trade_filter);
        append_opt_enum  (u, "side",      req.side);
        const auto r = http_.get(with_query("/trades", u));
        if (!r.ok()) throw_http(r, "/trades");
        return json::value_to<std::vector<Trade>>(json::parse(r.body));
    }

    std::vector<Activity> DataClient::activity(const ActivityRequest &req) {
        urls::url u;
        append_str       (u, "user",          req.user);
        append_market_filter(u, req.filter);
        append_activity_types(u, req.activity_types);
        append_opt_int   (u, "limit",         req.limit);
        append_opt_int   (u, "offset",        req.offset);
        append_opt_u64   (u, "start",         req.start);
        append_opt_u64   (u, "end",           req.end);
        append_opt_enum  (u, "sortBy",        req.sort_by);
        append_opt_enum  (u, "sortDirection", req.sort_direction);
        append_opt_enum  (u, "side",          req.side);
        const auto r = http_.get(with_query("/activity", u));
        if (!r.ok()) throw_http(r, "/activity");
        return json::value_to<std::vector<Activity>>(json::parse(r.body));
    }

    std::vector<MetaHolder> DataClient::holders(const HoldersRequest &req) {
        urls::url u;
        if (!req.markets.empty()) u.params().append({"market", join_vector(req.markets)});
        append_opt_int(u, "limit",      req.limit);
        append_opt_int(u, "minBalance", req.min_balance);
        const auto r = http_.get(with_query("/holders", u));
        if (!r.ok()) throw_http(r, "/holders");
        return json::value_to<std::vector<MetaHolder>>(json::parse(r.body));
    }

    std::vector<Value> DataClient::value(const ValueRequest &req) {
        urls::url u;
        append_str(u, "user", req.user);
        if (!req.markets.empty()) u.params().append({"market", join_vector(req.markets)});
        const auto r = http_.get(with_query("/value", u));
        if (!r.ok()) throw_http(r, "/value");
        return json::value_to<std::vector<Value>>(json::parse(r.body));
    }

    std::vector<ClosedPosition> DataClient::closed_positions(const ClosedPositionsRequest &req) {
        urls::url u;
        append_str       (u, "user",          req.user);
        append_market_filter(u, req.filter);
        append_str       (u, "title",         req.title);
        append_opt_int   (u, "limit",         req.limit);
        append_opt_int   (u, "offset",        req.offset);
        append_opt_enum  (u, "sortBy",        req.sort_by);
        append_opt_enum  (u, "sortDirection", req.sort_direction);
        const auto r = http_.get(with_query("/closed-positions", u));
        if (!r.ok()) throw_http(r, "/closed-positions");
        return json::value_to<std::vector<ClosedPosition>>(json::parse(r.body));
    }

    std::vector<TraderLeaderboardEntry> DataClient::leaderboard(const TraderLeaderboardRequest &req) {
        urls::url u;
        append_opt_enum(u, "category",   req.category);
        append_opt_enum(u, "timePeriod", req.time_period);
        append_opt_enum(u, "orderBy",    req.order_by);
        append_opt_int (u, "limit",      req.limit);
        append_opt_int (u, "offset",     req.offset);
        append_str     (u, "user",       req.user);
        append_str     (u, "userName",   req.user_name);
        const auto r = http_.get(with_query("/v1/leaderboard", u));
        if (!r.ok()) throw_http(r, "/v1/leaderboard");
        return json::value_to<std::vector<TraderLeaderboardEntry>>(json::parse(r.body));
    }

    Traded DataClient::traded(const TradedRequest &req) {
        urls::url u;
        append_str(u, "user", req.user);
        const auto r = http_.get(with_query("/traded", u));
        if (!r.ok()) throw_http(r, "/traded");
        return json::value_to<Traded>(json::parse(r.body));
    }

    std::vector<OpenInterest> DataClient::open_interest(const OpenInterestRequest &req) {
        urls::url u;
        if (!req.markets.empty()) u.params().append({"market", join_vector(req.markets)});
        const auto r = http_.get(with_query("/oi", u));
        if (!r.ok()) throw_http(r, "/oi");
        return json::value_to<std::vector<OpenInterest>>(json::parse(r.body));
    }

    std::vector<LiveVolume> DataClient::live_volume(const LiveVolumeRequest &req) {
        urls::url u;
        u.params().append({"id", std::to_string(req.id)});
        const auto r = http_.get(with_query("/live-volume", u));
        if (!r.ok()) throw_http(r, "/live-volume");
        return json::value_to<std::vector<LiveVolume>>(json::parse(r.body));
    }

    std::vector<BuilderLeaderboardEntry> DataClient::builder_leaderboard(const BuilderLeaderboardRequest &req) {
        urls::url u;
        append_opt_enum(u, "timePeriod", req.time_period);
        append_opt_int (u, "limit",      req.limit);
        append_opt_int (u, "offset",     req.offset);
        const auto r = http_.get(with_query("/v1/builders/leaderboard", u));
        if (!r.ok()) throw_http(r, "/v1/builders/leaderboard");
        return json::value_to<std::vector<BuilderLeaderboardEntry>>(json::parse(r.body));
    }

    std::vector<BuilderVolumeEntry> DataClient::builder_volume(const BuilderVolumeRequest &req) {
        urls::url u;
        append_opt_enum(u, "timePeriod", req.time_period);
        const auto r = http_.get(with_query("/v1/builders/volume", u));
        if (!r.ok()) throw_http(r, "/v1/builders/volume");
        return json::value_to<std::vector<BuilderVolumeEntry>>(json::parse(r.body));
    }
}
