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

void MPD::post(uint32_t aClient)
{
    if (aClient == Client_All)
    {
    }
    else
    {
    }
}

void MPD::update()
{
    // Check socket backlog
    // Accept connections
    // Read data
    // Generate messages
    // Handle messages
}

void MPD::handleMessage(uint32_t aClient)
{

}

void MPD::runCommandList(uint32_t aClient)
{
    std::list<std::string> commands;
    int i = 0;
    for (auto it = std::cbegin(commands); it != std::cend(commands); ++i, ++it)
    {
        if (!runCommand(aClient, 0))
        {
            return;
        }
    }

    // Send OK
}

bool MPD::runCommand(uint32_t aClient, uint32_t aCommand)
{
    return false;
}

