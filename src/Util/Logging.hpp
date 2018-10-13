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

class Logger
{
public:
    virtual ~Logger() = default;

    virtual void write(const std::string& aMsg) = 0;

    virtual Logger& operator<<(const std::string& aMsg) { write(aMsg); return *this; }
    virtual Logger& operator<<(const char* aMsg) { write(std::string(aMsg)); return *this; }

    template<typename T>
    Logger& operator<<(const T& aObj) { return (*this << std::to_string(aObj)); }
};

class LogWrapper : public Logger
{
public:
    LogWrapper(Logger& aRealLogger)
        : mRealLogger(aRealLogger)
    { }
    ~LogWrapper() {
        mRealLogger.write("\n");
    }
    void write(const std::string& aMsg) {
        mRealLogger.write(aMsg);
    }

private:
    Logger& mRealLogger;
};
LogWrapper Log(LogLevels aLogLevel);

class NullLogger : public Logger
{
public:
    void write(const std::string& /*aMsg*/) final {}
};

class StdoutLogger : public Logger
{
public:
    virtual void write(const std::string& aMsg) override;
};

class FileLogger : public Logger
{
public:
    FileLogger(const std::string& aPath);
    ~FileLogger();

    void setFile(const std::string& aPath);
    virtual void write(const std::string& aMsg) override;

private:
    FILE* mFile;
};

class CombinedLogger : public Logger
{
public:
    virtual void write(const std::string& aMsg) override;

private:
    std::vector<Logger*> mLoggers;
};

}
