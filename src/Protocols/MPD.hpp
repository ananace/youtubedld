#pragma once

#include "Base.hpp"

namespace Protocols
{

class MPD : public Base
{
public:
    MPD(uint16_t port = 0);

    void update();

private:
    void handleMessage(uint32_t client);
    void runCommandList(uint32_t client);
    bool runCommand(uint32_t client, uint32_t command);

    uint16_t m_port;
};

}
