#include "Logging.hpp"

namespace
{

Util::NullLogger sNullLogger;
std::unique_ptr<Util::Logger> sLogger;
Util::LogLevels sLogLevel = Util::Log_Info;

}

Util::LogLevels Util::GetLogLevel()
{
    return sLogLevel;
}

void Util::SetLogLevel(LogLevels aLogLevel)
{
    sLogLevel = aLogLevel;
}

void Util::SetLogger(Logger* aLogger)
{
    sLogger.reset(aLogger);
}
Util::Logger& Util::Log(LogLevels aLogLevel)
{
    if (aLogLevel < sLogLevel || !sLogger)
        return sNullLogger;
    return *sLogger;
}


Util::Logger& Util::StdoutLogger::operator<<(const std::string& aMsg)
{
    printf(aMsg.c_str());
    return *this;
}
Util::Logger& Util::FileLogger::operator<<(const std::string& aMsg)
{
    fprintf(mFile, aMsg.c_str());
    return *this;
}
Util::Logger& Util::CombinedLogger::operator<<(const std::string& aMsg)
{
    for (auto& loggerPtr : mLoggers)
    {
        auto& logger = *loggerPtr;
        logger << aMsg;
    }
    return *this;
}
