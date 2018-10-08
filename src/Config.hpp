#pragma once

#include <string>
#include <unordered_map>

class Config
{
public:
    Config();

    void clear();

    bool loadDefaults();
    bool loadFromArgs(int aArgc, const char** aArgv);
    bool loadFromFile(const std::string& aFile);
    bool loadFromMemory(size_t aSize, const char* aMemory);
    bool loadFromStream(std::basic_istream<char>& aStream);

    inline bool hasValue(const std::string& aPath) { return m_values.count(aPath) > 0; }
    template<typename T>
    T getValue(const std::string& aPath);
    template<typename T>
    T getValue(const std::string& aPath, const T& aDefault) {
        return hasValue(aPath) ? getValue<T>(aPath) : aDefault;
    }

private:
    std::unordered_map<std::string, std::string> m_values;
};
