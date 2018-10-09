#include "MPD.hpp"
#include "MPD/Commands.hpp"

#include <list>

using Protocols::MPD;

MPD::MPD(uint16_t port)
    : m_port(port)
{
}

MPD::~MPD()
{
}

bool MPD::supportsPost() const
{
    return true;
}

void MPD::post()
{
}

void MPD::update()
{
    // Check socket backlog
    // Accept connections
    // Read data
    // Generate messages
    // Handle messages
}

void MPD::handleMessage(uint32_t client)
{

}

void MPD::runCommandList(uint32_t client)
{
    std::list<std::string> commands;
    int i = 0;
    for (auto it = std::cbegin(commands); it != std::cend(commands); ++i, ++it)
    {
        if (!runCommand(client, 0))
        {
            return;
        }
    }

    // Send OK
}

bool MPD::runCommand(uint32_t client, uint32_t command)
{
}
