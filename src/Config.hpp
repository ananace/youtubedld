#pragma once

#include <string>

class Config
{
public:
    Config();

    bool loadDefaults();
    bool loadFromArgs(int aArgc, const char** aArgv);
    bool loadFromFile(const std::string& aFile);

    bool hasValue(const std::string& aPath);
    template<typename T>
    T getValue(const std::string& aPath);
    template<typename T>
    T getValue(const std::string& aPath, const T& aDefault);

private:
};
