//
// Created by Lorenzo P on 4/20/26.
//

#ifndef POLYMARKET_GAMMA_TYPES_JSON_H
#define POLYMARKET_GAMMA_TYPES_JSON_H

#pragma once

#include "gamma_types.h"

#include <string_view>
#include <type_traits>

#include <boost/describe.hpp>
#include <boost/json.hpp>
#include <boost/mp11.hpp>

namespace polymarket::gamma {
    namespace detail {
        constexpr std::string_view json_key(std::string_view cpp_name) noexcept {
            using namespace std::string_view_literals;
            return cpp_name == "isNew"sv ? "new"sv : cpp_name;
        }

        template <class T>
        void to_object(boost::json::object& obj, T const& t) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;
            boost::mp11::mp_for_each<Members>([&](auto D) {
                obj[json_key(D.name)] = boost::json::value_from(t.*D.pointer);
            });
        }

        template <class T>
        void from_object(boost::json::object const& obj, T& t) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;
            boost::mp11::mp_for_each<Members>([&](auto D) {
                if (auto const* v = obj.if_contains(json_key(D.name))) {
                    if (v->is_null()) return;
                    using MT = std::remove_cv_t<
                        std::remove_reference_t<decltype(t.*D.pointer)>>;
                    t.*D.pointer = boost::json::value_to<MT>(*v);
                }
            });
        }
    }

#define PGM_JSON_WITH_REMAP(T)                                                 \
    inline void tag_invoke(boost::json::value_from_tag,                        \
                           boost::json::value& jv, T const& t) {               \
        detail::to_object(jv.emplace_object(), t);                             \
    }                                                                          \
    inline T tag_invoke(boost::json::value_to_tag<T>,                          \
                        boost::json::value const& jv) {                        \
        T t{};                                                                 \
        if (auto const* o = jv.if_object()) {                                  \
            detail::from_object(*o, t);                                        \
        }                                                                      \
        return t;                                                              \
    }

PGM_JSON_WITH_REMAP(FeeSchedule)
PGM_JSON_WITH_REMAP(ImageOptimized)
PGM_JSON_WITH_REMAP(Category)
PGM_JSON_WITH_REMAP(Tag)
PGM_JSON_WITH_REMAP(Chat)
PGM_JSON_WITH_REMAP(Collection)
PGM_JSON_WITH_REMAP(EventCreator)
PGM_JSON_WITH_REMAP(EventTemplate)
PGM_JSON_WITH_REMAP(Series)
PGM_JSON_WITH_REMAP(Event)
PGM_JSON_WITH_REMAP(Market)
PGM_JSON_WITH_REMAP(MarketsResponse)
PGM_JSON_WITH_REMAP(Holder)
PGM_JSON_WITH_REMAP(MarketHolders)
PGM_JSON_WITH_REMAP(MarketOpenInterest)
PGM_JSON_WITH_REMAP(EventVolume)

#undef PGM_JSON_WITH_REMAP

}

#endif //POLYMARKET_GAMMA_TYPES_JSON_H
