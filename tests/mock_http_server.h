#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace polymarket::test {

struct CapturedRequest {
    std::string method;
    std::string target;
    std::string body;
};

struct CannedResponse {
    int status = 200;
    std::string body;
    std::string content_type = "application/json";
};

using RequestHandler =
    std::function<CannedResponse(const CapturedRequest&)>;

class MockHttpServer {
public:
    MockHttpServer()
        : ioc_(1),
          acceptor_(ioc_,
                    boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::make_address("127.0.0.1"), 0)) {
        port_ = acceptor_.local_endpoint().port();
    }

    ~MockHttpServer() { stop(); }

    void start() {
        thread_ = std::thread([this] { accept_loop(); });
    }

    void stop() {
        if (stopped_.exchange(true)) return;
        boost::system::error_code ec;
        acceptor_.close(ec);
        ioc_.stop();
        if (thread_.joinable()) thread_.join();
    }

    std::string base_url() const {
        return "http://127.0.0.1:" + std::to_string(port_);
    }

    unsigned short port() const { return port_; }

    void set_handler(RequestHandler h) {
        std::lock_guard lk(mu_);
        handler_ = std::move(h);
    }

    void enqueue(CannedResponse r) {
        std::lock_guard lk(mu_);
        queued_.push_back(std::move(r));
    }

    std::vector<CapturedRequest> requests() const {
        std::lock_guard lk(mu_);
        return captured_;
    }

    CapturedRequest last_request() const {
        std::lock_guard lk(mu_);
        return captured_.empty() ? CapturedRequest{} : captured_.back();
    }

    std::size_t request_count() const {
        std::lock_guard lk(mu_);
        return captured_.size();
    }

    void reset() {
        std::lock_guard lk(mu_);
        captured_.clear();
        queued_.clear();
        handler_ = nullptr;
    }

private:
    void accept_loop() {
        while (!stopped_.load()) {
            boost::system::error_code ec;
            boost::asio::ip::tcp::socket sock(ioc_);
            acceptor_.accept(sock, ec);
            if (ec) return;  // likely shutdown
            try {
                handle_session(std::move(sock));
            } catch (...) {
                // swallow per-session errors so the loop keeps serving
            }
        }
    }

    void handle_session(boost::asio::ip::tcp::socket sock) {
        namespace http = boost::beast::http;
        boost::beast::flat_buffer buffer;
        http::request<http::string_body> req;
        boost::beast::error_code ec;
        http::read(sock, buffer, req, ec);
        if (ec) return;

        CapturedRequest captured;
        captured.method = std::string(req.method_string());
        captured.target = std::string(req.target());
        captured.body = req.body();

        CannedResponse resp;
        {
            std::lock_guard lk(mu_);
            captured_.push_back(captured);
            if (handler_) {
                resp = handler_(captured);
            } else if (!queued_.empty()) {
                resp = queued_.front();
                queued_.erase(queued_.begin());
            } else {
                resp.status = 500;
                resp.body = R"({"error":"no response configured"})";
            }
        }

        http::response<http::string_body> res{
            static_cast<http::status>(resp.status), req.version()};
        res.set(http::field::server, "mock");
        res.set(http::field::content_type, resp.content_type);
        res.keep_alive(false);
        res.body() = resp.body;
        res.prepare_payload();
        http::write(sock, res, ec);
        sock.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }

    boost::asio::io_context ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    unsigned short port_ = 0;
    std::thread thread_;
    std::atomic<bool> stopped_{false};

    mutable std::mutex mu_;
    std::vector<CapturedRequest> captured_;
    std::vector<CannedResponse> queued_;
    RequestHandler handler_;
};

}  // namespace polymarket::test
