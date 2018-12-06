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
    // TODO: Allow configuring
    m_playbin->property("video-sink", Gst::ElementFactory::create_element("fakesink"));

    m_playbin->get_bus()->add_watch(sigc::mem_fun(*this, &ActivePlaylist::on_bus_message));

    signal_callback<void(const Glib::RefPtr<Gst::Bin>&, const Glib::RefPtr<Gst::Bin>&, void*)> signal_wrapper;
    signal_wrapper("about-to-finish", m_playbin).connect(sigc::mem_fun(*this, &ActivePlaylist::on_about_to_finish));

    signal_callback<void(const Glib::RefPtr<Gst::Bin>&, const Glib::RefPtr<Gst::Element>&, void*)> source_setup_wrapper;
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
    Gst::State state, pending;
    m_playbin->get_state(state, pending, {});

    if (state == Gst::STATE_PLAYING)
        return;

    m_playbin->set_state(Gst::STATE_PLAYING);
}
void ActivePlaylist::stop()
{
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

bool ActivePlaylist::changeSong(SongArray::const_iterator aSong, int aState)
{
    if (m_currentSong != m_songs.cend())
    {
        SongArray::iterator curSong = m_songs.erase(m_currentSong, m_currentSong);

        // Refresh stream URL if song is not local
        if (!curSong->isLocal())
            curSong->UpdateTime = std::chrono::system_clock::now();
    }

    m_currentSong = aSong;

    if (m_currentSong != m_songs.cend())
    {
        if (m_currentSong->UpdateTime >= std::chrono::system_clock::now())
            return false; // Force retest?

        m_playbin->property("url", m_currentSong->DataURL);
    }

    return true;
}

bool ActivePlaylist::on_bus_message(const Glib::RefPtr<Gst::Bus>& /* aBus */, const Glib::RefPtr<Gst::Message>& aMessage)
{
    switch(aMessage->get_message_type())
    {
    case Gst::MESSAGE_BUFFERING:
        {
            auto buf = Glib::RefPtr<Gst::MessageBuffering>::cast_static(aMessage);

            int perc = buf->parse_buffering();

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
        }
        break;

    case Gst::MESSAGE_TAG:
        {
            auto tagMsg = Glib::RefPtr<Gst::MessageTag>::cast_static(aMessage);
            auto tagList = tagMsg->parse_tag_list();

            SongArray::iterator curSong = m_songs.erase(m_currentSong, m_currentSong);

            Glib::ustring ustr;
            uint64_t u64;

            if (tagList.get(Gst::TAG_TITLE, ustr))
                curSong->Title = ustr.raw();

            if (tagList.get(Gst::TAG_ARTIST, ustr))
                curSong->Tags["ARTIST"] = ustr.raw();

            if (tagList.get(Gst::TAG_ALBUM, ustr))
                curSong->Tags["ALBUM"] = ustr.raw();

            if (tagList.get(Gst::TAG_DURATION, u64) && curSong->Duration.count() < u64)
                curSong->Duration = std::chrono::nanoseconds(u64);
        }
        break;

    default:
        break;
    }

    return true;
}

void ActivePlaylist::on_about_to_finish(const Glib::RefPtr<Gst::Bin>& /* aPipeline */, const Glib::RefPtr<Gst::Bin>& aBin, void* /* aUserData */)
{
    (void)aBin;
    Util::Log(Util::Log_Debug) << "About to finish current stream, calling next.";

    if (!hasSingle())
        next();
}

void ActivePlaylist::on_source_setup(const Glib::RefPtr<Gst::Bin>& /* aPipeline */, const Glib::RefPtr<Gst::Element>& aSource, void* /* aUserData */)
{
    Util::Log(Util::Log_Debug) << "Setting up source of type " << aSource->get_name().raw();
    if (aSource->get_name().raw() == "souphttpsrc")
    {
        aSource->property("automatic-redirect", true);
        aSource->property("ssl-strict", false);
    }
}
