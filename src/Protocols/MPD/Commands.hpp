#pragma once

#include <array>
#include <string>

namespace Procotols
{

namespace MPD
{

enum Commands
{
    Command_invalid = -2,
    Command_connect = -1,

    Command_status,

    Command_count,
};


static const std::array<std::string, Command_count> CommandNames = {
    { "status" }
};

}

}
