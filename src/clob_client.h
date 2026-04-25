//
// Created by Lorenzo P on 4/21/26.
//

#ifndef POLYMARKET_CLOB_CLIENT_H
#define POLYMARKET_CLOB_CLIENT_H

#include "clob_types.h"
#include "http_client.h"
#include "types.h"


namespace polymarket::clob {
    class ClobClient {
    public:
        explicit ClobClient(const APIConfig &config);
        ~ClobClient() = default;

        // ---------------- Health / metadata ----------------
        std::string  fetch_ok();
        std::int64_t fetch_server_time();
        std::uint32_t fetch_version();

        // ---------------- Order book ----------------
        OrderBook              fetch_order_book(const std::string &token_id);
        std::vector<OrderBook> fetch_order_books(const std::vector<std::string> &token_ids);

        // ---------------- Prices ----------------
        double          fetch_market_price(const std::string &token_id, TradeSide side);
        TokenSidePrices fetch_market_prices(const std::vector<std::string> &token_ids,
                                            const std::vector<TradeSide> &sides);

        double      fetch_midpoint_price(const std::string &token_id);
        TokenPrices fetch_midpoint_prices(const std::vector<std::string> &token_ids);

        double      fetch_spread(const std::string &token_id);
        TokenPrices fetch_spreads(const std::vector<std::string> &token_ids);

        LastTradePriceResponse           fetch_last_traded_price(const std::string &token_id);
        std::vector<LastTradesPriceItem> fetch_last_traded_prices(const std::vector<std::string> &token_ids);

        PriceHistoryResponse fetch_prices_history(const PriceHistoryRequest &req);

        // ---------------- Market info ----------------
        std::uint32_t fetch_fee_rate(const std::string &token_id);
        double        fetch_tick_size(const std::string &token_id);
        bool          fetch_neg_risk(const std::string &token_id);

        ClobMarketInfo        fetch_clob_market_info(const std::string &condition_id);
        MarketResponse        fetch_market(const std::string &condition_id);
        MarketByTokenResponse fetch_market_by_token(const std::string &token_id);
        boost::json::value    fetch_market_trades_events(const std::string &condition_id);

        // ---------------- Markets pagination ----------------
        Page<MarketResponse>   fetch_markets(const std::string &cursor = "");
        Page<MarketResponse>   fetch_sampling_markets(const std::string &cursor = "");
        Page<SimplifiedMarket> fetch_simplified_markets(const std::string &cursor = "");
        Page<SimplifiedMarket> fetch_sampling_simplified_markets(const std::string &cursor = "");

        std::vector<MarketResponse>   fetch_all_markets();
        std::vector<MarketResponse>   fetch_all_sampling_markets();
        std::vector<SimplifiedMarket> fetch_all_simplified_markets();
        std::vector<SimplifiedMarket> fetch_all_sampling_simplified_markets();

        // ---------------- Geoblock ----------------
        // Hits polymarket.com/api/geoblock (different host) — temporarily switches base URL.
        GeoblockResponse check_geoblock();

        // ---------------- Maker rebates (Polymarket-only) ----------------
        std::vector<MakerRebates> fetch_current_maker_rebates(const std::string &date,
                                                              const std::string &maker_address);

    private:
        APIConfig  config_;
        HttpClient http_;
    };
}


#endif //POLYMARKET_CLOB_CLIENT_H
