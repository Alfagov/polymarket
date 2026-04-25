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
        // Map a C++ identifier to its JSON wire key. Most pass through unchanged;
        // a handful of fields collide with C++ reserved words and need a remap.
        constexpr std::string_view json_key(std::string_view cpp_name) noexcept {
            using namespace std::string_view_literals;
            if (cpp_name == "isNew"sv) return "new"sv;
            if (cpp_name == "isMod"sv) return "mod"sv;
            return cpp_name;
        }

        template <class T>
        void to_object(boost::json::object& obj, T const& t) {
            using Members = boost::describe::describe_members<
                T, boost::describe::mod_public | boost::describe::mod_inherited>;
            boost::mp11::mp_for_each<Members>([&](auto D) {
                obj[json_key(D.name)] = boost::json::value_from(t.*D.pointer);
            });
        }

        template <class N>
        N as_number(boost::json::value const& v) {
            if (v.is_double()) return static_cast<N>(v.as_double());
            if (v.is_int64())  return static_cast<N>(v.as_int64());
            if (v.is_uint64()) return static_cast<N>(v.as_uint64());
            if (v.is_string()) {
                try { return static_cast<N>(std::stod(std::string(v.as_string()))); }
                catch (...) { return N{}; }
            }
            return N{};
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
                    if constexpr (std::is_same_v<MT, double>) {
                        t.*D.pointer = as_number<double>(*v);
                    } else if constexpr (std::is_integral_v<MT> && !std::is_same_v<MT, bool>) {
                        t.*D.pointer = as_number<MT>(*v);
                    } else {
                        try { t.*D.pointer = boost::json::value_to<MT>(*v); }
                        catch (...) { /* tolerate stray shape mismatches */ }
                    }
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
PGM_JSON_WITH_REMAP(ClobReward)
PGM_JSON_WITH_REMAP(Market)
PGM_JSON_WITH_REMAP(MarketsResponse)
PGM_JSON_WITH_REMAP(Holder)
PGM_JSON_WITH_REMAP(MarketHolders)
PGM_JSON_WITH_REMAP(MarketOpenInterest)
PGM_JSON_WITH_REMAP(EventVolume)
PGM_JSON_WITH_REMAP(Team)
PGM_JSON_WITH_REMAP(SportsMarketTypesResponse)
PGM_JSON_WITH_REMAP(RelatedTag)
PGM_JSON_WITH_REMAP(CommentPosition)
PGM_JSON_WITH_REMAP(CommentProfile)
PGM_JSON_WITH_REMAP(Reaction)
PGM_JSON_WITH_REMAP(Comment)
PGM_JSON_WITH_REMAP(PublicProfileUser)
PGM_JSON_WITH_REMAP(PublicProfile)
PGM_JSON_WITH_REMAP(SearchTag)
PGM_JSON_WITH_REMAP(Profile)
PGM_JSON_WITH_REMAP(Pagination)
PGM_JSON_WITH_REMAP(SearchResults)

#undef PGM_JSON_WITH_REMAP

    // SportsMetadata.tags is a comma-separated string on the wire — handle
    // serialization/deserialization manually rather than via the describe path.
    inline void tag_invoke(boost::json::value_from_tag,
                           boost::json::value& jv, SportsMetadata const& v) {
        boost::json::object obj;
        obj["id"]         = v.id;
        obj["sport"]      = v.sport;
        obj["image"]      = v.image;
        obj["resolution"] = v.resolution;
        obj["ordering"]   = v.ordering;
        std::string csv;
        for (size_t i = 0; i < v.tags.size(); ++i) {
            if (i) csv.push_back(',');
            csv.append(v.tags[i]);
        }
        obj["tags"]       = csv;
        obj["series"]     = v.series;
        obj["createdAt"]  = v.createdAt;
        jv = std::move(obj);
    }
    inline SportsMetadata tag_invoke(boost::json::value_to_tag<SportsMetadata>,
                                     boost::json::value const& jv) {
        SportsMetadata v;
        auto const* obj = jv.if_object();
        if (!obj) return v;

        if (auto const* p = obj->if_contains("id"); p && !p->is_null())
            v.id = static_cast<std::int32_t>(p->to_number<std::int64_t>());
        if (auto const* p = obj->if_contains("sport"); p && p->is_string())
            v.sport = std::string(p->as_string());
        if (auto const* p = obj->if_contains("image"); p && p->is_string())
            v.image = std::string(p->as_string());
        if (auto const* p = obj->if_contains("resolution"); p && p->is_string())
            v.resolution = std::string(p->as_string());
        if (auto const* p = obj->if_contains("ordering"); p && p->is_string())
            v.ordering = std::string(p->as_string());
        if (auto const* p = obj->if_contains("tags")) {
            if (p->is_string()) {
                std::string_view csv = p->as_string();
                size_t pos = 0;
                while (pos < csv.size()) {
                    auto next = csv.find(',', pos);
                    if (next == std::string_view::npos) next = csv.size();
                    v.tags.emplace_back(csv.substr(pos, next - pos));
                    pos = next + 1;
                }
            } else if (p->is_array()) {
                for (auto const& el : p->as_array()) {
                    if (el.is_string()) v.tags.emplace_back(el.as_string());
                }
            }
        }
        if (auto const* p = obj->if_contains("series"); p && p->is_string())
            v.series = std::string(p->as_string());
        if (auto const* p = obj->if_contains("createdAt"); p && p->is_string())
            v.createdAt = std::string(p->as_string());

        return v;
    }

}

#endif //POLYMARKET_GAMMA_TYPES_JSON_H
