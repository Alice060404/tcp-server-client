#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

#include "Connection.hpp"
#include "InetAddress.hpp"
#include "Socket.hpp"
#include "ThreadPool.hpp"

namespace
{
struct Options
{
    int threads = 100;
    int messages = 100;
    int waitSeconds = 0;
};

struct Statistics
{
    std::atomic<int> connected{0};
    std::atomic<int> completed{0};
    std::atomic<std::uint64_t> reads{0};
};

void usage(const char *program)
{
    std::cout << "用法：" << program << " -t <线程数> -m <每线程回显次数> [-w <等待秒数>]\n"
              << "参数也支持紧凑写法，例如 -t100 -m10 -w1。\n"
              << "目标：127.0.0.1:8888，请先启动服务端。\n"
              << "每线程独立连接；连接成功后等待 w 秒再发送，默认不等待。\n"
              << "默认 100 个线程、每连接 100 次收发；不设置收发超时，不校验回显内容。\n";
}

Options parseOptions(int argc, char **argv)
{
    Options options;
    opterr = 0;
    int flag;
    while ((flag = getopt(argc, argv, ":t:m:w:")) != -1)
    {
        if (flag == '?')
            throw std::invalid_argument(std::string("未知参数：-") + static_cast<char>(optopt));
        if (flag == ':')
            throw std::invalid_argument(std::string("参数缺少数值：-") + static_cast<char>(optopt));

        const std::string_view text = optarg;
        int value = 0;
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc{} || end != text.data() + text.size() || value < 0 || (flag != 'w' && value == 0))
            throw std::invalid_argument(std::string("参数数值无效：-") + static_cast<char>(flag));

        if (flag == 't')
            options.threads = value;
        else if (flag == 'm')
            options.messages = value;
        else
            options.waitSeconds = value;
    }
    if (optind < argc)
        throw std::invalid_argument("未知参数：" + std::string(argv[optind]));
    return options;
}

void oneClient(int messages, int waitSeconds, Statistics &stats)
{
    try
    {
        auto socket = std::make_unique<Socket>();
        InetAddress address("127.0.0.1", 8888);
        socket->connect(&address);
        ++stats.connected;
        Connection connection(nullptr, socket.release());

        sleep(waitSeconds);
        int count = 0;
        while (count < messages)
        {
            connection.setSendBuffer("I'm client!");
            connection.write();
            if (connection.getState() == Connection::State::Closed)
            {
                break;
            }
            connection.read();
            std::cout << "msg count " << count++ << ": " << connection.readBuffer() << std::endl;
            ++stats.reads;
        }
        if (count == messages)
            ++stats.completed;
    }
    catch (const std::exception &error)
    {
        std::fprintf(stderr, "%s\n", error.what());
        std::exit(EXIT_FAILURE);
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && (std::string_view(argv[1]) == "--help" || std::string_view(argv[1]) == "-h"))
    {
        usage(argv[0]);
        return 0;
    }

    Options options;
    try
    {
        options = parseOptions(argc, argv);
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        usage(argv[0]);
        return 2;
    }

    Statistics stats;
    const auto start = std::chrono::steady_clock::now();
    {
        ThreadPool pool(options.threads);
        std::function<void()> client = std::bind(oneClient, options.messages, options.waitSeconds, std::ref(stats));
        for (int i = 0; i < options.threads; ++i)
            pool.add(client);
    }

    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const auto expected = static_cast<std::uint64_t>(options.threads) * options.messages;
    const auto reads = stats.reads.load();
    std::cout << "\n测试汇总：\n"
              << "连接：累计建立 " << stats.connected.load() << " / " << options.threads << '\n'
              << "任务：完成全部收发轮次 " << stats.completed.load() << " / " << options.threads << '\n'
              << "读取轮次：已执行 " << reads << " / " << expected << "（"
              << 100.0 * static_cast<double>(reads) / static_cast<double>(expected) << "%）\n"
              << "总耗时（含建连和等待）：" << seconds << " 秒\n"
              << "注：每次 read() 返回后计数，包括空读取；未校验回显内容。\n";
    return 0;
}
