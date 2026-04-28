//
// Created by Lorenzo P on 4/27/26.
//

#ifndef POLYMARKET_WS_TYPES_H
#define POLYMARKET_WS_TYPES_H
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include <boost/json.hpp>

namespace polymarket::ws {

    enum class SubscriptionOperation {
        Subscribe,
        Unsubscribe,
    };

    inline std::string to_string(const SubscriptionOperation op) {
        switch (op) {
            case SubscriptionOperation::Subscribe:   return "subscribe";
            case SubscriptionOperation::Unsubscribe: return "unsubscribe";
        }
        return "subscribe";
    }

    inline SubscriptionOperation subscription_operation_from_string(std::string_view value) {
        if (value == "subscribe") return SubscriptionOperation::Subscribe;
        if (value == "unsubscribe") return SubscriptionOperation::Unsubscribe;
        return SubscriptionOperation::Subscribe;
    }

    struct MarketSubscriptionRequest {
        std::vector<std::string> assets_ids;
        std::string type = "market";
        std::optional<bool> initial_dump;
        std::optional<int> level;
        std::optional<bool> custom_feature_enabled;
    };

    struct MarketSubscriptionUpdate {
        SubscriptionOperation operation = SubscriptionOperation::Subscribe;
        std::vector<std::string> assets_ids;
        std::optional<int> level;
        std::optional<bool> custom_feature_enabled;
    };

    struct OrderSummary {
        std::string price;
        std::string size;
    };

    struct BookEvent {
        std::string event_type;
        std::string asset_id;
        std::string market;
        std::vector<OrderSummary> bids;
        std::vector<OrderSummary> asks;
        std::string timestamp;
        std::string hash;
    };

    struct PriceChangeMessage {
        std::string asset_id;
        std::string price;
        std::string size;
        std::string side;
        std::string hash;
        std::optional<std::string> best_bid;
        std::optional<std::string> best_ask;
    };

    struct PriceChangeEvent {
        std::string event_type;
        std::string market;
        std::vector<PriceChangeMessage> price_changes;
        std::string timestamp;
    };

    struct LastTradePriceEvent {
        std::string event_type;
        std::string asset_id;
        std::string market;
        std::string price;
        std::string size;
        std::optional<std::string> fee_rate_bps;
        std::string side;
        std::string timestamp;
        std::optional<std::string> transaction_hash;
    };

    struct TickSizeChangeEvent {
        std::string event_type;
        std::string asset_id;
        std::string market;
        std::string old_tick_size;
        std::string new_tick_size;
        std::string timestamp;
    };

    struct BestBidAskEvent {
        std::string event_type;
        std::string asset_id;
        std::string market;
        std::string best_bid;
        std::string best_ask;
        std::string spread;
        std::string timestamp;
    };

    struct EventMessage {
        std::optional<std::string> id;
        std::optional<std::string> ticker;
        std::optional<std::string> slug;
        std::optional<std::string> title;
        std::optional<std::string> description;
    };

    struct NewMarketEvent {
        std::string event_type;
        std::string id;
        std::string question;
        std::string market;
        std::string slug;
        std::optional<std::string> description;
        std::vector<std::string> assets_ids;
        std::vector<std::string> outcomes;
        std::optional<EventMessage> event_message;
        std::string timestamp;
        std::optional<std::vector<std::string>> tags;
        std::optional<std::string> condition_id;
        std::optional<bool> active;
        std::optional<std::vector<std::string>> clob_token_ids;
        std::optional<std::string> sports_market_type;
        std::optional<std::string> line;
        std::optional<std::string> game_start_time;
        std::optional<std::string> order_price_min_tick_size;
        std::optional<std::string> group_item_title;
    };

    struct MarketResolvedEvent {
        std::string event_type;
        std::string id;
        std::string market;
        std::vector<std::string> assets_ids;
        std::string winning_asset_id;
        std::string winning_outcome;
        std::optional<EventMessage> event_message;
        std::string timestamp;
        std::optional<std::vector<std::string>> tags;
    };

    struct PongEvent {
        std::string message = "PONG";
    };

    struct UnknownEvent {
        std::string event_type;
        std::string raw;
        std::string error;
    };

    using MarketEvent = std::variant<BookEvent, PriceChangeEvent, LastTradePriceEvent,
                                     TickSizeChangeEvent, BestBidAskEvent, NewMarketEvent,
                                     MarketResolvedEvent, PongEvent, UnknownEvent>;

    std::string serialize_subscription_request(const MarketSubscriptionRequest& request);
    std::string serialize_subscription_update(const MarketSubscriptionUpdate& update);
    std::string market_event_type(const MarketEvent& event);
    MarketEvent parse_market_event(std::string_view message);
    std::vector<MarketEvent> parse_market_events(std::string_view message);

    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv,
                    const MarketSubscriptionRequest& request);
    void tag_invoke(boost::json::value_from_tag, boost::json::value& jv,
                    const MarketSubscriptionUpdate& update);
}

#endif //POLYMARKET_WS_TYPES_H
