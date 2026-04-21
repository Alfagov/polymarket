//
// Created by Lorenzo P on 4/20/26.
//

#ifndef POLYMARKET_URL_UTILS_H
#define POLYMARKET_URL_UTILS_H
#include <iomanip>
#include <ios>
#include <sstream>
#include <string>

namespace polymarket {
    template <typename T>
    std::string join_vector(const std::vector<T>& vec) {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            oss << vec[i];
            if (i != vec.size() - 1) oss << ",";
        }
        return oss.str();
    }
}

#endif //POLYMARKET_URL_UTILS_H
