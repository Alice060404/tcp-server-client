#pragma once

#include <csignal>
#include <functional>
#include <map>
std::map<int, std::function<void()>> handlers;
void signalHandler(int sig)
{
    handlers[sig]();
}

struct Signal
{
    static void signal(int sig, const std::function<void()> &handler)
    {
        handlers[sig] = handler;
        ::signal(sig, signalHandler);
    }
};
