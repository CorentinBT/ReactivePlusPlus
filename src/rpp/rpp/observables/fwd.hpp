//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#pragma once

#include <rpp/observers/fwd.hpp>
#include <rpp/utils/constraints.hpp>
#include <rpp/utils/function_traits.hpp>

#include <concepts>
#include <type_traits>

namespace rpp::constraint
{
template<typename S, typename T>
concept observable_strategy = requires(const S& strategy, dynamic_observer<T>&& observer)
{
    {strategy.subscribe(std::move(observer))} -> std::same_as<void>;
};
}

namespace rpp::details::observable
{
template<constraint::decayed_type Type>
class dynamic_strategy;
}

namespace rpp
{
template<constraint::decayed_type Type, constraint::observable_strategy<Type> Strategy>
class base_observable;

/**
 * @brief Type-erased version of the `rpp::base_observable`. Any observable can be converted to dynamic_observable via `rpp::base_observable::as_dynamic` member function.
 * @details To provide type-erasure it uses `std::shared_ptr`. As a result it has worse performance.
 *
 * @tparam Type of value this obsevalbe can provide
 *
 * @ingroup observables
 */
template<constraint::decayed_type Type>
using dynamic_observable = base_observable<Type, details::observable::dynamic_strategy<Type>>;
}

namespace rpp::utils
{
namespace details
{
    template<typename T>
    struct extract_observable_type : std::false_type{};

    template<typename TT, typename Strategy>
    struct extract_observable_type<rpp::base_observable<TT, Strategy>> : std::true_type
    {
        using type = TT;
    };

} // namespace details
template<typename T>
using extract_observable_type_t = typename details::extract_observable_type<std::decay_t<T>>::type;
} // namespace rpp::utils

namespace rpp::constraint 
{
template<typename T>
concept observable = rpp::utils::details::extract_observable_type<std::decay_t<T>>::value;

template<typename Op, typename TObs>
concept operators = requires(const Op& op, TObs obs) 
{
    {op(static_cast<TObs>(obs))} -> rpp::constraint::observable;
};
}