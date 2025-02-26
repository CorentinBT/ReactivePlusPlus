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

#include <rpp/schedulers/details/queue.hpp>
#include <rpp/schedulers/details/utils.hpp>
#include <rpp/schedulers/details/worker.hpp>
#include <rpp/schedulers/fwd.hpp>
#include <thread>

namespace rpp::schedulers
{
/**
 * @brief Schedules execution of schedulables via queueing tasks to the caller thread with priority to time_point and order.
 * @warning Caller thread is thread where "schedule" called.
 *
 * @par Example
 * @snippet current_thread.cpp current_thread
 *
 * @ingroup schedulers
 */
class current_thread
{
    inline static thread_local std::optional<details::schedulables_queue> s_queue{};
    inline static thread_local time_point s_last_now_time{};

    static void sleep_until(const time_point timepoint)
    {
        if (timepoint <= s_last_now_time)
            return;

        const auto now = clock_type::now();
        std::this_thread::sleep_for(timepoint - now);
        s_last_now_time = std::max(now, timepoint);
    }

    static time_point get_now() { return s_last_now_time = clock_type::now(); }

    static void drain_queue(std::optional<details::schedulables_queue>& queue)
    {
        while (!queue->is_empty())
        {
            auto top = queue->pop();
            if (top->is_disposed())
                continue;

            sleep_until(top->get_timepoint());

            optional_duration duration{0};
            // immediate like scheduling
            do
            {
                if (duration.value() > duration::zero() && !top->is_disposed())
                    std::this_thread::sleep_for(duration.value());

                if (top->is_disposed())
                    duration.reset();
                else
                    duration = (*top)();

            } while (queue->is_empty() && duration.has_value());

            if (duration.has_value())
                queue->emplace(get_now() + duration.value(), std::move(top));
        }

        queue.reset();
    }

public:
    class worker_strategy
    {
    public:
        template<rpp::constraint::observer TObs, typename... Args, constraint::schedulable_fn<TObs, Args...> Fn>
        static void defer_for(duration duration, Fn&& fn, TObs&& obs, Args&&... args)
        {
            auto& queue = s_queue;
            const bool someone_owns_queue = queue.has_value();
            if (!someone_owns_queue)
            {
                queue.emplace();

                const auto optional_duration = details::immediate_scheduling_while_condition(duration, [&queue](){ return queue->is_empty(); }, fn, obs, args...);
                if (!optional_duration || obs.is_disposed())
                    return drain_queue(queue);
                duration = optional_duration.value();
            }
            else if (obs.is_disposed())
                return;

            queue->emplace(get_now() + duration, std::forward<Fn>(fn), std::forward<TObs>(obs), std::forward<Args>(args)...);

            if (!someone_owns_queue)
                drain_queue(queue);
        }

        static rpp::disposable_wrapper get_disposable() { return rpp::disposable_wrapper{}; }
    };

    static rpp::schedulers::worker<worker_strategy> create_worker()
    {
        return rpp::schedulers::worker<worker_strategy>{};
    }
};
} // namespace rpp::schedulers