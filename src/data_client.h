//
// Created by Lorenzo P on 4/25/26.
//
// Client for data-api.polymarket.com. Mirrors the Rust unauth client at
// /Users/alfagov/RustroverProjects/rs-clob-client-v2/src/data/client.rs.
//

#ifndef POLYMARKET_DATA_CLIENT_H
#define POLYMARKET_DATA_CLIENT_H
#pragma once

#include "data_types.h"
#include "http_client.h"
#include "types.h"

namespace polymarket::data {
    class DataClient {
    public:
        explicit DataClient(const APIConfig &config);
        ~DataClient() = default;

        Health                              health();
        std::vector<Position>               positions(const PositionsRequest &req);
        std::vector<Trade>                  trades(const TradesRequest &req);
        std::vector<Activity>               activity(const ActivityRequest &req);
        std::vector<MetaHolder>             holders(const HoldersRequest &req);
        std::vector<Value>                  value(const ValueRequest &req);
        std::vector<ClosedPosition>         closed_positions(const ClosedPositionsRequest &req);
        std::vector<TraderLeaderboardEntry> leaderboard(const TraderLeaderboardRequest &req);
        Traded                              traded(const TradedRequest &req);
        std::vector<OpenInterest>           open_interest(const OpenInterestRequest &req);
        std::vector<LiveVolume>             live_volume(const LiveVolumeRequest &req);
        std::vector<BuilderLeaderboardEntry> builder_leaderboard(const BuilderLeaderboardRequest &req);
        std::vector<BuilderVolumeEntry>     builder_volume(const BuilderVolumeRequest &req);

    private:
        APIConfig  config_;
        HttpClient http_;
    };
}

#endif  // POLYMARKET_DATA_CLIENT_H
