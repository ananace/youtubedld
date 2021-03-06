#include "ActivePlaylist.hpp"
#include "Server.hpp"
#include "Util/GObjectSignalWrapper.hpp"
#include "Util/Logging.hpp"

#include <random>

Gst::Structure structure_from_map(const std::string& type, const std::unordered_map<std::string, std::string>& umap)
{
    auto ret = Gst::Structure(type);
    for (auto& kv : umap)
        ret.set_field(kv.first, kv.second);

    return ret;
}

bool g_tempChange = false;

ActivePlaylist::ActivePlaylist()
    : m_server(nullptr)
    , m_playFlags(0)
    , m_currentSong(nullptr)
    , Playlist()
{
}

void ActivePlaylist::init(Server& aServer)
{
    m_server = &aServer;

    m_playbin = Gst::ElementFactory::create_element("playbin");
    // m_playbin->set_property("audio-sink", Gst::ElementFactory::create_element("autoaudiosink"));
    // m_playbin->set_property("video-sink", Gst::ElementFactory::create_element("fakesink"));

    int flags;
    m_playbin->get_property("flags", flags);

    flags |= Gst::PLAY_FLAG_AUDIO;

    // TODO: Allow configuring
    flags &= ~Gst::PLAY_FLAG_VIDEO;
    flags &= ~Gst::PLAY_FLAG_TEXT;

    if (m_server->getConfig().getValueConv("Cache/Enabled", false))
        flags |= Gst::PLAY_FLAG_DOWNLOAD;
    if (m_server->getConfig().hasValue("Cache/MaxSize"))
        m_playbin->set_property("ring-buffer-max-size", m_server->getConfig().getValueConv<uint64_t>("Cache/MaxSize"));

    m_playbin->set_property("flags", flags);

    m_playbin->get_bus()->add_watch(sigc::mem_fun(*this, &ActivePlaylist::on_bus_message));

    signal_callback<void()> signal_wrapper;
    signal_wrapper("about-to-finish", m_playbin).connect(sigc::mem_fun(*this, &ActivePlaylist::on_about_to_finish));

    signal_callback<void(const Glib::RefPtr<Gst::Element>&)> source_setup_wrapper;
    source_setup_wrapper("source-setup", m_playbin).connect(sigc::mem_fun(*this, &ActivePlaylist::on_source_setup));
}

void ActivePlaylist::update()
{
    Playlist::update();

    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state == Gst::STATE_PLAYING)
    {
        Gst::Format fmt = Gst::FORMAT_TIME;
        gint64 pos = 0;

        if (m_playbin->query_position(fmt, pos))
            m_currentSongPos = std::chrono::nanoseconds(pos);
    }
}
Glib::RefPtr<Gst::Element> ActivePlaylist::getPipeline() const
{
    return m_playbin;
}

void ActivePlaylist::playSong(size_t aId)
{
    auto* song = getSong(aId);
    if (song == nullptr)
        return;
    changeSong(song, Gst::STATE_PLAYING);
}
void ActivePlaylist::playSongID(size_t aId)
{
    auto* song = getSongID(aId);
    if (song == nullptr)
        return;
    changeSong(song, Gst::STATE_PLAYING);
}

void ActivePlaylist::play()
{
    Util::Log(Util::Log_Debug) << "Play()";

    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state == Gst::STATE_PLAYING)
    {
        changeSong(m_currentSong, Gst::STATE_PLAYING);
        return;
    }

    if (m_songs.empty())
        return;

    resetQueue();
    if (hasRandom())
        shuffleQueue();
    changeSong(m_playQueue.front(), Gst::STATE_PLAYING);
}
void ActivePlaylist::stop()
{
    m_playbin->set_state(Gst::STATE_NULL);
}
void ActivePlaylist::pause()
{
    m_playbin->set_state(Gst::STATE_PAUSED);
}
void ActivePlaylist::resume()
{
    if (!m_currentSong)
        return play();

    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state != Gst::STATE_PAUSED)
        return;

    m_playbin->set_state(Gst::STATE_PLAYING);
}
void ActivePlaylist::next()
{
    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    changeSong(nextSong(m_currentSong), state);
}
void ActivePlaylist::previous()
{
    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    changeSong(previousSong(m_currentSong), state);
}

const Playlist::Song& ActivePlaylist::addSong(const std::string& aUrl, int aPosition)
{
    auto& ret = Playlist::addSong(aUrl, aPosition);
    m_playQueue.push_back(const_cast<Song*>(&ret));
    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
    return ret;
}
void ActivePlaylist::removeSong(const std::string& aSearch)
{
    auto it = std::find_if(m_playQueue.begin(), m_playQueue.end(), [aSearch](auto* aSong) { return aSong->URL == aSearch || aSong->Title == aSearch; });
    if (it != m_playQueue.end())
        m_playQueue.erase(it);
    if (*it == m_currentSong)
    {
        if (m_playQueue.empty())
            stop();
        else
            next();
    }

    Playlist::removeSong(aSearch);

    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
}
void ActivePlaylist::removeSong(size_t aSong)
{
    auto id = getSong(aSong)->ID;
    auto it = std::find_if(m_playQueue.begin(), m_playQueue.end(), [id](auto* aSong) { return aSong->ID == id; });
    if (it != m_playQueue.end())
        m_playQueue.erase(it);
    if (*it == m_currentSong)
    {
        if (m_playQueue.empty())
            stop();
        else
            next();
    }

    Playlist::removeSong(aSong);

    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
}
void ActivePlaylist::removeSongID(size_t aID)
{
    auto it = std::find_if(m_playQueue.begin(), m_playQueue.end(), [aID](auto* aSong) { return aSong->ID == aID; });
    if (it != m_playQueue.end())
        m_playQueue.erase(it);
    if (*it == m_currentSong)
    {
        if (m_playQueue.empty())
            stop();
        else
            next();
    }

    Playlist::removeSongID(aID);

    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
}
void ActivePlaylist::removeAllSongs()
{
    Playlist::removeAllSongs();
    resetQueue();
    stop();

    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
}
void ActivePlaylist::shuffle()
{
    Playlist::shuffle();
    resetQueue();
    if (hasRandom())
        shuffleQueue();

    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
}

PlayStatus ActivePlaylist::getStatus() const
{
    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    switch(state)
    {
    case Gst::STATE_PAUSED:
        return PS_Paused;
    case Gst::STATE_PLAYING:
        return PS_Playing;

    default:
        return PS_Stopped;
    }
}

const Playlist::Song* ActivePlaylist::getCurrentSong() const
{
    return m_currentSong;
}

std::chrono::nanoseconds ActivePlaylist::getDuration() const
{
    if (!m_currentSong)
        return std::chrono::nanoseconds(0);
    return m_currentSong->Duration;
}
std::chrono::nanoseconds ActivePlaylist::getElapsed() const
{
    if (!m_currentSong)
        return std::chrono::nanoseconds(0);

    Gst::Format fmt = Gst::FORMAT_TIME;
    gint64 elapse = 0;

    if (m_playbin->query_position(fmt, elapse))
        return std::chrono::nanoseconds(elapse);
    return std::chrono::nanoseconds(0);
}

bool ActivePlaylist::hasConsume() const
{
    return (m_playFlags & PF_Consume) != 0;
}
void ActivePlaylist::setConsume(bool aConsume)
{
    if (aConsume)
        m_playFlags |= uint8_t(PF_Consume);
    else
        m_playFlags &= uint8_t(~PF_Consume);
    m_server->pushEvent(Protocols::Event(Protocols::Event_OptionChange));
}
bool ActivePlaylist::hasRandom() const
{
    return (m_playFlags & PF_Random) != 0;
}
void ActivePlaylist::setRandom(bool aRandom)
{
    if (aRandom)
    {
        if ((m_playFlags & uint8_t(PF_Random)) == 0)
            shuffleQueue();

        m_playFlags |= uint8_t(PF_Random);
    }
    else
    {
        if ((m_playFlags & uint8_t(PF_Random)) != 0)
            resetQueue();

        m_playFlags &= uint8_t(~PF_Random);
    }
    m_server->pushEvent(Protocols::Event(Protocols::Event_OptionChange));
}
bool ActivePlaylist::hasRepeat() const
{
    return (m_playFlags & PF_Repeat) != 0;
}
void ActivePlaylist::setRepeat(bool aRepeat)
{
    if (aRepeat)
        m_playFlags |= uint8_t(PF_Repeat);
    else
        m_playFlags &= uint8_t(~PF_Repeat);
    m_server->pushEvent(Protocols::Event(Protocols::Event_OptionChange));
}
SingleStatus ActivePlaylist::hasSingle() const
{
    SingleStatus status = Single_False;
    if ((m_playFlags & PF_Single) != 0)
        status = (m_playFlags & PF_SingleOS) == 0 ? Single_True : Single_Oneshot;

    return status;
}
void ActivePlaylist::setSingle(SingleStatus aSingle)
{
    if (aSingle > Single_False)
    {
        m_playFlags |= uint8_t(PF_Single);
        if (aSingle == Single_Oneshot)
            m_playFlags |= uint8_t(PF_SingleOS);
        else
            m_playFlags &= uint8_t(~PF_SingleOS);
    }
    else
        m_playFlags &= uint8_t(~PF_Single) & uint8_t(~PF_SingleOS);
    m_server->pushEvent(Protocols::Event(Protocols::Event_OptionChange));
}

bool ActivePlaylist::hasError() const
{
    return !m_errorMsg.empty();
}
const std::string& ActivePlaylist::getError() const
{
    return m_errorMsg;
}
void ActivePlaylist::setError(const std::string& aError)
{
    m_errorMsg = aError;
}
void ActivePlaylist::clearError()
{
    m_errorMsg.clear();
}

float ActivePlaylist::getVolume() const
{
    double volume = 0;
    m_playbin->get_property<double>("volume", volume);
    return float(volume);
}
void ActivePlaylist::setVolume(float aVolume)
{
    aVolume = std::min(std::max(aVolume, 0.f), 1.f);
    m_playbin->set_property<double>("volume", aVolume);
    m_server->pushEvent(Protocols::Event(Protocols::Event_VolumeChange));
}

bool ActivePlaylist::isLive() const
{
    return (m_playFlags & PF_Live) != 0;
}

bool ActivePlaylist::changeSong(const Song* aSong, Gst::State aState)
{
    if (aSong)
        Util::Log(Util::Log_Debug) << "ChangeSong(" << aSong->URL << ", " << aState << ")";
    else
        Util::Log(Util::Log_Debug) << "ChangeSong(null" << ", " << aState << ")";

    if (m_currentSong)
    {
        Util::Log(Util::Log_Debug) << "- resetting current song (" << m_currentSong->URL << ")";

        if (hasConsume())
        {
            m_playQueue.erase(std::find(m_playQueue.begin(), m_playQueue.end(), m_currentSong));
            m_songs.erase(std::find_if(m_songs.begin(), m_songs.end(), [this](auto& it) { return m_currentSong == &it; }));

            if (m_playQueue.empty())
                m_currentSong = nullptr;
        }
        // Refresh stream URL if song is not local?
        /*
        else if (!m_currentSong->isLocal())
            m_currentSong->UpdateTime = std::chrono::system_clock::now();
        */
    }

    m_currentSong = const_cast<Song*>(aSong);

    if (m_currentSong)
    {
        // TODO: Proper insert song into queue method
        if (std::find(m_playQueue.begin(), m_playQueue.end(), m_currentSong) == m_playQueue.end())
            _addedSong(*m_currentSong);

        Util::Log(Util::Log_Debug) << "- setting up new song (" << m_currentSong->URL << ")";

        if (!m_currentSong->UpdateTask.valid() && m_currentSong->NextUpdateTime >= std::chrono::system_clock::now())
            _queueUpdateSong(*m_currentSong);

        // Wait for the current song update to finish before playing it
        // TODO: timeout and handle that correctly
        if (m_currentSong->UpdateTask.valid())
        {
            auto task = m_currentSong->UpdateTask;
            Util::Log(Util::Log_Debug) << "- Task is available, waiting for it";
            task.wait();
            Util::Log(Util::Log_Debug) << "- Task finished";
        }

        auto uri = m_currentSong->DataURL;
        if (uri.empty())
            uri = m_currentSong->URL;

        Gst::State state, pending;
        m_playbin->get_state(state, pending, {});

        if (state == Gst::STATE_PLAYING)
        {
            Util::Log(Util::Log_Debug) << "- Playing (" << uri << ")";
            Util::Log(Util::Log_Debug) << "- Sending EOS";
            g_tempChange = true;
            m_playbin->send_event(Gst::EventEos::create());
            m_playbin->set_property("uri", Glib::ustring(uri));
        }
        else
        {
            Util::Log(Util::Log_Debug) << "- Playing (" << uri << ")";
            m_playbin->set_property("uri", Glib::ustring(uri));
        }
    }
    else if (aState == Gst::STATE_PLAYING)
        aState = Gst::STATE_READY;

    if (!m_currentSong)
        m_playbin->set_state(Gst::STATE_NULL);

    auto ret = m_playbin->set_state(aState);
    if (ret == Gst::STATE_CHANGE_NO_PREROLL)
        m_playFlags |= PF_Live;
    else
        m_playFlags &= ~PF_Live;

    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));

    if (ret == Gst::STATE_CHANGE_FAILURE)
    {
        Util::Log(Util::Log_Error) << "Failed to play song";
        setError("Failed to play song");
        return false;
    }

    return true;
}

const Playlist::Song* ActivePlaylist::nextSong(const Song* aCurSong)
{
    if (m_songs.empty())
        return nullptr;
    else if (m_playQueue.empty() && !hasRepeat())
        return nullptr;
    else if (hasRepeat())
    {
        resetQueue();
        if (hasRandom())
            shuffleQueue();
    }

    auto curSongIt = std::find(m_playQueue.begin(), m_playQueue.end(), aCurSong);
    // Should hopefully never happen, but let's be on the safe side
    if (curSongIt == m_playQueue.end())
        curSongIt = m_playQueue.begin();

    auto nextSongIt = curSongIt + 1;
    if (nextSongIt == m_playQueue.end())
    {
        if (hasRepeat())
            return m_playQueue.front();
        return nullptr;
    }

    return *nextSongIt;
}

const Playlist::Song* ActivePlaylist::previousSong(const Song* aCurSong)
{
    if (m_songs.empty())
        return nullptr;
    else if (m_playQueue.empty() && !hasRepeat())
        return nullptr;
    else if (hasRepeat())
    {
        resetQueue();
        if (hasRandom())
            shuffleQueue();
    }

    auto curSongIt = std::find(m_playQueue.begin(), m_playQueue.end(), aCurSong);
    // Should hopefully never happen, but let's be on the safe side
    if (curSongIt == m_playQueue.begin())
        curSongIt = m_playQueue.end() - 1;

    auto nextSongIt = curSongIt - 1;
    if (nextSongIt == m_playQueue.begin())
    {
        if (hasRepeat())
            return m_playQueue.back();
        return nullptr;
    }

    return *nextSongIt;
}

bool ActivePlaylist::on_bus_message(const Glib::RefPtr<Gst::Bus>& /* aBus */, const Glib::RefPtr<Gst::Message>& aMessage)
{
    switch(aMessage->get_message_type())
    {
    case Gst::MESSAGE_BUFFERING:
        {
            if (isLive())
                break;

            auto buf = Glib::RefPtr<Gst::MessageBuffering>::cast_static(aMessage);

            int perc = buf->parse_buffering();
            Util::Log(Util::Log_Debug) << "Buffering: " << perc << "%";

            if (perc >= 100)
                m_playbin->set_state(Gst::STATE_PLAYING);
            else
                m_playbin->set_state(Gst::STATE_PAUSED);
        }
        break;

    case Gst::MESSAGE_DURATION_CHANGED:
        {
            Gst::Format fmt = Gst::FORMAT_TIME;
            gint64 dur = 0;

            if (m_playbin->query_duration(fmt, dur))
                m_currentSongDur = std::chrono::nanoseconds(dur);

            Util::Log(Util::Log_Debug) << "Duration change: " << m_currentSongDur;
        }
        break;

    case Gst::MESSAGE_STATE_CHANGED:
        {
            auto msg = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(aMessage);
            if (msg->get_source() != m_playbin)
                break;

            Gst::State oldState, newState, pendingState;
            msg->parse(oldState, newState, pendingState);

            if ((newState == Gst::STATE_PLAYING && oldState <= Gst::STATE_PAUSED) ||
                (newState == Gst::STATE_PAUSED && oldState >= Gst::STATE_PLAYING))
                m_server->pushEvent(Protocols::Event(Protocols::Event_StateChange));
            Util::Log(Util::Log_Debug) << "State change for " << std::string(msg->get_source()->get_name()) << "(" << (msg->get_source() == m_playbin) << "): " << oldState << " -> " << newState << " (-> " << pendingState << ")";
        }
        break;

    case Gst::MESSAGE_CLOCK_LOST:
        {
            Util::Log(Util::Log_Debug) << "Resetting clock";
            m_playbin->set_state(Gst::STATE_PAUSED);
            m_playbin->set_state(Gst::STATE_PLAYING);
        }
        break;

    case Gst::MESSAGE_TAG:
        {
            auto tagMsg = Glib::RefPtr<Gst::MessageTag>::cast_static(aMessage);
            auto tagList = tagMsg->parse_tag_list();

            Glib::ustring ustr;
            uint64_t u64;

            if (tagList.get(Gst::TAG_TITLE, ustr))
                m_currentSong->Title = ustr.raw();

            if (tagList.get(Gst::TAG_ARTIST, ustr))
                m_currentSong->Tags["ARTIST"] = ustr.raw();

            if (tagList.get(Gst::TAG_ALBUM, ustr))
                m_currentSong->Tags["ALBUM"] = ustr.raw();

            if (tagList.get(Gst::TAG_DURATION, u64) && m_currentSong->Duration.count() < u64)
                m_currentSong->Duration = std::chrono::nanoseconds(u64);
        }
        break;

    case Gst::MESSAGE_ERROR:
        {
            auto errMsg = Glib::RefPtr<Gst::MessageError>::cast_static(aMessage);

            Util::Log(Util::Log_Error) << "Error: " << errMsg->parse_error().what().raw();
            Util::Log(Util::Log_Error) << errMsg->parse_debug();
        }
        break;

    case Gst::MESSAGE_WARNING:
        {
            auto warnMsg = Glib::RefPtr<Gst::MessageWarning>::cast_static(aMessage);

            Util::Log(Util::Log_Warning) << "Warning: " << warnMsg->parse_error().what().raw();
            Util::Log(Util::Log_Warning) << warnMsg->parse_debug();
        }
        break;


    default:
        {
            auto structure = aMessage->get_structure();

            if (structure)
                Util::Log(Util::Log_Debug) << "Unhandled Msg on the bus: " << structure.get_name().raw();
        }
        break;
    }

    return true;
}

void ActivePlaylist::on_about_to_finish()
{
    if (g_tempChange)
    {
        Util::Log(Util::Log_Debug) << "About to finish after song change.";
        g_tempChange = false;
        return;
    }

    Util::Log(Util::Log_Debug) << "About to finish current stream, calling next.";

    if (hasSingle() > Single_False)
    {
        if (hasSingle() == Single_Oneshot)
            setSingle(Single_False);

        if (!hasRepeat())
            stop();
        else
            changeSong(m_currentSong, Gst::STATE_PLAYING);
    }
    else
        changeSong(nextSong(m_currentSong), Gst::STATE_PLAYING);
}

void ActivePlaylist::on_source_setup(const Glib::RefPtr<Gst::Element>& aSource)
{
    Util::Log(Util::Log_Debug) << "Setting up source of type " << aSource->get_name().raw();

    if (aSource->get_name().raw() == "souphttpsrc")
    {
        aSource->set_property("automatic-redirect", true);
        aSource->set_property("compress", true);
        aSource->set_property("extra-headers", structure_from_map("extra-headers", m_currentSong->DataHeaders));
        aSource->set_property("ssl-strict", false);
    }
}

void ActivePlaylist::_addedSong(Song& aSong)
{
    // Playlist::_addedSong(aSong);

    if (hasRandom())
    {
        std::random_device dev;
        std::uniform_int_distribution<int> dist(0, m_playQueue.size());
        auto it = m_playQueue.begin() + dist(dev);
        m_playQueue.insert(it, &aSong);
    }
    else
        m_playQueue.push_back(&aSong);
}

void ActivePlaylist::_updatedSong(Song& aSong)
{
    Playlist::_updatedSong(aSong);
    m_server->pushEvent(Protocols::Event(Protocols::Event_QueueChange));
}

void ActivePlaylist::resetQueue()
{
    m_playQueue.clear();
    for (auto& song : *this)
        m_playQueue.push_back(&song);
}
void ActivePlaylist::shuffleQueue()
{
    std::shuffle(m_playQueue.begin(), m_playQueue.end(), std::random_device());
}
