//
// Created by Lorenzo P on 4/28/26.
//

#include "logging.h"

#include <utility>

#include <spdlog/sinks/stdout_color_sinks.h>

namespace polymarket {
    std::shared_ptr<spdlog::logger> rest_logger() {
        static auto logger = [] {
            auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            auto logger = std::make_shared<spdlog::logger>("polymarket.rest", std::move(sink));
            logger->set_level(spdlog::level::off);
            return logger;
        }();

        return logger;
    }

    std::shared_ptr<spdlog::logger> market_ws_logger() {
        static auto logger = [] {
            auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            auto logger = std::make_shared<spdlog::logger>("polymarket.ws.market", std::move(sink));
            logger->set_level(spdlog::level::off);
            return logger;
        }();

        return logger;
    }

    void set_log_level(LogLevel level) {
        set_rest_log_level(level);
        set_ws_log_level(level);
    }

    void set_rest_log_level(LogLevel level) {
        rest_logger()->set_level(level);
    }

    void set_ws_log_level(LogLevel level) {
        market_ws_logger()->set_level(level);
    }

    LogLevel get_log_level() {
        return rest_logger()->level();
    }
}
