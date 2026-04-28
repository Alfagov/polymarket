#include <gtest/gtest.h>

#include <string>
#include <variant>

#include <boost/json.hpp>

#include "ws_client.h"

namespace {

using namespace polymarket::ws;

TEST(WsTypesTest, SubscriptionRequestSerializesDocumentedShape) {
    const MarketSubscriptionRequest request{
        .assets_ids = {"asset-a", "asset-b"},
        .initial_dump = true,
        .level = 2,
        .custom_feature_enabled = true,
    };

    const auto parsed = boost::json::parse(serialize_subscription_request(request));
    const auto& obj = parsed.as_object();

    EXPECT_EQ(boost::json::value_to<std::string>(obj.at("type")), "market");
    ASSERT_TRUE(obj.at("assets_ids").is_array());
    EXPECT_EQ(obj.at("assets_ids").as_array().size(), 2u);
    EXPECT_EQ(boost::json::value_to<std::string>(obj.at("assets_ids").as_array()[0]), "asset-a");
    EXPECT_TRUE(obj.at("initial_dump").as_bool());
    EXPECT_EQ(obj.at("level").as_int64(), 2);
    EXPECT_TRUE(obj.at("custom_feature_enabled").as_bool());
}

TEST(WsTypesTest, SubscriptionUpdateSerializesDocumentedShape) {
    const MarketSubscriptionUpdate update{
        .operation = SubscriptionOperation::Unsubscribe,
        .assets_ids = {"asset-a"},
        .level = 3,
        .custom_feature_enabled = false,
    };

    const auto parsed = boost::json::parse(serialize_subscription_update(update));
    const auto& obj = parsed.as_object();

    EXPECT_EQ(boost::json::value_to<std::string>(obj.at("operation")), "unsubscribe");
    EXPECT_EQ(boost::json::value_to<std::string>(obj.at("assets_ids").as_array()[0]), "asset-a");
    EXPECT_EQ(obj.at("level").as_int64(), 3);
    EXPECT_FALSE(obj.at("custom_feature_enabled").as_bool());
}

TEST(WsTypesTest, ParsesBookEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"book",
        "asset_id":"65818619657568813474341868652308942079804919287380422192892211131408793125422",
        "market":"0xbd31dc8a20211944f6b70f31557f1001557b59905b7738480ca09bd4532f84af",
        "bids":[{"price":"0.48","size":"30"}],
        "asks":[{"price":"0.52","size":"25"}],
        "timestamp":"1757908892351",
        "hash":"0xabc123"
    })");

    ASSERT_TRUE(std::holds_alternative<BookEvent>(event));
    const auto& book = std::get<BookEvent>(event);
    EXPECT_EQ(book.event_type, "book");
    EXPECT_EQ(book.bids[0].price, "0.48");
    EXPECT_EQ(book.asks[0].size, "25");
    EXPECT_EQ(market_event_type(event), "book");
}

TEST(WsTypesTest, ParsesPriceChangeEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"price_change",
        "market":"0x5f65177b394277fd294cd75650044e32ba009a95022d88a0c1d565897d72f8f1",
        "price_changes":[{
            "asset_id":"71321045679252212594626385532706912750332728571942532289631379312455583992563",
            "price":"0.5",
            "size":"200",
            "side":"BUY",
            "hash":"56621a121a47ed9333273e21c83b660cff37ae50",
            "best_bid":"0.5",
            "best_ask":"1"
        }],
        "timestamp":"1757908892351"
    })");

    ASSERT_TRUE(std::holds_alternative<PriceChangeEvent>(event));
    const auto& price_change = std::get<PriceChangeEvent>(event);
    ASSERT_EQ(price_change.price_changes.size(), 1u);
    EXPECT_EQ(price_change.price_changes[0].side, "BUY");
    ASSERT_TRUE(price_change.price_changes[0].best_bid);
    EXPECT_EQ(*price_change.price_changes[0].best_bid, "0.5");
}

TEST(WsTypesTest, ParsesLastTradePriceEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"last_trade_price",
        "asset_id":"114122071509644379678018727908709560226618148003371446110114509806601493071694",
        "market":"0x6a67b9d828d53862160e470329ffea5246f338ecfffdf2cab45211ec578b0347",
        "price":"0.456",
        "size":"219.217767",
        "fee_rate_bps":"0",
        "side":"BUY",
        "timestamp":"1750428146322",
        "transaction_hash":"0xeeefffggghhh"
    })");

    ASSERT_TRUE(std::holds_alternative<LastTradePriceEvent>(event));
    const auto& trade = std::get<LastTradePriceEvent>(event);
    EXPECT_EQ(trade.price, "0.456");
    ASSERT_TRUE(trade.transaction_hash);
    EXPECT_EQ(*trade.transaction_hash, "0xeeefffggghhh");
}

TEST(WsTypesTest, ParsesTickSizeChangeEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"tick_size_change",
        "asset_id":"65818619657568813474341868652308942079804919287380422192892211131408793125422",
        "market":"0xbd31dc8a20211944f6b70f31557f1001557b59905b7738480ca09bd4532f84af",
        "old_tick_size":"0.01",
        "new_tick_size":"0.001",
        "timestamp":"1757908892351"
    })");

    ASSERT_TRUE(std::holds_alternative<TickSizeChangeEvent>(event));
    EXPECT_EQ(std::get<TickSizeChangeEvent>(event).new_tick_size, "0.001");
}

TEST(WsTypesTest, ParsesBestBidAskEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"best_bid_ask",
        "market":"0x0005c0d312de0be897668695bae9f32b624b4a1ae8b140c49f08447fcc74f442",
        "asset_id":"85354956062430465315924116860125388538595433819574542752031640332592237464430",
        "best_bid":"0.73",
        "best_ask":"0.77",
        "spread":"0.04",
        "timestamp":"1766789469958"
    })");

    ASSERT_TRUE(std::holds_alternative<BestBidAskEvent>(event));
    EXPECT_EQ(std::get<BestBidAskEvent>(event).spread, "0.04");
}

TEST(WsTypesTest, ParsesNewMarketEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"new_market",
        "id":"1031769",
        "question":"Will NVIDIA (NVDA) close above $240 end of January?",
        "market":"0x311d0c4b6671ab54af4970c06fcf58662516f5168997bdda209ec3db5aa6b0c1",
        "slug":"nvda-above-240-on-january-30-2026",
        "description":"Market description",
        "assets_ids":["76043073756653678226373981964075571318267289248134717369284518995922789326425","31690934263385727664202099278545688007799199447969475608906331829650099442770"],
        "outcomes":["Yes","No"],
        "event_message":{"id":"125819","ticker":"nvda-above-in-january-2026","slug":"nvda-above-in-january-2026","title":"Will NVIDIA close above ___?","description":"Parent event"},
        "timestamp":"1766790415550",
        "tags":["stocks"],
        "condition_id":"0x311d0c4b6671ab54af4970c06fcf58662516f5168997bdda209ec3db5aa6b0c1",
        "active":true,
        "clob_token_ids":["76043073756653678226373981964075571318267289248134717369284518995922789326425","31690934263385727664202099278545688007799199447969475608906331829650099442770"],
        "sports_market_type":"",
        "line":"",
        "game_start_time":"",
        "order_price_min_tick_size":"0.01",
        "group_item_title":"NVDA above $240"
    })");

    ASSERT_TRUE(std::holds_alternative<NewMarketEvent>(event));
    const auto& market = std::get<NewMarketEvent>(event);
    EXPECT_EQ(market.assets_ids.size(), 2u);
    ASSERT_TRUE(market.event_message);
    ASSERT_TRUE(market.event_message->ticker);
    EXPECT_EQ(*market.event_message->ticker, "nvda-above-in-january-2026");
    ASSERT_TRUE(market.active);
    EXPECT_TRUE(*market.active);
}

TEST(WsTypesTest, ParsesMarketResolvedEvent) {
    const auto event = parse_market_event(R"({
        "event_type":"market_resolved",
        "id":"1031769",
        "market":"0x311d0c4b6671ab54af4970c06fcf58662516f5168997bdda209ec3db5aa6b0c1",
        "assets_ids":["76043073756653678226373981964075571318267289248134717369284518995922789326425","31690934263385727664202099278545688007799199447969475608906331829650099442770"],
        "winning_asset_id":"76043073756653678226373981964075571318267289248134717369284518995922789326425",
        "winning_outcome":"Yes",
        "timestamp":"1766790415550",
        "tags":["stocks"]
    })");

    ASSERT_TRUE(std::holds_alternative<MarketResolvedEvent>(event));
    const auto& resolved = std::get<MarketResolvedEvent>(event);
    EXPECT_EQ(resolved.winning_outcome, "Yes");
    ASSERT_TRUE(resolved.tags);
    EXPECT_EQ((*resolved.tags)[0], "stocks");
}

TEST(WsTypesTest, ParsesPongAndUnknownMessagesWithoutThrowing) {
    const auto pong = parse_market_event("PONG");
    ASSERT_TRUE(std::holds_alternative<PongEvent>(pong));

    const auto malformed = parse_market_event("{");
    ASSERT_TRUE(std::holds_alternative<UnknownEvent>(malformed));
    EXPECT_FALSE(std::get<UnknownEvent>(malformed).error.empty());

    const auto unknown_event = parse_market_event(R"({"event_type":"mystery","x":1})");
    ASSERT_TRUE(std::holds_alternative<UnknownEvent>(unknown_event));
    EXPECT_EQ(std::get<UnknownEvent>(unknown_event).event_type, "mystery");
    EXPECT_EQ(market_event_type(unknown_event), "mystery");
}

TEST(WsTypesTest, ParsesInitialDumpArrayAsEvents) {
    const auto events = parse_market_events(R"([{
        "market":"0xe033fd230022d2c8835df16bb9e065b402690a15e9ed381ea00569b9218ce102",
        "asset_id":"43116321876789348651560631278691064226660030712941566518175748399907477755169",
        "timestamp":"1777336666453",
        "hash":"9dca02f5f69be038344301b4fdf096def5a26764",
        "bids":[{"price":"0.01","size":"9207.29"}],
        "asks":[{"price":"0.99","size":"14875.82"}],
        "tick_size":"0.01",
        "event_type":"book",
        "last_trade_price":"0.900"
    }])");

    ASSERT_EQ(events.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<BookEvent>(events[0]));
    const auto& book = std::get<BookEvent>(events[0]);
    EXPECT_EQ(book.market, "0xe033fd230022d2c8835df16bb9e065b402690a15e9ed381ea00569b9218ce102");
    EXPECT_EQ(book.asset_id, "43116321876789348651560631278691064226660030712941566518175748399907477755169");
    ASSERT_EQ(book.bids.size(), 1u);
    EXPECT_EQ(book.bids[0].size, "9207.29");
}

}  // namespace
