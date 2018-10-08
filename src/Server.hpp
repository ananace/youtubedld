#pragma once

#include "Config.hpp"
#include "Protocols/Base.hpp"

#include <memory>
#include <vector>

class Server
{
public:
    Server();

    void init(int aArgc, const char** aArgv);
    void run();

private:
    Config m_config;
    std::vector<std::unique_ptr<Protocols::Base>> m_activeProtocols;
};
