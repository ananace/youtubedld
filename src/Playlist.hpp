#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

class Server;

class Playlist
{
public:
    struct Song
    {
        std::string URL;
        size_t ID;

        std::string DataURL;
        std::string ThumbnailURL;

        std::string Title;
        std::unordered_map<std::string, std::string> Tags;

        std::chrono::nanoseconds Duration;
        std::chrono::system_clock::time_point UpdateTime;

        bool Direct;

        bool isDirect() const;
        bool isLocal() const;
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

    bool hasSongID(int aID) const;
    bool hasSong(const std::string& aSearch) const;
    const Song& addSong(const std::string& aUrl);
    void removeSong(const std::string& aSearch);
    void removeSong(size_t aSong);
    void removeSongID(int aID);
    void removeAllSongs();

    virtual void update();

    void addFromPlaylist(const Playlist& aPlaylist);
    bool addFromFile(const std::string& aPath);
    bool loadFromFile(const std::string& aPath);
    bool saveToFile(const std::string& aPath) const;

protected:
    SongArray m_songs;
    size_t m_songCounter;
};
