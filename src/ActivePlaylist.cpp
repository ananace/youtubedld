#include "ActivePlaylist.hpp"
#include "Server.hpp"
#include "Util/GObjectSignalWrapper.hpp"
#include "Util/Logging.hpp"

ActivePlaylist::ActivePlaylist()
    : m_server(nullptr)
    , m_playFlags(0)
    , m_currentSong(nullptr)
{
}

void ActivePlaylist::init(Server& aServer)
{
    m_server = &aServer;

    m_playbin = Gst::ElementFactory::create_element("playbin");

    int flags;
    m_playbin->get_property("flags", flags);

    flags |= Gst::PLAY_FLAG_AUDIO;

    // TODO: Allow configuring
    flags &= ~Gst::PLAY_FLAG_VIDEO;

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

    changeSong(&m_songs.front(), Gst::STATE_PLAYING);

}
void ActivePlaylist::stop()
{
    Util::Log(Util::Log_Debug) << "Stop()";

    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state != Gst::STATE_NULL)
        return;

    m_playbin->set_state(Gst::STATE_NULL);
}
void ActivePlaylist::pause()
{
    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state != Gst::STATE_PLAYING)
        return;

    m_playbin->set_state(Gst::STATE_PAUSED);
}
void ActivePlaylist::resume()
{
    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state != Gst::STATE_PAUSED)
        return;

    m_playbin->set_state(Gst::STATE_PLAYING);
}
void ActivePlaylist::next()
{
    auto it = std::find_if(m_songs.begin(), m_songs.end(), [this](auto& song) { return &song == m_currentSong; });
    if (it == m_songs.end() || ++it == m_songs.end())
    {
        if (!hasRepeat() && !hasRandom())
            return stop();

        if (hasRandom())
        {
            // TODO
            it = m_songs.begin();
        }
        else
            it = m_songs.begin();
    }

    changeSong(&(*it), Gst::STATE_PLAYING);
}
void ActivePlaylist::previous()
{

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
}
bool ActivePlaylist::hasRandom() const
{
    return (m_playFlags & PF_Random) != 0;
}
void ActivePlaylist::setRandom(bool aRandom)
{
    if (aRandom)
        m_playFlags |= uint8_t(PF_Random);
    else
        m_playFlags &= uint8_t(~PF_Random);
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
}
bool ActivePlaylist::hasSingle() const
{
    return (m_playFlags & PF_Single) != 0;
}
void ActivePlaylist::setSingle(bool aSingle)
{
    if (aSingle)
        m_playFlags |= uint8_t(PF_Single);
    else
        m_playFlags &= uint8_t(~PF_Single);
}

bool ActivePlaylist::isLive() const
{
    return (m_playFlags & PF_Live) != 0;
}

bool ActivePlaylist::changeSong(Song* aSong, Gst::State aState)
{
    Util::Log(Util::Log_Debug) << "ChangeSong(" << aSong->URL << ", " << aState << ")";

    if (m_currentSong)
    {
        Util::Log(Util::Log_Debug) << "- resetting current song (" << m_currentSong->URL << ")";

        // Refresh stream URL if song is not local
        if (!m_currentSong->isLocal())
            m_currentSong->UpdateTime = std::chrono::system_clock::now();
    }

    m_currentSong = aSong;

    if (m_currentSong)
    {
        Util::Log(Util::Log_Debug) << "- setting up new song";
        // if (m_currentSong->UpdateTime >= std::chrono::system_clock::now())
        //     return false; // Force retest?

        auto uri = m_currentSong->DataURL;
        if (uri.empty())
            uri = m_currentSong->URL;

        m_playbin->property("uri", Glib::ustring(uri));
    }

    auto ret = m_playbin->set_state(aState);
    if (ret == Gst::STATE_CHANGE_NO_PREROLL)
        m_playFlags |= PF_Live;
    else
        m_playFlags &= ~PF_Live;

    if (ret == Gst::STATE_CHANGE_FAILURE)
    {
        Util::Log(Util::Log_Error) << "Failed to play song";
    }

    return true;
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
            Gst::State oldState, newState, pendingState;
            Glib::RefPtr<Gst::MessageStateChanged>::cast_static(aMessage)->parse(oldState, newState, pendingState);

            // Util::Log(Util::Log_Debug) << "State change " << oldState << " -> " << newState << " (-> " << pendingState << ")";
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
    Util::Log(Util::Log_Debug) << "About to finish current stream, calling next.";

    if (hasSingle())
        stop();
    else
        next();
}

void ActivePlaylist::on_source_setup(const Glib::RefPtr<Gst::Element>& aSource)
{
    Util::Log(Util::Log_Debug) << "Setting up source of type " << aSource->get_name().raw();

    // if (aSource->get_name().raw() == "souphttpsrc")
    {
        aSource->set_property("automatic-redirect", true);
        aSource->set_property("ssl-strict", false);
    }
}
