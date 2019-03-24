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
        std::unordered_map<std::string, std::string> DataHeaders;
        std::string ThumbnailURL;

        std::string Title;
        std::unordered_map<std::string, std::string> Tags;

        std::chrono::nanoseconds Duration;
        std::chrono::system_clock::time_point UpdateTime;
        std::chrono::system_clock::time_point NextUpdateTime;

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
    size_t indexOf(const Song& aSong) const;

    bool hasSongID(size_t aID) const;
    bool hasSong(const std::string& aSearch) const;
    const Song* getSong(const std::string& aSearch) const;
    const Song* getSong(size_t aSong) const;
    const Song* getSongID(size_t aID) const;
    virtual const Song& addSong(const std::string& aUrl);
    virtual void removeSong(const std::string& aSearch);
    virtual void removeSong(size_t aSong);
    virtual void removeSongID(size_t aID);
    virtual void removeAllSongs();

    virtual void update();

    void addFromPlaylist(const Playlist& aPlaylist);
    bool addFromFile(const std::string& aPath);
    bool loadFromFile(const std::string& aPath);
    bool saveToFile(const std::string& aPath) const;

protected:
    Song& _addSong(const Song& aSong);
    Song& _addSong(const std::string& aUrl);
    void _updateSong(Song& aSong);
    virtual void _updatedSong(const Song& aSong);

    SongArray m_songs;
    size_t m_songCounter;
};
