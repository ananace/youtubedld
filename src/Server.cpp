#include "Server.hpp"
#include "Protocols/MPD.hpp"
#include "Protocols/MPRIS.hpp"
#include "Protocols/REST.hpp"
#include "Util/Logging.hpp"

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

    for (auto it = m_activeProtocols.begin(); it != m_activeProtocols.end(); )
    {
        if (!(*it)->init())
            m_activeProtocols.erase(it++);
        else
            ++it;
    }
}

void Server::run()
{
    if (m_activeProtocols.empty())
    {
        Util::Log(Util::Log_Info) << "No active protocols, exiting.";
        return;
    }

    while(true)
    {
        // Update protocols
        for (auto& prot : m_activeProtocols)
        {
            prot->update();
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
