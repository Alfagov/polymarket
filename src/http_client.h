//
// Created by Lorenzo P on 4/15/26.
//

#ifndef POLYMARKET_HTTP_CLIENT_H
#define POLYMARKET_HTTP_CLIENT_H

#include <map>
#include <string>
#include <thread>
#include <curl/curl.h>

namespace polymarket {
    struct HttpResponse {
        int status;
        std::string body;
        std::string error;
        double elapsed_ms;

        bool ok() const { return status >= 200 && status < 300; }
    };

    class HttpClient {
    public:
        HttpClient();  // Constructor
        ~HttpClient(); // Destructor

        // Disallow copy
        HttpClient(const HttpClient&) = delete;
        HttpClient& operator=(const HttpClient&) = delete;

        // Enable Move
        HttpClient(HttpClient &&other) noexcept;
        HttpClient& operator=(HttpClient &&other) noexcept;

        void set_timeout(int timeout_ms);
        void set_base_url(const std::string& base_url);
        void set_header(const std::string& header);
        void set_user_agent(const std::string& user_agent);
        void set_dns_cache_timeout(int timeout_sec); // DNS TTL
        void set_keepalive_timeout(int timeout_sec); // Cache TTL

        HttpResponse get(const std::string& path);
        HttpResponse get(const std::string& path, const std::map<std::string, std::string>& headers);
        HttpResponse post(const std::string& path, const std::string& body);
        HttpResponse post(const std::string& path, const std::string& body, const std::map<std::string, std::string>& headers);
        HttpResponse del(const std::string& path, const std::string& body = "");
        HttpResponse del(const std::string& path, const std::string& body, const std::map<std::string, std::string>& headers);

        bool warm_connection();

        struct ConnectionStats {
            uint64_t total_requests;
            uint64_t reused_connections;
            double total_latency_ms;
            double last_latency_ms;
            double avg_latency_ms;
            bool connection_warmed;
        };
        ConnectionStats get_connection_stats() const;

    private:
        CURL* curl_;
        std::string base_url_;
        struct curl_slist *headers_;
        int timeout_ms_;
        int dns_cache_timeout_;
        int keepalive_timeout_;

        // Stats
        uint64_t total_requests_;
        uint64_t reused_connections_;
        double last_latency_ms_;
        double total_latency_ms_;
        bool connection_warmed_;

        void init();
        void cleanup();
        HttpResponse perform_request(const std::string& url);

        static size_t write_callback(char *ptr, size_t size, size_t nmemb, void* userdata);
    };

    void http_global_init();
    void http_global_cleanup();
}

#endif //POLYMARKET_HTTP_CLIENT_H
