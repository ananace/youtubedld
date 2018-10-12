#pragma once

#include "Base.hpp"

#include <unordered_map>

#include <cstdint>

namespace Protocols
{

class MPDProto : public Base
{
public:
    MPDProto(uint16_t port = 0);
    ~MPDProto();

    bool init();
    void close();

    bool supportsPost() const { return true; }
    void post(uint32_t aClient);
    void update();

private:
    struct Client
    {
        int Socket;
        std::string Buffer;
    };

    void handleMessage(uint32_t aClient);
    void runCommandList(uint32_t aClient);
    bool runCommand(uint32_t aClient, uint32_t aCommand);

    uint16_t m_port;
    int m_socket;
    uint32_t m_clientCounter;

    std::unordered_map<uint32_t, Client> m_clientMap;
};

}
