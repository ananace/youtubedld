#include "Server.hpp"
#include "Protocols/MPD.hpp"
#include "Protocols/MPRIS.hpp"
#include "Protocols/REST.hpp"
#include "Util/Logging.hpp"
#include "Util/YoutubeDL.hpp"

#include <algorithm>
#include <chrono>
#include <thread>


using namespace std::literals::chrono_literals;

Server::Server()
{
    m_config.loadDefaults();
}

void Server::init(int aArgc, const char** aArgv)
{
    m_config.loadFromArgs(aArgc, aArgv);

    if (m_config.getValueConv("Verbose", false))
        Util::SetLogLevel(Util::Log_Debug);

    m_config.loadFromFile("config.ini");
    m_config.loadFromFile("/etc/youtubedld.conf");
    m_config.loadFromFile("${XDG_CONFIG_DIR:-$HOME/.config}/youtubedld.conf");
    m_config.loadFromFile("$HOME/.youtubedldrc");

    if (m_config.hasValue("ConfigDir"))
        m_config.loadFromDir(m_config.getValue("ConfigDir"));

    Util::Log(Util::Log_Info) << "Loaded conf";

    YoutubeDL& ydl = YoutubeDL::getSingleton();
    ydl.findInstall();
    if (!ydl.isAvailable())
    {
        Util::Log(Util::Log_Info) << "No YDL found";
    }
    else
        Util::Log(Util::Log_Info) << "YDL version: " << ydl.getVersion();

    {
        // Gst::init takes references
        int aArgcTemp = aArgc;
        char** aArgvTemp = const_cast<char**>(aArgv);
        Gst::init(aArgcTemp, aArgvTemp);
    }

    m_mainLoop = Glib::MainLoop::create();
    m_ticker = Glib::signal_timeout().connect(sigc::mem_fun(*this, &Server::on_tick), 100);

    m_activePlaylist.init(*this);

    m_pipeline = m_activePlaylist.getPipeline();
    m_pipeline->get_bus()->add_watch(sigc::mem_fun(*this, &Server::on_bus_message));

    // TODO: Plugin-ize
    if (m_config.getValueConv("MPD/Enabled", false))
    {
        uint16_t port = m_config.getValueConv<uint16_t>("MPD/Port", 6600);
        m_activeProtocols.push_back(std::make_unique<Protocols::MPDProto>(port));
        Util::Log(Util::Log_Debug) << "Enabling MPD on port " << port;
    }

    if (m_config.getValueConv("MPRIS/Enabled", false))
    {
        // m_activeProtocols.push_back(std::make_unique<Protocols::MPRIS>());
        Util::Log(Util::Log_Debug) << "Enabling MPRIS";
    }

    if (m_config.getValueConv("REST/Enabled", false))
    {
        uint16_t port = m_config.getValueConv<uint16_t>("REST/Port", 3000);
        // m_activeProtocols.push_back(std::make_unique<Protocols::REST>(port));
        Util::Log(Util::Log_Debug) << "Enabling REST on port " << port;
    }

    for (auto& prot : m_activeProtocols)
        prot->m_server = this;

    auto removed = std::remove_if(m_activeProtocols.begin(), m_activeProtocols.end(), [](auto& prot) {
        return !prot->init();
    });

    if (removed != m_activeProtocols.end())
        m_activeProtocols.erase(removed, m_activeProtocols.end());

    m_startTime = std::chrono::system_clock::now();
}

void Server::run()
{
    if (m_activeProtocols.empty())
    {
        Util::Log(Util::Log_Info) << "No active protocols, exiting.";
        m_mainLoop->quit();
    }

    // m_activePlaylist.addSong("https://freesound.org/data/previews/449/449728_4068345-lq.mp3");
    // m_activePlaylist.addSong("https://freemusicarchive.org/file/music/ccCommunity/Lobo_Loco/Vagabond/Lobo_Loco_-_09_-_Work_Wonders_ID_999.mp3");
    // m_activePlaylist.play();

    // play("https://freemusicarchive.org/file/music/ccCommunity/Lobo_Loco/Vagabond/Lobo_Loco_-_09_-_Work_Wonders_ID_999.mp3");

    m_mainLoop->run();
    m_activePlaylist.getPipeline()->set_state(Gst::STATE_NULL);
    Util::Log(Util::Log_Info) << "No active protocols, exiting.";
}

std::chrono::milliseconds Server::getUptime() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_startTime);
}

void Server::pushEvent(const Protocols::Event& aEvent)
{
    m_eventQueue.push_back(aEvent);
}

bool Server::on_tick()
{
    // Update protocols
    for (auto& prot : m_activeProtocols)
    {
        prot->update();

        Protocols::Event ev;
        while (prot->poll(ev))
        {
        }
    }

    // Post events
    if (!m_eventQueue.empty())
    {
        for (auto& prot : m_activeProtocols)
        {
            if (!prot->supportsPost())
                continue;

            for (auto& ev : m_eventQueue)
                prot->post(ev);
        }
        m_eventQueue.clear();
    }

    return true;
}

bool Server::on_bus_message(const Glib::RefPtr<Gst::Bus>& /* aBus */, const Glib::RefPtr<Gst::Message>& aMessage)
{
    Util::Log(Util::Log_Debug) << "Msg on the bus: " << aMessage->get_structure().get_name().raw();

    switch(aMessage->get_message_type())
    {
    case Gst::MESSAGE_ERROR:
        {
            auto errMsg = Glib::RefPtr<Gst::MessageError>::cast_static(aMessage);

            Util::Log(Util::Log_Error) << "Error: " << errMsg->parse_error().what().raw();
            Util::Log(Util::Log_Error) << errMsg->parse_debug();
        }
        return false;

    case Gst::MESSAGE_WARNING:
        {
            auto warnMsg = Glib::RefPtr<Gst::MessageWarning>::cast_static(aMessage);

            Util::Log(Util::Log_Warning) << "Warning: " << warnMsg->parse_error().what().raw();
            Util::Log(Util::Log_Warning) << warnMsg->parse_debug();
        }
        return false;

    default:
        break;
    }

    return true;
}
