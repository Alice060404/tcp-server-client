#include "ThreadPool.hpp"

ThreadPool::ThreadPool(unsigned int size)
{
    for (unsigned int i = 0; i < size; ++i)
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
