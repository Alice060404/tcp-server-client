#pragma once

#include <csignal>
#include <functional>
#include <map>

std::map<int, std::function<void()>> handlers_;
void signalHandler(int sig)
{
    handlers_[sig]();
}

struct Signal
{
    static void signal(int sig, const std::function<void()> &handler)
    {
        handlers_[sig] = handler;
        ::signal(sig, signalHandler);
    }
};
