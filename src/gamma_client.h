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

        // ---------------- Health ----------------
        std::string fetch_status();

        // ---------------- Teams / Sports ----------------
        std::vector<Team>             fetch_teams(const TeamsRequest &req = {});
        std::vector<SportsMetadata>   fetch_sports();
        SportsMarketTypesResponse     fetch_sports_market_types();

        // ---------------- Tags ----------------
        std::vector<Tag>        fetch_tags(const TagsRequest &req = {});
        Tag                     fetch_tag_by_id(const std::string &id);
        Tag                     fetch_tag_by_slug(const std::string &slug);
        std::vector<RelatedTag> fetch_related_tags_by_id(const std::string &id,
                                                          const RelatedTagsRequest &req = {});
        std::vector<RelatedTag> fetch_related_tags_by_slug(const std::string &slug,
                                                            const RelatedTagsRequest &req = {});
        std::vector<Tag>        fetch_tags_related_to_tag_by_id(const std::string &id,
                                                                 const RelatedTagsRequest &req = {});
        std::vector<Tag>        fetch_tags_related_to_tag_by_slug(const std::string &slug,
                                                                   const RelatedTagsRequest &req = {});

        // ---------------- Series ----------------
        std::vector<Series> fetch_series(const SeriesListRequest &req = {});
        Series              fetch_series_by_id(const std::string &id);

        // ---------------- Comments ----------------
        std::vector<Comment> fetch_comments(const CommentsRequest &req);
        std::vector<Comment> fetch_comments_by_id(const std::string &id);
        std::vector<Comment> fetch_comments_by_user_address(const std::string &user_address);

        // ---------------- Public profile / search ----------------
        PublicProfile fetch_public_profile(const std::string &address);
        SearchResults fetch_public_search(const SearchRequest &req);

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
