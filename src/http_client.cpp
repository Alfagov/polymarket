//
// Created by Lorenzo P on 4/15/26.
//

#include <curl/curl.h>
#include "http_client.h"

namespace polymarket {
    static bool p_curl_initialized = false;

    void http_client_init() {
        if (!p_curl_initialized) {
            curl_global_init(CURL_GLOBAL_ALL);
            p_curl_initialized = true;
        }
    }

    void http_global_cleanup() {
        if (p_curl_initialized) {
            curl_global_cleanup();
            p_curl_initialized = false;
        }
    }

    HttpClient::HttpClient() :
    curl_(nullptr), headers_(nullptr), timeout_ms_(50000), dns_cache_timeout_(60),
    keepalive_timeout_(20), total_latency_ms_(0),
    total_requests_(0), reused_connections_(0),
    last_latency_ms_(0), connection_warmed_(false)
    {
        init();
    }

    HttpClient::~HttpClient() {
        cleanup();
    }

    HttpClient::HttpClient(HttpClient &&other) noexcept
    : curl_(other.curl_), headers_(other.headers_), base_url_(std::move(other.base_url_)),
    timeout_ms_(other.timeout_ms_), dns_cache_timeout_(other.dns_cache_timeout_),
    keepalive_timeout_(other.keepalive_timeout_), total_requests_(other.total_requests_),
    reused_connections_(other.reused_connections_), total_latency_ms_(other.total_latency_ms_),
    connection_warmed_(other.connection_warmed_), last_latency_ms_(other.last_latency_ms_)
    {
        other.curl_ = nullptr;
        other.headers_ = nullptr;
    }

    HttpClient &HttpClient::operator=(HttpClient &&other) noexcept {
        if (this != &other) {
            cleanup();
            curl_ = other.curl_;
            base_url_ = std::move(other.base_url_);
            timeout_ms_ = other.timeout_ms_;
            dns_cache_timeout_ = other.dns_cache_timeout_;
            keepalive_timeout_ = other.keepalive_timeout_;
            total_requests_ = other.total_requests_;
            reused_connections_ = other.reused_connections_;
            total_latency_ms_ = other.total_latency_ms_;
            last_latency_ms_ = other.last_latency_ms_;
            connection_warmed_ = other.connection_warmed_;
            other.curl_ = nullptr;
            other.headers_ = nullptr;
        }
        return *this;
    }

    void HttpClient::init() {
        curl_ = curl_easy_init();
        if (!curl_) {
            throw std::runtime_error("curl_easy_init failed");
        }

        curl_easy_setopt(curl_, CURLOPT_TCP_NODELAY, 1L);
        curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
        curl_easy_setopt(curl_, CURLOPT_TCP_KEEPIDLE, 20L);
        curl_easy_setopt(curl_, CURLOPT_TCP_KEEPINTVL, 20L);
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_, CURLOPT_MAXREDIRS, 3L);
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);

        curl_easy_setopt(curl_, CURLOPT_FORBID_REUSE, 0L);
        curl_easy_setopt(curl_, CURLOPT_FRESH_CONNECT, 0L);
        curl_easy_setopt(curl_, CURLOPT_DNS_CACHE_TIMEOUT, dns_cache_timeout_);

        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);

        set_header("Connection: keep-alive");
        set_header("Accept: application/json");
        set_header("Content-Type: application/json");
    }

    void HttpClient::cleanup() {
        if (headers_) {
            curl_slist_free_all(headers_);
            headers_ = nullptr;
        }

        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
    }

    void HttpClient::set_timeout(int timeout_ms) {
        timeout_ms_ = timeout_ms;
    }

    void HttpClient::set_base_url(const std::string& url) {
        base_url_ = url;

        if (!base_url_.empty() && base_url_.back() == '/') {
            base_url_.pop_back();
        }
    }

    void HttpClient::set_header(const std::string& header) {
        curl_slist_append(headers_, header.c_str());
    }

    void HttpClient::set_user_agent(const std::string& user_agent) {
        if (curl_) {
            curl_easy_setopt(curl_, CURLOPT_USERAGENT, user_agent.c_str());
        }
    }

    void HttpClient::set_dns_cache_timeout(int timeout_ms) {
        dns_cache_timeout_ = timeout_ms;
        if (curl_) {
            curl_easy_setopt(curl_, CURLOPT_DNS_CACHE_TIMEOUT, dns_cache_timeout_);
        }
    }

    void HttpClient::set_keepalive_timeout(int timeout_sec) {
        keepalive_timeout_ = timeout_sec;
        if (curl_) {
            curl_easy_setopt(curl_, CURLOPT_TCP_KEEPIDLE, timeout_sec);
            curl_easy_setopt(curl_, CURLOPT_TCP_KEEPINTVL, timeout_sec);
        }
    }

    size_t HttpClient::write_callback(char *ptr, size_t size, size_t nmemb, void* userdata) {
        auto *response = static_cast<std::string*>(userdata);
        size_t len = size * nmemb;
        response->append(ptr, len);
        return len;
    }

    HttpResponse HttpClient::perform_request(const std::string& url) {
        HttpResponse response;
        response.status = 0;

        auto start = std::chrono::high_resolution_clock::now();
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers_);
        curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_ms_);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.body);

        CURLcode res = curl_easy_perform(curl_);

        auto end = std::chrono::high_resolution_clock::now();

        response.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

        if (res != CURLE_OK) {
            response.error = curl_easy_strerror(res);
            return response;
        }

        curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response.status);

        {
            total_requests_++;
            last_latency_ms_ = response.elapsed_ms;
            total_latency_ms_ += response.elapsed_ms;

            uint64_t reused = 0;
            curl_easy_getinfo(curl_, CURLINFO_NUM_CONNECTS, &reused);

            if (reused == 0) {
                reused_connections_++;
            }
        }

        return response;
    }

    HttpResponse HttpClient::get(const std::string &path) {
        std::string url = base_url_.empty() ? path : base_url_ + path;

        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl_, CURLOPT_POST, 0L);

        return perform_request(url);
    }

    HttpResponse HttpClient::get(const std::string &path, const std::map<std::string, std::string> &headers) {
        curl_slist *original_headers = headers_;
        curl_slist *temp_headers = nullptr;

        for (auto h = headers_; h; h = h->next)
        {
            temp_headers = curl_slist_append(temp_headers, h->data);
        }

        for (const auto &[key, value] : headers)
        {
            std::string header = key + ": " + value;
            temp_headers = curl_slist_append(temp_headers, header.c_str());
        }
        headers_ = temp_headers;

        auto response = get(path);

        curl_slist_free_all(headers_);
        headers_ = original_headers;

        return response;
    }

    HttpResponse HttpClient::post(const std::string &path, const std::string &body) {
        std::string url = base_url_.empty() ? path : base_url_ + path;

        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));

        return perform_request(url);
    }

    HttpResponse HttpClient::post(const std::string &path, const std::string &body, const std::map<std::string, std::string> &headers) {
        curl_slist *original_headers = headers_;
        curl_slist *temp_headers = nullptr;

        for (auto h = headers_; h; h = h->next)
        {
            temp_headers = curl_slist_append(temp_headers, h->data);
        }

        for (const auto &[key, value] : headers)
        {
            std::string header = key + ": " + value;
            temp_headers = curl_slist_append(temp_headers, header.c_str());
        }
        headers_ = temp_headers;

        auto response = post(path, body);

        curl_slist_free_all(headers_);
        headers_ = original_headers;

        return response;
    }

    HttpResponse HttpClient::del(const std::string &path, const std::string &body) {
        std::string url = base_url_.empty() ? path : base_url_ + path;

        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
        if (!body.empty())
        {
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
        }

        auto response = perform_request(url);

        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, nullptr);

        return response;
    }

    HttpResponse HttpClient::del(const std::string &path, const std::string &body, const std::map<std::string, std::string> &headers) {
        curl_slist *original_headers = headers_;
        curl_slist *temp_headers = nullptr;

        for (auto h = headers_; h; h = h->next)
        {
            temp_headers = curl_slist_append(temp_headers, h->data);
        }

        for (const auto &[key, value] : headers)
        {
            std::string header = key + ": " + value;
            temp_headers = curl_slist_append(temp_headers, header.c_str());
        }
        headers_ = temp_headers;

        auto response = del(path, body);

        curl_slist_free_all(headers_);
        headers_ = original_headers;

        return response;
    }

    bool HttpClient::warm_connection() {
        if (base_url_.empty()) {
            return false;
        }

        auto response = get("/");

        if (response.ok() || response.status == 404) {
            connection_warmed_ = true;
            return true;
        }

        return false;
    }

    HttpClient::ConnectionStats HttpClient::get_connection_stats() const {
        ConnectionStats stats;
        stats.total_requests = total_requests_;
        stats.reused_connections = reused_connections_;
        stats.total_latency_ms = total_latency_ms_;
        stats.last_latency_ms = last_latency_ms_;
        stats.avg_latency_ms = total_latency_ms_ / static_cast<double>(total_requests_);
        stats.connection_warmed = connection_warmed_;

        return stats;
    }
}
