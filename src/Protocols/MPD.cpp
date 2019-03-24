#include "MPD.hpp"
#include "MPD/Acks.hpp"
#include "MPD/Commands.hpp"
#include "../Util/Logging.hpp"
#include "../Util/Tokeniser.hpp"
#include "../Server.hpp"

#include <algorithm>
#include <deque>
#include <list>
#include <vector>
#include <iostream>
#include <iterator>
#include <sstream>

#include <sys/socket.h>

using Protocols::MPDProto;
using namespace Protocols::MPD;

struct MPDMessage
{
    uint32_t Client;
    std::string CommandLine;
    const CommandDefinition* Command;
    std::vector<std::string_view> Arguments;
};

MPDProto::MPDProto(uint16_t port)
    : m_server(port)
    , m_clientCounter(Client_None)
    , m_lastEvent(0)
{
}

MPDProto::~MPDProto()
{
    close();
}

bool MPDProto::init()
{
    Util::Log(Util::Log_Info) << "[MPD] Starting on port " << m_server.getPort();

    return m_server.start();
}

void MPDProto::close()
{
    m_server.stop();
}

void MPDProto::post(const Protocols::Event& aEv, uint32_t aClient)
{
    if (aClient == Client_None)
        return;

    if (aClient != Client_All)
        return;

    uint16_t triggeredIdleFlag = 0;
    if (aEv.Type == Event_StateChange)
        triggeredIdleFlag = Idle_player;
    else if (aEv.Type == Event_QueueChange)
        triggeredIdleFlag = Idle_playlist;
    else if (aEv.Type == Event_VolumeChange)
        triggeredIdleFlag = Idle_mixer;
    else if (aEv.Type == Event_OptionChange)
        triggeredIdleFlag = Idle_options;

    if (triggeredIdleFlag == 0)
        return;

    m_lastEvent = triggeredIdleFlag;
    std::string idleFlagName = getIdleName(triggeredIdleFlag);

    Util::Log(Util::Log_Debug) << "[MPD] Posting event about " << idleFlagName;

    for (auto& it : m_clientMap)
    {
        if ((it.second.IdleFlags & triggeredIdleFlag) != 0)
        {
            writeData(it.first, "changed: " + idleFlagName + "\n");
        }
    }
}

bool MPDProto::update()
{
    bool handled = false;
    epoll_event ev;
    for (int i = 0; i < 10 && m_server.pollEvent(ev); ++i)
    {
        if (ev.events & EPOLLRDHUP)
        {
            Util::Log(Util::Log_Info) << "[MPD] Connection from " << ev.data.fd << " closed";
            ::close(ev.data.fd);
            auto cl = std::find_if(std::begin(m_clientMap), std::end(m_clientMap), [ev](auto& it) { return it.second.Socket == ev.data.fd; });
            if (cl != std::end(m_clientMap))
                m_clientMap.erase(cl);
        }
        else if (ev.events & EPOLLERR ||
            ev.events & EPOLLHUP ||
            !(ev.events & EPOLLIN))
        {
            Util::Log(Util::Log_Info) << "[MPD] Connection error from " << ev.data.fd;
        }
        else if (ev.data.fd == m_server.getListenFd())
        {
            // Accept up to 5 connections per update
            int newConn = 0;
            for (int i = 0; i < 5 && m_server.accept(newConn); ++i)
            {
                int client = Client_None;
                do
                {
                    if (m_clientCounter + 1 >= Client_All)
                        m_clientCounter = 0;
                    client = ++m_clientCounter;
                } while (m_clientMap.count(client) > 0);

                m_clientMap[client].Socket = newConn;

                char buf[64];
                snprintf(buf, 64, "OK MPD %i.%i.%i\n", kProtocolVersionMajor, kProtocolVersionMinor, kProtocolVersionPatch);
                writeData(client, buf);
            }
        }
        else
        {
            auto cl = std::find_if(std::begin(m_clientMap), std::end(m_clientMap), [ev](auto& it) { return it.second.Socket == ev.data.fd; });
            if (cl == std::end(m_clientMap))
            {
                Util::Log(Util::Log_Warning) << "[MPD] Received data from unconnected client, ignoring";
                continue;
            }

            auto& clBuf = cl->second.Buffer;

            char buffer[256];
            int len = 0;
            do
            {
                len = recv(ev.data.fd, &buffer[0], 255, 0);
                if (len > 0)
                {
                    clBuf.append(&buffer[0], len);
                    Util::Log(Util::Log_Info) << "[MPD] Read " << len << "B from " << cl->first << " (" << std::string(buffer, len -1) << ")";
                }
            } while(len > 0);
        }

        std::deque<MPDMessage> messages;
        // Generate messages
        if (!m_clientMap.empty())
        {
            for (auto& cl : m_clientMap)
            {
                if (cl.second.Buffer.empty())
                    continue;

                auto cmdTokeniser = Util::LineTokeniser(cl.second.Buffer);
                for (auto& commandLine : cmdTokeniser)
                {
                    // if (commandLine.empty())
                    //     continue;

                    if (commandLine.back() == '\r')
                        commandLine.remove_suffix(1);

                    auto argTokeniser = Util::SpaceTokeniser(commandLine);
                    auto it = argTokeniser.cbegin();

                    auto command = *it++;
                    std::vector<std::string_view> arguments;
                    std::string_view carry;
                    for (; it != argTokeniser.cend(); ++it)
                    {
                        auto& cur = *it;
                        if (cur.front() == '"' && cur.back() == '"')
                            cur = cur.substr(1, cur.size() - 2);
                        else if (cur.front() == '"')
                        {
                            cur.remove_prefix('"');
                            carry = cur;
                            continue;
                        }
                        else if (cur.back() == '"')
                        {
                            cur.remove_suffix('"');
                            arguments.push_back(std::string_view(carry.data(), carry.length() + cur.length() + 1));
                            continue;
                        }

                        arguments.push_back(cur);
                    }

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
            handleMessage(&msg, false);
        if (!messages.empty())
            handled = true;
    }

    return handled;
}

void MPDProto::handleMessage(void* aMessageData, bool aCmdList)
{
    auto& msg = *reinterpret_cast<MPDMessage*>(aMessageData);
    auto& cl = m_clientMap[msg.Client];

    if (msg.Command != nullptr)
    {
        auto index = std::find_if(std::begin(AvailableCommands), std::end(AvailableCommands), [msg](auto& it) { return it.Name == msg.Command->Name; });

        if (!aCmdList && cl.InCmdList && msg.Command->Name != "command_list_end")
        {
            Util::Log(Util::Log_Debug) << "[MPD] Queuing command " << (index - std::begin(AvailableCommands)) << "|" << msg.Command->Name << " for " << msg.Client;
            cl.CmdList.push_back(new MPDMessage(msg));
            return;
        }

        if ((msg.Command->MinArgs > 0 && msg.Arguments.size() < msg.Command->MinArgs) || (msg.Command->MaxArgs >= 0 && msg.Arguments.size() > msg.Command->MaxArgs))
        {
            if (aCmdList)
                throw MPDError(ACK_ERROR_ARG, msg.Command->Name, "wrong number of arguments");
            writeData(msg.Client, "ACK [2@0] {" + std::string(msg.Command->Name) + "} wrong number of arguments for \"" + std::string(msg.Command->Name) + "\"\n");
            return;
        }

        std::ostringstream argsString;
        for (auto& arg : msg.Arguments)
        {
            if (arg != msg.Arguments.front())
                argsString << ", ";
            argsString << arg;
        }

        // Util::Log(Util::Log_Info) << "[MPD] Receieved command from " << msg.Client << ": "
        //     << msg.Command->Name << "(" << argsString.str() << ")";


        try
        {
            auto ret = runCommand(msg.Client, index - std::begin(AvailableCommands), msg.Arguments);
            if (ret == ACK_OK && aCmdList)
                writeData(msg.Client, "list_OK\n");
            else if (ret == ACK_OK)
                writeData(msg.Client, "OK\n");
        }
        catch (const MPDError& ex)
        {
            if (aCmdList)
                throw ex;
            writeData(msg.Client, ex.what());
        }
    }
    else
    {
        if (aCmdList)
            throw MPDError(ACK_ERROR_UNKNOWN, "unknown command\""+std::string(msg.CommandLine)+"\"");
        writeData(msg.Client, "ACK [5@0] {} unknown command \"" + std::string(msg.CommandLine) + "\"\n");
        Util::Log(Util::Log_Info) << "[MPD] Receieved unknown command \"" << msg.CommandLine << "\" from " << msg.Client;
    }
}

int MPDProto::runCommandList(uint32_t aClient)
{
    auto& cl = m_clientMap[aClient];

    auto commands = cl.CmdList;
    cl.CmdList.clear();

    int i = 0;
    int ret = 0;
    for (auto it = std::cbegin(commands); it != std::cend(commands); ++i, ++it)
    {
        MPDMessage* msg = reinterpret_cast<MPDMessage*>(*it);

        try
        {
            handleMessage(msg, true);
            delete msg;
        }
        catch (MPDError err)
        {
            delete msg;
            err.setCmdListIndex(i);
            throw err;
        }
    }

    return ACK_OK;
}

int MPDProto::runCommand(uint32_t aClient, uint32_t aCommand, const std::vector<std::string>& aArgs)
{
    std::vector<std::string_view> view(aArgs.size());
    for (auto& arg : aArgs)
        view.emplace_back(arg);
    return runCommand(aClient, aCommand, view);
}

int MPDProto::runCommand(uint32_t aClient, const CommandDefinition* aCommandDef, const std::vector<std::string>& aArgs)
{
    std::vector<std::string_view> view(aArgs.size());
    for (auto& arg : aArgs)
        view.emplace_back(arg);
    auto index = std::find_if(std::begin(AvailableCommands), std::end(AvailableCommands), [aCommandDef](auto& it) { return it.Name == aCommandDef->Name; });
    return runCommand(aClient, index, view);
}

int MPDProto::runCommand(uint32_t aClient, const CommandDefinition* aCommandDef, const std::vector<std::string_view>& aArgs)
{
    auto index = std::find_if(std::begin(AvailableCommands), std::end(AvailableCommands), [aCommandDef](auto& it) { return it.Name == aCommandDef->Name; });
    return runCommand(aClient, index, aArgs);
}

void MPDProto::writeData(uint32_t aClient, const std::string& aData)
{
    auto& cl = m_clientMap.at(aClient);

    send(cl.Socket, aData.c_str(), aData.size(), 0);

    Util::Log(Util::Log_Info) << "[MPD] Wrote " << aData.size() << "B to " << aClient << " (" << aData.substr(0, aData.size() - 1) << ")";
}

std::string MPDProto::getIdleName(uint16_t aFlag)
{
    if (aFlag == Idle_player)
        return "player";
    else if (aFlag == Idle_mixer)
        return "mixer";
    else if (aFlag == Idle_playlist)
        return "playlist";
    else if (aFlag == Idle_options)
        return "options";
    return "TODO";
}
