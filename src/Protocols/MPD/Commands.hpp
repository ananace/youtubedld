#pragma once

#include <array>
#include <string>

namespace Protocols
{

namespace MPD
{

enum Permissions : uint8_t
{
    PERMISSION_NONE = 0,
    PERMISSION_ADD,
    PERMISSION_READ,
    PERMISSION_CONTROL,
    PERMISSION_ADMIN,
};

struct CommandDefinition
{
    const char* Name;
    Permissions Permission;
    int8_t MinArgs, MaxArgs;
    bool Meta;

    constexpr CommandDefinition(const char* aName, Permissions aPermission, int8_t aMinArgs, int8_t aMaxArgs, bool aMeta = false)
        : Name(aName)
        , Permission(aPermission)
        , MinArgs(aMinArgs)
        , MaxArgs(aMaxArgs)
        , Meta(aMeta)
    { }
};

enum AvailableCommandIDs
{
    CommandID_add = 0,
    CommandID_addid,
    CommandID_addtagid,
    CommandID_albumart,
    CommandID_channels,
    CommandID_clear,
    CommandID_clearerror,
    CommandID_cleartagid,
    CommandID_close,
    CommandID_commands,
    CommandID_command_list_begin,
    CommandID_command_list_ok_begin,
    CommandID_command_list_end,
    CommandID_config,
    CommandID_consume,
#ifdef ENABLE_DATABASE
    CommandID_count,
#endif
    CommandID_crossfade,
    CommandID_currentsong,
    CommandID_decoders,
    CommandID_delete,
    CommandID_deleteid,
    CommandID_disableoutput,
    CommandID_enableoutput,
#ifdef ENABLE_DATABASE
    CommandID_find,
    CommandID_findadd,
#endif
    CommandID_idle,
    CommandID_noidle,
    CommandID_kill,
#ifdef ENABLE_DATABASE
    CommandID_list,
    CommandID_listall,
    CommandID_listallinfo,
#endif
    CommandID_listfiles,
#ifdef ENABLE_DATABASE
    CommandID_listmounts,
#endif
#ifdef ENABLE_NEIGHBOR_PLUGINS
    CommandID_listneighbors,
#endif
    CommandID_listpartitions,
    CommandID_listplaylist,
    CommandID_listplaylistinfo,
    CommandID_listplaylists,
    CommandID_load,
    CommandID_lsinfo,
    CommandID_mixrampdb,
    CommandID_mixrampdelay,
#ifdef ENABLE_DATABASE
    CommandID_mount,
#endif
    CommandID_move,
    CommandID_moveid,
    CommandID_newpartition,
    CommandID_next,
    CommandID_notcommands,
    CommandID_outputs,
    CommandID_outputset,
    CommandID_partition,
    CommandID_password,
    CommandID_pause,
    CommandID_ping,
    CommandID_play,
    CommandID_playid,
    CommandID_playlist,
    CommandID_playlistadd,
    CommandID_playlistclear,
    CommandID_playlistdelete,
    CommandID_playlistfind,
    CommandID_playlistid,
    CommandID_playlistinfo,
    CommandID_playlistmove,
    CommandID_playlistsearch,
    CommandID_plchanges,
    CommandID_plchangesposid,
    CommandID_previous,
    CommandID_prio,
    CommandID_prioid,
    CommandID_random,
    CommandID_rangeid,
    CommandID_readcomments,
    CommandID_readmessages,
    CommandID_rename,
    CommandID_repeat,
    CommandID_replay_gain_mode,
    CommandID_replay_gain_status,
    CommandID_rescan,
    CommandID_rm,
    CommandID_save,
#ifdef ENABLE_DATABASE
    CommandID_search,
    CommandID_searchadd,
    CommandID_searchaddpl,
#endif
    CommandID_seek,
    CommandID_seekcur,
    CommandID_seekid,
    CommandID_sendmessage,
    CommandID_setvol,
    CommandID_shuffle,
    CommandID_single,
    CommandID_stats,
    CommandID_status,
#ifdef ENABLE_SQLITE
    CommandID_sticker,
#endif
    CommandID_stop,
    CommandID_subscribe,
    CommandID_swap,
    CommandID_swapid,
    CommandID_tagtypes,
    CommandID_toggleoutput,
#ifdef ENABLE_DATABASE
    CommandID_unmount,
#endif
    CommandID_unsubscribe,
    CommandID_update,
    CommandID_urlhandlers,
    CommandID_volume,

    CommandID_COUNT,
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
    { "command_list_begin", PERMISSION_NONE, 0, 0, true },
    { "command_list_ok_begin", PERMISSION_NONE, 0, 0, true },
    { "command_list_end", PERMISSION_NONE, 0, 0, true },
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
    { "noidle", PERMISSION_READ, 0, 0, true },
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
