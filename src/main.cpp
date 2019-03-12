#include "Server.hpp"
#include "Util/Logging.hpp"

namespace
{

std::string formatTime(Util::LogLevels aLevel)
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ts = localtime(&time);
    char timeBuf[32];
    int len = std::strftime(timeBuf, 32, "%H:%M:%S|  ", ts);

    static constexpr char LogLevels[] = { 'D', 'I', 'W', 'E' };
    timeBuf[len-2] = LogLevels[int(aLevel)];
    return timeBuf;
}

}

int main(int argc, const char** argv)
{
    auto outputLogger = new Util::StdoutLogger;
    auto combinedLogger = new Util::CombinedLogger;
    auto timeLogger = new Util::PrependLogger(combinedLogger);

    combinedLogger->addLogger(outputLogger);
    timeLogger->setPrepend(formatTime);

    Util::SetLogger(timeLogger);
    Util::SetLogLevel(Util::Log_Debug);

    Server srv;

    srv.init(argc, argv);
    srv.run();

    return 0;
}
