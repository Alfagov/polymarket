//
// Created by Lorenzo P on 4/19/26.
//

#ifndef POLYMARKET_GAMMA_CLIENT_H
#define POLYMARKET_GAMMA_CLIENT_H
#include "http_client.h"
#include "types.h"
#include "gamma_types.h"


namespace polymarket::gamma {
    class GammaClient {
        public:
        explicit GammaClient(const APIConfig &config);
        ~GammaClient() = default;

        std::vector<Event> fetch_events(const EventParameters &params, const std::string &cursor = "");
        std::vector<Event> fetch_all_events(const EventParameters &params);
        Event fetch_event_by_id(int id);
        Event fetch_event_by_slug(const std::string &slug);
        std::vector<Tag> fetch_events_tags(int id);

        std::vector<Market> fetch_markets(const MarketParameters &params, const std::string &cursor = "");
        std::vector<Market> fetch_all_markets(const MarketParameters &params);
        Market fetch_market_by_id(const std::string &id);
        Market fetch_market_by_slug(const std::string &slug);
        std::vector<MarketHolders> fetch_market_top_holders(
            const std::vector<std::string> &market,
            int limit = 20,
            int min_balance = 1);
        std::vector<MarketOpenInterest> fetch_market_open_interests(const std::vector<std::string> &market);
        std::vector<EventVolume> fetch_event_live_volumes(int event);
        std::vector<Tag> fetch_market_tags_by_id(const std::string &id);


    private:
        APIConfig config_;
        HttpClient http_;

        std::string build_market_query_string(const MarketParameters& p);
        std::string build_event_query_string(const EventParameters& p);
        MarketsResponse parse_markets_response(const std::string &json_response);
        Market parse_market_response(const std::string &json_response);
        std::vector<MarketHolders> parse_market_holders_response(const std::string &json_response);
        std::vector<MarketOpenInterest> parse_market_open_interests_response(const std::string &json_response);
        std::vector<EventVolume> parse_event_live_volumes_response(const std::string &json_response);
        EventsResponse parse_events_response(const std::string &json_response);
    };
}




#endif //POLYMARKET_GAMMA_CLIENT_H
