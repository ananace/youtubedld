#pragma once

#include "Base.hpp"

namespace Protocols
{

class MPD : public Base
{
public:
    MPD(uint16_t port = 0);

    void Update();

private:
    void HandleMessage(uint32_t client);
    void RunCommandList(uint32_t client);
    bool RunCommand(uint32_t client, uint32_t command);

    uint16_t m_port;
};

}
