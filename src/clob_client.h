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

        OrderBook fetch_order_book(std::string token_id);
        std::vector<OrderBook> fetch_order_books(const std::vector<std::string> &token_ids);

        double fetch_market_price(std::string token_id, TradeSide side);
        TokenSidePrices fetch_market_prices(const std::vector<std::string> &token_ids, const std::vector<TradeSide> &sides);

        double fetch_midpoint_price(std::string token_id);
        TokenPrices fetch_midpoint_prices(std::vector<std::string> token_ids);

        double fetch_spread(std::string token_id);
        TokenPrices fetch_spreads(std::vector<std::string> token_ids);

        SidePriceQuote fetch_last_traded_price(std::string token_id);
        std::vector<TokenSidePriceQuote> fetch_last_traded_prices(std::vector<std::string> token_ids);

        PriceHistory fetch_prices_history(
            std::string market,
            double start_ts,
            double end_ts,
            std::string interval,
            int fidelity);

        int fetch_fee_rate(std::string token_id);
        double fetch_tick_size(std::string token_id);

        ClobMarketInfo fetch_clob_market_info(std::string condition_id);
        int fetch_server_time();

        std::vector<ClobMarket> fetch_simplified_markets();
        std::vector<ClobMarket> fetch_markets();
        std::vector<ClobMarket> fetch_all_simplified_markets();
        std::vector<ClobMarket> fetch_all_markets();

        std::vector<MakerRebates> fetch_current_maker_rebates(std::string date, std::string maker_address);


    private:
        APIConfig config_;
        HttpClient http_;
    };
}



#endif //POLYMARKET_CLOB_CLIENT_H
