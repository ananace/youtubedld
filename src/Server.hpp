#pragma once

#include "Config.hpp"
#include "ActivePlaylist.hpp"
#include "Protocols/Base.hpp"

#include <memory>
#include <vector>

#include <gstreamermm.h>
#include <glibmm/main.h>

class Server
{
public:
    Server();

    void init(int aArgc, const char** aArgv);
    void run();

    void play(const std::string& aMusic);

private:
    bool on_tick();
    bool on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage);
    void on_decoder_pad_added(const Glib::RefPtr<Gst::Pad>& aPad);

    Config m_config;
    std::vector<std::unique_ptr<Protocols::Base>> m_activeProtocols;
    ActivePlaylist m_activePlaylist;

    sigc::connection m_ticker;
    Glib::RefPtr<Glib::MainLoop> m_mainLoop;
};
