#include "Playlist.hpp"
#include "Util/Path.hpp"
#include "Util/WorkQueue.hpp"

#include <algorithm>

using namespace std::chrono_literals;

// TODO: Place somewhere more reasonable
// Util::WorkQueue s_songUpdateQueue;

Playlist::Playlist()
{
}

Playlist::~Playlist()
{
}

Playlist::SongArray::const_iterator Playlist::cbegin() const
{
    return std::cbegin(m_songs);
}
Playlist::SongArray::const_iterator Playlist::cend() const
{
    return std::cbegin(m_songs);
}
Playlist::SongArray::iterator Playlist::begin()
{
    return std::begin(m_songs);
}
Playlist::SongArray::iterator Playlist::end()
{
    return std::end(m_songs);
}
size_t Playlist::size() const
{
    return m_songs.size();
}

bool Playlist::hasSong(const std::string& aSearch) const
{
    return std::find_if(cbegin(), cend(), [aSearch](auto& it) {
        return it.URL == aSearch || it.Title == aSearch || it.StreamURL == aSearch;
    }) != cend();
}
void Playlist::addSong(const std::string& aUrl)
{
    m_songs.push_back({ aUrl });
}
void Playlist::removeSong(const std::string& aSearch)
{
    auto it = std::find_if(cbegin(), cend(), [aSearch](auto& it) {
        return it.URL == aSearch || it.Title == aSearch || it.StreamURL == aSearch;
    });

    if (it != cend())
        m_songs.erase(it);
}
void Playlist::removeSong(size_t aSong)
{
    if (aSong < m_songs.size())
        m_songs.erase(cbegin() + aSong);
}
void Playlist::removeAllSongs()
{
    m_songs.clear();
}

void Playlist::update()
{
    auto now = std::chrono::system_clock::now();

    for (auto& it : m_songs)
    {
        if (it.UpdateTime > now)
            continue;

        // TODO
        // s_songUpdateQueue.queueTask<void>([]() { });

        it.UpdateTime = now + 3600s;
    }
}

void Playlist::addFromPlaylist(const Playlist& aPlaylist)
{
    m_songs.resize(m_songs.size() + aPlaylist.size());
    for (const auto& s : aPlaylist.m_songs)
        m_songs.push_back(s);
}

bool Playlist::addFromFile(const std::string& aPath)
{
    auto path = Util::ExpandPath(aPath);

    auto toAdd = Playlist();
    if (!toAdd.loadFromFile(aPath))
        return false;

    addFromPlaylist(toAdd);
    return true;
}

bool Playlist::loadFromFile(const std::string& aPath)
{
    auto path = Util::ExpandPath(aPath);

    // TODO

    return false;
}
bool Playlist::saveToFile(const std::string& aPath) const
{
    auto path = Util::ExpandPath(aPath);

    // TODO

    return false;
}
