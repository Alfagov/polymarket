#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/asio/signal_set.hpp>

#include "src/types.h"
#include "src/ws_client.h"

namespace {
    constexpr const char* kSampleTokenId =
        "43116321876789348651560631278691064226660030712941566518175748399907477755169";

}

int main(int argc, char** argv) {
    const std::string token_id =
        argc > 1 ? argv[1] :
        (std::getenv("POLYMARKET_TOKEN_ID") ? std::getenv("POLYMARKET_TOKEN_ID") : kSampleTokenId);

    polymarket::APIConfig config;
    config.log_level = polymarket::LogLevel::info;

    boost::asio::io_context io_context;
    boost::asio::ssl::context ssl_context{boost::asio::ssl::context::tls_client};
    ssl_context.set_default_verify_paths();
    ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);

    polymarket::ws::MarketWsClient client{io_context, ssl_context, config};

    boost::asio::signal_set signals{io_context, SIGINT, SIGTERM};
    signals.async_wait([&](boost::beast::error_code, int) {
        std::cout << "\n[signal] closing websocket" << std::endl;
        client.close();
        io_context.stop();
    });

    std::cout << "Connecting to " << config.clob_ws_url << std::endl;
    std::cout << "Pass a token id as argv[1] or POLYMARKET_TOKEN_ID to override the sample." << std::endl;

    client.listen_markets({token_id}, polymarket::ws::ListenMarketsOptions{
        .initial_dump = true,
        .level = 2,
        .custom_feature_enabled = true,
        .heartbeat = true,
        .raw_message_handler = [](std::string_view message) {
            std::cout << "[raw] " << message << std::endl;
        },
        .market_event_handler = [](const polymarket::ws::MarketEvent& event) {
            std::cout << "[typed] " << polymarket::ws::market_event_type(event) << std::endl;
        },
        .open_handler = [&token_id] {
            std::cout << "[ws:open] subscribed to token " << token_id << std::endl;
        },
        .close_handler = [](boost::beast::error_code ec) {
            std::cout << "[ws:close]";
            if (ec) std::cout << " " << ec.message();
            std::cout << std::endl;
        },
        .error_handler = [](boost::beast::error_code ec, std::string_view where) {
            std::cerr << "[ws:error] " << where;
            if (ec) std::cerr << ": " << ec.message();
            std::cerr << std::endl;
        },
    });
    io_context.run();

    return 0;
}
