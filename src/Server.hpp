#pragma once

#include "Config.hpp"
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
    bool on_bus_message(const Glib::RefPtr<Gst::Bus>& aBus, const Glib::RefPtr<Gst::Message>& aMessage);

    Config m_config;
    std::vector<std::unique_ptr<Protocols::Base>> m_activeProtocols;

    Glib::RefPtr<Glib::MainLoop> m_mainLoop;
    Glib::RefPtr<Gst::Element> m_source;
    Glib::RefPtr<Gst::Element> m_decoder;
    Glib::RefPtr<Gst::Pipeline> m_pipeline;
};
