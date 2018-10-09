#include "Server.hpp"

#include "Protocols/MPD.hpp"
#include "Protocols/MPRIS.hpp"
#include "Protocols/REST.hpp"

#include <chrono>
#include <experimental/filesystem>
#include <thread>

using namespace std::literals::chrono_literals;
namespace fs = std::experimental::filesystem;

Server::Server()
{
}

void Server::init(int aArgc, const char** aArgv)
{
    m_config.loadDefaults();
    m_config.loadFromArgs(aArgc, aArgv);

    m_config.loadFromFile("config.ini");
    m_config.loadFromFile("/etc/youtubedld.conf");
    m_config.loadFromFile("$XDG_CONFIG_DIR/youtubedld.conf");
    m_config.loadFromFile("$HOME/.youtubedldrc");

    if (m_config.hasValue("ConfigDir"))
        for(auto& file : fs::directory_iterator(m_config.getValue("ConfigDir")))
        {
            if (file.path().extension() != ".conf")
                continue;

            m_config.loadFromFile(file.path().string());
        }

    printf("Loaded conf\n");

    // TODO: Plugin-ize
    if (m_config.getValueConv("MPD/Enabled", false))
    {
        uint16_t port = m_config.getValueConv<uint16_t>("MPD/Port", 6600);
        m_activeProtocols.push_back(std::make_unique<Protocols::MPD>(port));
        printf("Enabling MPD on port %i\n", port);
    }

    if (m_config.getValueConv("MPRIS/Enabled", false))
    {
        // m_activeProtocols.push_back(std::make_unique<Protocols::MPRIS>());
        printf("Enabling MPRIS");
    }

    if (m_config.getValueConv("REST/Enabled", false))
    {
        uint16_t port = m_config.getValueConv<uint16_t>("REST/Port", 3000);
        // m_activeProtocols.push_back(std::make_unique<Protocols::REST>(port));
        printf("Enabling REST on port %i\n", port);
    }
}

void Server::run()
{
    if (m_activeProtocols.empty())
    {
        printf("No active protocols, exiting.\n");
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
        std::this_thread::sleep_for(5ms);
    }
}
