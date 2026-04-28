#include <cstdlib>
#include <iostream>
#include <string>
#include <variant>

#include <boost/asio/signal_set.hpp>

#include "src/types.h"
#include "src/ws_client.h"

namespace {
    constexpr const char* kSampleTokenId =
        "43116321876789348651560631278691064226660030712941566518175748399907477755169";

    template <class... Ts>
    struct Overloaded : Ts... {
        using Ts::operator()...;
    };
    template <class... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;

    std::string event_name(const polymarket::ws::MarketEvent& event) {
        using namespace polymarket::ws;
        return std::visit(Overloaded{
            [](const BookEvent&) { return std::string("book"); },
            [](const PriceChangeEvent&) { return std::string("price_change"); },
            [](const LastTradePriceEvent&) { return std::string("last_trade_price"); },
            [](const TickSizeChangeEvent&) { return std::string("tick_size_change"); },
            [](const BestBidAskEvent&) { return std::string("best_bid_ask"); },
            [](const NewMarketEvent&) { return std::string("new_market"); },
            [](const MarketResolvedEvent&) { return std::string("market_resolved"); },
            [](const PongEvent&) { return std::string("PONG"); },
            [](const UnknownEvent& ev) {
                return ev.event_type.empty() ? std::string("unknown") : ev.event_type;
            },
        }, event);
    }
}

int main(int argc, char** argv) {
    const std::string token_id =
        argc > 1 ? argv[1] :
        (std::getenv("POLYMARKET_TOKEN_ID") ? std::getenv("POLYMARKET_TOKEN_ID") : kSampleTokenId);

    polymarket::APIConfig config;

    boost::asio::io_context io_context;
    boost::asio::ssl::context ssl_context{boost::asio::ssl::context::tls_client};
    ssl_context.set_default_verify_paths();
    ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);

    polymarket::ws::MarketWsClient client{io_context, ssl_context, config};

    client.set_error_handler([](boost::beast::error_code ec, std::string_view where) {
        std::cerr << "[ws:error] " << where;
        if (ec) std::cerr << ": " << ec.message();
        std::cerr << std::endl;
    });

    client.set_open_handler([&client, &token_id] {
        std::cout << "[ws:open] subscribing to token " << token_id << std::endl;
        client.subscribe(polymarket::ws::MarketSubscriptionRequest{
            .assets_ids = {token_id},
            .initial_dump = true,
            .level = 2,
            .custom_feature_enabled = true,
        });
        client.start_heartbeat();
    });

    client.set_close_handler([](boost::beast::error_code ec) {
        std::cout << "[ws:close]";
        if (ec) std::cout << " " << ec.message();
        std::cout << std::endl;
    });

    client.set_raw_message_handler([](std::string_view message) {
        std::cout << "[raw] " << message << std::endl;
    });

    client.set_market_event_handler([](const polymarket::ws::MarketEvent& event) {
        std::cout << "[typed] " << event_name(event) << std::endl;
    });

    boost::asio::signal_set signals{io_context, SIGINT, SIGTERM};
    signals.async_wait([&](boost::beast::error_code, int) {
        std::cout << "\n[signal] closing websocket" << std::endl;
        client.close();
        io_context.stop();
    });

    std::cout << "Connecting to " << config.clob_ws_url << std::endl;
    std::cout << "Pass a token id as argv[1] or POLYMARKET_TOKEN_ID to override the sample." << std::endl;

    client.connect();
    io_context.run();

    return 0;
}
