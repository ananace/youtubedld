#pragma once

#include <string>

class Config
{
public:
    Config();

    bool loadFromFile(const std::string& aFile);

private:
};
