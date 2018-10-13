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

    virtual void write(const std::string& aMsg) const = 0;

    virtual const Logger& operator<<(const std::string& aMsg) const { write(aMsg); return *this; }
    virtual const Logger& operator<<(const char* aMsg) const { write(std::string(aMsg)); return *this; }

    template<typename T>
    const Logger& operator<<(const T& aObj) const { return (*this << std::to_string(aObj)); }
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
    void write(const std::string& aMsg) const {
        mRealLogger.write(aMsg);
    }

private:
    Logger& mRealLogger;
};

LogWrapper Log(LogLevels aLogLevel);

class NullLogger : public Logger
{
public:
    void write(const std::string& /*aMsg*/) const final {}
};

class StdoutLogger : public Logger
{
public:
    virtual void write(const std::string& aMsg) const override;
};

class FileLogger : public Logger
{
public:
    FileLogger(const std::string& aPath);
    ~FileLogger();

    void setFile(const std::string& aPath);
    virtual void write(const std::string& aMsg) const override;

private:
    FILE* mFile;
};

class CombinedLogger : public Logger
{
public:
    virtual void write(const std::string& aMsg) const override;

private:
    std::vector<Logger*> mLoggers;
};

class PrependLogger : public Logger
{
public:
    typedef std::string(*Prepend_t)();

    void setPrepend(Prepend_t);
    virtual void write(const std::string& aMsg) const override;

private:
    std::unique_ptr<Logger> mRealLogger;
};

}
