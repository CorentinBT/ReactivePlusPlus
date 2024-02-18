//                  ReactivePlusPlus library
//
//          Copyright Aleksey Loginov 2023 - present.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/victimsnino/ReactivePlusPlus
//

#include <snitch/snitch.hpp>

#include <rpp/operators/on_error_resume_next.hpp>
#include <rpp/schedulers/immediate.hpp>
#include <rpp/sources/create.hpp>
#include <rpp/sources/empty.hpp>
#include <rpp/sources/just.hpp>

#include "disposable_observable.hpp"
#include "mock_observer.hpp"

TEMPLATE_TEST_CASE("on_error_resume_next switches observable on error", "", rpp::memory_model::use_stack, rpp::memory_model::use_shared)
{
    auto mock = mock_observer_strategy<int>();
    SECTION("observable without error emission")
    {
        auto obs = rpp::source::just<TestType>(rpp::schedulers::immediate{}, 1, 2, 3);
        SECTION("subscribe")
        {
            obs | rpp::operators::on_error_resume_next([](const std::exception_ptr&) { return rpp::source::empty<int>(); })
                | rpp::ops::subscribe(mock);
            SECTION("observer obtains values from observable")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    SECTION("observable with one error emission")
    {
        auto obs = rpp::source::create<int>([](const auto& sub) {
            sub.on_next(1);
            sub.on_next(2);
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
        });
        SECTION("subscribe")
        {
            obs | rpp::operators::on_error_resume_next([](const std::exception_ptr&) {
                return rpp::source::just<TestType>(rpp::schedulers::immediate{}, 3);
            })
                | rpp::ops::subscribe(mock);
            SECTION("observer obtains values from both outer and inner observable")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_total_on_next_count() == 3);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    SECTION("observable with two error emissions")
    {
        auto obs = rpp::source::create<int>([](const auto& sub) {
            sub.on_next(1);
            sub.on_next(2);
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
        });
        SECTION("subscribe")
        {
            obs | rpp::operators::on_error_resume_next([](const std::exception_ptr&) {
                return rpp::source::just<TestType>(rpp::schedulers::immediate{}, 3);
            })
                | rpp::ops::subscribe(mock);
            SECTION("observer only receives values from first inner observable")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_total_on_next_count() == 3);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }

    SECTION("inner observable different emission type")
    {
        auto obs = rpp::source::create<int>([](const auto& sub) {
            sub.on_next(1);
            sub.on_next(2);
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
            sub.on_error(std::make_exception_ptr(std::runtime_error{""}));
        });
        SECTION("subscribe")
        {
            obs | rpp::operators::on_error_resume_next([](const std::exception_ptr&) {
                return rpp::source::just<TestType>(rpp::schedulers::immediate{}, 3);
            })
                | rpp::ops::subscribe(mock);
            SECTION("observer only receives values from first inner observable")
            {
                CHECK(mock.get_received_values() == std::vector{1, 2, 3});
                CHECK(mock.get_total_on_next_count() == 3);
                CHECK(mock.get_on_error_count() == 0);
                CHECK(mock.get_on_completed_count() == 1);
            }
        }
    }
}

TEST_CASE("on_error_resume_next satisfies disposable contracts")
{
    auto observable_disposable = rpp::composite_disposable_wrapper::make();
    {
        auto observable = observable_with_disposable<int>(observable_disposable);

        test_operator_with_disposable<int>(
            rpp::ops::on_error_resume_next([](const std::exception_ptr&) { return rpp::source::empty<int>(); }));
    }

    CHECK(observable_disposable.is_disposed() || observable_disposable.lock().use_count() == 2);
}