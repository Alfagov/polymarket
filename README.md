# polymarket

Small C++20 client project for exploring Polymarket market data and building toward a full trading client.

The repository is currently focused on read-only APIs:
- `HttpClient` wraps `libcurl` for simple GET/POST/DELETE requests.
- `GammaClient` exposes typed access to Gamma discovery endpoints.
- `ClobClient` exposes unauthenticated CLOB REST endpoints for books, prices, market metadata, pagination, geoblock, and maker rebates.
- `DataClient` exposes typed access to `data-api.polymarket.com` endpoints.
- `*_types.h` and `*_types_json.h` define Boost.JSON-backed models and conversions.
- `tests/fixtures/` contains captured live API responses used by the offline test suite.
- `WsClient` exists as a scaffold, but the websocket flow is not implemented yet.

## Current status

Implemented:
- Gamma event and market discovery, including keyset pagination helpers.
- Gamma tags, related tags, series, comments, teams, sports metadata, public profiles, and public search.
- Gamma convenience methods for top holders, open interest, and live event volumes.
- Data API methods for health, positions, trades, activity, holders, value, closed positions, trader leaderboard, traded totals, open interest, live volume, builder leaderboard, and builder volume.
- CLOB health/version/time, order books, bulk books, prices, bulk prices, midpoints, spreads, last traded prices, price history, fee rate, tick size, neg risk, market info, market lookup, token-to-market lookup, live market activity, paginated markets, simplified markets, geoblock, and current maker rebates.
- Fixture-backed tests using live captured Gamma, CLOB, and Data API responses.
- A simple `main.cpp` smoke-test style entrypoint.

Not implemented yet:
- Auth, signing, and order lifecycle support.
- Live order book subscription and reconciliation.
- Websocket market data handling beyond the current scaffold.

## Build

Prerequisites:
- C++20 compiler
- CMake
- `libcurl`
- OpenSSL

Build and run:

```bash
cmake -S . -B build
cmake --build build
./build/polymarket
```

Run tests:

```bash
ctest --test-dir build --output-on-failure
```

Note: the CMake setup uses `FetchContent` for Boost and GoogleTest, so the first configure may need network access. The test binary uses `tests/fixtures` via `POLYMARKET_TEST_FIXTURE_DIR` and should not need live API access.

## Configuration

`APIConfig` centralizes endpoint configuration:

```cpp
polymarket::APIConfig config;
config.gamma_api_url = "https://gamma-api.polymarket.com";
config.clob_rest_url = "https://clob.polymarket.com";
config.data_api_url = "https://data-api.polymarket.com";
config.polymarket_url = "https://polymarket.com";
config.http_timeout_ms = 5000;
```

Tests override these URLs with the local mock server so captured fixtures can exercise parsing and request construction deterministically.

## Repo layout

```text
src/
  http_client.*        low-level HTTP wrapper
  gamma_client.*       Gamma API client
  gamma_types*.h       Gamma models + JSON conversion
  clob_client.*        CLOB REST client
  clob_types.h         CLOB models + JSON conversion
  data_client.*        Data API client
  data_types.h         Data API models + JSON conversion
  ws_client.*          websocket client scaffold
  types.h              shared API config
tests/
  fixtures/            captured live API responses for offline tests
  mock_http_server.h   local canned-response HTTP server
  test_gamma_client.cpp
main.cpp               smoke-test entrypoint
```

## Next steps

1. Implement order authentication/signing with API key, secret/passphrase, wallet address, chain id, and signing helpers.
2. Implement the core order flow: create order payload, sign, submit, fetch/open orders, cancel, and reconcile fills.
3. Add websocket market data support: subscribe to book/trade channels, maintain an in-memory `OrderBook`, and detect stale snapshots.
4. Join Gamma markets to CLOB token ids cleanly so strategies can move from discovery to execution.
5. Add authenticated and websocket integration tests for auth failures, partial fills, cancels, and reconnect behavior.
6. Replace `main.cpp` with small examples: fetch active markets, select a token, stream the book, and place/cancel a paper-trading style order.
