//
// Created by Lorenzo P on 4/28/26.
//

#ifndef POLYMARKET_LOGGING_H
#define POLYMARKET_LOGGING_H

#include <memory>

#include <spdlog/common.h>
#include <spdlog/logger.h>

namespace polymarket {
    using LogLevel = spdlog::level::level_enum;

    std::shared_ptr<spdlog::logger> rest_logger();
    std::shared_ptr<spdlog::logger> market_ws_logger();
    void set_log_level(LogLevel level);
    void set_rest_log_level(LogLevel level);
    void set_ws_log_level(LogLevel level);
    LogLevel get_log_level();
}

#endif //POLYMARKET_LOGGING_H
