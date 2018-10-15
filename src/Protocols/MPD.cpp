#include "MPD.hpp"
#include "MPD/Commands.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Tokeniser.hpp"

#include <algorithm>
#include <deque>
#include <list>
#include <vector>
#include <iostream>
#include <iterator>
#include <sstream>

#include <fcntl.h>
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
    close();
}

bool MPDProto::init()
{
    Util::Log(Util::Log_Debug) << "[MPD] Starting on port " << m_port;

    m_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_port);

    int ret;
    if ((ret = bind(m_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) != 0)
    {
        Util::Log(Util::Log_Warning) << "[MPD] Bind to port " << m_port << " failed. (" << ret << ")";
        return false;
    }

    if ((ret = listen(m_socket, 5)) != 0)
    {
        Util::Log(Util::Log_Warning) << "[MPD] Starting listening for TCP connections failed (" << ret << ")";
        return false;
    }

    return true;
}

void MPDProto::close()
{
    if (m_socket == 0)
        return;

    ::close(m_socket);
    m_socket = 0;
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

    bool waiting = false;
    // Check socket backlog
    if (true)
    {
        // Util::Log(Util::Log_Debug) << "[MPD] Enter backlog";

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(m_socket, &fds);

        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 5000;

        int ret = select(m_socket + 1, &fds, nullptr, nullptr, &tv);

        waiting = FD_ISSET(m_socket, &fds);

        // Util::Log(Util::Log_Debug) << "[MPD] Exit backlog";
    }

    // Accept connections
    if (waiting)
    {
        // Util::Log(Util::Log_Debug) << "[MPD] Enter accept";

        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int ret = accept(m_socket, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (ret != EWOULDBLOCK)
        {
            int client = Client_None;
            do
            {
                if (m_clientCounter + 1 >= Client_All)
                    m_clientCounter = 0;
                client = ++m_clientCounter;
            } while (m_clientMap.count(client) > 0);

            m_clientMap[client].Socket = ret;

            char buf[64];
            sprintf(buf, "OK MPD %i.%i.%i\n", kProtocolVersionMajor, kProtocolVersionMinor, kProtocolVersionPatch);
            writeData(client, buf);

            Util::Log(Util::Log_Info) << "[MPD] Accepted connection from " << client_addr.sin_addr.s_addr << ":" << client_addr.sin_port;
        }

        // Util::Log(Util::Log_Debug) << "[MPD] Exit accept";
    }

    // Read data
    if (!m_clientMap.empty())
    {
        // Util::Log(Util::Log_Debug) << "[MPD] Enter read";

        fd_set fds;
        FD_ZERO(&fds);

        int maxSocket;
        for (auto& cl : m_clientMap)
        {
            FD_SET(cl.second.Socket, &fds);
            maxSocket = std::max(maxSocket, cl.second.Socket);
        }

        timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 5000;

        int ret = select(maxSocket + 1, &fds, nullptr, nullptr, &tv);

        for (auto& cl : m_clientMap)
        {
            if (!FD_ISSET(cl.second.Socket, &fds))
                continue;

            char buffer[256];
            int len = recv(cl.second.Socket, &buffer[0], 255, 0);
            Util::Log(Util::Log_Debug) << "[MPD] recv = " << len;
            if (len > 0)
            {
                cl.second.Buffer.append(&buffer[0], len);
                Util::Log(Util::Log_Info) << "[MPD] Read " << len << "B (\"" << std::string(buffer, len) << "\")";
            }
        }

        // Util::Log(Util::Log_Debug) << "[MPD] Exit read";
    }

    std::deque<MPDMessage> messages;
    // Generate messages
    if (!m_clientMap.empty())
    {
        for (auto& cl : m_clientMap)
        {
            if (cl.second.Buffer.empty())
                continue;

            // Util::Log(Util::Log_Debug) << "Buffer: \"" << cl.second.Buffer << "\"";
            auto cmdTokeniser = Util::LineTokeniser(cl.second.Buffer);
            for (auto& commandLine : cmdTokeniser)
            {
                // if (commandLine.empty())
                //     continue;

                if (commandLine.back() == '\r')
                    commandLine.remove_suffix(1);

                Util::Log(Util::Log_Debug) << "Line: \"" << std::string(commandLine) << "\"";

                auto argTokeniser = Util::SpaceTokeniser(commandLine);
                auto it = argTokeniser.cbegin();

                auto command = *it++;
                std::vector<string_view> arguments;
                std::copy(it, argTokeniser.cend(), std::back_inserter(arguments));

                // Util::Log(Util::Log_Debug) << "Cmd: \"" << std::string(command) << "\"";

                const CommandDefinition* cmd = nullptr;
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

            cl.second.Buffer = cl.second.Buffer.substr(cl.second.Buffer.find_last_of('\n') + 1);
        }
    }

    // Handle messages
    for (auto& msg : messages)
        handleMessage(&msg);
}

void MPDProto::handleMessage(void* aMessageData)
{
    auto& msg = *reinterpret_cast<MPDMessage*>(aMessageData);

    if (msg.Command != nullptr)
    {
        std::ostringstream argsString;
        std::copy(msg.Arguments.begin(), msg.Arguments.end() - 1, std::ostream_iterator<string_view>(argsString, ", "));
        std::copy(msg.Arguments.end() - 1, msg.Arguments.end(), std::ostream_iterator<string_view>(argsString));

        Util::Log(Util::Log_Info) << "[MPD] Receieved command from " << msg.Client << ": "
            << msg.Command->Name << "(" << argsString.str() << ")";
    }
    else
        Util::Log(Util::Log_Info) << "[MPD] Receieved unknown command from " << msg.Client;
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

void MPDProto::writeData(uint32_t aClient, const std::string& aData)
{
    auto& cl = m_clientMap.at(aClient);

    send(cl.Socket, aData.c_str(), aData.size(), 0);
}
