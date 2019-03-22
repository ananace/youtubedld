#include "../MPD.hpp"
#include "../../Server.hpp"
#include "../../Util/Logging.hpp"
#include "Acks.hpp"
#include "Commands.hpp"

#include <sstream>

using Protocols::MPDProto;
using namespace Protocols::MPD;

int MPDProto::runCommand(uint32_t aClient, uint32_t aCommand, const std::vector<std::string_view>& aArgs)
{
    (void)aClient;
    (void) aCommand;

    auto& command = AvailableCommands[aCommand];
    Util::Log(Util::Log_Debug) << "[MPD] Running command " << aCommand << "|" << command.Name << " for " << aClient;

    int ret = 0;
    switch (aCommand)
    {
        case CommandID_add:
        case CommandID_addid:
            ret = doAdd(aClient, aCommand, std::string(aArgs.front())); break;
        case CommandID_commands:
            ret = doCommands(aClient, aCommand); break;
        case CommandID_command_list_begin:
        case CommandID_command_list_ok_begin:
        case CommandID_command_list_end:
            ret = doCommandList(aClient, aCommand); break;
        case CommandID_decoders:
            ret = doDecoders(aClient, aCommand); break;
        case CommandID_idle:
            {
                uint16_t flags;
                if (aArgs.empty())
                    flags = Idle_all;
                else
                    for (auto& arg : aArgs)
                    {
                        if (arg == "database")
                            flags |= Idle_database;
                        else if (arg == "update")
                            flags |= Idle_update;
                        else if (arg == "stored_playlist")
                            flags |= Idle_stored_playlist;
                        else if (arg == "playlist")
                            flags |= Idle_playlist;
                        else if (arg == "player")
                            flags |= Idle_player;
                        else if (arg == "mixer")
                            flags |= Idle_mixer;
                        else if (arg == "output")
                            flags |= Idle_output;
                        else if (arg == "partition")
                            flags |= Idle_partition;
                        else if (arg == "sticker")
                            flags |= Idle_sticker;
                        else if (arg == "subscription")
                            flags |= Idle_subscription;
                        else if (arg == "message")
                            flags |= Idle_message;
                        else
                            throw MPDError(ACK_ERROR_ARG, command.Name, "unknown idle \""+std::string(arg)+"\"");
                    }
                ret = doIdle(aClient, aCommand, flags);
            }break;
        case CommandID_noidle:
            ret = doNoidle(aClient, aCommand); break;
        case CommandID_notcommands:
            ret = doCommands(aClient, aCommand); break;
        case CommandID_ping:
            ret = doPing(aClient, aCommand); break;
        case CommandID_plchanges:
            ret = doPlchanges(aClient, aCommand); break;
        case CommandID_status:
            ret = doStatus(aClient, aCommand); break;
        case CommandID_volume:
            ret = doVolume(aClient, aCommand, std::stoi(std::string(aArgs.front()))); break;

        default:
            throw MPDError(ACK_ERROR_UNKNOWN, command.Name, "unimplemented command");
            break;
    }

    return ret;
}

int MPDProto::doAdd(uint32_t aClient, uint32_t aCommand, const std::string& aUrl)
{
    auto ret = getServer().getQueue().addSong(aUrl);
    if (AvailableCommands[aCommand].Name == "addid")
        writeData(aClient, "Id: " + std::to_string(ret.ID) + "\n");
    return ACK_OK;
}
int MPDProto::doClose(uint32_t aClient, uint32_t aCommand)
{
    // TODO
    return ACK_OK_SILENT;
}
int MPDProto::doCommands(uint32_t aClient, uint32_t aCommand)
{
    auto& cl = m_clientMap[aClient];
    Permissions userPerm = Permissions(cl.UserFlags & 0x07);
    bool invert = aCommand == CommandID_notcommands;
    for (auto& cmd : AvailableCommands)
    {
        if (!invert ? (cmd.Permission > userPerm) : (cmd.Permission <= userPerm))
            continue;

        writeData(aClient, std::string(cmd.Name) + "\n");
    }

    return ACK_OK;
}
int MPDProto::doConsume(uint32_t aClient, uint32_t aCommand, bool aConsume)
{
    getServer().getQueue().setConsume(aConsume);
    return ACK_OK;
}
int MPDProto::doCurrentsong(uint32_t aClient, uint32_t aCommand)
{
    return ACK_OK;
}
int MPDProto::doDecoders(uint32_t aClient, uint32_t aCommand)
{
    return ACK_OK;
}

int MPDProto::doIdle(uint32_t aClient, uint32_t aCommand, uint16_t aFlags)
{
    m_clientMap[aClient].IdleFlags = aFlags;
    return ACK_OK_SILENT;
}

int MPDProto::doNoidle(uint32_t aClient, uint32_t aCommand)
{
    m_clientMap[aClient].IdleFlags = Idle_none;
    return ACK_OK;
}

int MPDProto::doPing(uint32_t aClient, uint32_t aCommand)
{
    return ACK_OK;
}

int MPDProto::doPlchanges(uint32_t aClient, uint32_t aCommand)
{
    auto& queue = getServer().getQueue();

    int i = 0;
    for (auto& song : queue)
    {
        std::ostringstream oss;
        oss << "file: " << song.URL << "\n"
            << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(song.Duration).count() << "\n"
            << "Pos: " << i++ << "\n"
            << "Id: " << song.ID << "\n";

        writeData(aClient, oss.str());
    }
    return ACK_OK;
}

int MPDProto::doStatus(uint32_t aClient, uint32_t aCommand)
{
    auto& queue = getServer().getQueue();
    std::ostringstream oss;

    auto status = queue.getStatus();

    oss << "volume: " << int(queue.getVolume() * 100) << "\n"
        << "repeat: " << int(queue.hasRepeat()) << "\n"
        << "random: " << int(queue.hasRandom()) << "\n"
        << "single: " << int(queue.hasSingle()) << "\n"
        << "consume: " << int(queue.hasConsume()) << "\n"
        << "playlist: " << 0 << "\n"
        << "playlistlength: " << queue.size() << "\n"
        << "xfade: " << 0 << "\n";

    switch(status)
    {
    case PS_Stopped:
        oss << "state: " << "stop" << "\n";
        break;

    default:
        {
            std::string statestr = status == PS_Playing ? "play" : "pause";
            auto& cursong = *queue.getSong();
            float seconds = std::chrono::duration<float>(queue.getElapsed()).count();

            oss << "state: " << statestr << "\n"
                << "song: " << queue.indexOf(cursong) << "\n"
                << "songid: " << cursong.ID << "\n"
                << "time: " << int(seconds) << ":" << std::chrono::duration_cast<std::chrono::seconds>(cursong.Duration).count() << "\n"
                << "elapsed: " << seconds << "\n";
        }
    }

    if (queue.hasError())
    {
        oss << "error: " << queue.getError() << "\n";
        queue.clearError();
    }

    writeData(aClient, oss.str());

    return ACK_OK;
}

int MPDProto::doVolume(uint32_t aClient, uint32_t aCommand, int aChange)
{
    auto& queue = getServer().getQueue();

    float vol = queue.getVolume() * 100.f;
    vol += aChange;
    queue.setVolume(vol / 100.f);

    return ACK_OK;
}

int MPDProto::doCommandList(uint32_t aClient, uint32_t aCommand)
{
    auto& cl = m_clientMap[aClient];

    if (aCommand == CommandID_command_list_end)
    {
        cl.InCmdList = false;
        runCommandList(aClient);
        return ACK_OK;
    }

    cl.InCmdList = true;
    cl.CmdListVerbose = aCommand == CommandID_command_list_ok_begin;

    return ACK_OK_SILENT;
}

