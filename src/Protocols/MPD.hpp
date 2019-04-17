#pragma once

#include "Base.hpp"
#include "../Util/EpollServer.hpp"

#include <chrono>
#include <unordered_map>
#include <vector>

#include <cstdint>

#if __has_include(<string_view>)
#include <string_view>
#else
#include <experimental/string_view>
namespace std
{
    using string_view = std::experimental::string_view;
}
#endif

namespace Protocols
{

namespace MPD
{

struct CommandDefinition;

enum
{
    kProtocolVersionMajor = 0,
    kProtocolVersionMinor = 16,
    kProtocolVersionPatch = 1,
};

enum IdleFlags : uint16_t
{
    Idle_database        = 1 << 0,
    Idle_update          = 1 << 1,
    Idle_stored_playlist = 1 << 2,
    Idle_playlist        = 1 << 3,
    Idle_player          = 1 << 4,
    Idle_mixer           = 1 << 5,
    Idle_output          = 1 << 6,
    Idle_options         = 1 << 7,
    Idle_partition       = 1 << 8,
    Idle_sticker         = 1 << 9,
    Idle_subscription    = 1 << 10,
    Idle_message         = 1 << 11,

    Idle_none            = 0,
    Idle_all             = 0xFFFF,
};

}

class MPDProto : public Base
{
public:
    MPDProto(uint16_t port = 0);
    ~MPDProto();

    bool init();
    void close();

    bool supportsPost() const { return true; }
    void post(const Protocols::Event& aEvent, uint32_t aClient);
    bool update();

private:
    struct Client
    {
        int Socket;
        int UserFlags;
        std::string Buffer;
        std::chrono::system_clock::time_point LastActivity;
        uint16_t IdleFlags, ActiveIdleFlags;
        bool InCmdList, CmdListVerbose;
        std::deque<void*> CmdList;

        Client()
            : Socket(0)
            , UserFlags(0)
            , IdleFlags(0)
            , ActiveIdleFlags(0)
            , InCmdList(false)
            , CmdListVerbose(false)
        { }
        Client(int aSocket)
            : Socket(aSocket)
            , UserFlags(0)
            , IdleFlags(0)
            , ActiveIdleFlags(0)
            , InCmdList(false)
            , CmdListVerbose(false)
        { }
    };

    void handleMessage(void* aMessageData, bool aCmdList);
    int runCommandList(uint32_t aClient);
    int runCommand(uint32_t aClient, uint32_t aCommand, const std::vector<std::string_view>& aArgs);
    int runCommand(uint32_t aClient, const Protocols::MPD::CommandDefinition* aCommand, const std::vector<std::string_view>& aArgs);
    int runCommand(uint32_t aClient, uint32_t aCommand, const std::vector<std::string>& aArgs);
    int runCommand(uint32_t aClient, const Protocols::MPD::CommandDefinition* aCommand, const std::vector<std::string>& aArgs);

    int doAdd(uint32_t aClient, uint32_t aCommand, const std::string& aUrl);
    int doAddid(uint32_t aClient, uint32_t aCommand, const std::string& aUrl, int aPosition);
    int doClearerror(uint32_t aClient, uint32_t aCommand);
    int doClose(uint32_t aClient, uint32_t aCommand);
    int doCommands(uint32_t aClient, uint32_t aCommand);
    int doCommandList(uint32_t aClient, uint32_t aCommand);
    int doConsume(uint32_t aClient, uint32_t aCommand, bool aConsume);
    int doCurrentsong(uint32_t aClient, uint32_t aCommand);
    int doDecoders(uint32_t aClient, uint32_t aCommand);
    int doDeleteid(uint32_t aClient, uint32_t aCommand, int aId);
    int doIdle(uint32_t aClient, uint32_t aCommand, uint16_t aIdleFlags);
    int doNext(uint32_t aClient, uint32_t aCommand);
    int doNoidle(uint32_t aClient, uint32_t aCommand);
    int doPause(uint32_t aClient, uint32_t aCommand, bool aPause);
    int doPing(uint32_t aClient, uint32_t aCommand);
    int doPlayid(uint32_t aClient, uint32_t aCommand, int aId);
    int doPlchanges(uint32_t aClient, uint32_t aCommand);
    int doPrevious(uint32_t aClient, uint32_t aCommand);
    int doOption(uint32_t aClient, uint32_t aCommand, bool aOption);
    int doSetvol(uint32_t aClient, uint32_t aCommand, int aVolume);
    int doShuffle(uint32_t aClient, uint32_t aCommand);
    int doSingle(uint32_t aClient, uint32_t aCommand, int8_t aSingle);
    int doStats(uint32_t aClient, uint32_t aCommand);
    int doStatus(uint32_t aClient, uint32_t aCommand);
    int doVolume(uint32_t aClient, uint32_t aCommand, int aChange);

    void writeData(uint32_t aClient, const std::string& aData);

    std::string getIdleName(uint16_t aFlag);

    Util::EpollServer m_server;
    uint32_t m_clientCounter;

    std::unordered_map<uint32_t, Client> m_clientMap;
};

}
