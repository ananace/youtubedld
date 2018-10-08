#include "MPD.hpp"
#include "MPD/Commands.hpp"

#include <list>

using Protocols::MPD;

MPD::MPD(uint16_t port)
    : m_port(port)
{
}

void MPD::Update()
{
    // Check socket backlog
    // Accept connections
    // Read data
    // Generate messages
    // Handle messages
}

void MPD::HandleMessage(uint32_t client)
{

}

void MPD::RunCommandList(uint32_t client)
{
    std::list<std::string> commands;
    int i = 0;
    for (auto& it = commands.cbegin(); it != commands.cend(); ++i, ++it)
    {
        if (!RunCommand(client, 0))
        {
            return;
        }
    }

    // Send OK
}
