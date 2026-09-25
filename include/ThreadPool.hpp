#pragma once

#include "Macros.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

class ThreadPool
{
  public:
    explicit ThreadPool(unsigned int size = std::thread::hardware_concurrency());
    ~ThreadPool();

    DISALLOW_COPY_AND_MOVE(ThreadPool)

    template <typename F, typename... Args>
    auto add(F &&f, Args &&...args) -> std::future<typename std::invoke_result_t<F, Args...>>;

  private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queueMutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
};

template <typename F, typename... Args>
auto ThreadPool::add(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>>
{
    using returnType = std::invoke_result_t<F, Args...>;

    auto task =
        std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<returnType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queueMutex_);
        if (stop_)
        {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    cv_.notify_one();
    return res;
}
