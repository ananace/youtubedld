#pragma once

#include "Config.hpp"
#include "ActivePlaylist.hpp"
#include "Protocols/Base.hpp"

#include <chrono>
#include <deque>
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

    const Config& getConfig() const noexcept { return m_config; }
    Config& getConfig() noexcept { return m_config; }

    ActivePlaylist& getQueue() { return m_activePlaylist; }

    std::chrono::milliseconds getUptime() const;

    void pushEvent(const Protocols::Event& aEvent);

private:
    bool on_tick();
    bool on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage);
    void on_decoder_pad_added(const Glib::RefPtr<Gst::Pad>& aPad);

    Config m_config;
    std::vector<std::unique_ptr<Protocols::Base>> m_activeProtocols;
    ActivePlaylist m_activePlaylist;
    std::deque<Protocols::Event> m_eventQueue;
    std::chrono::system_clock::time_point m_startTime;

    sigc::connection m_ticker;

    Glib::RefPtr<Gst::Element> m_pipeline;
    Glib::RefPtr<Glib::MainLoop> m_mainLoop;
};
