#include "Logging.hpp"
#include "Path.hpp"

#include <iomanip>
#include <iostream>

namespace
{

Util::NullLogger sNullLogger;
std::unique_ptr<Util::Logger> sLogger;
Util::LogLevels sLogLevel = Util::Log_Info;

}

std::string std::to_string(std::chrono::nanoseconds input)
{
    using namespace std::chrono;
    typedef duration<int, std::ratio<86400>> days;
    auto d = duration_cast<days>(input);
    input -= d;
    auto h = duration_cast<hours>(input);
    input -= h;
    auto m = duration_cast<minutes>(input);
    input -= m;
    auto s = duration_cast<seconds>(input);
    input -= s;
    auto ms = duration_cast<milliseconds>(input);

    auto dc = d.count();
    auto hc = h.count();
    auto mc = m.count();
    auto sc = s.count();
    auto msc = ms.count();

    std::stringstream ss;
    ss.fill('0');
    if (dc) {
        ss << d.count();
    }
    if (dc || hc) {
        if (dc) { ss << ':' << std::setw(2); } //pad if second set of numbers
        ss << h.count();
    }
    if (dc || hc || mc) {
        if (dc || hc) { ss << ':' << std::setw(2); }
        ss << m.count();
    }
    if (dc || hc || mc || sc) {
        if (dc || hc || mc) { ss << ':' << std::setw(2); }
        ss << s.count();
    }
    if (dc || hc || mc || sc || msc) {
        if (dc || hc || mc || sc) { ss << '.' << std::setw(2); }
        ss << ms.count();
    }

    if (!dc && !hc && !mc && !sc && !msc)
        ss << "null";

    return ss.str();
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
    std::cout << aMsg;
    std::cout.flush();
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
