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
    auto ret = LogWrapper(*sLogger);
    ret.begin(aLogLevel);
    return std::move(ret);
}

void Util::StdoutLogger::write(const std::string& aMsg) const
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

void Util::FileLogger::write(const std::string& aMsg) const
{
    if (mFile)
        fprintf(mFile, aMsg.c_str());
}

void Util::CombinedLogger::write(const std::string& aMsg) const
{
    for (auto& logger : mLoggers)
        logger->write(aMsg);
}

void Util::CombinedLogger::begin(LogLevels aLevel) const
{
    for (auto& logger : mLoggers)
        logger->begin(aLevel);
}

void Util::CombinedLogger::addLogger(Logger* aLogger)
{
    mLoggers.push_back(aLogger);
}

Util::PrependLogger::PrependLogger(Logger* aLogger, Prepend_t aPrependMethod)
    : mRealLogger(aLogger)
    , mPrepend(aPrependMethod)
{
}

void Util::PrependLogger::setPrepend(Prepend_t aPrependMethod)
{
    mPrepend = aPrependMethod;
}

void Util::PrependLogger::begin(LogLevels aLevel) const
{
    mRealLogger->write(mPrepend(aLevel));
    mRealLogger->begin(aLevel);
}

void Util::PrependLogger::write(const std::string& aMsg) const
{
    mRealLogger->write(aMsg);
}
