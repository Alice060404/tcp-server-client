#include "ThreadPool.hpp"

#include <functional>
#include <iostream>
#include <string>

void print(int a, double b, const char *str, const std::string &s)
{
    std::cout << a << " " << b << " " << str << " " << s << '\n';
}

void test()
{
    std::cout << "test." << '\n';
}

int main()
{
    ThreadPool *threadPool = new ThreadPool();
    std::function<void()> func = std::bind(print, 1, 3.14, "this is a ", std::string("test."));
    threadPool->add(func);
    func = test;
    threadPool->add(func);

    delete threadPool;

    return 0;
}
