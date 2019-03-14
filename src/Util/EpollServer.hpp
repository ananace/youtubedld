#pragma once

#include <deque>

#include <sys/epoll.h>

namespace Util
{

class EpollServer
{
public:
    enum
    {
    	kEpollEventLimit = 32,
    	kAnyPort = 0,
    };

    EpollServer(uint16_t aPort = kAnyPort);
    ~EpollServer();

    void setPort(uint16_t aPort);
    uint16_t getPort() const;

    int getListenFd() const;

    bool start();
    void stop();

    bool accept(int& aSocket);

    bool hasEvent() const;
    bool getEvent(epoll_event& ev);
    bool pollEvent(epoll_event& ev);

private:
    void update(bool aBlock);
    bool mark_nonblock(int aFD) const;

    int m_listenFd,
      	m_epollFd;
    uint16_t m_port;

    std::deque<epoll_event> m_events;
};

}
