#include "MPD.hpp"
#include "MPD/Commands.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Tokeniser.hpp"

#include <algorithm>
#include <deque>
#include <list>
#include <vector>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using Protocols::MPDProto;
using namespace Protocols::MPD;

struct MPDMessage
{
    uint32_t Client;
    std::string CommandLine;
    const CommandDefinition* Command;
    std::vector<string_view> Arguments;
};

MPDProto::MPDProto(uint16_t port)
    : m_port(port)
    , m_socket(0)
    , m_clientCounter(Client_None)
{
}

MPDProto::~MPDProto()
{
}

bool MPDProto::init()
{
    Util::Log(Util::Log_Debug) << "[MPD] Starting on port " << m_port;
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    if (bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)))
        return false;

    listen(m_socket, 5);
}

void MPDProto::close()
{
}

void MPDProto::post(uint32_t aClient)
{
    if (aClient == Client_All)
    {
    }
    else
    {
    }
}

void MPDProto::update()
{
    if (m_socket == 0)
        return;

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

    std::deque<MPDMessage> messages;
    // Generate messages
    for (auto& cl : m_clientMap)
    {
        if (cl.second.Buffer.empty())
            continue;

        auto cmdTokeniser = Util::LineTokeniser(cl.second.Buffer);
        for (auto& commandLine : cmdTokeniser)
        {
            auto argTokeniser = Util::SpaceTokeniser(cl.second.Buffer);
            auto it = argTokeniser.cbegin();

            auto command = *it++;
            std::vector<string_view> arguments;
            std::copy(it, argTokeniser.cend(), std::back_inserter(arguments));

            const CommandDefinition* cmd = nullptr;;
            auto cit = std::find_if(std::cbegin(AvailableCommands), std::cend(AvailableCommands), [command](const auto& def) { return command == def.Name; });
            if (cit != std::cend(AvailableCommands))
                cmd = &(*cit);

            messages.push_back(MPDMessage{
                cl.first,
                std::string(commandLine),
                cmd,
                std::move(arguments)
            });
        }
    }

    // Handle messages
    for (auto& msg : messages)
    {
        handleMessage(msg.Client);
    }
}

void MPDProto::handleMessage(uint32_t aClient)
{

}

void MPDProto::runCommandList(uint32_t aClient)
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

bool MPDProto::runCommand(uint32_t aClient, uint32_t aCommand)
{
    return false;
}

