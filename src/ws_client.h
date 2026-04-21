//
// Created by Lorenzo P on 4/19/26.
//

#ifndef POLYMARKET_WS_CLIENT_H
#define POLYMARKET_WS_CLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;


namespace polymarket {
    class WsClient : public std::enable_shared_from_this<WsClient> {
        tcp::resolver resolver_;
        websocket::stream<ssl::stream<beast::tcp_stream>> ws_;
        beast::flat_buffer buffer_;
        std::string host_;
        std::string port_;

    public:
        explicit WsClient(net::io_context& io_context, ssl::context& ssl_context);

        void run(char const* host, char const* port);
        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
        void on_ssl_handshake(beast::error_code ec);
        void on_handshake(beast::error_code ec);
        void on_write(beast::error_code ec, std::size_t bytes_transferred);
        void on_read(beast::error_code ec, std::size_t bytes_transferred);
        void on_close(beast::error_code ec);
    };
}


#endif //POLYMARKET_WS_CLIENT_H
