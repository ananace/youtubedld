#pragma once

#include <string>

class Playlist
{
public:
    Playlist();
    Playlist(const Playlist& copy) = default;
    Playlist(Playlist&& move) = default;
    ~Playlist();

    Playlist& operator=(const Playlist& copy) = default;
    Playlist& operator=(Playlist&& move) = default;

    bool hasSong(const std::string& aUrl) const;
    bool addSong(const std::string& aUrl);
    bool removeSong(const std::string& aUrl);
    bool removeSong(size_t aSong);
    bool removeAllSongs();

    void update();

    void loadFromFile(const std::string& aPath);
    void saveToFile(const std::string& aPath) const;

private:
};
