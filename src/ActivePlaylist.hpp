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

enum PlayStatus : uint8_t
{
    PS_Stopped,
    PS_Paused,
    PS_Playing
};

class ActivePlaylist : public Playlist
{
public:
    ActivePlaylist();

    void init(Server& aServer);
    void update();

    Glib::RefPtr<Gst::Element> getPipeline() const;

    void playSong(size_t aId);
    void playSongID(size_t aId);

    void play();
    void stop();
    void pause();
    void resume();
    void next();
    void previous();

    const Song& addSong(const std::string& aUrl);
    void removeSong(const std::string& aSearch);
    void removeSong(size_t aSong);
    void removeSongID(int aID);
    void removeAllSongs();

    PlayStatus getStatus() const;
    const Song* getCurrentSong() const;

    std::chrono::nanoseconds getDuration() const;
    std::chrono::nanoseconds getElapsed() const;

    bool hasConsume() const;
    void setConsume(bool aConsume = true);
    bool hasRandom() const;
    void setRandom(bool aRandom = true);
    bool hasRepeat() const;
    void setRepeat(bool aRepeat = true);
    bool hasSingle() const;
    void setSingle(bool aSingle = true);

    bool hasError() const;
    const std::string& getError() const;
    void setError(const std::string& aError);
    void clearError();

    float getVolume() const;
    void setVolume(float aVolume);

    bool isLive() const;

private:
    void _updatedSong(const Song& aSong) override;
    bool changeSong(const Song* aSong, Gst::State aState);
    Song* nextSong(const Song* aCurSong);

    bool on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage);
    void on_about_to_finish();
    void on_source_setup(const Glib::RefPtr<Gst::Element>& aSource);

    Server* m_server;
    Glib::RefPtr<Gst::Element> m_playbin;

    uint8_t m_playFlags;
    Song* m_currentSong;
    std::chrono::nanoseconds m_currentSongDur, m_currentSongPos;
    std::string m_errorMsg;
};
