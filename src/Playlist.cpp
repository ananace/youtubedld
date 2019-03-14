#include "Playlist.hpp"
#include "Util/Path.hpp"
#include "Util/WorkQueue.hpp"

#if __has_include(<string_view>)
#include <string_view>
#else
#include <experimental/string_view>
namespace std
{
    using string_view = std::experimental::string_view;
}
#endif

#include <algorithm>
#include <fstream>
#include <iomanip>

using namespace std::chrono_literals;

// TODO: Place somewhere more reasonable
// Util::WorkQueue s_songUpdateQueue;

bool Playlist::Song::isDirect() const
{
    return isLocal() || Direct;
}

bool Playlist::Song::isLocal() const
{
    std::string_view urlView = URL;
    return urlView[0] == '/' || urlView.find("file://") == 0 || urlView.find("://") == std::string::npos;
}

Playlist::Playlist()
    : m_songCounter(0)
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
        return it.URL == aSearch || it.Title == aSearch;
    }) != cend();
}
const Playlist::Song& Playlist::addSong(const std::string& aUrl)
{
    // TODO: Initial update
    m_songs.push_back({ aUrl });
    auto& added = m_songs.back();

    added.ID = m_songCounter++;
    if (added.isLocal())
    {
        if (std::string_view(added.URL).find("file://") == std::string_view::npos)
            added.URL = "file://" + added.URL;

        added.DataURL = added.URL;
        added.UpdateTime = std::chrono::system_clock::now() + 24h;
    }
    else
    {
        // s_songUpdateQueue.queueTask<void>([]() { });
    }

    return added;
}
void Playlist::removeSong(const std::string& aSearch)
{
    auto it = std::find_if(cbegin(), cend(), [aSearch](auto& it) {
        return it.URL == aSearch || it.Title == aSearch;
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

        if (!it.isDirect())
        {
            // TODO
            // s_songUpdateQueue.queueTask<void>([]() { });

            it.UpdateTime = now + 3600s;
        }
        else
            it.UpdateTime = now + 24h;
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

    m_songs.clear();
    auto fss = std::ifstream(path);
    std::string line;
    while (fss)
    {
        std::getline(fss, line);
        if (line.empty() || line[0] == '#')
            continue;

        addSong(line);
    }

    return true;
}
bool Playlist::saveToFile(const std::string& aPath) const
{
    auto path = Util::ExpandPath(aPath);

    auto fss = std::ofstream(path);
    fss << "#EXTM3U" << std::endl;

    for (auto& entry : m_songs)
    {

        fss << "#EXTINF:" << std::chrono::duration_cast<std::chrono::seconds>(entry.Duration).count() << "," << entry.Title << std::endl;

        fss << "#YTDLD:" << entry.DataURL << "," << entry.ThumbnailURL << ",";
        for (auto& tag : entry.Tags)
        {
            if (tag != *entry.Tags.begin())
                fss << ",";
            fss << tag.first << "=" << std::quoted(tag.second);
        }
        fss << std::endl;

        fss << entry.URL << std::endl;
    }

    return true;
}
