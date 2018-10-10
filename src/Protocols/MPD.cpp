#include "MPD.hpp"
#include "MPD/Commands.hpp"

#include <dequeue>
#include <list>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct MPDMessage
{
    uint32_t Client;
    MPD::Commands Command;
    std::string Data;
};

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

bool MPD::init()
{
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
        return false;

    listen(m_socket, 5);
}

void MPD::close()
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
    if (true)
    {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int ret = accept(m_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        int client = Client_None;
        do
        {
            if (m_clientCounter + 1 >= Client_All)
                m_clientCounter = 0;
            client = ++m_clientCounter;
        } while (m_clientMap.count(client) > 0);

        m_clientMap[client].Socket = ret;
    }

    // Read data
    if (true)
    {
        for (auto& cl : m_clientMap)
        {
            char buffer[256];
            int len = read(cl.second.Socket, &buffer[0], 255);
            cl.second.Buffer.append(&buffer[0], len);
        }
    }

    std::dequeue<MPDMessage> messages;
    // Generate messages
    for (auto& cl : m_clientMap)
    {
        if (cl.second.Buffer.empty())
            continue;


    }

    // Handle messages
    for (auto& msg : messages)
    {
        handleMessage(msg.Client);
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

