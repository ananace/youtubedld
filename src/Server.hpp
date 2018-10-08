#pragma once

#include "Protocols/Base.hpp"

#include <memory>
#include <vector>

class Server
{
public:
    void Update();

private:
    std::vector<std::unique_ptr<Protocol::Base>> m_activeProtocols;

};
