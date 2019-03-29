#include "Playlist.hpp"
#include "Util/Path.hpp"
#include "Util/WorkQueue.hpp"
#include "Util/YoutubeDL.hpp"
#include "Util/Logging.hpp"

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
#include <limits>

using namespace std::chrono_literals;

// TODO: Place somewhere more reasonable?
Util::WorkQueue s_songUpdateQueue;

Playlist::Song::Song()
    : ID(0)
    , Priority(0)
    , Direct(false)
{ }
Playlist::Song::Song(const Song& aSong)
    : URL(aSong.URL)
    , ID(aSong.ID)
    , Priority(aSong.Priority)
    , DataURL(aSong.DataURL)
    , DataHeaders(aSong.DataHeaders)
    , ThumbnailURL(aSong.ThumbnailURL)
    , Title(aSong.Title)
    , Tags(aSong.Tags)
    , Duration(aSong.Duration)
    , UpdateTime(aSong.UpdateTime)
    , NextUpdateTime(aSong.NextUpdateTime)
    , Direct(aSong.Direct)
{ }
Playlist::Song::Song(Song&& aSong)
    : URL(std::move(aSong.URL))
    , ID(std::move(aSong.ID))
    , Priority(std::move(aSong.Priority))
    , DataURL(std::move(aSong.DataURL))
    , DataHeaders(std::move(aSong.DataHeaders))
    , ThumbnailURL(std::move(aSong.ThumbnailURL))
    , Title(std::move(aSong.Title))
    , Tags(std::move(aSong.Tags))
    , Duration(std::move(aSong.Duration))
    , UpdateTime(std::move(aSong.UpdateTime))
    , NextUpdateTime(std::move(aSong.NextUpdateTime))
    , UpdateTask(std::move(aSong.UpdateTask))
    , Direct(std::move(aSong.Direct))
{ }
Playlist::Song::Song(const std::string& aUrl)
    : URL(aUrl)
    , ID(0)
    , Priority(0)
    , Direct(false)
{ }

Playlist::Song& Playlist::Song::operator=(const Song& aRhs)
{
    if (&aRhs == this)
        return *this;

    URL = aRhs.URL;
    ID = aRhs.ID;
    Priority = aRhs.Priority;
    DataURL = aRhs.DataURL;
    DataHeaders = aRhs.DataHeaders;
    ThumbnailURL = aRhs.ThumbnailURL;
    Title = aRhs.Title;
    Tags = aRhs.Tags;
    Duration = aRhs.Duration;
    UpdateTime = aRhs.UpdateTime;
    NextUpdateTime = aRhs.NextUpdateTime;
    Direct = aRhs.Direct;

    return *this;
}
Playlist::Song& Playlist::Song::operator=(Song&& aRhs)
{
    if (&aRhs == this)
        return *this;

    URL = std::move(aRhs.URL);
    ID = std::move(aRhs.ID);
    Priority = std::move(aRhs.Priority);
    DataURL = std::move(aRhs.DataURL);
    DataHeaders = std::move(aRhs.DataHeaders);
    ThumbnailURL = std::move(aRhs.ThumbnailURL);
    Title = std::move(aRhs.Title);
    Tags = std::move(aRhs.Tags);
    Duration = std::move(aRhs.Duration);
    UpdateTime = std::move(aRhs.UpdateTime);
    NextUpdateTime = std::move(aRhs.NextUpdateTime);
    UpdateTask = std::move(aRhs.UpdateTask);
    Direct = std::move(aRhs.Direct);

    return *this;
}
bool Playlist::Song::isDirect() const
{
    return isLocal() || Direct;
}

bool Playlist::Song::isLocal() const
{
    std::string_view urlView = URL;
    return urlView[0] == '/' || urlView.find("file://") == 0 || urlView.find("://") == std::string::npos;
}

bool Playlist::Song::hasArtist() const
{
    return Tags.count("ARTIST") > 0;
}
const std::string& Playlist::Song::getArtist() const
{
    return Tags.at("ARTIST");
}
bool Playlist::Song::hasAlbum() const
{
    return Tags.count("ALBUM") > 0;
}
const std::string& Playlist::Song::getAlbum() const
{
    return Tags.at("ALBUM");
}

Playlist::Playlist()
    : m_songCounter(0)
{
    if (!s_songUpdateQueue.running())
        s_songUpdateQueue.start();
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
    return std::cend(m_songs);
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
size_t Playlist::indexOf(const Song& aSong) const
{
    auto it = std::find_if(cbegin(), cend(), [aSong](auto& it) {
        return it.ID == aSong.ID;
    });
    if (it == cend())
        return std::numeric_limits<size_t>::max();
    return cend() - it;
}

bool Playlist::hasSong(const std::string& aSearch) const
{
    return std::find_if(cbegin(), cend(), [aSearch](auto& it) {
        return it.URL == aSearch || it.Title == aSearch;
    }) != cend();
}
bool Playlist::hasSongID(size_t aId) const
{
    return std::find_if(cbegin(), cend(), [aId](auto& it) {
        return it.ID == aId;
    }) != cend();
}
const Playlist::Song* Playlist::getSong(const std::string& aSearch) const
{
    auto it = std::find_if(cbegin(), cend(), [aSearch](auto& it) {
        return it.URL == aSearch || it.Title == aSearch;
    });
    if (it == cend())
        return nullptr;
    return &(*it);
}
const Playlist::Song* Playlist::getSong(size_t aSong) const
{
    if (aSong < m_songs.size())
        return &m_songs[aSong];
    return nullptr;
}
const Playlist::Song* Playlist::getSongID(size_t aID) const
{
    auto it = std::find_if(cbegin(), cend(), [aID](auto& it) {
        return it.ID == aID;
    });
    if (it == cend())
        return nullptr;
    return &(*it);
}
const Playlist::Song& Playlist::addSong(const std::string& aUrl)
{
    auto& added = _addSong(aUrl);
    if (!added.isLocal())
        s_songUpdateQueue.queueTask<void>([this,&added]() { _updateSong(added); });

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
void Playlist::removeSongID(size_t aID)
{
    auto it = std::find_if(cbegin(), cend(), [aID](auto& it) {
        return it.ID == aID;
    });

    if (it != cend())
        m_songs.erase(it);
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
        if (it.NextUpdateTime > now)
            continue;

        _queueUpdateSong(it);
    }
}

void Playlist::addFromPlaylist(const Playlist& aPlaylist)
{
    m_songs.resize(m_songs.size() + aPlaylist.size());
    for (const auto& s : aPlaylist.m_songs)
        _addSong(s);
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
        // TODO: Parse EXTINF + YTDLD
        if (line.empty() || line[0] == '#')
            continue;

        _addSong(line);
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

Playlist::Song& Playlist::_addSong(const std::string& aUrl)
{
    m_songs.push_back({ aUrl });
    auto& added = m_songs.back();

    added.ID = m_songCounter++;
    if (added.isLocal())
    {
        if (std::string_view(added.URL).find("file://") == std::string_view::npos)
            added.URL = "file://" + added.URL;

        added.DataURL = added.URL;
        added.NextUpdateTime = std::chrono::system_clock::now() + 24h;
    }

    _addedSong(added);

    return added;
}

Playlist::Song& Playlist::_addSong(const Song& aSong)
{
    m_songs.push_back(aSong);
    auto& added = m_songs.back();
    added.ID = m_songCounter++;

    _addedSong(added);

    return added;
}

void Playlist::_addedSong(Song& aSong)
{
}


void Playlist::_queueUpdateSong(Song& aSong)
{
    auto now = std::chrono::system_clock::now();

    if (!aSong.isDirect())
    {
        aSong.UpdateTask = s_songUpdateQueue.queueTask<bool>([this,&aSong]() { _updateSong(aSong); return true; });
        aSong.NextUpdateTime = now + 1h;
    }
    else
    {
        aSong.UpdateTime = now;
        aSong.NextUpdateTime = now + 24h;
    }
}

void Playlist::_updateSong(Song& aSong)
{
    auto& ydl = YoutubeDL::getSingleton();

    if (!ydl.isAvailable())
        return;

    auto response = ydl.request({ aSong.URL });

    aSong.Duration = std::chrono::seconds(response.Duration);
    aSong.Title = response.Title;
    aSong.ThumbnailURL = response.ThumbnailUrl;
    aSong.DataURL = response.DownloadUrl;
    aSong.DataHeaders = response.DownloadHeaders;
    aSong.UpdateTime = std::chrono::system_clock::now();

    _updatedSong(aSong);
}

void Playlist::_updatedSong(Song& aSong)
{
    Util::Log(Util::Log_Debug) << "[Song] Received song information; Title=" << aSong.Title << " duration=" << aSong.Duration.count();
}
