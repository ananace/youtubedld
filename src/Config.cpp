#include "Config.hpp"

Config::Config()
{
}

bool Config::loadDefaults()
{
    return true;
}

bool Config::loadFromArgs(int aArgc, const char** aArgv)
{
    return true;
}

bool Config::loadFromFile(const std::string& aFile)
{
    return false;
}
