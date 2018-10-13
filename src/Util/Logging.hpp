#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Util
{

enum LogLevels
{
    Log_Debug = 0,
    Log_Info,
    Log_Warning,
    Log_Error,
};

class Logger;

LogLevels GetLogLevel();
void SetLogLevel(LogLevels aLogLevel);
void SetLogger(Logger* aLogger);
template<typename T, typename... Args>
void SetLogger(Args&&... aArgs) {
    SetLogger(std::make_unique<T>(std::forward<Args>(aArgs)...));
}
Logger& Log(LogLevels aLogLevel);

class Logger
{
public:
    // virtual void Write(const std::string& aMsg) = 0;

    virtual Logger& operator<<(const std::string& aMsg) = 0;
    virtual Logger& operator<<(const char* aMsg) { return *this << std::string(aMsg); }

    template<typename T>
    Logger& operator<<(const T& aObj) { return (*this << std::to_string(aObj)); }
};

class NullLogger : public Logger
{
public:
    Logger& operator<<(const std::string&) final { return *this; }
};

class StdoutLogger : public Logger
{
public:
    Logger& operator<<(const std::string& aMsg);
};

class FileLogger : public Logger
{
public:
    Logger& operator<<(const std::string& aMsg);

private:
    FILE* mFile;
};

class CombinedLogger : public Logger
{
public:
    Logger& operator<<(const std::string& aMsg);

private:
    std::vector<Logger*> mLoggers;
};

}
