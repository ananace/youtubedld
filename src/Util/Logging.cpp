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
