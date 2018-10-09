#include "Config.hpp"

#include <algorithm>
#include <experimental/filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include <cassert>

namespace fs = std::experimental::filesystem;

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
    setValue("CacheDir", "$XDG_CACHE_DIR/youtubedld");
    setValue("DataDir", "$XDG_DATA_DIR/youtubedld");
    // setValue("LogFile", "/var/log/youtubedld.log");

    return true;
}

bool Config::loadFromArgs(int aArgc, const char** aArgv)
{
    for (int i = 1; i < aArgc; ++i)
    {
        std::string arg(aArgv[i]);

        if (arg == "-v")
            setValue("Verbose", "1");
        else if (arg == "-c")
        {
            assert(++i < aArgc);
            setValue("ConfigDir", aArgv[i]);
        }
    }

    return true;
}

bool Config::loadFromEnv()
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

bool Config::loadFromMemory(const char* aMemory, size_t aSize)
{
    std::string data(aMemory, aSize);
    std::istringstream iss(data);
    return loadFromStream(iss);
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
                if (end == std::string::npos)
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
                if (mid == std::string::npos)
                    return false;

                std::copy_if(it, line.begin() + mid, std::back_inserter(valueName), isalnum);
                std::copy(line.begin() + mid + 1, line.end(), std::back_inserter(valueValue));

                _stripWhitespace(valueName);
                _stripWhitespace(valueValue);

                std::string name = valueName;
                if (!curSection.empty())
                {
                    name = curSection;
                    name += "/";
                    name += valueName;
                }

                setValue(name, std::move(valueValue));
            }
        }
    }

    return true;
}

bool Config::hasValue(const std::string& aPath) const
{
    auto data = aPath;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    return m_values.count(data) > 0;
}

const std::string& Config::getValue(const std::string& aPath) const
{
    printf("[CFG] Reading %s\n", aPath.c_str());

    auto data = aPath;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);

    return m_values.at(data);
}

void Config::setValue(const std::string& aPath, const std::string& aValue)
{
    printf("[CFG] Setting %s\n", aPath.c_str());

    auto data = aPath;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);

    m_values[data] = aValue;
}

void Config::setValue(const std::string& aPath, std::string&& aValue)
{
    printf("[CFG] Setting %s\n", aPath.c_str());

    auto data = aPath;
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);

    m_values[data] = std::move(aValue);
}

// Template specialiazations for reading data

template<>
fs::path Config::getValueConv<fs::path>(const std::string& aPath) const {
    return fs::path(getValue(aPath));
}


template<>
bool Config::getValueConv<bool>(const std::string& aPath) const
{
    auto data = getValue(aPath);
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);

    return data == "true" || data == "on" || data == "1";
}

template<>
int8_t Config::getValueConv<int8_t>(const std::string& aPath) const {
    return std::stoi(getValue(aPath));
}
template<>
int16_t Config::getValueConv<int16_t>(const std::string& aPath) const {
    return std::stoi(getValue(aPath));
}
template<>
int32_t Config::getValueConv<int32_t>(const std::string& aPath) const {
    return std::stol(getValue(aPath));
}
template<>
int64_t Config::getValueConv<int64_t>(const std::string& aPath) const {
    return std::stoll(getValue(aPath));
}
template<>
uint8_t Config::getValueConv<uint8_t>(const std::string& aPath) const {
    return std::stoul(getValue(aPath));
}
template<>
uint16_t Config::getValueConv<uint16_t>(const std::string& aPath) const {
    return std::stoul(getValue(aPath));
}
template<>
uint32_t Config::getValueConv<uint32_t>(const std::string& aPath) const {
    return std::stoul(getValue(aPath));
}
template<>
uint64_t Config::getValueConv<uint64_t>(const std::string& aPath) const {
    return std::stoull(getValue(aPath));
}
template<>
float Config::getValueConv<float>(const std::string& aPath) const {
    return std::stof(getValue(aPath));
}
template<>
double Config::getValueConv<double>(const std::string& aPath) const {
    return std::stod(getValue(aPath));
}
template<>
long double Config::getValueConv<long double>(const std::string& aPath) const {
    return std::stold(getValue(aPath));
}
