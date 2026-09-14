#include <cstdint>

class Epoll;
class Channel
{
  private:
    Epoll *ep;
    int fd;
    uint32_t events;
    uint32_t revents;
    bool inEpoll;

  public:
    Channel(Epoll *ep, int fd);

    ~Channel();

    void enableReading();

    int getFd() const;

    uint32_t getEvents() const;

    uint32_t getRevents() const;

    bool getInepoll() const;

    void setInepoll();

    void setRevents(uint32_t _ev);
};
