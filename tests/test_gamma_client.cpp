#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

#include "clob_client.h"
#include "data_client.h"
#include "gamma_client.h"
#include "mock_http_server.h"
#include "types.h"

namespace {

using polymarket::APIConfig;
using polymarket::clob::ClobClient;
using polymarket::data::DataClient;
using polymarket::gamma::GammaClient;
using polymarket::gamma::MarketParameters;
using polymarket::gamma::EventParameters;
using polymarket::test::CannedResponse;
using polymarket::test::CapturedRequest;
using polymarket::test::MockHttpServer;

constexpr int kLiveEventId = 2890;
constexpr std::string_view kLiveEventSlug =
    "nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup";
constexpr std::string_view kLiveMarketId = "239826";
constexpr int kCommentEventId = 10366;
constexpr std::string_view kCommentId = "27526";
constexpr std::string_view kCommentUserAddress =
    "0x47e72a14b695174062396a3d7d91f109ce672e20";
constexpr std::string_view kPublicProfileAddress =
    "0xecdbd79566a25693b9971c48d7de84bc05f7da79";
constexpr std::string_view kTagId = "100215";
constexpr std::string_view kTagSlug = "all";
constexpr std::string_view kSeriesId = "11286";
constexpr std::string_view kSeriesSlug = "power-slap";
constexpr std::string_view kClobConditionId =
    "0x5eed579ff6763914d78a966c83473ba2485ac8910d0a0914eef6d9fcb33085de";
constexpr std::string_view kClobTokenId =
    "73470541315377973562501025254719659796416871135081220986683321361000395461644";
constexpr std::string_view kActiveClobConditionId =
    "0x004230fb1f54a139d50ba2041e062a01461c931a6725ce482e3a9ab61b2925bb";
constexpr std::string_view kActiveClobTokenId =
    "69422147515888934539342749343952767069189843320299332157580167457035825622340";
constexpr std::string_view kActiveClobNoTokenId =
    "110140034559013996398860253157541589571780375034092711496574418799581677182004";

std::string fixture(std::string_view relative_path) {
    const auto path =
        std::filesystem::path(POLYMARKET_TEST_FIXTURE_DIR) / std::string(relative_path);
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("failed to open fixture: " + path.string());
    }
    return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
}

class GammaClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_.start();
        config_.gamma_api_url = server_.base_url();
        config_.data_api_url = server_.base_url();
        config_.http_timeout_ms = 2000;
        client_ = std::make_unique<GammaClient>(config_);
    }

    void TearDown() override {
        client_.reset();
        server_.stop();
    }

    static bool target_has(const std::string& target, std::string_view needle) {
        return target.find(needle) != std::string::npos;
    }

    MockHttpServer server_;
    APIConfig config_;
    std::unique_ptr<GammaClient> client_;
};

// ---------------------------------------------------------------------------
// Events
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchEventsReturnsParsedEventsFromArrayBody) {
    server_.enqueue({200, R"([{"id":"1","slug":"a"},{"id":"2","slug":"b"}])"});

    EventParameters p;
    auto events = client_->fetch_events(p);

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].id, "1");
    EXPECT_EQ(events[0].slug, "a");
    EXPECT_EQ(events[1].id, "2");

    const auto& req = server_.last_request();
    EXPECT_EQ(req.method, "GET");
    EXPECT_TRUE(target_has(req.target, "/events/keyset"));
    EXPECT_TRUE(target_has(req.target, "limit=20"));
}

TEST_F(GammaClientTest, FetchEventsAppendsAfterCursorWhenProvided) {
    server_.enqueue({200, R"([])"});

    EventParameters p;
    client_->fetch_events(p, "abc123");

    const auto& req = server_.last_request();
    EXPECT_TRUE(target_has(req.target, "after_cursor=abc123"))
        << "target was: " << req.target;
}

TEST_F(GammaClientTest, FetchEventsThrowsOnNon2xxStatus) {
    server_.enqueue({500, R"({"error":"boom"})"});

    EventParameters p;
    EXPECT_THROW(client_->fetch_events(p), std::runtime_error);
}

TEST_F(GammaClientTest, FetchAllEventsPaginatesViaNextCursor) {
    server_.set_handler([n = 0](const CapturedRequest& req) mutable -> CannedResponse {
        ++n;
        if (n == 1) {
            EXPECT_EQ(req.target.find("after_cursor="), std::string::npos);
            return {200, R"({"events":[{"id":"1"}],"next_cursor":"CUR"})"};
        }
        if (n == 2) {
            EXPECT_NE(req.target.find("after_cursor=CUR"), std::string::npos);
            return {200, R"({"events":[{"id":"2"}],"next_cursor":""})"};
        }
        return {500, "{}"};
    });

    EventParameters p;
    auto events = client_->fetch_all_events(p);

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].id, "1");
    EXPECT_EQ(events[1].id, "2");
    EXPECT_EQ(server_.request_count(), 2u);
}

TEST_F(GammaClientTest, FetchAllEventsStopsOnEmptyPage) {
    server_.enqueue({200, R"({"events":[],"next_cursor":"ignored"})"});

    EventParameters p;
    auto events = client_->fetch_all_events(p);

    EXPECT_TRUE(events.empty());
    EXPECT_EQ(server_.request_count(), 1u);
}

TEST_F(GammaClientTest, FetchEventByIdHitsCorrectPath) {
    server_.enqueue({200, fixture("gamma/event_2890.json")});

    auto ev = client_->fetch_event_by_id(kLiveEventId);

    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    EXPECT_EQ(ev.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target, "/events/2890");
}

TEST_F(GammaClientTest, FetchEventBySlugHitsCorrectPath) {
    server_.enqueue({200, fixture("gamma/event_2890.json")});

    auto ev = client_->fetch_event_by_slug(std::string(kLiveEventSlug));

    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    EXPECT_EQ(ev.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target,
              "/events/slug/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup");
}

TEST_F(GammaClientTest, FetchEventsTagsHitsCorrectPathAndParses) {
    server_.enqueue({200, fixture("gamma/event_2890_tags.json")});

    auto tags = client_->fetch_events_tags(kLiveEventId);

    ASSERT_EQ(tags.size(), 1u);
    EXPECT_EQ(tags[0].label, "All");
    EXPECT_EQ(tags[0].slug, "all");
    EXPECT_EQ(server_.last_request().target, "/events/2890/tags");
}

TEST_F(GammaClientTest, FetchEventByIdThrowsOn404) {
    server_.enqueue({404, R"({"error":"not found"})"});
    EXPECT_THROW(client_->fetch_event_by_id(1), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Markets
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchMarketsParsesArrayBody) {
    server_.enqueue({200, R"([{"id":"m1","slug":"s1"}])"});

    MarketParameters p;
    auto markets = client_->fetch_markets(p);

    ASSERT_EQ(markets.size(), 1u);
    EXPECT_EQ(markets[0].id, "m1");
    EXPECT_TRUE(target_has(server_.last_request().target, "/markets/keyset"));
}

TEST_F(GammaClientTest, FetchMarketsPassesCursor) {
    server_.enqueue({200, R"([])"});

    MarketParameters p;
    client_->fetch_markets(p, "cur42");

    EXPECT_TRUE(target_has(server_.last_request().target, "after_cursor=cur42"));
}

TEST_F(GammaClientTest, FetchAllMarketsPaginates) {
    server_.set_handler([n = 0](const CapturedRequest&) mutable -> CannedResponse {
        ++n;
        if (n == 1) return {200, R"({"markets":[{"id":"a"}],"next_cursor":"X"})"};
        if (n == 2) return {200, R"({"markets":[{"id":"b"},{"id":"c"}],"next_cursor":""})"};
        return {500, "{}"};
    });

    MarketParameters p;
    auto markets = client_->fetch_all_markets(p);
    EXPECT_EQ(markets.size(), 3u);
    EXPECT_EQ(server_.request_count(), 2u);
}

TEST_F(GammaClientTest, FetchMarketByIdHitsPath) {
    server_.enqueue({200, fixture("gamma/market_239826.json")});

    auto m = client_->fetch_market_by_id(std::string(kLiveMarketId));

    EXPECT_EQ(m.id, kLiveMarketId);
    EXPECT_EQ(m.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target, "/markets/239826");
}

TEST_F(GammaClientTest, FetchMarketBySlugHitsPath) {
    server_.enqueue({200, fixture("gamma/market_239826_by_slug.json")});

    auto m = client_->fetch_market_by_slug(std::string(kLiveEventSlug));

    EXPECT_EQ(m.id, kLiveMarketId);
    EXPECT_EQ(m.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target,
              "/markets/slug/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup");
}

TEST_F(GammaClientTest, FetchMarketTagsByIdParsesTags) {
    server_.enqueue({200, fixture("gamma/market_239826_tags.json")});

    auto tags = client_->fetch_market_tags_by_id(std::string(kLiveMarketId));

    ASSERT_EQ(tags.size(), 1u);
    EXPECT_EQ(tags[0].label, "All");
    EXPECT_EQ(tags[0].slug, "all");
    EXPECT_EQ(server_.last_request().target, "/markets/239826/tags");
}

// ---------------------------------------------------------------------------
// Data-API endpoints (holders, open interest, live volume)
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchMarketTopHoldersBuildsQueryAndParses) {
    server_.enqueue({200, fixture("gamma/market_top_holders_sample.json")});

    auto res = client_->fetch_market_top_holders({"tokenA", "tokenB"}, 5, 2);

    ASSERT_EQ(res.size(), 1u);
    EXPECT_EQ(res[0].token, "t1");
    ASSERT_EQ(res[0].holders.size(), 1u);
    EXPECT_DOUBLE_EQ(res[0].holders[0].amount, 10.5);

    const auto& req = server_.last_request();
    EXPECT_TRUE(target_has(req.target, "/holders"));
    EXPECT_TRUE(target_has(req.target, "limit=5"));
    EXPECT_TRUE(target_has(req.target, "minBalance=2"));
    EXPECT_TRUE(target_has(req.target, "market=tokenA,tokenB"));
}

TEST_F(GammaClientTest, FetchMarketTopHoldersUsesDefaults) {
    server_.enqueue({200, fixture("gamma/empty_array.json")});
    client_->fetch_market_top_holders({"tok"});

    const auto& req = server_.last_request();
    EXPECT_TRUE(target_has(req.target, "limit=20"));
    EXPECT_TRUE(target_has(req.target, "minBalance=1"));
}

TEST_F(GammaClientTest, FetchMarketOpenInterestsParses) {
    server_.enqueue({200, fixture(
        "data/open_interest_0x5eed579ff6763914d78a966c83473ba2485ac8910d0a0914eef6d9fcb33085de.json")});

    auto ois = client_->fetch_market_open_interests({std::string(kClobConditionId)});

    ASSERT_EQ(ois.size(), 1u);
    EXPECT_EQ(ois[0].market, kClobConditionId);
    EXPECT_DOUBLE_EQ(ois[0].value, 0.0);
    EXPECT_EQ(server_.last_request().target, "/oi?market=" + std::string(kClobConditionId));
}

TEST_F(GammaClientTest, FetchEventLiveVolumesParses) {
    server_.enqueue({200, fixture("data/live_volume_2890.json")});

    auto vols = client_->fetch_event_live_volumes(kLiveEventId);

    ASSERT_EQ(vols.size(), 1u);
    EXPECT_DOUBLE_EQ(vols[0].total, 0.0);
    EXPECT_TRUE(vols[0].markets.empty());
    EXPECT_EQ(server_.last_request().target, "/live-volume?id=2890");
}

// ---------------------------------------------------------------------------
// Base-URL restoration: after a data-API call (success OR failure) the next
// gamma-API call must still land on the gamma endpoint.
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, DataApiCallDoesNotCorruptBaseUrlOnSuccess) {
    // Both point to the same mock, but the test still validates that a
    // subsequent /events path is reachable and reaches a /events/... target.
    server_.enqueue({200, R"([])"});              // holders
    server_.enqueue({200, fixture("gamma/event_2890.json")});     // event-by-id

    client_->fetch_market_top_holders({"tok"});
    auto ev = client_->fetch_event_by_id(kLiveEventId);

    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[1].target, "/events/2890");
}

TEST_F(GammaClientTest, DataApiCallDoesNotCorruptBaseUrlOnFailure) {
    server_.enqueue({502, "bad gateway"});        // holders fails
    server_.enqueue({200, fixture("gamma/event_2890.json")});     // event-by-id succeeds

    EXPECT_THROW(client_->fetch_market_top_holders({"tok"}), std::runtime_error);

    auto ev = client_->fetch_event_by_id(kLiveEventId);
    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[1].target, "/events/2890");
}

// ---------------------------------------------------------------------------
// Query-string construction (via observable side effects in request target)
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, ParametersAreSerializedIntoQueryString) {
    server_.enqueue({200, R"([])"});

    EventParameters p;
    p.limit = 25;
    p.order = "volume";
    p.ascending = true;
    p.closed = false;
    p.slug = {"foo", "bar"};
    p.id = {1, 2, 3};
    p.liquidity_min = 100;
    p.volume_max = 9999;
    p.start_date_min = "2026-01-01";
    p.end_date_max = "2026-12-31";
    p.related_tags = true;

    client_->fetch_events(p);

    const auto& t = server_.last_request().target;
    EXPECT_TRUE(target_has(t, "limit=25")) << t;
    EXPECT_TRUE(target_has(t, "order=volume")) << t;
    EXPECT_TRUE(target_has(t, "ascending=true")) << t;
    EXPECT_TRUE(target_has(t, "closed=false")) << t;
    EXPECT_TRUE(target_has(t, "slug=foo,bar")) << t;
    EXPECT_TRUE(target_has(t, "id=1,2,3")) << t;
    EXPECT_TRUE(target_has(t, "liquidity_min=100")) << t;
    EXPECT_TRUE(target_has(t, "volume_max=9999")) << t;
    EXPECT_TRUE(target_has(t, "start_date_min=2026-01-01")) << t;
    EXPECT_TRUE(target_has(t, "end_date_max=2026-12-31")) << t;
    EXPECT_TRUE(target_has(t, "related_tags=true")) << t;
}

TEST_F(GammaClientTest, OptionalParametersOmittedWhenUnset) {
    server_.enqueue({200, R"([])"});

    EventParameters p;  // defaults: only limit set
    client_->fetch_events(p);

    const auto& t = server_.last_request().target;
    EXPECT_TRUE(target_has(t, "limit=20"));
    EXPECT_TRUE(target_has(t, "ascending="));
    EXPECT_FALSE(target_has(t, "closed="));
    EXPECT_FALSE(target_has(t, "liquidity_min="));
    EXPECT_FALSE(target_has(t, "slug="));
    EXPECT_FALSE(target_has(t, "id="));
    EXPECT_FALSE(target_has(t, "start_date_min="));
}

// ---------------------------------------------------------------------------
// Additional gamma-api endpoints
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchStatusReturnsCapturedStatus) {
    server_.enqueue({200, fixture("gamma/status.txt"), "text/plain"});

    const auto status = client_->fetch_status();

    EXPECT_EQ(status, "OK");
    EXPECT_EQ(server_.last_request().target, "/status");
}

TEST_F(GammaClientTest, FetchTeamsBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/teams_nba_limit_1.json")});

    polymarket::gamma::TeamsRequest req;
    req.limit = 1;
    req.league = "nba";
    const auto teams = client_->fetch_teams(req);

    ASSERT_EQ(teams.size(), 1u);
    EXPECT_EQ(teams[0].league, "nba");
    EXPECT_EQ(server_.last_request().target, "/teams?limit=1&league=nba");
}

TEST_F(GammaClientTest, FetchSportsParsesFixture) {
    server_.enqueue({200, fixture("gamma/sports.json")});

    const auto sports = client_->fetch_sports();

    ASSERT_FALSE(sports.empty());
    EXPECT_EQ(sports[0].sport, "ncaab");
    EXPECT_EQ(server_.last_request().target, "/sports");
}

TEST_F(GammaClientTest, FetchSportsMarketTypesParsesFixture) {
    server_.enqueue({200, fixture("gamma/sports_market_types.json")});

    const auto response = client_->fetch_sports_market_types();

    EXPECT_NE(std::find(response.marketTypes.begin(), response.marketTypes.end(), "moneyline"),
              response.marketTypes.end());
    EXPECT_EQ(server_.last_request().target, "/sports/market-types");
}

TEST_F(GammaClientTest, FetchTagsBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/tags_limit_1.json")});

    polymarket::gamma::TagsRequest req;
    req.limit = 1;
    const auto tags = client_->fetch_tags(req);

    ASSERT_EQ(tags.size(), 1u);
    EXPECT_EQ(tags[0].id, "101867");
    EXPECT_EQ(server_.last_request().target, "/tags?limit=1");
}

TEST_F(GammaClientTest, FetchTagByIdParsesFixture) {
    server_.enqueue({200, fixture("gamma/tag_100215.json")});

    const auto tag = client_->fetch_tag_by_id(std::string(kTagId));

    EXPECT_EQ(tag.id, kTagId);
    EXPECT_EQ(tag.slug, kTagSlug);
    EXPECT_EQ(server_.last_request().target, "/tags/100215");
}

TEST_F(GammaClientTest, FetchTagBySlugParsesFixture) {
    server_.enqueue({200, fixture("gamma/tag_slug_all.json")});

    const auto tag = client_->fetch_tag_by_slug(std::string(kTagSlug));

    EXPECT_EQ(tag.id, kTagId);
    EXPECT_EQ(tag.slug, kTagSlug);
    EXPECT_EQ(server_.last_request().target, "/tags/slug/all");
}

TEST_F(GammaClientTest, FetchRelatedTagsByIdBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/related_tags_100215_all.json")});

    polymarket::gamma::RelatedTagsRequest req;
    req.status = "all";
    const auto tags = client_->fetch_related_tags_by_id(std::string(kTagId), req);

    ASSERT_FALSE(tags.empty());
    EXPECT_EQ(tags[0].id, "34178");
    EXPECT_EQ(tags[0].rank, 1);
    EXPECT_EQ(server_.last_request().target, "/tags/100215/related-tags?status=all");
}

TEST_F(GammaClientTest, FetchRelatedTagsBySlugBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/related_tags_slug_all_all.json")});

    polymarket::gamma::RelatedTagsRequest req;
    req.status = "all";
    const auto tags = client_->fetch_related_tags_by_slug(std::string(kTagSlug), req);

    ASSERT_FALSE(tags.empty());
    EXPECT_EQ(tags[0].id, "34178");
    EXPECT_EQ(tags[0].rank, 1);
    EXPECT_EQ(server_.last_request().target, "/tags/slug/all/related-tags?status=all");
}

TEST_F(GammaClientTest, FetchTagsRelatedToTagByIdBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/related_tag_records_100215_all.json")});

    polymarket::gamma::RelatedTagsRequest req;
    req.status = "all";
    const auto tags = client_->fetch_tags_related_to_tag_by_id(std::string(kTagId), req);

    ASSERT_FALSE(tags.empty());
    EXPECT_EQ(tags[0].slug, "trump");
    EXPECT_EQ(server_.last_request().target, "/tags/100215/related-tags/tags?status=all");
}

TEST_F(GammaClientTest, FetchTagsRelatedToTagBySlugBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/related_tag_records_slug_all_all.json")});

    polymarket::gamma::RelatedTagsRequest req;
    req.status = "all";
    const auto tags = client_->fetch_tags_related_to_tag_by_slug(std::string(kTagSlug), req);

    ASSERT_FALSE(tags.empty());
    EXPECT_EQ(tags[0].slug, "trump");
    EXPECT_EQ(server_.last_request().target, "/tags/slug/all/related-tags/tags?status=all");
}

TEST_F(GammaClientTest, FetchSeriesBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/series_power_slap.json")});

    polymarket::gamma::SeriesListRequest req;
    req.limit = 1;
    req.slug = {std::string(kSeriesSlug)};
    const auto series = client_->fetch_series(req);

    ASSERT_EQ(series.size(), 1u);
    EXPECT_EQ(series[0].id, kSeriesId);
    EXPECT_EQ(series[0].slug, kSeriesSlug);
    EXPECT_EQ(server_.last_request().target, "/series?limit=1&slug=power-slap");
}

TEST_F(GammaClientTest, FetchSeriesByIdParsesFixture) {
    server_.enqueue({200, fixture("gamma/series_11286.json")});

    const auto series = client_->fetch_series_by_id(std::string(kSeriesId));

    EXPECT_EQ(series.id, kSeriesId);
    EXPECT_EQ(series.slug, kSeriesSlug);
    EXPECT_EQ(server_.last_request().target, "/series/11286");
}

TEST_F(GammaClientTest, FetchCommentsBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/comments_event_10366_limit_1.json")});

    polymarket::gamma::CommentsRequest req;
    req.parent_entity_type = "Event";
    req.parent_entity_id = std::to_string(kCommentEventId);
    req.limit = 1;
    const auto comments = client_->fetch_comments(req);

    ASSERT_FALSE(comments.empty());
    EXPECT_EQ(comments[0].id, kCommentId);
    EXPECT_EQ(comments[0].parentEntityID, kCommentEventId);
    EXPECT_EQ(server_.last_request().target,
              "/comments?parent_entity_type=Event&parent_entity_id=10366&limit=1");
}

TEST_F(GammaClientTest, FetchCommentsByIdParsesFixture) {
    server_.enqueue({200, fixture("gamma/comments_27526.json")});

    const auto comments = client_->fetch_comments_by_id(std::string(kCommentId));

    ASSERT_FALSE(comments.empty());
    EXPECT_EQ(comments[0].id, kCommentId);
    EXPECT_EQ(server_.last_request().target, "/comments/27526");
}

TEST_F(GammaClientTest, FetchCommentsByUserAddressParsesFixture) {
    server_.enqueue({200, fixture(
        "gamma/comments_user_0x47e72a14b695174062396a3d7d91f109ce672e20.json")});

    const auto comments = client_->fetch_comments_by_user_address(std::string(kCommentUserAddress));

    ASSERT_FALSE(comments.empty());
    EXPECT_EQ(comments[0].userAddress, kCommentUserAddress);
    EXPECT_EQ(server_.last_request().target,
              "/comments/user_address/0x47e72a14b695174062396a3d7d91f109ce672e20");
}

TEST_F(GammaClientTest, FetchPublicProfileBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture(
        "gamma/public_profile_0xecdbd79566a25693b9971c48d7de84bc05f7da79.json")});

    const auto profile = client_->fetch_public_profile(std::string(kPublicProfileAddress));

    EXPECT_EQ(profile.proxyWallet, kPublicProfileAddress);
    EXPECT_EQ(profile.name, "BigMike11");
    EXPECT_EQ(server_.last_request().target,
              "/public-profile?address=0xecdbd79566a25693b9971c48d7de84bc05f7da79");
}

TEST_F(GammaClientTest, FetchPublicSearchBuildsQueryAndParsesFixture) {
    server_.enqueue({200, fixture("gamma/public_search_nba_limit_1.json")});

    polymarket::gamma::SearchRequest req;
    req.q = "nba";
    req.limit_per_type = 1;
    req.page = 1;
    const auto results = client_->fetch_public_search(req);

    ASSERT_FALSE(results.events.empty());
    EXPECT_EQ(results.events[0].slug, "nba-dailies-2024-04-21");
    EXPECT_EQ(server_.last_request().target, "/public-search?q=nba&limit_per_type=1&page=1");
}

// ---------------------------------------------------------------------------
// Error surfacing
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchAllMarketsPropagatesError) {
    server_.enqueue({503, "unavailable"});
    MarketParameters p;
    EXPECT_THROW(client_->fetch_all_markets(p), std::runtime_error);
}

TEST_F(GammaClientTest, FetchEventsTagsThrowsOnError) {
    server_.enqueue({500, "{}"});
    EXPECT_THROW(client_->fetch_events_tags(1), std::runtime_error);
}

class ClobClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_.start();
        config_.clob_rest_url = server_.base_url();
        config_.polymarket_url = server_.base_url();
        config_.http_timeout_ms = 2000;
        client_ = std::make_unique<ClobClient>(config_);
    }

    void TearDown() override {
        client_.reset();
        server_.stop();
    }

    MockHttpServer server_;
    APIConfig config_;
    std::unique_ptr<ClobClient> client_;
};

TEST_F(ClobClientTest, FetchServerTimeParsesCapturedApiNumber) {
    server_.enqueue({200, fixture("clob/time.json")});

    const auto server_time = client_->fetch_server_time();

    EXPECT_GT(server_time, 0);
    EXPECT_EQ(server_.last_request().method, "GET");
    EXPECT_EQ(server_.last_request().target, "/time");
}

TEST_F(ClobClientTest, FetchOkReturnsCapturedStatus) {
    server_.enqueue({200, fixture("clob/ok.json")});

    const auto ok = client_->fetch_ok();

    EXPECT_EQ(ok, "\"OK\"");
    EXPECT_EQ(server_.last_request().method, "GET");
    EXPECT_EQ(server_.last_request().target, "/");
}

TEST_F(ClobClientTest, FetchVersionParsesCapturedVersion) {
    server_.enqueue({200, fixture("clob/version.json")});

    const auto version = client_->fetch_version();

    EXPECT_EQ(version, 1u);
    EXPECT_EQ(server_.last_request().target, "/version");
}

TEST_F(ClobClientTest, FetchOrderBookParsesCapturedBook) {
    server_.enqueue({200, fixture(
        "clob/book_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto book = client_->fetch_order_book(std::string(kActiveClobTokenId));

    EXPECT_EQ(book.market, kActiveClobConditionId);
    EXPECT_EQ(book.asset_id, kActiveClobTokenId);
    EXPECT_FALSE(book.bids.empty());
    EXPECT_FALSE(book.asks.empty());
    EXPECT_GT(book.min_order_size, 0.0);
    EXPECT_GT(book.tick_size, 0.0);
    EXPECT_EQ(server_.last_request().target,
              "/book?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchOrderBooksPostsTokensAndParsesCapturedBooks) {
    server_.enqueue({200, fixture(
        "clob/books_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto books = client_->fetch_order_books({std::string(kActiveClobTokenId)});

    ASSERT_EQ(books.size(), 1u);
    EXPECT_EQ(books[0].market, kActiveClobConditionId);
    EXPECT_EQ(server_.last_request().method, "POST");
    EXPECT_EQ(server_.last_request().target, "/books");
    EXPECT_EQ(server_.last_request().body,
              "[{\"token_id\":\"" + std::string(kActiveClobTokenId) + "\"}]");
}

TEST_F(ClobClientTest, FetchMarketPriceParsesCapturedPrice) {
    server_.enqueue({200, fixture(
        "clob/price_69422147515888934539342749343952767069189843320299332157580167457035825622340_buy.json")});

    const auto price = client_->fetch_market_price(std::string(kActiveClobTokenId),
                                                   polymarket::clob::TradeSide::Buy);

    EXPECT_GT(price, 0.0);
    EXPECT_EQ(server_.last_request().target,
              "/price?token_id=" + std::string(kActiveClobTokenId) + "&side=BUY");
}

TEST_F(ClobClientTest, FetchMarketPricesPostsTokenSidesAndParsesCapturedPrices) {
    server_.enqueue({200, fixture(
        "clob/prices_69422147515888934539342749343952767069189843320299332157580167457035825622340_buy.json")});

    const auto prices = client_->fetch_market_prices(
        {std::string(kActiveClobTokenId)}, {polymarket::clob::TradeSide::Buy});

    ASSERT_EQ(prices.size(), 1u);
    const auto quote = prices.at(std::string(kActiveClobTokenId));
    EXPECT_EQ(quote.side, polymarket::clob::TradeSide::Buy);
    EXPECT_GT(quote.price, 0.0);
    EXPECT_EQ(server_.last_request().method, "POST");
    EXPECT_EQ(server_.last_request().target, "/prices");
    EXPECT_EQ(server_.last_request().body,
              "[{\"token_id\":\"" + std::string(kActiveClobTokenId) + "\",\"side\":\"BUY\"}]");
}

TEST_F(ClobClientTest, FetchMidpointPriceParsesCapturedMidpoint) {
    server_.enqueue({200, fixture(
        "clob/midpoint_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto midpoint = client_->fetch_midpoint_price(std::string(kActiveClobTokenId));

    EXPECT_GT(midpoint, 0.0);
    EXPECT_EQ(server_.last_request().target,
              "/midpoint?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchMidpointPricesPostsTokensAndParsesCapturedMap) {
    server_.enqueue({200, fixture(
        "clob/midpoints_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto midpoints = client_->fetch_midpoint_prices({std::string(kActiveClobTokenId)});

    ASSERT_EQ(midpoints.size(), 1u);
    EXPECT_GT(midpoints.at(std::string(kActiveClobTokenId)), 0.0);
    EXPECT_EQ(server_.last_request().target, "/midpoints");
}

TEST_F(ClobClientTest, FetchSpreadParsesCapturedSpread) {
    server_.enqueue({200, fixture(
        "clob/spread_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto spread = client_->fetch_spread(std::string(kActiveClobTokenId));

    EXPECT_GT(spread, 0.0);
    EXPECT_EQ(server_.last_request().target,
              "/spread?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchSpreadsPostsTokensAndParsesCapturedMap) {
    server_.enqueue({200, fixture(
        "clob/spreads_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto spreads = client_->fetch_spreads({std::string(kActiveClobTokenId)});

    ASSERT_EQ(spreads.size(), 1u);
    EXPECT_GT(spreads.at(std::string(kActiveClobTokenId)), 0.0);
    EXPECT_EQ(server_.last_request().target, "/spreads");
}

TEST_F(ClobClientTest, FetchLastTradedPriceParsesCapturedTrade) {
    server_.enqueue({200, fixture(
        "clob/last_trade_price_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto trade = client_->fetch_last_traded_price(std::string(kActiveClobTokenId));

    EXPECT_GT(trade.price, 0.0);
    EXPECT_EQ(trade.side, polymarket::clob::TradeSide::Sell);
    EXPECT_EQ(server_.last_request().target,
              "/last-trade-price?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchLastTradedPricesPostsTokensAndParsesCapturedTrades) {
    server_.enqueue({200, fixture(
        "clob/last_trades_prices_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto trades = client_->fetch_last_traded_prices({std::string(kActiveClobTokenId)});

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].token_id, kActiveClobTokenId);
    EXPECT_GT(trades[0].price, 0.0);
    EXPECT_EQ(server_.last_request().method, "POST");
    EXPECT_EQ(server_.last_request().target, "/last-trades-prices");
}

TEST_F(ClobClientTest, FetchPricesHistoryBuildsIntervalQueryAndParsesCapturedHistory) {
    server_.enqueue({200, fixture(
        "clob/prices_history_69422147515888934539342749343952767069189843320299332157580167457035825622340_1h_60.json")});

    polymarket::clob::PriceHistoryRequest req;
    req.market = std::string(kActiveClobTokenId);
    req.interval = polymarket::clob::PriceHistoryInterval::OneHour;
    req.fidelity = 60;
    const auto history = client_->fetch_prices_history(req);

    ASSERT_FALSE(history.history.empty());
    EXPECT_GT(history.history[0].t, 0);
    EXPECT_GT(history.history[0].p, 0.0);
    EXPECT_EQ(server_.last_request().target,
              "/prices-history?market=" + std::string(kActiveClobTokenId) + "&interval=1h&fidelity=60");
}

TEST_F(ClobClientTest, FetchFeeRateParsesCapturedFeeRate) {
    server_.enqueue({200, fixture(
        "clob/fee_rate_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto fee_rate = client_->fetch_fee_rate(std::string(kActiveClobTokenId));

    EXPECT_EQ(fee_rate, 0u);
    EXPECT_EQ(server_.last_request().target,
              "/fee-rate?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchTickSizeParsesCapturedTickSize) {
    server_.enqueue({200, fixture(
        "clob/tick_size_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto tick_size = client_->fetch_tick_size(std::string(kActiveClobTokenId));

    EXPECT_GT(tick_size, 0.0);
    EXPECT_EQ(server_.last_request().target,
              "/tick-size?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchNegRiskParsesCapturedNegRisk) {
    server_.enqueue({200, fixture(
        "clob/neg_risk_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto neg_risk = client_->fetch_neg_risk(std::string(kActiveClobTokenId));

    EXPECT_FALSE(neg_risk);
    EXPECT_EQ(server_.last_request().target,
              "/neg-risk?token_id=" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchClobMarketInfoParsesCapturedCompactMarket) {
    server_.enqueue({200, fixture(
        "clob/clob_market_info_0x004230fb1f54a139d50ba2041e062a01461c931a6725ce482e3a9ab61b2925bb.json")});

    const auto market = client_->fetch_clob_market_info(std::string(kActiveClobConditionId));

    EXPECT_EQ(market.condition_id, kActiveClobConditionId);
    EXPECT_DOUBLE_EQ(market.min_order_size, 5.0);
    EXPECT_DOUBLE_EQ(market.min_tick_size, 0.01);
    ASSERT_EQ(market.tokens.size(), 2u);
    EXPECT_EQ(market.tokens[0].token_id, kActiveClobTokenId);
    EXPECT_EQ(market.tokens[1].token_id, kActiveClobNoTokenId);
    EXPECT_EQ(server_.last_request().target,
              "/clob-markets/" + std::string(kActiveClobConditionId));
}

TEST_F(ClobClientTest, FetchMarketParsesCapturedApiMarket) {
    server_.enqueue({200, fixture(
        "clob/market_0x004230fb1f54a139d50ba2041e062a01461c931a6725ce482e3a9ab61b2925bb.json")});

    const auto market = client_->fetch_market(std::string(kActiveClobConditionId));

    EXPECT_EQ(market.condition_id, kActiveClobConditionId);
    EXPECT_TRUE(market.enable_order_book);
    EXPECT_TRUE(market.active);
    EXPECT_FALSE(market.closed);
    ASSERT_EQ(market.tokens.size(), 2u);
    EXPECT_EQ(market.tokens[0].token_id, kActiveClobTokenId);
    EXPECT_EQ(server_.last_request().target, "/markets/" + std::string(kActiveClobConditionId));
}

TEST_F(ClobClientTest, FetchMarketByTokenParsesCapturedApiRelationship) {
    server_.enqueue({200, fixture(
        "clob/market_by_token_69422147515888934539342749343952767069189843320299332157580167457035825622340.json")});

    const auto market = client_->fetch_market_by_token(std::string(kActiveClobTokenId));

    EXPECT_EQ(market.condition_id, kActiveClobConditionId);
    EXPECT_EQ(market.primary_token_id, kActiveClobNoTokenId);
    EXPECT_EQ(market.secondary_token_id, kActiveClobTokenId);
    EXPECT_EQ(server_.last_request().target,
              "/markets-by-token/" + std::string(kActiveClobTokenId));
}

TEST_F(ClobClientTest, FetchMarketTradesEventsReturnsCapturedJson) {
    server_.enqueue({200, fixture(
        "clob/market_live_activity_0x004230fb1f54a139d50ba2041e062a01461c931a6725ce482e3a9ab61b2925bb.json")});

    const auto activity = client_->fetch_market_trades_events(std::string(kActiveClobConditionId));

    ASSERT_TRUE(activity.is_object());
    EXPECT_EQ(boost::json::value_to<std::string>(activity.as_object().at("condition_id")),
              kActiveClobConditionId);
    EXPECT_EQ(server_.last_request().target,
              "/markets/live-activity/" + std::string(kActiveClobConditionId));
}

TEST_F(ClobClientTest, FetchMarketsParsesCapturedPage) {
    server_.enqueue({200, fixture("clob/markets_page.json")});

    const auto page = client_->fetch_markets();

    ASSERT_FALSE(page.data.empty());
    EXPECT_EQ(page.count, page.data.size());
    EXPECT_FALSE(page.next_cursor.empty());
    EXPECT_EQ(server_.last_request().target, "/markets");
}

TEST_F(ClobClientTest, FetchSamplingMarketsParsesCapturedPage) {
    server_.enqueue({200, fixture("clob/sampling_markets_page.json")});

    const auto page = client_->fetch_sampling_markets();

    ASSERT_FALSE(page.data.empty());
    EXPECT_EQ(page.count, page.data.size());
    EXPECT_FALSE(page.next_cursor.empty());
    EXPECT_EQ(server_.last_request().target, "/sampling-markets");
}

TEST_F(ClobClientTest, FetchSimplifiedMarketsParsesCapturedPage) {
    server_.enqueue({200, fixture("clob/simplified_markets_page.json")});

    const auto page = client_->fetch_simplified_markets();

    ASSERT_FALSE(page.data.empty());
    EXPECT_EQ(page.count, page.data.size());
    EXPECT_FALSE(page.next_cursor.empty());
    EXPECT_EQ(server_.last_request().target, "/simplified-markets");
}

TEST_F(ClobClientTest, FetchSamplingSimplifiedMarketsParsesCapturedPage) {
    server_.enqueue({200, fixture("clob/sampling_simplified_markets_page.json")});

    const auto page = client_->fetch_sampling_simplified_markets();

    ASSERT_FALSE(page.data.empty());
    EXPECT_EQ(page.count, page.data.size());
    EXPECT_FALSE(page.next_cursor.empty());
    EXPECT_EQ(server_.last_request().target, "/sampling-simplified-markets");
}

TEST_F(ClobClientTest, FetchMarketsAppendsCursorWhenProvided) {
    server_.enqueue({200, fixture("clob/markets_page_empty_lte.json")});

    client_->fetch_markets("CURSOR");

    EXPECT_EQ(server_.last_request().target, "/markets?next_cursor=CURSOR");
}

TEST_F(ClobClientTest, FetchAllMarketsDrainsCapturedPages) {
    server_.enqueue({200, fixture("clob/markets_page.json")});
    server_.enqueue({200, fixture("clob/markets_page_empty_lte.json")});

    const auto markets = client_->fetch_all_markets();

    ASSERT_FALSE(markets.empty());
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[0].target, "/markets");
    EXPECT_EQ(server_.requests()[1].target, "/markets?next_cursor=MTAwMA==");
}

TEST_F(ClobClientTest, FetchAllSamplingMarketsDrainsCapturedPages) {
    server_.enqueue({200, fixture("clob/sampling_markets_page.json")});
    server_.enqueue({200, fixture("clob/markets_page_empty_lte.json")});

    const auto markets = client_->fetch_all_sampling_markets();

    ASSERT_FALSE(markets.empty());
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[0].target, "/sampling-markets");
    EXPECT_EQ(server_.requests()[1].target, "/sampling-markets?next_cursor=MTAwMA==");
}

TEST_F(ClobClientTest, FetchAllSimplifiedMarketsDrainsCapturedPages) {
    server_.enqueue({200, fixture("clob/simplified_markets_page.json")});
    server_.enqueue({200, fixture("clob/simplified_markets_page_empty_lte.json")});

    const auto markets = client_->fetch_all_simplified_markets();

    ASSERT_FALSE(markets.empty());
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[0].target, "/simplified-markets");
    EXPECT_EQ(server_.requests()[1].target, "/simplified-markets?next_cursor=MTAwMA==");
}

TEST_F(ClobClientTest, FetchAllSamplingSimplifiedMarketsDrainsCapturedPages) {
    server_.enqueue({200, fixture("clob/sampling_simplified_markets_page.json")});
    server_.enqueue({200, fixture("clob/simplified_markets_page_empty_lte.json")});

    const auto markets = client_->fetch_all_sampling_simplified_markets();

    ASSERT_FALSE(markets.empty());
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[0].target, "/sampling-simplified-markets");
    EXPECT_EQ(server_.requests()[1].target,
              "/sampling-simplified-markets?next_cursor=MTAwMA==");
}

TEST_F(ClobClientTest, CheckGeoblockUsesConfiguredPolymarketHostAndRestoresClobHost) {
    server_.enqueue({200, fixture("clob/geoblock.json")});
    server_.enqueue({200, fixture("clob/version.json")});

    const auto geoblock = client_->check_geoblock();
    const auto version = client_->fetch_version();

    EXPECT_TRUE(geoblock.blocked);
    EXPECT_EQ(geoblock.country, "US");
    EXPECT_EQ(version, 1u);
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[0].target, "/api/geoblock");
    EXPECT_EQ(server_.requests()[1].target, "/version");
}

TEST_F(ClobClientTest, FetchCurrentMakerRebatesTreatsCapturedNullAsEmpty) {
    server_.enqueue({200, fixture("clob/maker_rebates_null.json")});

    const auto rebates = client_->fetch_current_maker_rebates(
        "2026-04-01", std::string(kPublicProfileAddress));

    EXPECT_TRUE(rebates.empty());
    EXPECT_EQ(server_.last_request().target,
              "/rebates/current?date=2026-04-01&maker_address=" + std::string(kPublicProfileAddress));
}

class DataClientTest : public ::testing::Test {
protected:
    void SetUp() override {
        server_.start();
        config_.data_api_url = server_.base_url();
        config_.http_timeout_ms = 2000;
        client_ = std::make_unique<DataClient>(config_);
    }

    void TearDown() override {
        client_.reset();
        server_.stop();
    }

    MockHttpServer server_;
    APIConfig config_;
    std::unique_ptr<DataClient> client_;
};

TEST_F(DataClientTest, HealthParsesCapturedApiResponse) {
    server_.enqueue({200, fixture("data/health.json")});

    const auto health = client_->health();

    EXPECT_EQ(health.data, "OK");
    EXPECT_EQ(server_.last_request().method, "GET");
    EXPECT_EQ(server_.last_request().target, "/");
}

TEST_F(DataClientTest, OpenInterestParsesCapturedApiResponse) {
    server_.enqueue({200, fixture(
        "data/open_interest_0x5eed579ff6763914d78a966c83473ba2485ac8910d0a0914eef6d9fcb33085de.json")});

    polymarket::data::OpenInterestRequest req;
    req.markets = {std::string(kClobConditionId)};
    const auto open_interest = client_->open_interest(req);

    ASSERT_EQ(open_interest.size(), 1u);
    EXPECT_EQ(open_interest[0].market, kClobConditionId);
    EXPECT_DOUBLE_EQ(open_interest[0].value, 0.0);
    EXPECT_EQ(server_.last_request().target, "/oi?market=" + std::string(kClobConditionId));
}

TEST_F(DataClientTest, LiveVolumeParsesCapturedApiResponse) {
    server_.enqueue({200, fixture("data/live_volume_2890.json")});

    polymarket::data::LiveVolumeRequest req;
    req.id = kLiveEventId;
    const auto volumes = client_->live_volume(req);

    ASSERT_EQ(volumes.size(), 1u);
    EXPECT_DOUBLE_EQ(volumes[0].total, 0.0);
    EXPECT_TRUE(volumes[0].markets.empty());
    EXPECT_EQ(server_.last_request().target, "/live-volume?id=2890");
}

}  // namespace
