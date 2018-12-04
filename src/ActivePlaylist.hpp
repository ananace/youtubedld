#pragma once

#include "Playlist.hpp"

#include <gstreamermm.h>

enum PlayFlags : uint8_t
{
    PF_Consume = 1u << 0u,
    PF_Random  = 1u << 1u,
    PF_Repeat  = 1u << 2u,
    PF_Single  = 1u << 3u,
};

class ActivePlaylist : public Playlist
{
public:
    ActivePlaylist();

    void init(Server& aServer);
    void update();

    Glib::RefPtr<Gst::Element> getPipeline() const;

    void play();
    void stop();
    void pause();
    void resume();
    void next();
    void previous();

    bool hasConsume() const;
    void setConsume(bool aConsume = true);
    bool hasRandom() const;
    void setRandom(bool aRandom = true);
    bool hasRepeat() const;
    void setRepeat(bool aRepeat = true);
    bool hasSingle() const;
    void setSingle(bool aSingle = true);

private:
    bool changeSong(int aSong, int aState);

    bool on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage);
    void on_about_to_finish(const Glib::RefPtr<Gst::Bin>& aSelf, const Glib::RefPtr<Gst::Bin>& aBin, void*);
    void on_source_setup(const Glib::RefPtr<Gst::Bin>& aSelf, const Glib::RefPtr<Gst::Element>& aSource, void*);

    Server* m_server;
    Glib::RefPtr<Gst::Element> m_playbin;

    uint8_t m_playFlags;
    SongArray::const_iterator m_currentSong;
    std::chrono::nanoseconds m_currentSongDur, m_currentSongPos;
};
