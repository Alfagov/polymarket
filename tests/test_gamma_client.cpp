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
using polymarket::gamma::MarketParameters;
using polymarket::gamma::EventParameters;
using polymarket::test::CannedResponse;
using polymarket::test::CapturedRequest;
using polymarket::test::MockHttpServer;

constexpr int kLiveEventId = 2890;
constexpr std::string_view kLiveEventSlug =
    "nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup";
constexpr std::string_view kLiveMarketId = "239826";

constexpr std::string_view kLiveEventResponse = R"json({"id":"2890","ticker":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","slug":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","title":"NBA: Will the Mavericks beat the Grizzlies by more than 5.5 points in their December 4 matchup?","description":"In the upcoming NBA game, scheduled for December 4:\n\nIf the Dallas Mavericks win by over 5.5 points, the market will resolve to \u201cYes\u201d.\n\nIf the Memphis Grizzlies lose by less than 5.5 points or win, the market will resolve \u201cNo.\u201d \n\nIf the game is not completed by December 11, 2021, the market will resolve 50-50.","resolutionSource":"https://www.nba.com/games","startDate":"2021-12-04T00:00:00Z","creationDate":"2021-12-04T00:00:00Z","endDate":"2021-12-04T00:00:00Z","image":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-543e7263-67da-4905-8732-cd3f220ae751.png","icon":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-543e7263-67da-4905-8732-cd3f220ae751.png","active":true,"closed":true,"archived":false,"new":false,"featured":false,"restricted":false,"liquidity":0,"volume":1335.05,"openInterest":0,"sortBy":"ascending","category":"Sports","published_at":"2022-07-27 14:40:02.064+00","createdAt":"2022-07-27T14:40:02.074Z","updatedAt":"2026-04-15T17:43:00.69317Z","competitive":0,"volume24hr":0,"volume1wk":0,"volume1mo":0,"volume1yr":0,"liquidityAmm":0,"liquidityClob":0,"commentCount":8125,"markets":[{"id":"239826","question":"NBA: Will the Mavericks beat the Grizzlies by more than 5.5 points in their December 4 matchup?","conditionId":"0x064d33e3f5703792aafa92bfb0ee10e08f461b1b34c02c1f02671892ede1609a","slug":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","resolutionSource":"https://www.nba.com/games","endDate":"2021-12-04T00:00:00Z","category":"Sports","liquidity":"50.000009","startDate":"2021-12-04T19:35:03.796Z","fee":"20000000000000000","image":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-aa2992d1-38df-404a-9190-49a909775014.png","icon":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-aa2992d1-38df-404a-9190-49a909775014.png","description":"In the upcoming NBA game, scheduled for December 4:\n\nIf the Dallas Mavericks win by over 5.5 points, the market will resolve to \u201cYes\u201d.\n\nIf the Memphis Grizzlies lose by less than 5.5 points or win, the market will resolve \u201cNo.\u201d \n\nIf the game is not completed by December 11, 2021, the market will resolve 50-50.","outcomes":"[\"Yes\", \"No\"]","outcomePrices":"[\"0.0000004113679809846114013590098187297978\", \"0.9999995886320190153885986409901813\"]","volume":"1335.045385","active":true,"marketType":"normal","closed":true,"marketMakerAddress":"0x9c568Ce9a316e7CF9bCCA352b409dfDdCD9b2C08","updatedBy":15,"createdAt":"2021-12-04T10:33:13.541Z","updatedAt":"2024-04-24T23:35:51.063381Z","closedTime":"2021-12-05 20:37:01+00","wideFormat":false,"new":false,"sentDiscord":false,"featured":false,"submitted_by":"0x790A4485e5198763C0a34272698ed0cd9506949B","twitterCardLocation":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup.png?1638736245595","twitterCardLastRefreshed":"1638736245595","archived":false,"resolvedBy":"0x0dD333859cF16942dd333D7570D839b8946Ac221","restricted":false,"volumeNum":1335.05,"liquidityNum":50,"endDateIso":"2021-12-04","startDateIso":"2021-12-04","hasReviewedDates":true,"readyForCron":true,"volume24hr":0,"volume1wk":0,"volume1mo":0,"volume1yr":0,"clobTokenIds":"[\"28182404005967940652495463228537840901055649726248190462854914416579180110833\", \"47044845753450022047436429968808601130811164131571549682541703866165095016290\"]","fpmmLive":true,"volume1wkAmm":0,"volume1moAmm":0,"volume1yrAmm":0,"volume1wkClob":0,"volume1moClob":0,"volume1yrClob":0,"creator":"","ready":false,"funded":false,"cyom":false,"competitive":0,"pagerDutyNotificationEnabled":false,"approved":true,"rewardsMinSize":0,"rewardsMaxSpread":0,"spread":1,"oneDayPriceChange":0,"oneHourPriceChange":0,"oneWeekPriceChange":0,"oneMonthPriceChange":0,"oneYearPriceChange":0,"lastTradePrice":0,"bestBid":0,"bestAsk":1,"clearBookOnStart":true,"manualActivation":false,"negRiskOther":false,"umaResolutionStatuses":"[]","pendingDeployment":false,"deploying":false,"rfqEnabled":false,"holdingRewardsEnabled":false,"feesEnabled":false,"requiresTranslation":false,"feeType":null}],"series":[{"id":"2","ticker":"nba","slug":"nba","title":"NBA","seriesType":"single","recurrence":"daily","image":"https://polymarket-upload.s3.us-east-2.amazonaws.com/super+cool+basketball+in+red+and+blue+wow.png","icon":"https://polymarket-upload.s3.us-east-2.amazonaws.com/super+cool+basketball+in+red+and+blue+wow.png","layout":"default","active":true,"closed":false,"archived":false,"new":false,"featured":false,"restricted":true,"publishedAt":"2023-01-30 17:13:39.006+00","createdBy":"15","updatedBy":"15","createdAt":"2022-10-13T00:36:01.131Z","updatedAt":"2026-04-21T20:27:45.300704Z","commentsEnabled":false,"competitive":"0","volume24hr":11.073004,"startDate":"2021-01-01T17:00:00Z","commentCount":6274,"requiresTranslation":false}],"tags":[{"id":"100215","label":"All","slug":"all","forceShow":false,"updatedAt":"2026-04-17T20:23:54.340488Z","requiresTranslation":false}],"cyom":false,"closedTime":"2022-07-27T14:40:02.074Z","showAllOutcomes":false,"showMarketImages":true,"enableNegRisk":false,"seriesSlug":"nba","negRiskAugmented":false,"pendingDeployment":false,"deploying":false,"requiresTranslation":false,"eventMetadata":{"context_requires_regen":true}})json";
constexpr std::string_view kLiveEventTagsResponse =
    R"json([{"id":"100215","label":"All","slug":"all","forceShow":false,"updatedAt":"2026-04-17T20:23:54.340488Z","requiresTranslation":false}])json";
constexpr std::string_view kLiveMarketResponse = R"json({"id":"239826","question":"NBA: Will the Mavericks beat the Grizzlies by more than 5.5 points in their December 4 matchup?","conditionId":"0x064d33e3f5703792aafa92bfb0ee10e08f461b1b34c02c1f02671892ede1609a","slug":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","resolutionSource":"https://www.nba.com/games","endDate":"2021-12-04T00:00:00Z","category":"Sports","liquidity":"50.000009","startDate":"2021-12-04T19:35:03.796Z","fee":"20000000000000000","image":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-aa2992d1-38df-404a-9190-49a909775014.png","icon":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-aa2992d1-38df-404a-9190-49a909775014.png","description":"In the upcoming NBA game, scheduled for December 4:\n\nIf the Dallas Mavericks win by over 5.5 points, the market will resolve to \u201cYes\u201d.\n\nIf the Memphis Grizzlies lose by less than 5.5 points or win, the market will resolve \u201cNo.\u201d \n\nIf the game is not completed by December 11, 2021, the market will resolve 50-50.","outcomes":"[\"Yes\", \"No\"]","outcomePrices":"[\"0.0000004113679809846114013590098187297978\", \"0.9999995886320190153885986409901813\"]","volume":"1335.045385","active":true,"marketType":"normal","closed":true,"marketMakerAddress":"0x9c568Ce9a316e7CF9bCCA352b409dfDdCD9b2C08","updatedBy":15,"createdAt":"2021-12-04T10:33:13.541Z","updatedAt":"2024-04-24T23:35:51.063381Z","closedTime":"2021-12-05 20:37:01+00","wideFormat":false,"new":false,"sentDiscord":false,"featured":false,"submitted_by":"0x790A4485e5198763C0a34272698ed0cd9506949B","twitterCardLocation":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup.png?1638736245595","twitterCardLastRefreshed":"1638736245595","archived":false,"resolvedBy":"0x0dD333859cF16942dd333D7570D839b8946Ac221","restricted":false,"volumeNum":1335.05,"liquidityNum":50,"endDateIso":"2021-12-04","startDateIso":"2021-12-04","hasReviewedDates":true,"readyForCron":true,"volume24hr":0,"volume1wk":0,"volume1mo":0,"volume1yr":0,"clobTokenIds":"[\"28182404005967940652495463228537840901055649726248190462854914416579180110833\", \"47044845753450022047436429968808601130811164131571549682541703866165095016290\"]","fpmmLive":true,"volume1wkAmm":0,"volume1moAmm":0,"volume1yrAmm":0,"volume1wkClob":0,"volume1moClob":0,"volume1yrClob":0,"creator":"","ready":false,"funded":false,"cyom":false,"competitive":0,"pagerDutyNotificationEnabled":false,"approved":true,"rewardsMinSize":0,"rewardsMaxSpread":0,"spread":1,"oneDayPriceChange":0,"oneHourPriceChange":0,"oneWeekPriceChange":0,"oneMonthPriceChange":0,"oneYearPriceChange":0,"lastTradePrice":0,"bestBid":0,"bestAsk":1,"clearBookOnStart":true,"manualActivation":false,"negRiskOther":false,"umaResolutionStatuses":"[]","pendingDeployment":false,"deploying":false,"rfqEnabled":false,"holdingRewardsEnabled":false,"feesEnabled":false,"requiresTranslation":false,"feeType":null})json";
constexpr std::string_view kLiveMarketBySlugResponse = R"json({"id":"239826","question":"NBA: Will the Mavericks beat the Grizzlies by more than 5.5 points in their December 4 matchup?","conditionId":"0x064d33e3f5703792aafa92bfb0ee10e08f461b1b34c02c1f02671892ede1609a","slug":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","resolutionSource":"https://www.nba.com/games","endDate":"2021-12-04T00:00:00Z","category":"Sports","liquidity":"50.000009","startDate":"2021-12-04T19:35:03.796Z","fee":"20000000000000000","image":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-aa2992d1-38df-404a-9190-49a909775014.png","icon":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-aa2992d1-38df-404a-9190-49a909775014.png","description":"In the upcoming NBA game, scheduled for December 4:\n\nIf the Dallas Mavericks win by over 5.5 points, the market will resolve to \u201cYes\u201d.\n\nIf the Memphis Grizzlies lose by less than 5.5 points or win, the market will resolve \u201cNo.\u201d \n\nIf the game is not completed by December 11, 2021, the market will resolve 50-50.","outcomes":"[\"Yes\", \"No\"]","outcomePrices":"[\"0.0000004113679809846114013590098187297978\", \"0.9999995886320190153885986409901813\"]","volume":"1335.045385","active":true,"marketType":"normal","closed":true,"marketMakerAddress":"0x9c568Ce9a316e7CF9bCCA352b409dfDdCD9b2C08","updatedBy":15,"createdAt":"2021-12-04T10:33:13.541Z","updatedAt":"2024-04-24T23:35:51.063381Z","closedTime":"2021-12-05 20:37:01+00","wideFormat":false,"new":false,"sentDiscord":false,"featured":false,"submitted_by":"0x790A4485e5198763C0a34272698ed0cd9506949B","twitterCardLocation":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup.png?1638736245595","twitterCardLastRefreshed":"1638736245595","archived":false,"resolvedBy":"0x0dD333859cF16942dd333D7570D839b8946Ac221","restricted":false,"volumeNum":1335.05,"liquidityNum":50,"endDateIso":"2021-12-04","startDateIso":"2021-12-04","hasReviewedDates":true,"readyForCron":true,"volume24hr":0,"volume1wk":0,"volume1mo":0,"volume1yr":0,"clobTokenIds":"[\"28182404005967940652495463228537840901055649726248190462854914416579180110833\", \"47044845753450022047436429968808601130811164131571549682541703866165095016290\"]","fpmmLive":true,"volume1wkAmm":0,"volume1moAmm":0,"volume1yrAmm":0,"volume1wkClob":0,"volume1moClob":0,"volume1yrClob":0,"events":[{"id":"2890","ticker":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","slug":"nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup","title":"NBA: Will the Mavericks beat the Grizzlies by more than 5.5 points in their December 4 matchup?","description":"In the upcoming NBA game, scheduled for December 4:\n\nIf the Dallas Mavericks win by over 5.5 points, the market will resolve to \u201cYes\u201d.\n\nIf the Memphis Grizzlies lose by less than 5.5 points or win, the market will resolve \u201cNo.\u201d \n\nIf the game is not completed by December 11, 2021, the market will resolve 50-50.","resolutionSource":"https://www.nba.com/games","startDate":"2021-12-04T00:00:00Z","creationDate":"2021-12-04T00:00:00Z","endDate":"2021-12-04T00:00:00Z","image":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-543e7263-67da-4905-8732-cd3f220ae751.png","icon":"https://polymarket-upload.s3.us-east-2.amazonaws.com/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-55-points-in-their-december-4-matchup-543e7263-67da-4905-8732-cd3f220ae751.png","active":true,"closed":true,"archived":false,"new":false,"featured":false,"restricted":false,"liquidity":0,"volume":1335.05,"openInterest":0,"sortBy":"ascending","category":"Sports","published_at":"2022-07-27 14:40:02.064+00","createdAt":"2022-07-27T14:40:02.074Z","updatedAt":"2026-04-15T17:43:00.69317Z","competitive":0,"volume24hr":0,"volume1wk":0,"volume1mo":0,"volume1yr":0,"liquidityAmm":0,"liquidityClob":0,"commentCount":8125,"cyom":false,"closedTime":"2022-07-27T14:40:02.074Z","showAllOutcomes":false,"showMarketImages":true,"enableNegRisk":false,"seriesSlug":"nba","negRiskAugmented":false,"pendingDeployment":false,"deploying":false,"requiresTranslation":false,"eventMetadata":{"context_requires_regen":true}}],"creator":"","ready":false,"funded":false,"cyom":false,"competitive":0,"pagerDutyNotificationEnabled":false,"approved":true,"rewardsMinSize":0,"rewardsMaxSpread":0,"spread":1,"oneDayPriceChange":0,"oneHourPriceChange":0,"oneWeekPriceChange":0,"oneMonthPriceChange":0,"oneYearPriceChange":0,"lastTradePrice":0,"bestBid":0,"bestAsk":1,"clearBookOnStart":true,"manualActivation":false,"negRiskOther":false,"umaResolutionStatuses":"[]","pendingDeployment":false,"deploying":false,"rfqEnabled":false,"holdingRewardsEnabled":false,"feesEnabled":false,"requiresTranslation":false,"feeType":null})json";
constexpr std::string_view kLiveMarketTagsResponse =
    R"json([{"id":"100215","label":"All","slug":"all","forceShow":false,"updatedAt":"2026-04-17T20:23:54.340488Z","requiresTranslation":false}])json";

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
    server_.enqueue({200, std::string(kLiveEventResponse)});

    auto ev = client_->fetch_event_by_id(kLiveEventId);

    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    EXPECT_EQ(ev.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target, "/events/2890");
}

TEST_F(GammaClientTest, FetchEventBySlugHitsCorrectPath) {
    server_.enqueue({200, std::string(kLiveEventResponse)});

    auto ev = client_->fetch_event_by_slug(std::string(kLiveEventSlug));

    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    EXPECT_EQ(ev.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target,
              "/events/slug/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup");
}

TEST_F(GammaClientTest, FetchEventsTagsHitsCorrectPathAndParses) {
    server_.enqueue({200, std::string(kLiveEventTagsResponse)});

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
    server_.enqueue({200, std::string(kLiveMarketResponse)});

    auto m = client_->fetch_market_by_id(std::string(kLiveMarketId));

    EXPECT_EQ(m.id, kLiveMarketId);
    EXPECT_EQ(m.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target, "/markets/239826");
}

TEST_F(GammaClientTest, FetchMarketBySlugHitsPath) {
    server_.enqueue({200, std::string(kLiveMarketBySlugResponse)});

    auto m = client_->fetch_market_by_slug(std::string(kLiveEventSlug));

    EXPECT_EQ(m.id, kLiveMarketId);
    EXPECT_EQ(m.slug, kLiveEventSlug);
    EXPECT_EQ(server_.last_request().target,
              "/markets/slug/nba-will-the-mavericks-beat-the-grizzlies-by-more-than-5pt5-points-in-their-december-4-matchup");
}

TEST_F(GammaClientTest, FetchMarketTagsByIdParsesTags) {
    server_.enqueue({200, std::string(kLiveMarketTagsResponse)});

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
    server_.enqueue({200, std::string(kLiveEventResponse)});      // event-by-id

    client_->fetch_market_top_holders({"tok"});
    auto ev = client_->fetch_event_by_id(kLiveEventId);

    EXPECT_EQ(ev.id, std::to_string(kLiveEventId));
    ASSERT_EQ(server_.request_count(), 2u);
    EXPECT_EQ(server_.requests()[1].target, "/events/2890");
}

TEST_F(GammaClientTest, DataApiCallDoesNotCorruptBaseUrlOnFailure) {
    server_.enqueue({502, "bad gateway"});        // holders fails
    server_.enqueue({200, std::string(kLiveEventResponse)});      // event-by-id succeeds

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
    p.order = {"volume"};
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

}  // namespace
