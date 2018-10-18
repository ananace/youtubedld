#pragma once

#include <string>

#include <cstdint>

namespace Protocols
{

enum EventType
{
    Event_Invalid = 0,

    Event_AppendSong,
    Event_InsertSong,
    Event_RemoveSong,
    Event_MoveSong,

    Event_ActionNext,
    Event_ActionPrevious,
    Event_ActionPlay,
    Event_ActionPause,
    Event_ActionStop,
};


struct Event
{
    Event()
        : Type(Event_Invalid)
    { }
    ~Event() {}

    EventType Type;
    union
    {
        struct {
            std::string Song;
        } AppendSong;
        struct {
            std::string Song;
            size_t Position;
        } InsertSong;
        struct {
            size_t Position;
        } RemoveSong;
        struct {
            size_t OldPosition,
                   NewPosition;
        } MoveSong;
    };
};

}
