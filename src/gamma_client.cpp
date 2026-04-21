//
// Created by Lorenzo P on 4/19/26.
//

#include "gamma_client.h"

#include <iostream>
#include <boost/json.hpp>
#include "gamma_types.h"
// Needs to stay for JSON parsing
#include "gamma_types_json.h"

namespace json  = boost::json;

namespace polymarket::gamma {

    GammaClient::GammaClient(const APIConfig &config)
        : config_(config)
    {
        http_.set_base_url(config_.gamma_api_url);
        http_.set_timeout(config_.http_timeout_ms);
    }

    // Returns without initial "?"
    std::string GammaClient::build_market_query_string(const MarketParameters& p) {
        boost::urls::url url;

        url.params().append({"limit", std::to_string(p.limit)});

        // --- Strings ---
        if (!p.start_date_min.empty()) url.params().append({"start_date_min", p.start_date_min});
        if (!p.start_date_max.empty()) url.params().append({"start_date_max", p.start_date_max});
        if (!p.end_date_min.empty()) url.params().append({"end_date_min", p.end_date_min});
        if (!p.end_date_max.empty()) url.params().append({"end_date_max", p.end_date_max});
        if (!p.game_id.empty()) url.params().append({"game_id", p.game_id});

        // --- Vectors (Joined by Commas) ---
        if (!p.order.empty()) url.params().append({"order", join_vector(p.order)});
        if (!p.id.empty()) url.params().append({"id", join_vector(p.id)});
        if (!p.slug.empty()) url.params().append({"slug", join_vector(p.slug)});
        if (!p.clob_token_ids.empty()) url.params().append({"clob_token_ids", join_vector(p.clob_token_ids)});
        if (!p.question_ids.empty()) url.params().append({"question_ids", join_vector(p.question_ids)});
        if (!p.mkt_maker_address.empty()) url.params().append({"mkt_maker_address", join_vector(p.mkt_maker_address)});
        if (!p.tag_id.empty()) url.params().append({"tag_id", join_vector(p.tag_id)});

        // --- Optionals (Ints & Bools) ---
        if (p.ascending.has_value()) {
            url.params().append({"ascending", p.ascending.value() ? "true" : "false"});
        }
        if (p.closed.has_value()) {
            url.params().append({"closed", p.closed.value() ? "true" : "false"});
        }
        if (p.liquidity_num_min.has_value()) {
            url.params().append({"liquidity_num_min", std::to_string(p.liquidity_num_min.value())});
        }
        if (p.liquidity_num_max.has_value()) {
            url.params().append({"liquidity_num_max",std::to_string(p.liquidity_num_max.value())});
        }
        if (p.volume_num_min.has_value()) {
            url.params().append({"volume_num_min", std::to_string(p.volume_num_min.value())});
        }
        if (p.volume_num_max.has_value()) {
            url.params().append({"volume_num_max", std::to_string(p.volume_num_max.value())});
        }
        if (p.related_tags.has_value()) {
            url.params().append({"related_tags", p.related_tags.value() ? "true" : "false"});
        }
        if (p.include_tag.has_value()) {
            url.params().append({"include_tag", p.include_tag.value() ? "true" : "false"});
        }

        std::string query_string = static_cast<std::string>(url.encoded_query());

        return query_string;
    }

    std::string GammaClient::build_event_query_string(const EventParameters& p) {
        boost::urls::url url;

        // --- Pagination (Required) ---
        url.params().append({"limit", std::to_string(p.limit)});

        // --- Strings (!empty checks) ---
        if (!p.order.empty()) url.params().append({"order", p.order});
        if (!p.after_cursor.empty()) url.params().append({"after_cursor", p.after_cursor});
        if (!p.title_search.empty()) url.params().append({"title_search", p.title_search});
        if (!p.start_date_min.empty()) url.params().append({"start_date_min", p.start_date_min});
        if (!p.start_date_max.empty()) url.params().append({"start_date_max", p.start_date_max});
        if (!p.end_date_min.empty()) url.params().append({"end_date_min", p.end_date_min});
        if (!p.end_date_max.empty()) url.params().append({"end_date_max", p.end_date_max});
        if (!p.start_time_min.empty()) url.params().append({"start_time_min", p.start_time_min});
        if (!p.start_time_max.empty()) url.params().append({"start_time_max", p.start_time_max});
        if (!p.event_date.empty()) url.params().append({"event_date", p.event_date});
        if (!p.tag_slug.empty()) url.params().append({"tag_slug", p.tag_slug});
        if (!p.tag_match.empty()) url.params().append({"tag_match", p.tag_match});
        if (!p.recurrence.empty()) url.params().append({"recurrence", p.recurrence});
        if (!p.partner_slug.empty()) url.params().append({"partner_slug", p.partner_slug});
        if (!p.locale.empty()) url.params().append({"locale", p.locale});

        // --- Vectors (Joined by Commas) ---
        // Note: Ensure your `join_vector` helper is a template or is overloaded
        // to handle both std::vector<int> and std::vector<std::string>.
        if (!p.id.empty()) url.params().append({"id", join_vector(p.id)});
        if (!p.slug.empty()) url.params().append({"slug", join_vector(p.slug)});
        if (!p.tag_id.empty()) url.params().append({"tag_id", join_vector(p.tag_id)});
        if (!p.exclude_tag_id.empty()) url.params().append({"exclude_tag_id", join_vector(p.exclude_tag_id)});
        if (!p.series_id.empty()) url.params().append({"series_id", join_vector(p.series_id)});
        if (!p.game_id.empty()) url.params().append({"game_id", join_vector(p.game_id)});
        if (!p.created_by.empty()) url.params().append({"created_by", join_vector(p.created_by)});

        // --- Optionals (Bools) ---
        if (p.ascending.has_value()) url.params().append({"ascending", p.ascending.value() ? "true" : "false"});
        if (p.closed.has_value()) url.params().append({"closed", p.closed.value() ? "true" : "false"});
        if (p.live.has_value()) url.params().append({"live", p.live.value() ? "true" : "false"});
        if (p.featured.has_value()) url.params().append({"featured", p.featured.value() ? "true" : "false"});
        if (p.cyom.has_value()) url.params().append({"cyom", p.cyom.value() ? "true" : "false"});
        if (p.related_tags.has_value()) url.params().append({"related_tags", p.related_tags.value() ? "true" : "false"});
        if (p.featured_order.has_value()) url.params().append({"featured_order", p.featured_order.value() ? "true" : "false"});
        if (p.include_children.has_value()) url.params().append({"include_children", p.include_children.value() ? "true" : "false"});
        if (p.include_chat.has_value()) url.params().append({"include_chat", p.include_chat.value() ? "true" : "false"});
        if (p.include_template.has_value()) url.params().append({"include_template", p.include_template.value() ? "true" : "false"});
        if (p.include_best_lines.has_value()) url.params().append({"include_best_lines", p.include_best_lines.value() ? "true" : "false"});

        // --- Optionals (Numbers) ---
        if (p.liquidity_min.has_value()) url.params().append({"liquidity_min", std::to_string(p.liquidity_min.value())});
        if (p.liquidity_max.has_value()) url.params().append({"liquidity_max", std::to_string(p.liquidity_max.value())});
        if (p.volume_min.has_value()) url.params().append({"volume_min", std::to_string(p.volume_min.value())});
        if (p.volume_max.has_value()) url.params().append({"volume_max", std::to_string(p.volume_max.value())});
        if (p.event_week.has_value()) url.params().append({"event_week", std::to_string(p.event_week.value())});
        if (p.parent_event_id.has_value()) url.params().append({"parent_event_id", std::to_string(p.parent_event_id.value())});

        std::string query_string = static_cast<std::string>(url.encoded_query());

        return query_string;
    }

    std::vector<Event> GammaClient::fetch_events(const EventParameters &params, const std::string &cursor) {
        std::vector<Event> events;
        std::string path = std::format("/events/keyset?{}",
            build_event_query_string(params));

        if (!cursor.empty()) {
            path += "&after_cursor=" + cursor;
        }

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + response.error);

        return parse_events_response(response.body).events;
    }

    std::vector<Event> GammaClient::fetch_all_events(const EventParameters &params) {
        std::vector<Event> events;
        std::string cursor = "";

        while (true) {
            std::string path = std::format("/events/keyset?{}",
                               build_event_query_string(params));

            if (!cursor.empty()) {
                path += "&after_cursor=" + cursor;
            }

            auto response = http_.get(path);
            if (!response.ok())
                throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

            auto parsed = parse_events_response(response.body);
            if (parsed.events.empty())
                break;

            std::cout << "Downloaded " << parsed.events.size() << " markets" << " Total: " << events.size() << std::endl;

            for (auto const& event : parsed.events) {
                events.push_back(std::move(event));
            }

            if (!parsed.next_cursor.empty()) {
                cursor = parsed.next_cursor;
            } else {
                break;
            }
        }

        return events;
    }

    Event GammaClient::fetch_event_by_id(int id) {
        std::string path = "/events/" + std::to_string(id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<Event>(json::parse(response.body));
    }

    Event GammaClient::fetch_event_by_slug(const std::string &slug) {
        std::string path = "/events/slug/" + slug;

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<Event>(json::parse(response.body));
    }

    std::vector<Tag> GammaClient::fetch_events_tags(int id) {
        const std::string path = std::format("/events/{}/tags", id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<std::vector<Tag>>(json::parse(response.body));
    }

    std::vector<Market> GammaClient::fetch_markets(const MarketParameters &params, const std::string &cursor) {
        std::vector<Market> markets;

        std::string path = std::format("/markets/keyset?{}",
                               build_market_query_string(params));

        if (!cursor.empty()) {
            path += "&after_cursor=" + cursor;
        }

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return parse_markets_response(response.body).markets;
    }

    std::vector<Market> GammaClient::fetch_all_markets(const MarketParameters &params) {
        std::vector<Market> markets;
        std::string cursor = "";

        while (true) {
            std::string path = std::format("/markets/keyset?{}",
                               build_market_query_string(params));

            if (!cursor.empty()) {
                path += "&after_cursor=" + cursor;
            }

            auto response = http_.get(path);
            if (!response.ok())
                throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

            auto parsed = parse_markets_response(response.body);
            if (parsed.markets.empty())
                break;

            std::cout << "Downloaded " << parsed.markets.size() << " markets" << " Total: " << markets.size() << std::endl;

            for (auto const& market : parsed.markets) {
                markets.push_back(std::move(market));
            }

            if (!parsed.next_cursor.empty()) {
                cursor = parsed.next_cursor;
            } else {
                break;
            }
        }

        return markets;
    }

    Market GammaClient::fetch_market_by_id(const std::string &id) {
        std::string path = "/markets/" + id;

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return parse_market_response(response.body);
    }

    Market GammaClient::fetch_market_by_slug(const std::string &slug) {
        std::string path = "/markets/slug/" + slug;

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return parse_market_response(response.body);
    }

    std::vector<MarketHolders> GammaClient::fetch_market_top_holders(const std::vector<std::string> &market, int limit, int min_balance) {
        http_.set_base_url(config_.data_api_url);

        const std::string path = std::format("/holders?limit={}&minBalance={}&market={}",
                               limit, min_balance, join_vector(market));

        const auto response = http_.get(path);
        if (!response.ok()) {
            http_.set_base_url(config_.gamma_api_url);
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);
        }

        http_.set_base_url(config_.gamma_api_url);
        return parse_market_holders_response(response.body);
    }

    std::vector<MarketOpenInterest> GammaClient::fetch_market_open_interests(const std::vector<std::string> &market) {
        http_.set_base_url(config_.data_api_url);
        const std::string path = std::format("/oi?market={}", join_vector(market));

        std::cout << market[0] << std::endl;
        std::cout << path << std::endl;

        const auto response = http_.get(path);
        if (!response.ok()) {
            http_.set_base_url(config_.gamma_api_url);
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);
        }

        http_.set_base_url(config_.gamma_api_url);
        return parse_market_open_interests_response(response.body);
    }

    std::vector<EventVolume> GammaClient::fetch_event_live_volumes(int event) {
        http_.set_base_url(config_.data_api_url);
        const std::string path = std::format("/live-volume?id={}", event);

        const auto response = http_.get(path);
        if (!response.ok()) {
            http_.set_base_url(config_.gamma_api_url);
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);
        }

        http_.set_base_url(config_.gamma_api_url);
        return parse_event_live_volumes_response(response.body);
    }

    std::vector<Tag> GammaClient::fetch_market_tags_by_id(const std::string &id) {
        const std::string path = std::format("/markets/{}/tags", id);

        const auto response = http_.get(path);
        if (!response.ok())
            throw std::runtime_error("HTTP request failed: " + std::to_string(response.status) + " - " + response.error);

        return json::value_to<std::vector<Tag>>(json::parse(response.body));
    }

    MarketsResponse GammaClient::parse_markets_response(const std::string &json_response) {
        auto jv = json::parse(json_response);

        MarketsResponse out;
        if (jv.is_array()) {
            out.markets = json::value_to<std::vector<Market>>(jv);
        } else {
            out = json::value_to<MarketsResponse>(jv);
        }

        return out;
    }

    EventsResponse GammaClient::parse_events_response(const std::string &json_response) {
        auto jv = json::parse(json_response);

        EventsResponse out;
        if (jv.is_array()) {
            out.events = json::value_to<std::vector<Event>>(jv);
        } else {
            out = json::value_to<EventsResponse>(jv);
        }

        return out;
    }

    Market GammaClient::parse_market_response(const std::string &json_response) {
        return json::value_to<Market>(json::parse(json_response));
    }

    std::vector<MarketHolders> GammaClient::parse_market_holders_response(const std::string &json_response) {
        return json::value_to<std::vector<MarketHolders>>(json::parse(json_response));
    }

    std::vector<MarketOpenInterest> GammaClient::parse_market_open_interests_response(const std::string &json_response) {
        return json::value_to<std::vector<MarketOpenInterest>>(json::parse(json_response));
    }

    std::vector<EventVolume> GammaClient::parse_event_live_volumes_response(const std::string &json_response) {
        return json::value_to<std::vector<EventVolume>>(json::parse(json_response));
    }
}
