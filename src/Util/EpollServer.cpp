#include "EpollServer.hpp"
#include "Logging.hpp"

#include <array>
#include <algorithm>

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using Util::EpollServer;

EpollServer::EpollServer(uint16_t aPort)
    : m_listenFd(-1)
    , m_epollFd(-1)
    , m_port(aPort)
{
}
EpollServer::~EpollServer()
{
    stop();
}

void EpollServer::setPort(uint16_t aPort)
{
    m_port = aPort;
}
uint16_t EpollServer::getPort() const
{
    return m_port;
}

int EpollServer::getListenFd() const
{
    return m_listenFd;
}

bool EpollServer::start()
{
    if (m_listenFd > 0)
    	return false;

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    int ret;
    if ((ret = bind(m_listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) != 0)
    {
        Util::Log(Util::Log_Warning) << "[Epoll] Bind to port " << m_port << " failed. (" << ret << ")";
        return false;
    }

    if ((ret = listen(m_listenFd, SOMAXCONN)) != 0)
    {
        Util::Log(Util::Log_Warning) << "[Epoll] Starting listening for TCP connections failed (" << ret << ")";
        return false;
    }

    if (!mark_nonblock(m_listenFd))
    	return false;

    m_epollFd = epoll_create1(0);
    if (m_epollFd == -1)
    {
        Util::Log(Util::Log_Error) << "[Epoll] Failed to create epoll (" << errno << ")";
        return false;
    }

    struct epoll_event event;
    event.data.fd = m_listenFd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_listenFd, &event) == -1)
    {
        Util::Log(Util::Log_Error) << "[Epoll] Failed to register listen socket in epoll (" << errno << ")";
        return false;
    }

    return true;
}
void EpollServer::stop()
{
    if (m_listenFd > 0)
      	::close(m_listenFd);
    m_listenFd = 0;
    if (m_epollFd > 0)
      	::close(m_epollFd);
    m_epollFd = 0;
}

bool EpollServer::accept(int& aSocket)
{
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    int infd = ::accept(m_listenFd, &in_addr, &in_len);
    if (infd == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) // Done processing incoming connections
            return false;
        else
        {
            Util::Log(Util::Log_Error) << "[Epoll] Failed to accept connection (" << errno << ")";
            return false;
        }
    }

    std::string hbuf(NI_MAXHOST, '\0');
    std::string sbuf(NI_MAXSERV, '\0');
    if (getnameinfo(&in_addr, in_len,
                    const_cast<char*>(hbuf.data()), hbuf.size(),
                    const_cast<char*>(sbuf.data()), sbuf.size(),
		    NI_NUMERICHOST | NI_NUMERICSERV) == 0)
    	Util::Log(Util::Log_Info) << "[Epoll] Accepted connection from " << hbuf << ":" << sbuf;

    if (!mark_nonblock(infd))
        return false;

    struct epoll_event event;
    event.data.fd = infd;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, infd, &event) == -1)
    {
        Util::Log(Util::Log_Error) << "[Epoll] Failed to register accepted socket in epoll (" << errno << ")";
        return false;
    }

    aSocket = infd;
    return true;
}

bool EpollServer::hasEvent() const
{
    return !m_events.empty();
}
bool EpollServer::getEvent(epoll_event& ev)
{
    if (m_events.empty())
    	update(true);
    else
    	update(false);

    if (m_events.empty())
    	return false;

    ev = m_events.front();
    m_events.pop_front();

    return true;
}
bool EpollServer::pollEvent(epoll_event& ev)
{
    update(false);
    if (m_events.empty())
    	return false;

    ev = m_events.front();
    m_events.pop_front();

    return true;
}

void EpollServer::update(bool aBlock)
{
    std::array<struct epoll_event, kEpollEventLimit> events;

    int count = epoll_wait(m_epollFd, events.data(), kEpollEventLimit, aBlock ? -1 : 0);
    if (count < 0)
    {
	Util::Log(Util::Log_Error) << "[Epoll] epoll_wait failed with error " << errno;
    	return;
    }
    std::copy_n(std::begin(events), count, std::back_inserter(m_events));
}
bool EpollServer::mark_nonblock(int aFD) const
{
    int flags = fcntl(aFD, F_GETFL, 0);
    if (flags == -1)
    {
	Util::Log(Util::Log_Error) << "[Epoll] fcntl(F_GETFL) failed with error " << errno;
        return false;
    }

    flags |= O_NONBLOCK;
    int s = fcntl(aFD, F_SETFL, flags);
    if (s == -1)
    {
	Util::Log(Util::Log_Error) << "[Epoll] fcntl(F_SETFL) failed with error " << errno;
        return false;
    }

    return true;
}
