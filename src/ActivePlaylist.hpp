#pragma once

#include "Playlist.hpp"

#include <gstreamermm.h>

enum PlayFlags : uint8_t
{
    PF_Consume = 1u << 0u,
    PF_Random  = 1u << 1u,
    PF_Repeat  = 1u << 2u,
    PF_Single  = 1u << 3u,

    PF_Live    = 1u << 4u,
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

    float getVolume() const;
    void setVolume(float aVolume);

    bool isLive() const;

private:
    bool changeSong(Song* aSong, Gst::State aState);
    Song* nextSong(Song* aCurSong);

    bool on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage);
    void on_about_to_finish();
    void on_source_setup(const Glib::RefPtr<Gst::Element>& aSource);

    Server* m_server;
    Glib::RefPtr<Gst::Element> m_playbin;

    uint8_t m_playFlags;
    Song* m_currentSong;
    std::chrono::nanoseconds m_currentSongDur, m_currentSongPos;
};
