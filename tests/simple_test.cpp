#include <atomic>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

#include "Buffer.hpp"
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
    std::atomic<std::uint64_t> echoes{0};
};

void usage(const char *program)
{
    std::cout << "用法：" << program << " -t <线程数> -m <每线程回显次数> [-w <等待秒数>]\n"
              << "目标：127.0.0.1:8888，请先启动服务端。\n"
              << "每线程独立连接；连接成功后等待 w 秒再发送，默认不等待。\n"
              << "默认 100 个线程、每连接 100 次回显；不设置收发超时，不校验回显内容。\n";
}

Options parseOptions(int argc, char **argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view flag = argv[i];
        if (flag != "-t" && flag != "-m" && flag != "-w")
            throw std::invalid_argument("未知参数：" + std::string(flag));
        if (++i == argc)
            throw std::invalid_argument("参数缺少数值：" + std::string(flag));

        const std::string_view text = argv[i];
        int value = 0;
        const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
        if (error != std::errc{} || end != text.data() + text.size() || value < 0 || (flag != "-w" && value == 0))
            throw std::invalid_argument("参数数值无效：" + std::string(flag));

        if (flag == "-t")
            options.threads = value;
        else if (flag == "-m")
            options.messages = value;
        else
            options.waitSeconds = value;
    }
    return options;
}

// 保持 day11 的收发与计数方式，便于使用相同负载对比服务端。
void oneClient(int messages, int waitSeconds, Statistics &stats)
{
    try
    {
        Socket socket;
        InetAddress address("127.0.0.1", 8888);
        socket.connect(&address);
        ++stats.connected;
        const int fd = socket.getFd();
        Buffer sendBuffer;
        Buffer readBuffer;

        sleep(waitSeconds);
        int count = 0;
        while (count < messages)
        {
            sendBuffer.setBuf("I'm client!");
            const ssize_t writeBytes = write(fd, sendBuffer.c_str(), sendBuffer.size());
            if (writeBytes == -1)
            {
                std::printf("socket already disconnected, can't write any more!\n");
                break;
            }

            int alreadyRead = 0;
            while (true)
            {
                char buf[1024]{};
                const ssize_t readBytes = read(fd, buf, sizeof(buf));
                if (readBytes > 0)
                {
                    readBuffer.append(buf, readBytes);
                    alreadyRead += readBytes;
                }
                else if (readBytes == 0)
                {
                    // day11 在 EOF 时直接以成功状态结束整个测试进程。
                    std::printf("server disconnected!\n");
                    std::exit(EXIT_SUCCESS);
                }
                if (alreadyRead >= sendBuffer.size())
                {
                    std::printf("count: %d, message from server: %s\n", count++, readBuffer.c_str());
                    ++stats.echoes;
                    break;
                }
            }
            readBuffer.clear();
        }
        if (count == messages)
            ++stats.completed;
    }
    catch (const std::exception &error)
    {
        // 本项目的 Socket 抛异常；day11 的对应错误处理会打印后退出。
        // 在任务内处理，避免异常仅存入未读取的 future 而失去错误提示。
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
        // 先创建全部工作线程，再像 day11 一样提交相同数量的客户端任务。
        ThreadPool pool(options.threads);
        std::function<void()> client = std::bind(oneClient, options.messages, options.waitSeconds, std::ref(stats));
        for (int i = 0; i < options.threads; ++i)
            pool.add(client);
        // 离开作用域时等待所有任务结束，避免汇总时仍有任务更新计数。
    }

    const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    const auto expected = static_cast<std::uint64_t>(options.threads) * options.messages;
    const auto echoes = stats.echoes.load();
    std::cout << "\n测试汇总：\n"
              << "连接：累计建立 " << stats.connected.load() << " / " << options.threads << '\n'
              << "任务：完成全部回显 " << stats.completed.load() << " / " << options.threads << '\n'
              << "回显：已完成 " << echoes << " / " << expected << "（"
              << 100.0 * static_cast<double>(echoes) / static_cast<double>(expected) << "%）\n"
              << "总耗时（含建连和等待）：" << seconds << " 秒\n"
              << "注：按接收字节数计数，未校验回显内容。\n";
    return 0;
}
