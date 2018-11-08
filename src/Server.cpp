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

namespace
{

std::string formatTime(Util::LogLevels aLevel)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ts = localtime(&time);
    char timeBuf[32];
    std::strftime(timeBuf, 32, "[ |%H:%M:%S] ", ts);

    static constexpr char LogLevels[] = { 'D', 'I', 'W', 'E' };
    timeBuf[1] = LogLevels[int(aLevel)];
    return timeBuf;
}

}

Server::Server()
{
    // m_mainLoop = Glib::MainLoop::create();
    // m_pipeline = Gst::Pipeline::create("audio-player");
    // m_pipeline->get_bus()->add_watch(sigc::mem_fun(*this, &Server::on_bus_message));
}

void Server::init(int aArgc, const char** aArgv)
{
    {
        auto outputLogger = new Util::StdoutLogger;
        auto combinedLogger = new Util::CombinedLogger;
        auto timeLogger = new Util::PrependLogger(combinedLogger);

        combinedLogger->addLogger(outputLogger);
        timeLogger->setPrepend(formatTime);

        Util::SetLogger(timeLogger);
    }

    Util::SetLogLevel(Util::Log_Debug);

    m_config.loadDefaults();
    m_config.loadFromArgs(aArgc, aArgv);

    m_config.loadFromFile("config.ini");
    m_config.loadFromFile("/etc/youtubedld.conf");
    m_config.loadFromFile("${XDG_CONFIG_DIR:-$HOME/.config}/youtubedld.conf");
    m_config.loadFromFile("$HOME/.youtubedldrc");

    if (m_config.hasValue("ConfigDir"))
        m_config.loadFromDir(m_config.getValue("ConfigDir"));

    Util::Log(Util::Log_Info) << "Loaded conf";

    YoutubeDL ydl;
    ydl.findInstall();
    if (!ydl.isAvailable())
    {
        Util::Log(Util::Log_Info) << "No YDL found";
    }

    Util::Log(Util::Log_Info) << "YDL version: " << ydl.getVersion();

    auto resp = ydl.request({ "https://www.youtube.com/watch?v=iL5DY8HVJPE", false, "opus" });

    Util::Log(Util::Log_Info) << "Video is: " << resp.Title << " ( " << resp.DownloadUrl << " ) {";
    for (auto& hh : resp.DownloadHeaders)
        Util::Log(Util::Log_Info) << "  " << hh.first << ": " << hh.second;
    Util::Log(Util::Log_Info) << "}";

    exit(0);

    {
        // Gst::init takes references
        int aArgcTemp = aArgc;
        char** aArgvTemp = const_cast<char**>(aArgv);
        Gst::init(aArgcTemp, aArgvTemp);
    }

    m_source = Gst::ElementFactory::create_element("filesrc");
    m_decoder = Gst::ElementFactory::create_element("decodebin");

    m_pipeline->add(m_source)->add(m_decoder);
    m_source->link(m_decoder);

    // TODO: Plugin-ize
    if (m_config.getValueConv("MPD/Enabled", false))
    {
        uint16_t port = m_config.getValueConv<uint16_t>("MPD/Port", 6600);
        m_activeProtocols.push_back(std::make_unique<Protocols::MPDProto>(port));
        Util::Log(Util::Log_Info) << "Enabling MPD on port " << port;
    }

    if (m_config.getValueConv("MPRIS/Enabled", false))
    {
        // m_activeProtocols.push_back(std::make_unique<Protocols::MPRIS>());
        Util::Log(Util::Log_Info) << "Enabling MPRIS";
    }

    if (m_config.getValueConv("REST/Enabled", false))
    {
        uint16_t port = m_config.getValueConv<uint16_t>("REST/Port", 3000);
        // m_activeProtocols.push_back(std::make_unique<Protocols::REST>(port));
        Util::Log(Util::Log_Info) << "Enabling REST on port " << port;
    }


    auto removed = std::remove_if(m_activeProtocols.begin(), m_activeProtocols.end(), [](auto& prot) {
        return !prot->init();
    });

    if (removed != m_activeProtocols.end())
        m_activeProtocols.erase(removed, m_activeProtocols.end());
}

void Server::run()
{
    if (m_activeProtocols.empty())
    {
        Util::Log(Util::Log_Info) << "No active protocols, exiting.";
        return;
    }

    // Glib::signal_timeout().connect(sigc::ptr_fun(), 5);

    m_mainLoop->run();

    while(true)
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

        // Update songs / playlists

        // Post events
        for (auto& prot : m_activeProtocols)
        {
            if (!prot->supportsPost())
                continue;

            prot->post();
        }

        /// TODO: CV and locking instead
        // std::this_thread::sleep_for(5ms);
    }
}

bool Server::on_bus_message(const Glib::RefPtr<Gst::Bus>& /* aBus */, const Glib::RefPtr<Gst::Message>& aMessage)
{
    switch(aMessage->get_message_type())
    {
    case Gst::MESSAGE_EOS:
        Util::Log(Util::Log_Info) << "End of stream";
        m_mainLoop->quit();
        return false;

    case Gst::MESSAGE_ERROR:
        Util::Log(Util::Log_Error) << "Error." << Glib::RefPtr<Gst::MessageError>::cast_static(aMessage)->parse_debug();
        m_mainLoop->quit();
        return false;

    default:
        break;
    }

    return true;
}
