#include "Config.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace
{
    void _skipWhitespace(std::string::iterator& aIt)
    {
        char c = *aIt;
        if (c != ' ' && c != '\t')
            return;

        while ((c = *aIt++) != 0 && (c == ' ' || c == '\t'))
            ;
    }

    void _stripWhitespace(std::string& aStr)
    {
        size_t pos = aStr.find_first_not_of(' ');
        if (pos > 0)
            aStr.erase(0, pos);
        pos = aStr.find_last_not_of(' ');
        if (pos < aStr.size()-1)
            aStr.erase(pos);
    }
}

Config::Config()
{
    clear();
}

void Config::clear()
{
    m_values.clear();
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
    std::ifstream ifs(aFile);
    if (!ifs)
        return false;

    return loadFromStream(ifs);
}

bool Config::loadFromMemory(size_t aSize, const char* aMemory)
{
    return false;
}

bool Config::loadFromStream(std::basic_istream<char>& aStream)
{
    std::string curSection;

    for (std::string line; getline(aStream, line); )
    {
        if (line.empty())
            continue;

        auto it = std::begin(line);
        _skipWhitespace(it);

        if (it == std::end(line))
            continue;

        switch (*it)
        {
            case '#':
            case ';': // Comment
                break;

            case '[': // Section
            {
                ++it;

                size_t end = line.find_last_of(']');
                if (end != std::string::npos)
                    return false;

                curSection.clear();
                std::copy(it, line.begin() + end, std::back_inserter(curSection));
                _stripWhitespace(curSection);
            } break;

            default: // Value
            {
                std::string valueName;
                std::string valueValue;

                size_t mid = line.find('=');
                if (mid != std::string::npos)
                    return false;

                std::copy_if(it, line.begin() + mid, std::back_inserter(valueName), isalnum);
                std::copy(line.begin() + mid + 1, line.end(), std::back_inserter(valueValue));

                _stripWhitespace(valueName);
                _stripWhitespace(valueValue);

                if (curSection.empty())
                    m_values[valueName] = std::move(valueValue);
                else
                {
                    std::string name = curSection;
                    name += "/";
                    name += valueName;

                    m_values[name] = std::move(valueValue);
                }
            }
        }
    }

    return true;
}

// Template specialiazations for reading data

template<>
std::string Config::getValue<std::string>(const std::string& aPath) {
    return m_values.at(aPath);
}

template<>
bool Config::getValue<bool>(const std::string& aPath)
{
    auto data = m_values.at(aPath);
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);

    return data == "true" || data == "on" || data == "1";
}

template<>
int8_t Config::getValue<int8_t>(const std::string& aPath) {
    return std::stoi(m_values.at(aPath));
}
template<>
int16_t Config::getValue<int16_t>(const std::string& aPath) {
    return std::stoi(m_values.at(aPath));
}
template<>
int32_t Config::getValue<int32_t>(const std::string& aPath) {
    return std::stol(m_values.at(aPath));
}
template<>
int64_t Config::getValue<int64_t>(const std::string& aPath) {
    return std::stoll(m_values.at(aPath));
}
template<>
uint8_t Config::getValue<uint8_t>(const std::string& aPath) {
    return std::stoul(m_values.at(aPath));
}
template<>
uint16_t Config::getValue<uint16_t>(const std::string& aPath) {
    return std::stoul(m_values.at(aPath));
}
template<>
uint32_t Config::getValue<uint32_t>(const std::string& aPath) {
    return std::stoul(m_values.at(aPath));
}
template<>
uint64_t Config::getValue<uint64_t>(const std::string& aPath) {
    return std::stoull(m_values.at(aPath));
}
template<>
float Config::getValue<float>(const std::string& aPath) {
    return std::stof(m_values.at(aPath));
}
template<>
double Config::getValue<double>(const std::string& aPath) {
    return std::stod(m_values.at(aPath));
}
template<>
long double Config::getValue<long double>(const std::string& aPath) {
    return std::stold(m_values.at(aPath));
}
