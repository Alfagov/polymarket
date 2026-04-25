#include <iostream>

#include "src/clob_client.h"
#include "src/gamma_client.h"
#include "src/types.h"
#include "src/gamma_client.h"

using namespace polymarket::gamma;
using namespace polymarket;


int main() {

    const auto cfg = APIConfig();
    auto client = GammaClient(cfg);


   // auto mkts = client.fetch_all_markets();
    auto oi = client.fetch_event_live_volumes(99802);
    //std::cout << "TOTAL: " << mkts.size() << std::endl;
    std::cout << "OI: " << oi.size() << std::endl;
    //mkts = client.fetch_markets(MarketParameters{.limit = 1}, "");
    //std::cout << "TOTAL: " << mkts.size() << std::endl;


    auto clob_client = clob::ClobClient(cfg);
    auto mkts = clob_client.fetch_all_simplified_markets();
    auto spread = clob_client.fetch_spread(mkts[0].tokens[0].token_id);

    std::cout << "SPREAD: " << spread << std::endl;
    return 0;
}
