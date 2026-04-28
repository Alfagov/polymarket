//
// Created by Lorenzo P on 4/19/26.
//

#ifndef POLYMARKET_WS_CLIENT_H
#define POLYMARKET_WS_CLIENT_H
#pragma once

#include "types.h"
#include "ws_types.h"

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <string_view>

#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>

namespace polymarket::ws {

    namespace beast = boost::beast;
    namespace websocket = beast::websocket;
    namespace net = boost::asio;
    namespace ssl = boost::asio::ssl;
    using tcp = boost::asio::ip::tcp;

    using RawMessageHandler = std::function<void(std::string_view)>;
    using MarketEventHandler = std::function<void(const MarketEvent&)>;
    using OpenHandler = std::function<void()>;
    using CloseHandler = std::function<void(beast::error_code)>;
    using ErrorHandler = std::function<void(beast::error_code, std::string_view)>;

    class WsClient : public std::enable_shared_from_this<WsClient> {
    public:
        explicit WsClient(net::io_context& io_context, ssl::context& ssl_context);

        WsClient(const WsClient&) = delete;
        WsClient& operator=(const WsClient&) = delete;

        void connect(const std::string& url);
        void send_text(std::string message);
        void close(websocket::close_reason reason = websocket::close_code::normal);
        void start_heartbeat(std::chrono::milliseconds interval, std::string message);
        void stop_heartbeat();

        bool is_open() const { return connected_; }

        void set_message_handler(RawMessageHandler handler) { message_handler_ = std::move(handler); }
        void set_open_handler(OpenHandler handler) { open_handler_ = std::move(handler); }
        void set_close_handler(CloseHandler handler) { close_handler_ = std::move(handler); }
        void set_error_handler(ErrorHandler handler) { error_handler_ = std::move(handler); }

    private:
        void do_connect(std::string url);
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
        void on_ssl_handshake(beast::error_code ec);
        void on_handshake(beast::error_code ec);
        void do_read();
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void do_write();
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
        void on_close(beast::error_code ec);
        void schedule_heartbeat();
        void do_stop_heartbeat();
        void notify_error(beast::error_code ec, std::string_view where);

        tcp::resolver resolver_;
        websocket::stream<ssl::stream<beast::tcp_stream>> ws_;
        net::steady_timer heartbeat_timer_;
        beast::flat_buffer buffer_;
        std::deque<std::string> write_queue_;

        std::string host_;
        std::string port_;
        std::string target_;
        std::string handshake_host_;

        bool connected_ = false;
        bool closing_ = false;
        bool writing_ = false;
        bool heartbeat_enabled_ = false;
        std::chrono::milliseconds heartbeat_interval_{10000};
        std::string heartbeat_message_ = "PING";

        RawMessageHandler message_handler_;
        OpenHandler open_handler_;
        CloseHandler close_handler_;
        ErrorHandler error_handler_;
    };

    class MarketWsClient {
    public:
        explicit MarketWsClient(net::io_context& io_context,
                                ssl::context& ssl_context,
                                APIConfig config = {});
        ~MarketWsClient();

        MarketWsClient(const MarketWsClient&) = delete;
        MarketWsClient& operator=(const MarketWsClient&) = delete;

        void connect();
        void send_text(std::string message);
        void subscribe(const MarketSubscriptionRequest& request);
        void update_subscription(const MarketSubscriptionUpdate& update);
        void unsubscribe(const std::vector<std::string>& asset_ids);
        void ping();
        void start_heartbeat();
        void stop_heartbeat();
        void close();

        void set_raw_message_handler(RawMessageHandler handler) { raw_message_handler_ = std::move(handler); }
        void set_market_event_handler(MarketEventHandler handler) { market_event_handler_ = std::move(handler); }
        void set_open_handler(OpenHandler handler);
        void set_close_handler(CloseHandler handler);
        void set_error_handler(ErrorHandler handler);

    private:
        void handle_message(std::string_view message);

        APIConfig config_;
        std::shared_ptr<WsClient> transport_;
        RawMessageHandler raw_message_handler_;
        MarketEventHandler market_event_handler_;
        ErrorHandler error_handler_;
    };
}

namespace polymarket {
    using WsClient = ws::WsClient;
}

#endif //POLYMARKET_WS_CLIENT_H
