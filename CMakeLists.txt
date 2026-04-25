cmake_minimum_required(VERSION 4.2)
project(polymarket)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_ENABLE_CMAKE ON CACHE BOOL "Enable Boost CMake support")
set(POLYMARKET_BOOST_LIBRARIES
        asio
        filesystem
        system
        json
        beast
        url
        scope)
set(BOOST_INCLUDE_LIBRARIES "${POLYMARKET_BOOST_LIBRARIES}" CACHE STRING "Boost libraries to include" FORCE)

FetchContent_Declare(
        Boost
        URL https://github.com/boostorg/boost/releases/download/boost-1.90.0/boost-1.90.0-cmake.tar.xz
)

FetchContent_MakeAvailable(Boost)
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(polymarket main.cpp
        src/http_client.cpp
        src/http_client.h
        src/http_client.h
        src/ws_client.cpp
        src/ws_client.h
        src/gamma_client.cpp
        src/gamma_client.h
        src/types.h
        src/gamma_types.h
        src/gamma_types_json.h
        src/url_utils.h
        src/clob_client.cpp
        src/clob_client.h
        src/clob_types.h
        src/data_client.cpp
        src/data_client.h
        src/data_types.h)

target_link_libraries(polymarket PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        Boost::system
        CURL::libcurl
        Boost::headers
        Boost::json
        Boost::beast
        Boost::asio
        Boost::url
        Boost::scope)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
option(POLYMARKET_BUILD_TESTS "Build polymarket tests" ON)

if (POLYMARKET_BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(gamma_client_tests
            tests/test_gamma_client.cpp
            src/http_client.cpp
            src/gamma_client.cpp
            src/clob_client.cpp
            src/clob_client.h
            src/clob_types.h
            src/data_client.cpp
            src/data_client.h
            src/data_types.h)

    target_include_directories(gamma_client_tests PRIVATE src tests)

    target_link_libraries(gamma_client_tests PRIVATE
            GTest::gtest_main
            OpenSSL::SSL
            OpenSSL::Crypto
            Boost::system
            CURL::libcurl
            Boost::headers
            Boost::json
            Boost::beast
            Boost::asio
            Boost::url
            Boost::scope)

    include(GoogleTest)
    gtest_discover_tests(gamma_client_tests)
endif()
