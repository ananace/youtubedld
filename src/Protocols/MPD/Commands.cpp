#include "../MPD.hpp"
#include "../../Server.hpp"
#include "../../Util/Logging.hpp"
#include "Acks.hpp"
#include "Commands.hpp"

#include <sstream>

using Protocols::MPDProto;
using namespace Protocols::MPD;

typedef std::pair<int,int> MPDRange;

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

const Protocols::MPD::CommandDefinition& MPDProto::CommandParams::getDefinition() const
{
    return AvailableCommands[Command];
}

bool MPDProto::CommandParams::hasArg(size_t aIndex) const
{
    return Arguments.size() > aIndex;
}

template<>
bool MPDProto::CommandParams::isArg<std::string>(size_t aIndex) const
{
    return hasArg(aIndex) && true;
}
template<>
std::string MPDProto::CommandParams::getArg(size_t aIndex) const
{
    return std::string(Arguments.at(aIndex));
}

template<>
bool MPDProto::CommandParams::isArg<MPDRange>(size_t aIndex) const
{
    if (!hasArg(aIndex))
        return false;
    auto& arg = Arguments.at(aIndex);
    return std::find_if(arg.cbegin(), arg.cend(), [](char c) { return !std::isdigit(c) && c != ':'; }) == arg.cend();
}
template<>
MPDRange MPDProto::CommandParams::getArg(size_t aIndex) const
{
    auto str = getArg<std::string>(aIndex);
    int start = std::stoi(str.substr(0, str.find_first_of(':'))),
        end = std::stoi(str.substr(str.find_first_of(':') + 1));

    return MPDRange(start, end);
}

template<>
bool MPDProto::CommandParams::isArg<int>(size_t aIndex) const
{
    if (!hasArg(aIndex))
        return false;
    auto& arg = Arguments.at(aIndex);
    return std::find_if(arg.cbegin(), arg.cend(), [](char c) { return !std::isdigit(c); }) == arg.cend();
}
template<>
int MPDProto::CommandParams::getArg(size_t aIndex) const
{
    return std::stoi(std::string(Arguments.at(aIndex)));
}

int MPDProto::runCommand(uint32_t aClient, uint32_t aCommand, const std::vector<std::string_view>& aArgs)
{
    CommandParams params { aClient, aCommand, aArgs };

    auto& command = params.getDefinition();
    Util::Log(Util::Log_Debug) << "[MPD] Running command " << aCommand << "|" << command.Name << " for " << aClient;

    static std::unordered_map<std::string, int(MPDProto::*)(const CommandParams&)> cmdMap = {
        { "add", &MPDProto::doAdd },
        { "addid", &MPDProto::doAddid },
        { "clearerror", &MPDProto::doClearerror },
        { "commands", &MPDProto::doCommands },
        { "notcommands", &MPDProto::doCommands }
    };

    if (cmdMap.count(command.Name) > 0)
        return (this->*cmdMap.at(command.Name))(params);

    int ret = 0;
    switch (aCommand)
    {
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

int MPDProto::doAdd(const CommandParams& aParams)
{
    auto url = aParams.getArg<std::string>(0);
    auto ret = getServer().getQueue().addSong(url);
    return ACK_OK;
}
int MPDProto::doAddid(const CommandParams& aParams)
{
    std::string url = aParams.getArg<std::string>(0);
    int pos = -1;
    if (aParams.hasArg(1))
        pos = aParams.getArg<int>(1);

    auto ret = getServer().getQueue().addSong(url, pos);
    writeData(aParams.Client, "Id: " + std::to_string(ret.ID) + "\n");
    return ACK_OK;
}
int MPDProto::doClearerror(const CommandParams&)
{
    getServer().getQueue().clearError();
    return ACK_OK;
}
int MPDProto::doClose(const CommandParams&)
{
    // TODO
    return ACK_OK_SILENT;
}
int MPDProto::doCommands(const CommandParams& aParams)
{
    auto& cl = m_clientMap[aParams.Client];
    Permissions userPerm = Permissions(cl.UserFlags & 0x07);
    bool invert = aParams.Command == CommandID_notcommands;
    for (auto& cmd : AvailableCommands)
    {
        if (!invert ? (cmd.Permission > userPerm) : (cmd.Permission <= userPerm))
            continue;

        writeData(aParams.Client, std::string(cmd.Name) + "\n");
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

int MPDProto::doStats(uint32_t aClient, uint32_t aCommand)
{
    auto uptime = getServer().getUptime();

    std::ostringstream oss;

    oss << "artists: 0\n"
        << "albums: 0\n"
        << "songs: 0\n"
        << "uptime: " << std::chrono::duration_cast<std::chrono::seconds>(uptime).count() << "\n"
        << "db_playtime: 0\n"
        << "db_update: 0\n"
        << "playtime: 0\n";

    writeData(aClient, oss.str());

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
            oss << "nextsong: " << queue.indexOf(*nextsong) << "\n"
                << "nextsongid: " << nextsong->ID << "\n";
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

