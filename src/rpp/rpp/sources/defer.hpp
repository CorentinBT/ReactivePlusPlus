//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/sources/fwd.hpp>
#include <rpp/observables/observable.hpp>

namespace rpp::details
{
template<constraint::decayed_type Type, typename Factory>
struct defer_strategy
{
    using ValueType = Type;

    Factory observable_factory;
    void subscribe(auto&& obs) const { observable_factory().subscribe(std::forward<decltype(obs)>(obs)); }
};
}

namespace rpp
{
template<constraint::decayed_type Type, typename Factory>
using defer_observable = observable<Type, details::defer_strategy<Type, Factory>>;
}

namespace rpp::source
{

/**
 * @brief Creates rpp::observable that calls the specified observable factory to create an observable for each new observer that subscribes.
 *
 * @tparam Factory the type of the observable factory
 *
 * @ingroup creational_operators
 * @see https://reactivex.io/documentation/operators/defer.html
 */
template<std::invocable Factory>
	requires(rpp::utils::is_no_argument_function<Factory> && rpp::constraint::observable<std::invoke_result_t<Factory>>)
auto defer(Factory&& observable_factory)
{
    return defer_observable<rpp::utils::extract_observable_type_t<std::invoke_result_t<Factory>>, Factory>{std::forward<Factory>(observable_factory)};
}
}