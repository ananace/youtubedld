#pragma once

#include <vector>
#include <string>

class Playlist
{
public:
    struct Song
    {
        std::string URL;

        std::string Title;
        std::string StreamURL;
        std::string ThumbnailURL;

        uint32_t UpdateTime;
    };

    using SongArray = std::vector<Song>;

    Playlist();
    Playlist(const Playlist& copy) = default;
    Playlist(Playlist&& move) = default;
    virtual ~Playlist();

    Playlist& operator=(const Playlist& copy) = default;
    Playlist& operator=(Playlist&& move) = default;

    SongArray::const_iterator cbegin() const;
    SongArray::const_iterator cend() const;
    SongArray::iterator begin();
    SongArray::iterator end();
    size_t size() const;

    bool hasSong(const std::string& aSearch) const;
    void addSong(const std::string& aUrl);
    void removeSong(const std::string& aSearch);
    void removeSong(size_t aSong);
    void removeAllSongs();

    virtual void update();

    void addFromPlaylist(const Playlist& aPlaylist);
    bool addFromFile(const std::string& aPath);
    bool loadFromFile(const std::string& aPath);
    bool saveToFile(const std::string& aPath) const;

private:
    SongArray m_songs;
};

enum PlayFlags : uint8_t
{
    PF_Random  = 1u << 0u,
    PF_Consume = 1u << 1u,
    PF_Single  = 1u << 2u,
};

class ActivePlaylist : public Playlist
{
public:
    ActivePlaylist();

    void update();

    bool hasRandom() const;
    void setRandom(bool aRandom = true);
    bool hasConsume() const;
    void setConsume(bool aConsume = true);
    bool hasSingle() const;
    void setSingle(bool aSingle = true);

private:
    uint8_t m_playFlags;
};
