#include "MPD.hpp"
#include "MPD/Commands.hpp"

#include <list>

using Protocols::MPD;

MPD::MPD(uint16_t port)
    : m_port(port)
    , m_socket(0)
    , m_clientCounter(Client_None)
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
    if (false)
    {
        int client = Client_None;
        do
        {
            if (m_clientCounter + 1 >= Client_All)
                m_clientCounter = 0;
            client = ++m_clientCounter;
        } while (m_clientMap.count(client) > 0);

        m_clientMap[client];
    }

    // Read data
    // Generate messages
    // Handle messages
    for (auto& cl : m_clientMap)
    {
        auto sock = cl.second;
        handleMessage(cl.first);
    }
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

