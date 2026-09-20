#pragma once

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
  private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex taskMtx;
    std::condition_variable cv;
    bool stop;

  public:
    ThreadPool(int size = 10);
    ~ThreadPool();

    template <typename F, typename... Args>
    auto add(F &&f, Args &&...args) -> std::future<typename std::invoke_result_t<F, Args...>>;
};

template <typename F, typename... Args>
auto ThreadPool::add(F &&f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>>
{
    using returnType = std::invoke_result_t<F, Args...>;

    auto task =
        std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<returnType> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(taskMtx);
        if (stop)
        {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }
        tasks.emplace([task]() { (*task)(); });
    }
    cv.notify_one();
    return res;
}
