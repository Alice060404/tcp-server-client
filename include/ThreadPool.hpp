#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
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

    void add(std::function<void()> func);
};
