#pragma once

#include "Base.hpp"

#include <cstdint>

namespace Protocols
{

class MPD : public Base
{
public:
    MPD(uint16_t port = 0);
    ~MPD();

    bool supportsPost() const { return true; }
    void post(uint32_t aClient);
    void update();

private:
    void handleMessage(uint32_t aClient);
    void runCommandList(uint32_t aClient);
    bool runCommand(uint32_t aClient, uint32_t aCommand);

    uint16_t m_port;
};

}
