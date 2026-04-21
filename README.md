# polymarket

Small C++20 client project for exploring Polymarket market data and building toward a full trading client.

Right now the repository is mainly focused on the read side:
- `HttpClient` wraps `libcurl` for simple GET/POST/DELETE requests.
- `GammaClient` exposes typed access to Polymarket Gamma and Data API endpoints.
- `gamma_types.h` and `gamma_types_json.h` define and parse market/event models with Boost.JSON.
- `tests/` covers Gamma client request building, pagination, parsing, and base-url switching.
- `WsClient` exists as a scaffold, but the websocket flow is not implemented yet.

## Current status

Implemented today:
- Fetch events, markets, tags, top holders, open interest, and live event volumes.
- Pagination helpers for `fetch_all_events()` and `fetch_all_markets()`.
- A simple `main.cpp` smoke-test style entrypoint.

Not implemented yet:
- CLOB REST client methods.
- Auth, signing, and order lifecycle support.
- Live order book subscription and reconciliation.

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

Note: the CMake setup uses `FetchContent` for Boost and GoogleTest, so the first configure may need network access.

## Next steps

1. Add a dedicated `ClobClient` with typed requests/responses for markets, order books, trades, balances, and orders.
2. Implement order authentication/signing:
   use a config object for API key, secret/passphrase, wallet address, chain id, and signing helpers.
3. Implement the core order flow:
   create order payload -> sign -> submit -> fetch/open orders -> cancel -> reconcile fills.
4. Add websocket market data support:
   subscribe to book/trade channels, maintain an in-memory `OrderBook`, and detect stale snapshots.
5. Join Gamma markets to CLOB token ids cleanly:
   map `Market::clobTokenIds` into typed token metadata so strategies can move from discovery to execution.
6. Add integration-style tests:
   mock CLOB responses for happy path, auth failures, partial fills, cancels, and reconnect behavior.
7. Replace `main.cpp` with small examples:
   fetch active markets, select a token, stream the book, and place/cancel a paper-trading style order.

## Repo layout

```text
src/
  http_client.*        low-level HTTP wrapper
  gamma_client.*       Gamma/Data API client
  gamma_types*.h       typed models + JSON conversion
  ws_client.*          websocket client scaffold
  types.h              shared trading/order-book types
tests/
  test_gamma_client.cpp
  mock_http_server.h
main.cpp               smoke-test entrypoint
```
