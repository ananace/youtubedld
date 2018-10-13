#include "Logging.hpp"
#include "Path.hpp"

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
Util::LogWrapper Util::Log(LogLevels aLogLevel)
{
    if (aLogLevel < sLogLevel || !sLogger)
        return LogWrapper(sNullLogger);
    return LogWrapper(*sLogger);
}

void Util::StdoutLogger::write(const std::string& aMsg)
{
    printf(aMsg.c_str());
}

Util::FileLogger::FileLogger(const std::string& aPath)
    : mFile(nullptr)
{
    setFile(aPath);
}

Util::FileLogger::~FileLogger()
{
    if (mFile)
        fclose(mFile);
}

void Util::FileLogger::setFile(const std::string& aPath)
{
    if (mFile)
        fclose(mFile);

    auto path = Util::ExpandPath(aPath);
    mFile = fopen(path.c_str(), "a");
}

void Util::FileLogger::write(const std::string& aMsg)
{
    if (mFile)
        fprintf(mFile, aMsg.c_str());
}

void Util::CombinedLogger::write(const std::string& aMsg)
{
    for (auto& logger : mLoggers)
        logger->write(aMsg);
}
