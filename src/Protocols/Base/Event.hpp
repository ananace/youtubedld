#pragma once

#include <string>

#include <cstdint>

namespace Protocols
{

enum EventType
{
    Event_Invalid = 0,

    Event_AddSong,
    Event_RemoveSong,
    Event_MoveSong,

    Event_StateChange,
    Event_VolumeChange,
    Event_QueueChange,
    Event_OptionChange,
    Event_SongChange,

    Event_ActionNext,
    Event_ActionPrevious,
    Event_ActionPlay,
    Event_ActionPause,
    Event_ActionStop,
};

struct Event
{
    Event(EventType aType = Event_Invalid, int aClient = 0)
        : Type(aType)
        , Client(aClient)
    { }
    ~Event() {}

    EventType Type;
    int Client;
    union
    {
        struct {
            int SongID;
            size_t Position;
        } AddSong;
        struct {
            int SongID;
            size_t Position;
        } RemoveSong;
        struct {
            int SongID;
            size_t OldPosition,
                   NewPosition;
        } MoveSong;
    };
};

}
