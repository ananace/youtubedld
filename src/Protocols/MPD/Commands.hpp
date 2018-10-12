#pragma once

#include <array>
#include <string>

namespace Protocols
{

namespace MPD
{

enum Permissions : uint8_t
{
    PERMISSION_NONE    = 0,
    PERMISSION_ADD     = 1u << 0u,
    PERMISSION_READ    = 1u << 1u,
    PERMISSION_CONTROL = 1u << 2u,
    PERMISSION_ADMIN   = 1u << 3u,
};

struct CommandDefinition
{
    const char* Name;
    Permissions Permission;
    int8_t MinArgs, MaxArgs;
};

static constexpr struct CommandDefinition AvailableCommands[] = {
    { "add", PERMISSION_ADD, 1, 1 },
    { "addid", PERMISSION_ADD, 1, 2 },
    { "addtagid", PERMISSION_ADD, 3, 3 },
    { "albumart", PERMISSION_READ, 2, 2 },
    { "channels", PERMISSION_READ, 0, 0 },
    { "clear", PERMISSION_CONTROL, 0, 0 },
    { "clearerror", PERMISSION_CONTROL, 0, 0 },
    { "cleartagid", PERMISSION_ADD, 1, 2 },
    { "close", PERMISSION_NONE, -1, -1 },
    { "commands", PERMISSION_NONE, 0, 0 },
    { "config", PERMISSION_ADMIN, 0, 0 },
    { "consume", PERMISSION_CONTROL, 1, 1 },
#ifdef ENABLE_DATABASE
    { "count", PERMISSION_READ, 1, -1 },
#endif
    { "crossfade", PERMISSION_CONTROL, 1, 1 },
    { "currentsong", PERMISSION_READ, 0, 0 },
    { "decoders", PERMISSION_READ, 0, 0 },
    { "delete", PERMISSION_CONTROL, 1, 1 },
    { "deleteid", PERMISSION_CONTROL, 1, 1 },
    { "disableoutput", PERMISSION_ADMIN, 1, 1 },
    { "enableoutput", PERMISSION_ADMIN, 1, 1 },
#ifdef ENABLE_DATABASE
    { "find", PERMISSION_READ, 1, -1 },
    { "findadd", PERMISSION_ADD, 1, -1 },
#endif
    { "idle", PERMISSION_READ, 0, -1 },
    { "kill", PERMISSION_ADMIN, -1, -1 },
#ifdef ENABLE_DATABASE
    { "list", PERMISSION_READ, 1, -1 },
    { "listall", PERMISSION_READ, 0, 1 },
    { "listallinfo", PERMISSION_READ, 0, 1 },
#endif
    { "listfiles", PERMISSION_READ, 0, 1 },
#ifdef ENABLE_DATABASE
    { "listmounts", PERMISSION_READ, 0, 0 },
#endif
#ifdef ENABLE_NEIGHBOR_PLUGINS
    { "listneighbors", PERMISSION_READ, 0, 0 },
#endif
    { "listpartitions", PERMISSION_READ, 0, 0 },
    { "listplaylist", PERMISSION_READ, 1, 1 },
    { "listplaylistinfo", PERMISSION_READ, 1, 1 },
    { "listplaylists", PERMISSION_READ, 0, 0 },
    { "load", PERMISSION_ADD, 1, 2 },
    { "lsinfo", PERMISSION_READ, 0, 1 },
    { "mixrampdb", PERMISSION_CONTROL, 1, 1 },
    { "mixrampdelay", PERMISSION_CONTROL, 1, 1 },
#ifdef ENABLE_DATABASE
    { "mount", PERMISSION_ADMIN, 2, 2 },
#endif
    { "move", PERMISSION_CONTROL, 2, 2 },
    { "moveid", PERMISSION_CONTROL, 2, 2 },
    { "newpartition", PERMISSION_ADMIN, 1, 1 },
    { "next", PERMISSION_CONTROL, 0, 0 },
    { "notcommands", PERMISSION_NONE, 0, 0 },
    { "outputs", PERMISSION_READ, 0, 0 },
    { "outputset", PERMISSION_ADMIN, 3, 3 },
    { "partition", PERMISSION_READ, 1, 1 },
    { "password", PERMISSION_NONE, 1, 1 },
    { "pause", PERMISSION_CONTROL, 0, 1 },
    { "ping", PERMISSION_NONE, 0, 0 },
    { "play", PERMISSION_CONTROL, 0, 1 },
    { "playid", PERMISSION_CONTROL, 0, 1 },
    { "playlist", PERMISSION_READ, 0, 0 },
    { "playlistadd", PERMISSION_CONTROL, 2, 2 },
    { "playlistclear", PERMISSION_CONTROL, 1, 1 },
    { "playlistdelete", PERMISSION_CONTROL, 2, 2 },
    { "playlistfind", PERMISSION_READ, 1, -1 },
    { "playlistid", PERMISSION_READ, 0, 1 },
    { "playlistinfo", PERMISSION_READ, 0, 1 },
    { "playlistmove", PERMISSION_CONTROL, 3, 3 },
    { "playlistsearch", PERMISSION_READ, 1, -1 },
    { "plchanges", PERMISSION_READ, 1, 2 },
    { "plchangesposid", PERMISSION_READ, 1, 2 },
    { "previous", PERMISSION_CONTROL, 0, 0 },
    { "prio", PERMISSION_CONTROL, 2, -1 },
    { "prioid", PERMISSION_CONTROL, 2, -1 },
    { "random", PERMISSION_CONTROL, 1, 1 },
    { "rangeid", PERMISSION_ADD, 2, 2 },
    { "readcomments", PERMISSION_READ, 1, 1 },
    { "readmessages", PERMISSION_READ, 0, 0 },
    { "rename", PERMISSION_CONTROL, 2, 2 },
    { "repeat", PERMISSION_CONTROL, 1, 1 },
    { "replay_gain_mode", PERMISSION_CONTROL, 1, 1 },
    { "replay_gain_status", PERMISSION_READ, 0, 0 },
    { "rescan", PERMISSION_CONTROL, 0, 1 },
    { "rm", PERMISSION_CONTROL, 1, 1 },
    { "save", PERMISSION_CONTROL, 1, 1 },
#ifdef ENABLE_DATABASE
    { "search", PERMISSION_READ, 1, -1 },
    { "searchadd", PERMISSION_ADD, 1, -1 },
    { "searchaddpl", PERMISSION_CONTROL, 2, -1 },
#endif
    { "seek", PERMISSION_CONTROL, 2, 2 },
    { "seekcur", PERMISSION_CONTROL, 1, 1 },
    { "seekid", PERMISSION_CONTROL, 2, 2 },
    { "sendmessage", PERMISSION_CONTROL, 2, 2 },
    { "setvol", PERMISSION_CONTROL, 1, 1 },
    { "shuffle", PERMISSION_CONTROL, 0, 1 },
    { "single", PERMISSION_CONTROL, 1, 1 },
    { "stats", PERMISSION_READ, 0, 0 },
    { "status", PERMISSION_READ, 0, 0 },
#ifdef ENABLE_SQLITE
    { "sticker", PERMISSION_ADMIN, 3, -1 },
#endif
    { "stop", PERMISSION_CONTROL, 0, 0 },
    { "subscribe", PERMISSION_READ, 1, 1 },
    { "swap", PERMISSION_CONTROL, 2, 2 },
    { "swapid", PERMISSION_CONTROL, 2, 2 },
    { "tagtypes", PERMISSION_READ, 0, -1 },
    { "toggleoutput", PERMISSION_ADMIN, 1, 1 },
#ifdef ENABLE_DATABASE
    { "unmount", PERMISSION_ADMIN, 1, 1 },
#endif
    { "unsubscribe", PERMISSION_READ, 1, 1 },
    { "update", PERMISSION_CONTROL, 0, 1 },
    { "urlhandlers", PERMISSION_READ, 0, 0 },
    { "volume", PERMISSION_CONTROL, 1, 1 },
};

}

}
