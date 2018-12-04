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
        std::string StreamURL;
        std::string ThumbnailURL;

        std::string Title;
        std::unordered_map<std::string, std::string> Tags;

        std::chrono::system_clock::time_point UpdateTime;
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
