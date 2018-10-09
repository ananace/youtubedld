#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class Config
{
public:
    Config();

    void clear();

    bool loadDefaults();
    bool loadFromArgs(int aArgc, const char** aArgv);
    bool loadFromEnv();
    bool loadFromFile(const std::string& aFile);
    bool loadFromMemory(const char* aMemory, size_t aSize);
    bool loadFromStream(std::basic_istream<char>& aStream);

    bool hasValue(const std::string& aPath) const;
    const std::string& getValue(const std::string& aPath) const;
    const std::string& getValue(const std::string& aPath, const std::string& aDefault) const {
        return hasValue(aPath) ? getValue(aPath) : aDefault;
    }
    void setValue(const std::string& aPath, const std::string& aValue);
    void setValue(const std::string& aPath, std::string&& aValue);

    template<typename T>
    T getValueConv(const std::string& aPath) const;
    template<typename T>
    T getValueConv(const std::string& aPath, const T& aDefault) const {
        return hasValue(aPath) ? getValueConv<T>(aPath) : aDefault;
    }

private:
    std::unordered_map<std::string, std::string> m_values;
};
