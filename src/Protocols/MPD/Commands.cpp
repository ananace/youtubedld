#include "../MPD.hpp"
#include "../../Server.hpp"
#include "../../Util/Logging.hpp"
#include "Acks.hpp"
#include "Commands.hpp"

#include <sstream>

using Protocols::MPDProto;
using namespace Protocols::MPD;

std::string helperCursongToStr(ActivePlaylist& aQueue)
{
    auto* cursong = aQueue.getCurrentSong();
    if (cursong == nullptr)
        return "";

    float seconds = std::chrono::duration<float>(aQueue.getElapsed()).count();
    std::ostringstream oss;
    oss << "song: " << aQueue.indexOf(*cursong) << "\n"
        << "songid: " << cursong->ID << "\n"
        << "time: " << int(seconds) << ":" << std::chrono::duration_cast<std::chrono::seconds>(cursong->Duration).count() << "\n"
        << "duration: " << std::chrono::duration_cast<std::chrono::seconds>(cursong->Duration).count() << "\n"
        << "elapsed: " << seconds << "\n";
    return oss.str();
}

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
            ret = doAdd(aClient, aCommand, std::string(aArgs.front())); break;
        case CommandID_addid:
            {
                int pos = -1;
                if (aArgs.size() > 1)
                    pos = std::stoi(std::string(aArgs[1]));
                ret = doAddid(aClient, aCommand, std::string(aArgs.front()), pos);
            } break;
        case CommandID_clearerror:
            ret = doClearerror(aClient, aCommand); break;
        case CommandID_commands:
            ret = doCommands(aClient, aCommand); break;
        case CommandID_command_list_begin:
        case CommandID_command_list_ok_begin:
        case CommandID_command_list_end:
            ret = doCommandList(aClient, aCommand); break;
        case CommandID_consume:
        case CommandID_random:
        case CommandID_repeat:
            ret = doOption(aClient, aCommand, std::stoi(std::string(aArgs.front())) == 1); break;
        case CommandID_decoders:
            ret = doDecoders(aClient, aCommand); break;
        case CommandID_delete:
        case CommandID_deleteid:
            ret = doDeleteid(aClient, aCommand, std::stoi(std::string(aArgs.front()))); break;
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
        case CommandID_next:
            ret = doNext(aClient, aCommand); break;
        case CommandID_noidle:
            ret = doNoidle(aClient, aCommand); break;
        case CommandID_notcommands:
            ret = doCommands(aClient, aCommand); break;
        case CommandID_pause:
            {
                bool pause = true;
                if (aArgs.size() > 0)
                    pause = std::stoi(std::string(aArgs.front())) == 1;
                else
                    pause = getServer().getQueue().getStatus() != PS_Paused;
                ret = doPause(aClient, aCommand, pause);
            } break;
        case CommandID_ping:
            ret = doPing(aClient, aCommand); break;
        case CommandID_play:
        case CommandID_playid:
            ret = doPlayid(aClient, aCommand, std::stoi(std::string(aArgs.front()))); break;
        case CommandID_plchanges:
            ret = doPlchanges(aClient, aCommand); break;
        case CommandID_previous:
            ret = doPrevious(aClient, aCommand); break;
        case CommandID_setvol:
            ret = doSetvol(aClient, aCommand, std::stoi(std::string(aArgs.front()))); break;
        case CommandID_single:
            {
                SingleStatus single = Single_False;
                if (aArgs.front() == "oneshot")
                    single = Single_Oneshot;
                else
                    single = SingleStatus(std::stoi(std::string(aArgs.front())));
                ret = doSingle(aClient, aCommand, int8_t(single));
            } break;
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
    return ACK_OK;
}
int MPDProto::doAddid(uint32_t aClient, uint32_t aCommand, const std::string& aUrl, int aPosition)
{
    auto ret = getServer().getQueue().addSong(aUrl, aPosition);
    writeData(aClient, "Id: " + std::to_string(ret.ID) + "\n");
    return ACK_OK;
}
int MPDProto::doClearerror(uint32_t aClient, uint32_t aCommand)
{
    getServer().getQueue().clearError();
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
    writeData(aClient, helperCursongToStr(getServer().getQueue()));

    return ACK_OK;
}

int MPDProto::doDecoders(uint32_t aClient, uint32_t aCommand)
{
    return ACK_OK;
}

int MPDProto::doDeleteid(uint32_t aClient, uint32_t aCommand, int aId)
{
    auto& queue = getServer().getQueue();
    if (aCommand == CommandID_delete)
    {
        if (aId >= queue.size())
            throw MPDError(ACK_ERROR_ARG, AvailableCommands[aCommand].Name, "invalid song number");

        aId = queue.getSong(aId)->ID;
    }

    if (!queue.hasSongID(aId))
        throw MPDError(ACK_ERROR_NO_EXIST, AvailableCommands[aCommand].Name, "song does not exist");

    queue.removeSongID(aId);
    return ACK_OK;
}

int MPDProto::doIdle(uint32_t aClient, uint32_t aCommand, uint16_t aFlags)
{
    auto& cl = m_clientMap[aClient];
    uint16_t triggered;
    if ((triggered = (cl.ActiveIdleFlags & aFlags)) != 0)
    {
        writeData(aClient, std::string("changed: ") + getIdleName(triggered) + "\n");

        m_clientMap[aClient].IdleFlags = Idle_none;
        m_clientMap[aClient].ActiveIdleFlags &= ~aFlags;
        return ACK_OK_SILENT;
    }

    m_clientMap[aClient].IdleFlags = aFlags;
    return ACK_OK_SILENT;
}

int MPDProto::doNext(uint32_t aClient, uint32_t aCommand)
{
    auto& queue = getServer().getQueue();
    queue.next();

    return ACK_OK;
}

int MPDProto::doNoidle(uint32_t aClient, uint32_t aCommand)
{
    m_clientMap[aClient].IdleFlags = Idle_none;
    return ACK_OK;
}

int MPDProto::doOption(uint32_t aClient, uint32_t aCommand, bool aOption)
{
    auto& queue = getServer().getQueue();
    if (aCommand == CommandID_consume)
        queue.setConsume(aOption);
    else if (aCommand == CommandID_random)
        queue.setRandom(aOption);
    else
        queue.setRepeat(aOption);

    return ACK_OK;
}

int MPDProto::doPause(uint32_t aClient, uint32_t aCommand, bool aPause)
{
    auto& queue = getServer().getQueue();

    if (aPause)
        queue.pause();
    else
    {
        queue.clearError();
        queue.resume();
    }

    return ACK_OK;
}

int MPDProto::doPing(uint32_t aClient, uint32_t aCommand)
{
    return ACK_OK;
}

int MPDProto::doPlayid(uint32_t aClient, uint32_t aCommand, int aId)
{
    auto& queue = getServer().getQueue();
    if (aCommand == CommandID_play)
    {
        if (aId >= queue.size())
            throw MPDError(ACK_ERROR_ARG, AvailableCommands[aCommand].Name, "invalid song number");

        aId = queue.getSong(aId)->ID;
    }

    if (!queue.hasSongID(aId))
        throw MPDError(ACK_ERROR_NO_EXIST, AvailableCommands[aCommand].Name, "song does not exist");

    queue.clearError();
    queue.playSongID(aId);
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
            << "Title: " << song.Title << "\n";

        if (song.hasArtist())
            oss << "Artist: " << song.getArtist() << "\n";
        if (song.hasAlbum())
            oss << "Album: " << song.getAlbum() << "\n";

        oss << "Pos: " << i++ << "\n"
            << "Id: " << song.ID << "\n";

        writeData(aClient, oss.str());
    }
    return ACK_OK;
}

int MPDProto::doPrevious(uint32_t aClient, uint32_t aCommand)
{
    auto& queue = getServer().getQueue();
    queue.previous();

    return ACK_OK;
}

int MPDProto::doSetvol(uint32_t aClient, uint32_t aCommand, int aVolume)
{
    auto& queue = getServer().getQueue();
    queue.setVolume(aVolume / 100.f);

    return ACK_OK;
}

int MPDProto::doShuffle(uint32_t aClient, uint32_t aCommand)
{
    auto& queue = getServer().getQueue();
    queue.shuffle();

    return ACK_OK;
}

int MPDProto::doSingle(uint32_t aClient, uint32_t aCommand, int8_t aSingle)
{
    auto& queue = getServer().getQueue();
    queue.setSingle(SingleStatus(aSingle));

    return ACK_OK;
}

int MPDProto::doStatus(uint32_t aClient, uint32_t aCommand)
{
    auto& queue = getServer().getQueue();
    std::ostringstream oss;

    auto status = queue.getStatus();

    SingleStatus single = queue.hasSingle();
    std::string singleStr = (single == Single_Oneshot) ? "oneshot" : std::to_string(int(single));

    oss << "volume: " << int(queue.getVolume() * 100) << "\n"
        << "repeat: " << int(queue.hasRepeat()) << "\n"
        << "random: " << int(queue.hasRandom()) << "\n"
        << "single: " << singleStr << "\n"
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
            oss << "state: " << statestr << "\n";
            oss << helperCursongToStr(queue);
        }
    }

    auto* cursong = queue.getCurrentSong();
    if (cursong != nullptr)
    {
        auto* nextsong = queue.nextSong(cursong);

        if (nextsong != nullptr)
            oss << "nextsong: " << queue.indexOf(*nextsong)
                << "nextsongid: " << nextsong->ID;
    }

    if (queue.hasError())
    {
        oss << "error: " << queue.getError() << "\n";
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

