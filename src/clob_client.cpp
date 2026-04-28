//
// Created by Lorenzo P on 4/21/26.
//

#include "clob_client.h"

#include <format>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace json = boost::json;
namespace urls = boost::urls;

namespace polymarket::clob {

    namespace {
        // Centralized error throw to keep call sites compact.
        [[noreturn]] void throw_http(const HttpResponse &r) {
            throw std::runtime_error(
                "HTTP request failed: " + std::to_string(r.status) + " - " + r.error);
        }

        // POST a JSON array of objects keyed by "token_id" (and optionally "side").
        std::string make_token_array(const std::vector<std::string> &token_ids) {
            json::array arr;
            for (const auto &id : token_ids) {
                arr.push_back(json::object{{"token_id", id}});
            }
            return json::serialize(arr);
        }

        std::string make_token_side_array(const std::vector<std::string> &token_ids,
                                          const std::vector<TradeSide> &sides) {
            json::array arr;
            for (size_t i = 0; i < token_ids.size(); ++i) {
                arr.push_back(json::object{
                    {"token_id", token_ids[i]},
                    {"side",     to_string(sides[i])},
                });
            }
            return json::serialize(arr);
        }

        TokenPrices parse_token_prices(const std::string &body) {
            TokenPrices prices;
            const auto parsed = json::parse(body);
            const auto *obj = parsed.if_object();
            if (!obj) return prices;

            for (const auto &item : *obj) {
                prices.emplace(std::string(item.key()), detail::as_number_or_string(item.value()));
            }
            return prices;
        }

        TokenSidePrices parse_token_side_prices(const std::string &body) {
            TokenSidePrices prices;
            const auto parsed = json::parse(body);
            const auto *obj = parsed.if_object();
            if (!obj) return prices;

            for (const auto &token_item : *obj) {
                const auto *side_obj = token_item.value().if_object();
                if (!side_obj) continue;

                for (const auto &side_item : *side_obj) {
                    SidePriceQuote quote;
                    quote.side = trade_side_from_string(side_item.key());
                    quote.price = detail::as_number_or_string(side_item.value());
                    prices[std::string(token_item.key())] = quote;
                }
            }
            return prices;
        }

        // Drain cursor-paginated endpoints. Stops when next_cursor is empty or "LTE=".
        template <class T>
        std::vector<T> drain_pages(HttpClient &http, const std::string &base_path) {
            std::vector<T> out;
            std::string cursor;
            while (true) {
                std::string path = base_path;
                if (!cursor.empty()) {
                    path += (path.find('?') == std::string::npos ? "?" : "&");
                    path += "next_cursor=" + cursor;
                }

                const auto response = http.get(path);
                if (!response.ok()) throw_http(response);

                auto page = json::value_to<Page<T>>(json::parse(response.body));
                cursor = page.next_cursor;

                std::cout << "Downloaded: " << page.count
                          << " Total: " << out.size()
                          << " Cursor: " << cursor << std::endl;

                if (page.data.empty()) break;
                for (auto &row : page.data) out.push_back(std::move(row));
                if (cursor.empty() || cursor == "LTE=") break;
            }
            return out;
        }

    } // namespace

    ClobClient::ClobClient(const APIConfig &config)
        : config_(config)
    {
        if (config_.log_level.has_value()) {
            http_.set_log_level(*config_.log_level);
        }
        http_.set_base_url(config.clob_rest_url);
        http_.set_timeout(config_.http_timeout_ms);
    }

    // ============================================================
    // Health / metadata
    // ============================================================
    std::string ClobClient::fetch_ok() {
        const auto response = http_.get("/");
        if (!response.ok()) throw_http(response);
        return response.body;
    }

    std::int64_t ClobClient::fetch_server_time() {
        const auto response = http_.get("/time");
        if (!response.ok()) throw_http(response);
        // Server returns a JSON number (i64 milliseconds). Body is "<digits>".
        // Strip trailing whitespace/newlines defensively.
        return std::stoll(response.body);
    }

    std::uint32_t ClobClient::fetch_version() {
        const auto response = http_.get("/version");
        if (!response.ok()) throw_http(response);
        auto jv = json::parse(response.body);
        if (auto const* obj = jv.if_object()) {
            if (auto const* v = obj->if_contains("version"))
                return static_cast<std::uint32_t>(v->to_number<std::uint64_t>());
        }
        return 0;
    }

    // ============================================================
    // Order book
    // ============================================================
    OrderBook ClobClient::fetch_order_book(const std::string &token_id) {
        const std::string path = std::format("/book?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<OrderBook>(json::parse(response.body));
    }

    std::vector<OrderBook> ClobClient::fetch_order_books(const std::vector<std::string> &token_ids) {
        const auto response = http_.post("/books", make_token_array(token_ids));
        if (!response.ok()) throw_http(response);
        return json::value_to<std::vector<OrderBook>>(json::parse(response.body));
    }

    // ============================================================
    // Prices
    // ============================================================
    double ClobClient::fetch_market_price(const std::string &token_id, const TradeSide side) {
        const std::string path = std::format("/price?token_id={}&side={}",
                                             token_id, to_string(side));
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<PriceResponse>(json::parse(response.body)).price;
    }

    TokenSidePrices ClobClient::fetch_market_prices(
        const std::vector<std::string> &token_ids,
        const std::vector<TradeSide> &sides)
    {
        if (token_ids.size() != sides.size())
            throw std::runtime_error("token_ids and sides size mismatch");

        const auto response = http_.post("/prices", make_token_side_array(token_ids, sides));
        if (!response.ok()) throw_http(response);
        return parse_token_side_prices(response.body);
    }

    double ClobClient::fetch_midpoint_price(const std::string &token_id) {
        const std::string path = std::format("/midpoint?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<MidpointResponse>(json::parse(response.body)).mid;
    }

    TokenPrices ClobClient::fetch_midpoint_prices(const std::vector<std::string> &token_ids) {
        const auto response = http_.post("/midpoints", make_token_array(token_ids));
        if (!response.ok()) throw_http(response);
        return parse_token_prices(response.body);
    }

    double ClobClient::fetch_spread(const std::string &token_id) {
        const std::string path = std::format("/spread?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<SpreadResponse>(json::parse(response.body)).spread;
    }

    TokenPrices ClobClient::fetch_spreads(const std::vector<std::string> &token_ids) {
        const auto response = http_.post("/spreads", make_token_array(token_ids));
        if (!response.ok()) throw_http(response);
        return parse_token_prices(response.body);
    }

    LastTradePriceResponse ClobClient::fetch_last_traded_price(const std::string &token_id) {
        const std::string path = std::format("/last-trade-price?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<LastTradePriceResponse>(json::parse(response.body));
    }

    std::vector<LastTradesPriceItem>
    ClobClient::fetch_last_traded_prices(const std::vector<std::string> &token_ids) {
        // Rust client uses GET with a JSON body — preserve that contract.
        const auto response = http_.post("/last-trades-prices", make_token_array(token_ids));
        if (!response.ok()) throw_http(response);
        return json::value_to<std::vector<LastTradesPriceItem>>(json::parse(response.body));
    }

    PriceHistoryResponse ClobClient::fetch_prices_history(const PriceHistoryRequest &req) {
        urls::url u;
        u.params().append({"market", req.market});

        // interval and (start_ts/end_ts) are mutually exclusive on the server.
        if (req.interval && *req.interval != PriceHistoryInterval::None) {
            u.params().append({"interval", to_string(*req.interval)});
        } else {
            if (req.start_ts) u.params().append({"startTs", std::to_string(*req.start_ts)});
            if (req.end_ts)   u.params().append({"endTs",   std::to_string(*req.end_ts)});
        }
        if (req.fidelity) u.params().append({"fidelity", std::to_string(*req.fidelity)});

        const std::string path = "/prices-history?" + std::string(u.encoded_query());
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<PriceHistoryResponse>(json::parse(response.body));
    }

    // ============================================================
    // Market info
    // ============================================================
    std::uint32_t ClobClient::fetch_fee_rate(const std::string &token_id) {
        const std::string path = std::format("/fee-rate?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<FeeRateResponse>(json::parse(response.body)).base_fee;
    }

    double ClobClient::fetch_tick_size(const std::string &token_id) {
        const std::string path = std::format("/tick-size?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<TickSizeResponse>(json::parse(response.body)).minimum_tick_size;
    }

    bool ClobClient::fetch_neg_risk(const std::string &token_id) {
        const std::string path = std::format("/neg-risk?token_id={}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<NegRiskResponse>(json::parse(response.body)).neg_risk;
    }

    ClobMarketInfo ClobClient::fetch_clob_market_info(const std::string &condition_id) {
        const std::string path = std::format("/clob-markets/{}", condition_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<ClobMarketInfo>(json::parse(response.body));
    }

    MarketResponse ClobClient::fetch_market(const std::string &condition_id) {
        const std::string path = std::format("/markets/{}", condition_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<MarketResponse>(json::parse(response.body));
    }

    MarketByTokenResponse ClobClient::fetch_market_by_token(const std::string &token_id) {
        const std::string path = std::format("/markets-by-token/{}", token_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::value_to<MarketByTokenResponse>(json::parse(response.body));
    }

    json::value ClobClient::fetch_market_trades_events(const std::string &condition_id) {
        const std::string path = std::format("/markets/live-activity/{}", condition_id);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        return json::parse(response.body);
    }

    // ============================================================
    // Markets pagination (single page)
    // ============================================================
    static std::string with_cursor(const std::string &base, const std::string &cursor) {
        if (cursor.empty()) return base;
        return base + "?next_cursor=" + cursor;
    }

    Page<MarketResponse> ClobClient::fetch_markets(const std::string &cursor) {
        const auto response = http_.get(with_cursor("/markets", cursor));
        if (!response.ok()) throw_http(response);
        return json::value_to<Page<MarketResponse>>(json::parse(response.body));
    }

    Page<MarketResponse> ClobClient::fetch_sampling_markets(const std::string &cursor) {
        const auto response = http_.get(with_cursor("/sampling-markets", cursor));
        if (!response.ok()) throw_http(response);
        return json::value_to<Page<MarketResponse>>(json::parse(response.body));
    }

    Page<SimplifiedMarket> ClobClient::fetch_simplified_markets(const std::string &cursor) {
        const auto response = http_.get(with_cursor("/simplified-markets", cursor));
        if (!response.ok()) throw_http(response);
        return json::value_to<Page<SimplifiedMarket>>(json::parse(response.body));
    }

    Page<SimplifiedMarket> ClobClient::fetch_sampling_simplified_markets(const std::string &cursor) {
        const auto response = http_.get(with_cursor("/sampling-simplified-markets", cursor));
        if (!response.ok()) throw_http(response);
        return json::value_to<Page<SimplifiedMarket>>(json::parse(response.body));
    }

    // ============================================================
    // Markets pagination (drain all pages)
    // ============================================================
    std::vector<MarketResponse> ClobClient::fetch_all_markets() {
        return drain_pages<MarketResponse>(http_, "/markets");
    }

    std::vector<MarketResponse> ClobClient::fetch_all_sampling_markets() {
        return drain_pages<MarketResponse>(http_, "/sampling-markets");
    }

    std::vector<SimplifiedMarket> ClobClient::fetch_all_simplified_markets() {
        return drain_pages<SimplifiedMarket>(http_, "/simplified-markets");
    }

    std::vector<SimplifiedMarket> ClobClient::fetch_all_sampling_simplified_markets() {
        return drain_pages<SimplifiedMarket>(http_, "/sampling-simplified-markets");
    }

    // ============================================================
    // Geoblock (different host)
    // ============================================================
    GeoblockResponse ClobClient::check_geoblock() {
        http_.set_base_url(config_.polymarket_url);
        const auto response = http_.get("/api/geoblock");
        http_.set_base_url(config_.clob_rest_url);
        if (!response.ok()) throw_http(response);
        return json::value_to<GeoblockResponse>(json::parse(response.body));
    }

    // ============================================================
    // Maker rebates (preserved as-is — live Polymarket endpoint not in Rust client)
    // ============================================================
    std::vector<MakerRebates>
    ClobClient::fetch_current_maker_rebates(const std::string &date,
                                            const std::string &maker_address) {
        const std::string path = std::format("/rebates/current?date={}&maker_address={}",
                                             date, maker_address);
        const auto response = http_.get(path);
        if (!response.ok()) throw_http(response);
        const auto parsed = json::parse(response.body);
        if (parsed.is_null()) {
            return {};
        }
        return json::value_to<std::vector<MakerRebates>>(parsed);
    }
}
