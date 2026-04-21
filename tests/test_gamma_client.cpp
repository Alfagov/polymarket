#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <string_view>

#include "gamma_client.h"
#include "mock_http_server.h"
#include "types.h"

namespace {

using polymarket::APIConfig;
using polymarket::gamma::GammaClient;
using polymarket::gamma::Parameters;
using polymarket::test::CannedResponse;
using polymarket::test::CapturedRequest;
using polymarket::test::MockHttpServer;

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

    Parameters p;
    auto events = client_->fetch_events(p);

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].id, "1");
    EXPECT_EQ(events[0].slug, "a");
    EXPECT_EQ(events[1].id, "2");

    const auto& req = server_.last_request();
    EXPECT_EQ(req.method, "GET");
    EXPECT_TRUE(target_has(req.target, "/events/keyset"));
    EXPECT_TRUE(target_has(req.target, "limit=1000"));
}

TEST_F(GammaClientTest, FetchEventsAppendsAfterCursorWhenProvided) {
    server_.enqueue({200, R"([])"});

    Parameters p;
    client_->fetch_events(p, "abc123");

    const auto& req = server_.last_request();
    EXPECT_TRUE(target_has(req.target, "after_cursor=abc123"))
        << "target was: " << req.target;
}

TEST_F(GammaClientTest, FetchEventsThrowsOnNon2xxStatus) {
    server_.enqueue({500, R"({"error":"boom"})"});

    Parameters p;
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

    Parameters p;
    auto events = client_->fetch_all_events(p);

    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events[0].id, "1");
    EXPECT_EQ(events[1].id, "2");
    EXPECT_EQ(server_.request_count(), 2u);
}

TEST_F(GammaClientTest, FetchAllEventsStopsOnEmptyPage) {
    server_.enqueue({200, R"({"events":[],"next_cursor":"ignored"})"});

    Parameters p;
    auto events = client_->fetch_all_events(p);

    EXPECT_TRUE(events.empty());
    EXPECT_EQ(server_.request_count(), 1u);
}

TEST_F(GammaClientTest, FetchEventByIdHitsCorrectPath) {
    server_.enqueue({200, R"({"id":"42","slug":"life"})"});

    auto ev = client_->fetch_event_by_id(42);

    EXPECT_EQ(ev.id, "42");
    EXPECT_EQ(ev.slug, "life");
    EXPECT_EQ(server_.last_request().target, "/events/42");
}

TEST_F(GammaClientTest, FetchEventBySlugHitsCorrectPath) {
    server_.enqueue({200, R"({"id":"7","slug":"will-it-rain"})"});

    auto ev = client_->fetch_event_by_slug("will-it-rain");

    EXPECT_EQ(ev.id, "7");
    EXPECT_EQ(server_.last_request().target, "/events/slug/will-it-rain");
}

TEST_F(GammaClientTest, FetchEventsTagsHitsCorrectPathAndParses) {
    server_.enqueue({200,
                     R"([{"id":"10","label":"Politics","slug":"politics"},
                         {"id":"11","label":"Sports","slug":"sports"}])"});

    auto tags = client_->fetch_events_tags(99);

    ASSERT_EQ(tags.size(), 2u);
    EXPECT_EQ(tags[0].label, "Politics");
    EXPECT_EQ(tags[1].slug, "sports");
    EXPECT_EQ(server_.last_request().target, "/events/99/tags");
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

    Parameters p;
    auto markets = client_->fetch_markets(p);

    ASSERT_EQ(markets.size(), 1u);
    EXPECT_EQ(markets[0].id, "m1");
    EXPECT_TRUE(target_has(server_.last_request().target, "/markets/keyset"));
}

TEST_F(GammaClientTest, FetchMarketsPassesCursor) {
    server_.enqueue({200, R"([])"});

    Parameters p;
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

    Parameters p;
    auto markets = client_->fetch_all_markets(p);
    EXPECT_EQ(markets.size(), 3u);
    EXPECT_EQ(server_.request_count(), 2u);
}

TEST_F(GammaClientTest, FetchMarketByIdHitsPath) {
    server_.enqueue({200, R"({"id":"0xabc","slug":"trump"})"});

    auto m = client_->fetch_market_by_id("0xabc");

    EXPECT_EQ(m.id, "0xabc");
    EXPECT_EQ(server_.last_request().target, "/markets/0xabc");
}

TEST_F(GammaClientTest, FetchMarketBySlugHitsPath) {
    server_.enqueue({200, R"({"id":"m","slug":"foo"})"});

    auto m = client_->fetch_market_by_slug("foo");

    EXPECT_EQ(m.slug, "foo");
    EXPECT_EQ(server_.last_request().target, "/markets/slug/foo");
}

TEST_F(GammaClientTest, FetchMarketTagsByIdParsesTags) {
    server_.enqueue({200, R"([{"id":"1","label":"A","slug":"a"}])"});

    auto tags = client_->fetch_market_tags_by_id("mkt1");

    ASSERT_EQ(tags.size(), 1u);
    EXPECT_EQ(tags[0].label, "A");
    EXPECT_EQ(server_.last_request().target, "/markets/mkt1/tags");
}

// ---------------------------------------------------------------------------
// Data-API endpoints (holders, open interest, live volume)
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchMarketTopHoldersBuildsQueryAndParses) {
    server_.enqueue({200, R"([{"token":"t1","holders":[
        {"proxyWallet":"0x1","amount":10.5,"outcomeIndex":0}
    ]}])"});

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
    server_.enqueue({200, R"([])"});
    client_->fetch_market_top_holders({"tok"});

    const auto& req = server_.last_request();
    EXPECT_TRUE(target_has(req.target, "limit=20"));
    EXPECT_TRUE(target_has(req.target, "minBalance=1"));
}

TEST_F(GammaClientTest, FetchMarketOpenInterestsParses) {
    server_.enqueue({200, R"([{"market":"m1","value":123.5},
                              {"market":"m2","value":0.0}])"});

    auto ois = client_->fetch_market_open_interests({"m1", "m2"});

    ASSERT_EQ(ois.size(), 2u);
    EXPECT_EQ(ois[0].market, "m1");
    EXPECT_DOUBLE_EQ(ois[0].value, 123.5);
    EXPECT_TRUE(target_has(server_.last_request().target, "/oi?market=m1,m2"));
}

TEST_F(GammaClientTest, FetchEventLiveVolumesParses) {
    server_.enqueue({200, R"([{"total":42.0,"markets":[
        {"market":"mm","value":1.0}
    ]}])"});

    auto vols = client_->fetch_event_live_volumes(7);

    ASSERT_EQ(vols.size(), 1u);
    EXPECT_DOUBLE_EQ(vols[0].total, 42.0);
    ASSERT_EQ(vols[0].markets.size(), 1u);
    EXPECT_EQ(vols[0].markets[0].market, "mm");
    EXPECT_EQ(server_.last_request().target, "/live-volume?id=7");
}

// ---------------------------------------------------------------------------
// Base-URL restoration: after a data-API call (success OR failure) the next
// gamma-API call must still land on the gamma endpoint.
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, DataApiCallDoesNotCorruptBaseUrlOnSuccess) {
    // Both point to the same mock, but the test still validates that a
    // subsequent /events path is reachable and reaches a /events/... target.
    server_.enqueue({200, R"([])"});              // holders
    server_.enqueue({200, R"({"id":"1"})"});      // event-by-id

    client_->fetch_market_top_holders({"tok"});
    auto ev = client_->fetch_event_by_id(1);

    EXPECT_EQ(ev.id, "1");
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[1].target, "/events/1");
}

TEST_F(GammaClientTest, DataApiCallDoesNotCorruptBaseUrlOnFailure) {
    server_.enqueue({502, "bad gateway"});        // holders fails
    server_.enqueue({200, R"({"id":"9"})"});      // event-by-id succeeds

    EXPECT_THROW(client_->fetch_market_top_holders({"tok"}), std::runtime_error);

    auto ev = client_->fetch_event_by_id(9);
    EXPECT_EQ(ev.id, "9");
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[1].target, "/events/9");
}

// ---------------------------------------------------------------------------
// Query-string construction (via observable side effects in request target)
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, ParametersAreSerializedIntoQueryString) {
    server_.enqueue({200, R"([])"});

    Parameters p;
    p.limit = 25;
    p.order = {"volume"};
    p.ascending = true;
    p.closed = false;
    p.slug = {"foo", "bar"};
    p.id = {1, 2, 3};
    p.liquidity_num_min = 100;
    p.volume_num_max = 9999;
    p.start_date_min = "2026-01-01";
    p.end_date_max = "2026-12-31";
    p.tag_id = {"politics"};
    p.related_tags = true;
    p.include_tag = false;
    p.game_id = "NBA-001";

    client_->fetch_events(p);

    const auto& t = server_.last_request().target;
    EXPECT_TRUE(target_has(t, "limit=25")) << t;
    EXPECT_TRUE(target_has(t, "order=volume")) << t;
    EXPECT_TRUE(target_has(t, "ascending=true")) << t;
    EXPECT_TRUE(target_has(t, "closed=false")) << t;
    EXPECT_TRUE(target_has(t, "slug=foo,bar")) << t;
    EXPECT_TRUE(target_has(t, "id=1,2,3")) << t;
    EXPECT_TRUE(target_has(t, "liquidity_num_min=100")) << t;
    EXPECT_TRUE(target_has(t, "volume_num_max=9999")) << t;
    EXPECT_TRUE(target_has(t, "start_date_min=2026-01-01")) << t;
    EXPECT_TRUE(target_has(t, "end_date_max=2026-12-31")) << t;
    EXPECT_TRUE(target_has(t, "tag_id=politics")) << t;
    EXPECT_TRUE(target_has(t, "related_tags=true")) << t;
    EXPECT_TRUE(target_has(t, "include_tag=false")) << t;
    EXPECT_TRUE(target_has(t, "game_id=NBA-001")) << t;
}

TEST_F(GammaClientTest, OptionalParametersOmittedWhenUnset) {
    server_.enqueue({200, R"([])"});

    Parameters p;  // defaults: only limit set
    client_->fetch_events(p);

    const auto& t = server_.last_request().target;
    EXPECT_TRUE(target_has(t, "limit=1000"));
    EXPECT_FALSE(target_has(t, "ascending="));
    EXPECT_FALSE(target_has(t, "closed="));
    EXPECT_FALSE(target_has(t, "liquidity_num_min="));
    EXPECT_FALSE(target_has(t, "slug="));
    EXPECT_FALSE(target_has(t, "id="));
    EXPECT_FALSE(target_has(t, "start_date_min="));
}

// ---------------------------------------------------------------------------
// Error surfacing
// ---------------------------------------------------------------------------

TEST_F(GammaClientTest, FetchAllMarketsPropagatesError) {
    server_.enqueue({503, "unavailable"});
    Parameters p;
    EXPECT_THROW(client_->fetch_all_markets(p), std::runtime_error);
}

TEST_F(GammaClientTest, FetchEventsTagsThrowsOnError) {
    server_.enqueue({500, "{}"});
    EXPECT_THROW(client_->fetch_events_tags(1), std::runtime_error);
}

}  // namespace
