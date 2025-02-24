//                   ReactivePlusPlus library
//
//           Copyright Aleksey Loginov 2023 - present.
//  Distributed under the Boost Software License, Version 1.0.
//     (See accompanying file LICENSE_1_0.txt or copy at
//           https://www.boost.org/LICENSE_1_0.txt)
//
//  Project home: https://github.com/victimsnino/ReactivePlusPlus

#include "rpp/disposables/fwd.hpp"
#include <snitch/snitch.hpp>
#include <rpp/observables.hpp>
#include <rpp/sources/create.hpp>

TEST_CASE("create observable works properly as base_observable")
{
    size_t on_subscribe_called{};
    auto observable = rpp::source::create<int>([&](auto&& observer)
    {
        ++on_subscribe_called;
        observer.on_next(1);
        observer.on_completed();
    });

    auto test = [&](auto&& observable)
    {
        SECTION("subscribe valid observer")
        {
            std::vector<int> on_next_vals{};
            size_t           on_error{};
            size_t           on_completed{};

            observable.subscribe([&](int v) { on_next_vals.push_back(v); },
                                [&](const std::exception_ptr&) { ++on_error; },
                                [&]() { ++on_completed; });

            CHECK(on_subscribe_called == 1u);
            CHECK(on_next_vals == std::vector{1});
            CHECK(on_error == 0u);
            CHECK(on_completed == 1u);
        }

        SECTION("subscribe disposed callbacks")
        {
            observable.subscribe(rpp::disposable_wrapper{}, [](int) {}, [](const std::exception_ptr&) {}, []() {});

            CHECK(on_subscribe_called == 0u);
        }

        SECTION("subscribe disposed observer")
        {
            observable.subscribe(rpp::disposable_wrapper{}, rpp::make_lambda_observer([](int) {}, [](const std::exception_ptr&) {}, []() {}));

            CHECK(on_subscribe_called == 0u);
        }

        SECTION("subscribe non-disposed callbacks")
        {
            observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, [](int) {}, [](const std::exception_ptr&) {}, []() {});

            CHECK(on_subscribe_called == 1u);
        }

        SECTION("subscribe non-disposed observer")
        {
            observable.subscribe(rpp::disposable_wrapper{std::make_shared<rpp::base_disposable>()}, rpp::make_lambda_observer([](int) {}, [](const std::exception_ptr&) {}, []() {}));

            CHECK(on_subscribe_called == 1u);
        }
    };

    SECTION("original observable")
    {
        test(observable);
    }

    SECTION("dynamic observable")
    {
        test(observable.as_dynamic());
    }

    SECTION("dynamic observable via move")
    {
        test(std::move(observable).as_dynamic()); // NOLINT
    }
}