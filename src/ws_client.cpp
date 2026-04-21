//
// Created by Lorenzo P on 4/19/26.
//

#include "ws_client.h"

#include <iostream>

namespace polymarket {
    void fail(beast::error_code ec, char const* what) {
        std::cerr << what << ": " << ec.message() << std::endl;
    }


    WsClient::WsClient(net::io_context& io_context, ssl::context& ssl_context)
        : resolver_(net::make_strand(io_context)), ws_(net::make_strand(io_context), ssl_context) {}

    void WsClient::run(char const* host, char const* port) {
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
            beast::error_code ec{
                static_cast<int>(::ERR_get_error()),
                net::error::get_ssl_category()};

            std::cerr << "Startup Error: " << ec.message() << std::endl;
            return;
        }

        ws_.next_layer().set_verify_callback(ssl::host_name_verification(host));

        host_ = host;

        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(&WsClient::on_resolve, shared_from_this()));
    }

    void WsClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec)
            return fail(ec, "Resolve");

        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &WsClient::on_connect,
                shared_from_this()));
    }

    void WsClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {}

    void WsClient::on_ssl_handshake(beast::error_code ec) {}

    void WsClient::on_handshake(beast::error_code ec) {}

    void WsClient::on_write(beast::error_code ec, std::size_t bytes_transferred) {}

    void WsClient::on_read(beast::error_code ec, std::size_t bytes_transferred) {}

    void WsClient::on_close(beast::error_code ec) {}
}
