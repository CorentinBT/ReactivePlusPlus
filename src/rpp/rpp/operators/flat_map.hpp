//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#pragma once

#include <rpp/operators/map.hpp>
#include <rpp/operators/merge.hpp>
#include <rpp/defs.hpp>

namespace rpp::operators::details
{

template<rpp::constraint::decayed_type Fn>
struct flat_map_t 
{
    RPP_NO_UNIQUE_ADDRESS Fn m_fn;

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> 
                    && rpp::constraint::observable<std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) const &
    {
        return std::forward<TObservable>(observable)
                | rpp::ops::map(m_fn)
                | rpp::ops::merge();
    }

    template<rpp::constraint::observable TObservable>
        requires (std::invocable<Fn, rpp::utils::extract_observable_type_t<TObservable>> 
                    && rpp::constraint::observable<std::invoke_result_t<Fn, rpp::utils::extract_observable_type_t<TObservable>>>)
    auto operator()(TObservable&& observable) &&
    {
        return std::forward<TObservable>(observable)
                | rpp::ops::map(std::move(m_fn))
                | rpp::ops::merge();
    }
};

} // namespace rpp::operators::details

namespace rpp::operators
{

/**
 * @brief Transform the items emitted by an Observable into Observables, then flatten the emissions from those into a single Observable
 *
 * @details Note that flat_map merges the emissions of these Observables, so that they may interleave. 
 *
 * @param a function that returns an observable for each item emitted by the source observable. 
 * @warning #include <rpp/operators/flat_map.hpp>
 *
 * @ingroup transforming_operators
 * @see https://reactivex.io/documentation/operators/flatmap.html
 */
template<typename Fn>
    requires (!utils::is_not_template_callable<Fn> || rpp::constraint::observable<std::invoke_result_t<Fn, utils::convertible_to_any>>)
auto flat_map(Fn&& callable)
{
    return details::flat_map_t<std::decay_t<Fn>>{std::forward<Fn>(callable)};
}

} // namespace rpp::operators