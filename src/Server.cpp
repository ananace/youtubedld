#include "Server.hpp"

#include "Protocols/MPD.hpp"
#include "Protocols/MPRIS.hpp"
#include "Protocols/REST.hpp"

#include <chrono>
#include <thread>

using namespace std::literals::chrono_literals;

Server::Server()
{
}

void Server::init(int aArgc, const char** aArgv)
{
    m_config.loadDefaults();
    m_config.loadFromArgs(aArgc, aArgv);

    m_config.loadFromFile("/etc/youtubedld.conf");
    m_config.loadFromFile("$XDG_CONFIG_DIR/youtubedld.conf");
    m_config.loadFromFile("$HOME/.youtubedldrc");

    // TODO: Plugin-ize
    if (m_config.getValue("MPD/enabled", false))
    {
        uint16_t port = m_config.getValue("MPD/port", 6600);
        m_activeProtocols.push_back(std::make_unique<Protocols::MPD>(port));
    }

    if (m_config.getValue("MPRIS/enabled", false))
    {
        // m_activeProtocols.push_back(std::make_unique<Protocols::MPRIS>());
    }

    if (m_config.getValue("REST/enabled", false))
    {
        uint16_t port = m_config.getValue("REST/port", 3000);
        // m_activeProtocols.push_back(std::make_unique<Protocols::REST>(port));
    }
}

void Server::run()
{
    if (m_activeProtocols.empty())
        return;

    while(true)
    {
        for (auto& prot : m_activeProtocols)
        {
            prot->update();
        }

        std::this_thread::sleep_for(1ms);
    }
}
