#include "ActivePlaylist.hpp"
#include "Server.hpp"
#include "Util/GObjectSignalWrapper.hpp"

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
    m_playbin->property("video-sink", Gst::ElementFactory::create_element("fakesink"));

    m_playbin->get_bus()->add_watch(sigc::mem_fun(*this, &ActivePlaylist::on_bus_message));

    signal_callback<void(const Glib::RefPtr<Gst::Bin>&)> signal_wrapper;
    signal_wrapper("about-to-finish", m_playbin).connect(sigc::mem_fun(*this, &ActivePlaylist::on_about_to_finish));
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

bool ActivePlaylist::on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage)
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
            auto tag = Glib::RefPtr<Gst::MessageTag>::cast_static(aMessage);


        }
        break;

    default:
        break;
    }

    return true;
}

void ActivePlaylist::on_about_to_finish(const Glib::RefPtr<Gst::Bin>& aBin)
{
}
