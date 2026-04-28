//
// Created by Lorenzo P on 4/19/26.
//

#include "ws_client.h"

#include <chrono>
#include <stdexcept>
#include <utility>

#include <boost/json.hpp>
#include <boost/url/parse.hpp>
#include <boost/url/url_view.hpp>
#include <openssl/err.h>

namespace json = boost::json;
namespace urls = boost::urls;

namespace polymarket::ws {

    namespace {
        std::string string_or_serialized(const json::value& v) {
            if (v.is_string()) return std::string(v.as_string());
            if (v.is_null()) return {};
            return json::serialize(v);
        }

        std::string get_string(const json::object& obj, std::string_view key) {
            if (const auto* v = obj.if_contains(key); v && !v->is_null()) {
                return string_or_serialized(*v);
            }
            return {};
        }

        std::optional<std::string> get_optional_string(const json::object& obj, std::string_view key) {
            if (const auto* v = obj.if_contains(key); v && !v->is_null()) {
                return string_or_serialized(*v);
            }
            return std::nullopt;
        }

        std::optional<bool> get_optional_bool(const json::object& obj, std::string_view key) {
            if (const auto* v = obj.if_contains(key); v && v->is_bool()) {
                return v->as_bool();
            }
            return std::nullopt;
        }

        std::vector<std::string> get_string_array(const json::object& obj, std::string_view key) {
            std::vector<std::string> out;
            const auto* v = obj.if_contains(key);
            if (!v || !v->is_array()) return out;

            for (const auto& item : v->as_array()) {
                if (!item.is_null()) out.push_back(string_or_serialized(item));
            }
            return out;
        }

        std::optional<std::vector<std::string>>
        get_optional_string_array(const json::object& obj, std::string_view key) {
            const auto* v = obj.if_contains(key);
            if (!v || !v->is_array()) return std::nullopt;

            std::vector<std::string> out;
            for (const auto& item : v->as_array()) {
                if (!item.is_null()) out.push_back(string_or_serialized(item));
            }
            return out;
        }

        std::vector<OrderSummary> get_order_summaries(const json::object& obj, std::string_view key) {
            std::vector<OrderSummary> out;
            const auto* v = obj.if_contains(key);
            if (!v || !v->is_array()) return out;

            for (const auto& item : v->as_array()) {
                const auto* item_obj = item.if_object();
                if (!item_obj) continue;
                out.push_back(OrderSummary{
                    .price = get_string(*item_obj, "price"),
                    .size = get_string(*item_obj, "size"),
                });
            }
            return out;
        }

        std::vector<PriceChangeMessage> get_price_changes(const json::object& obj) {
            std::vector<PriceChangeMessage> out;
            const auto* v = obj.if_contains("price_changes");
            if (!v || !v->is_array()) return out;

            for (const auto& item : v->as_array()) {
                const auto* item_obj = item.if_object();
                if (!item_obj) continue;
                out.push_back(PriceChangeMessage{
                    .asset_id = get_string(*item_obj, "asset_id"),
                    .price = get_string(*item_obj, "price"),
                    .size = get_string(*item_obj, "size"),
                    .side = get_string(*item_obj, "side"),
                    .hash = get_string(*item_obj, "hash"),
                    .best_bid = get_optional_string(*item_obj, "best_bid"),
                    .best_ask = get_optional_string(*item_obj, "best_ask"),
                });
            }
            return out;
        }

        std::optional<EventMessage> get_event_message(const json::object& obj) {
            const auto* v = obj.if_contains("event_message");
            if (!v || !v->is_object()) return std::nullopt;

            const auto& event_obj = v->as_object();
            return EventMessage{
                .id = get_optional_string(event_obj, "id"),
                .ticker = get_optional_string(event_obj, "ticker"),
                .slug = get_optional_string(event_obj, "slug"),
                .title = get_optional_string(event_obj, "title"),
                .description = get_optional_string(event_obj, "description"),
            };
        }

        UnknownEvent unknown(std::string_view raw, std::string error, std::string event_type = {}) {
            return UnknownEvent{
                .event_type = std::move(event_type),
                .raw = std::string(raw),
                .error = std::move(error),
            };
        }

        MarketEvent parse_market_event_value(const json::value& parsed, std::string_view raw) {
            const auto* obj = parsed.if_object();
            if (!obj) return unknown(raw, "WebSocket message is not a JSON object");

            const std::string event_type = get_string(*obj, "event_type");
            if (event_type == "book") {
                return BookEvent{
                    .event_type = event_type,
                    .asset_id = get_string(*obj, "asset_id"),
                    .market = get_string(*obj, "market"),
                    .bids = get_order_summaries(*obj, "bids"),
                    .asks = get_order_summaries(*obj, "asks"),
                    .timestamp = get_string(*obj, "timestamp"),
                    .hash = get_string(*obj, "hash"),
                };
            }
            if (event_type == "price_change") {
                return PriceChangeEvent{
                    .event_type = event_type,
                    .market = get_string(*obj, "market"),
                    .price_changes = get_price_changes(*obj),
                    .timestamp = get_string(*obj, "timestamp"),
                };
            }
            if (event_type == "last_trade_price") {
                return LastTradePriceEvent{
                    .event_type = event_type,
                    .asset_id = get_string(*obj, "asset_id"),
                    .market = get_string(*obj, "market"),
                    .price = get_string(*obj, "price"),
                    .size = get_string(*obj, "size"),
                    .fee_rate_bps = get_optional_string(*obj, "fee_rate_bps"),
                    .side = get_string(*obj, "side"),
                    .timestamp = get_string(*obj, "timestamp"),
                    .transaction_hash = get_optional_string(*obj, "transaction_hash"),
                };
            }
            if (event_type == "tick_size_change") {
                return TickSizeChangeEvent{
                    .event_type = event_type,
                    .asset_id = get_string(*obj, "asset_id"),
                    .market = get_string(*obj, "market"),
                    .old_tick_size = get_string(*obj, "old_tick_size"),
                    .new_tick_size = get_string(*obj, "new_tick_size"),
                    .timestamp = get_string(*obj, "timestamp"),
                };
            }
            if (event_type == "best_bid_ask") {
                return BestBidAskEvent{
                    .event_type = event_type,
                    .asset_id = get_string(*obj, "asset_id"),
                    .market = get_string(*obj, "market"),
                    .best_bid = get_string(*obj, "best_bid"),
                    .best_ask = get_string(*obj, "best_ask"),
                    .spread = get_string(*obj, "spread"),
                    .timestamp = get_string(*obj, "timestamp"),
                };
            }
            if (event_type == "new_market") {
                return NewMarketEvent{
                    .event_type = event_type,
                    .id = get_string(*obj, "id"),
                    .question = get_string(*obj, "question"),
                    .market = get_string(*obj, "market"),
                    .slug = get_string(*obj, "slug"),
                    .description = get_optional_string(*obj, "description"),
                    .assets_ids = get_string_array(*obj, "assets_ids"),
                    .outcomes = get_string_array(*obj, "outcomes"),
                    .event_message = get_event_message(*obj),
                    .timestamp = get_string(*obj, "timestamp"),
                    .tags = get_optional_string_array(*obj, "tags"),
                    .condition_id = get_optional_string(*obj, "condition_id"),
                    .active = get_optional_bool(*obj, "active"),
                    .clob_token_ids = get_optional_string_array(*obj, "clob_token_ids"),
                    .sports_market_type = get_optional_string(*obj, "sports_market_type"),
                    .line = get_optional_string(*obj, "line"),
                    .game_start_time = get_optional_string(*obj, "game_start_time"),
                    .order_price_min_tick_size = get_optional_string(*obj, "order_price_min_tick_size"),
                    .group_item_title = get_optional_string(*obj, "group_item_title"),
                };
            }
            if (event_type == "market_resolved") {
                return MarketResolvedEvent{
                    .event_type = event_type,
                    .id = get_string(*obj, "id"),
                    .market = get_string(*obj, "market"),
                    .assets_ids = get_string_array(*obj, "assets_ids"),
                    .winning_asset_id = get_string(*obj, "winning_asset_id"),
                    .winning_outcome = get_string(*obj, "winning_outcome"),
                    .event_message = get_event_message(*obj),
                    .timestamp = get_string(*obj, "timestamp"),
                    .tags = get_optional_string_array(*obj, "tags"),
                };
            }

            return unknown(raw, "Unknown market event_type", event_type);
        }

        void put_optional(json::object& obj, std::string_view key, const std::optional<bool>& value) {
            if (value) obj[key] = *value;
        }

        void put_optional(json::object& obj, std::string_view key, const std::optional<int>& value) {
            if (value) obj[key] = *value;
        }
    }

    void tag_invoke(json::value_from_tag, json::value& jv,
                    const MarketSubscriptionRequest& request) {
        json::object obj;
        obj["assets_ids"] = json::value_from(request.assets_ids);
        obj["type"] = request.type;
        put_optional(obj, "initial_dump", request.initial_dump);
        put_optional(obj, "level", request.level);
        put_optional(obj, "custom_feature_enabled", request.custom_feature_enabled);
        jv = std::move(obj);
    }

    void tag_invoke(json::value_from_tag, json::value& jv,
                    const MarketSubscriptionUpdate& update) {
        json::object obj;
        obj["operation"] = to_string(update.operation);
        obj["assets_ids"] = json::value_from(update.assets_ids);
        put_optional(obj, "level", update.level);
        put_optional(obj, "custom_feature_enabled", update.custom_feature_enabled);
        jv = std::move(obj);
    }

    std::string serialize_subscription_request(const MarketSubscriptionRequest& request) {
        return json::serialize(json::value_from(request));
    }

    std::string serialize_subscription_update(const MarketSubscriptionUpdate& update) {
        return json::serialize(json::value_from(update));
    }

    MarketEvent parse_market_event(std::string_view message) {
        if (message == "PONG") return PongEvent{};

        try {
            const auto parsed = json::parse(message);
            return parse_market_event_value(parsed, message);
        } catch (const std::exception& e) {
            return unknown(message, e.what());
        }
    }

    std::vector<MarketEvent> parse_market_events(std::string_view message) {
        if (message == "PONG") return {PongEvent{}};

        try {
            const auto parsed = json::parse(message);
            if (const auto* arr = parsed.if_array()) {
                std::vector<MarketEvent> events;
                events.reserve(arr->size());
                for (const auto& item : *arr) {
                    events.push_back(parse_market_event_value(item, json::serialize(item)));
                }
                return events;
            }
            return {parse_market_event_value(parsed, message)};
        } catch (const std::exception& e) {
            return {unknown(message, e.what())};
        }
    }

    WsClient::WsClient(net::io_context& io_context, ssl::context& ssl_context)
        : resolver_(net::make_strand(io_context)),
          ws_(net::make_strand(io_context), ssl_context),
          heartbeat_timer_(ws_.get_executor()) {}

    void WsClient::connect(const std::string& url) {
        net::dispatch(ws_.get_executor(),
                      [self = shared_from_this(), url] { self->do_connect(url); });
    }

    void WsClient::send_text(std::string message) {
        net::post(ws_.get_executor(),
                  [self = shared_from_this(), message = std::move(message)]() mutable {
                      self->write_queue_.push_back(std::move(message));
                      self->do_write();
                  });
    }

    void WsClient::close(websocket::close_reason reason) {
        net::post(ws_.get_executor(), [self = shared_from_this(), reason] {
            if (self->closing_) return;
            self->closing_ = true;
            self->do_stop_heartbeat();

            if (!self->connected_) {
                if (self->close_handler_) self->close_handler_({});
                return;
            }

            self->ws_.async_close(
                reason,
                beast::bind_front_handler(&WsClient::on_close, self));
        });
    }

    void WsClient::start_heartbeat(std::chrono::milliseconds interval, std::string message) {
        net::post(ws_.get_executor(),
                  [self = shared_from_this(), interval, message = std::move(message)]() mutable {
                      self->do_stop_heartbeat();
                      self->heartbeat_interval_ = interval;
                      self->heartbeat_message_ = std::move(message);
                      self->heartbeat_enabled_ = true;
                      self->schedule_heartbeat();
                  });
    }

    void WsClient::stop_heartbeat() {
        net::post(ws_.get_executor(), [self = shared_from_this()] {
            self->do_stop_heartbeat();
        });
    }

    void WsClient::do_stop_heartbeat() {
        heartbeat_enabled_ = false;
        heartbeat_timer_.cancel();
    }

    void WsClient::do_connect(std::string url) {
        const auto parsed = urls::parse_uri(url);
        if (!parsed) {
            notify_error(parsed.error(), "parse websocket url");
            return;
        }

        const urls::url_view view = *parsed;
        if (view.scheme() != "wss") {
            notify_error(net::error::operation_not_supported, "only wss websocket URLs are supported");
            return;
        }

        host_ = view.host();
        port_ = view.has_port() ? std::string(view.port()) : "443";
        target_ = view.encoded_target().empty() ? "/" : std::string(view.encoded_target());
        handshake_host_ = host_;
        if (view.has_port()) handshake_host_ += ":" + port_;

        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            beast::error_code ec{
                static_cast<int>(::ERR_get_error()),
                net::error::get_ssl_category()};
            notify_error(ec, "set TLS SNI hostname");
            return;
        }
        ws_.next_layer().set_verify_callback(ssl::host_name_verification(host_));

        resolver_.async_resolve(
            host_,
            port_,
            beast::bind_front_handler(&WsClient::on_resolve, shared_from_this()));
    }

    void WsClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec) return notify_error(ec, "resolve");

        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(&WsClient::on_connect, shared_from_this()));
    }

    void WsClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if (ec) return notify_error(ec, "connect");

        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(&WsClient::on_ssl_handshake, shared_from_this()));
    }

    void WsClient::on_ssl_handshake(beast::error_code ec) {
        if (ec) return notify_error(ec, "ssl handshake");

        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(boost::beast::http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) + " polymarket-ws-client");
            }));

        ws_.async_handshake(
            handshake_host_,
            target_,
            beast::bind_front_handler(&WsClient::on_handshake, shared_from_this()));
    }

    void WsClient::on_handshake(beast::error_code ec) {
        if (ec) return notify_error(ec, "websocket handshake");

        connected_ = true;
        if (open_handler_) open_handler_();
        do_write();
        do_read();
    }

    void WsClient::do_read() {
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(&WsClient::on_read, shared_from_this()));
    }

    void WsClient::on_read(beast::error_code ec, std::size_t) {
        if (ec == websocket::error::closed) {
            connected_ = false;
            if (close_handler_) close_handler_(ec);
            return;
        }
        if (ec) return notify_error(ec, "read");

        const std::string message = beast::buffers_to_string(buffer_.data());
        buffer_.consume(buffer_.size());

        if (message_handler_) message_handler_(message);
        do_read();
    }

    void WsClient::do_write() {
        if (!connected_ || closing_ || writing_ || write_queue_.empty()) return;

        writing_ = true;
        ws_.text(true);
        ws_.async_write(
            net::buffer(write_queue_.front()),
            beast::bind_front_handler(&WsClient::on_write, shared_from_this()));
    }

    void WsClient::on_write(beast::error_code ec, std::size_t) {
        if (ec) {
            writing_ = false;
            return notify_error(ec, "write");
        }

        write_queue_.pop_front();
        writing_ = false;
        do_write();
    }

    void WsClient::on_close(beast::error_code ec) {
        connected_ = false;
        closing_ = false;
        do_stop_heartbeat();
        if (ec) notify_error(ec, "close");
        if (close_handler_) close_handler_(ec);
    }

    void WsClient::schedule_heartbeat() {
        if (!heartbeat_enabled_) return;

        heartbeat_timer_.expires_after(heartbeat_interval_);
        heartbeat_timer_.async_wait([self = shared_from_this()](beast::error_code ec) {
            if (ec || !self->heartbeat_enabled_) return;
            self->send_text(self->heartbeat_message_);
            self->schedule_heartbeat();
        });
    }

    void WsClient::notify_error(beast::error_code ec, std::string_view where) {
        if (error_handler_) error_handler_(ec, where);
    }

    MarketWsClient::MarketWsClient(net::io_context& io_context,
                                   ssl::context& ssl_context,
                                   APIConfig config)
        : config_(std::move(config)),
          transport_(std::make_shared<WsClient>(io_context, ssl_context)) {
        transport_->set_message_handler([this](std::string_view message) {
            handle_message(message);
        });
    }

    MarketWsClient::~MarketWsClient() {
        transport_->set_message_handler({});
        transport_->set_open_handler({});
        transport_->set_close_handler({});
        transport_->set_error_handler({});
        transport_->close();
    }

    void MarketWsClient::connect() {
        transport_->connect(config_.clob_ws_url);
    }

    void MarketWsClient::send_text(std::string message) {
        transport_->send_text(std::move(message));
    }

    void MarketWsClient::subscribe(const MarketSubscriptionRequest& request) {
        transport_->send_text(serialize_subscription_request(request));
    }

    void MarketWsClient::update_subscription(const MarketSubscriptionUpdate& update) {
        transport_->send_text(serialize_subscription_update(update));
    }

    void MarketWsClient::unsubscribe(const std::vector<std::string>& asset_ids) {
        update_subscription(MarketSubscriptionUpdate{
            .operation = SubscriptionOperation::Unsubscribe,
            .assets_ids = asset_ids,
        });
    }

    void MarketWsClient::ping() {
        transport_->send_text("PING");
    }

    void MarketWsClient::start_heartbeat() {
        transport_->start_heartbeat(
            std::chrono::milliseconds(config_.ws_ping_interval_ms),
            "PING");
    }

    void MarketWsClient::stop_heartbeat() {
        transport_->stop_heartbeat();
    }

    void MarketWsClient::close() {
        transport_->close();
    }

    void MarketWsClient::set_open_handler(OpenHandler handler) {
        transport_->set_open_handler(std::move(handler));
    }

    void MarketWsClient::set_close_handler(CloseHandler handler) {
        transport_->set_close_handler(std::move(handler));
    }

    void MarketWsClient::set_error_handler(ErrorHandler handler) {
        error_handler_ = std::move(handler);
        transport_->set_error_handler(error_handler_);
    }

    void MarketWsClient::handle_message(std::string_view message) {
        if (raw_message_handler_) raw_message_handler_(message);

        for (const auto& event : parse_market_events(message)) {
            if (const auto* unknown_event = std::get_if<UnknownEvent>(&event);
                unknown_event && !unknown_event->error.empty() && error_handler_) {
                error_handler_({}, unknown_event->error);
            }
            if (market_event_handler_) market_event_handler_(event);
        }
    }
}
