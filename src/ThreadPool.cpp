#include "ThreadPool.hpp"
#include <functional>
#include <mutex>
#include <stdexcept>
#include <thread>

ThreadPool::ThreadPool(int size) : stop(false)
{
    for (int i = 0; i < size; ++i)
    {
        threads.emplace_back(std::thread([this]() {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(taskMtx);
                    cv.wait(lock, [this]() { return stop || !tasks.empty(); });
                    if (stop && tasks.empty())
                        return;
                    task = tasks.front();
                    tasks.pop();
                }
                task();
            }
        }));
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(taskMtx);
        stop = true;
    }
    cv.notify_all();
    for (std::thread &thr : threads)
    {
        if (thr.joinable())
            thr.join();
    }
}

void ThreadPool::add(std::function<void()> func)
{
    {
        std::unique_lock<std::mutex> lock(taskMtx);
        if (stop)
            throw std::runtime_error("ThreadPool already stop, can not add task.");
        tasks.emplace(func);
    }
    cv.notify_one();
}
