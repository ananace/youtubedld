#include "YoutubeDL.hpp"
#include "Logging.hpp"

#include "../External/json.hpp"

#include <algorithm>
#include <experimental/filesystem>
#include <iomanip>
#include <sstream>

#include <cstdio>

namespace fs = std::experimental::filesystem;

namespace
{

const std::string SAFE_PROTOS[] = {
    "http", "https", "ftp", "ftps",
    "rtmp", "rtmps", "rtmpe", "rtmpt", "rtmpts", "rtmpte",
    "data"
};

const std::string SAFE_AUDIO_FORMATS[] = {
    "best", "aac", "flac", "mp3",
    "m4a", "opus", "vorbis", "wav"
};

}

YoutubeDL::YoutubeDL()
{
}

YoutubeDL::~YoutubeDL()
{
}

void YoutubeDL::findInstall()
{
    std::string path(getenv("PATH"));
    std::istringstream iss(path);

    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(iss, token, ':'))
        tokens.push_back(token);

    findInstall(tokens);
}

void YoutubeDL::findInstall(const std::vector<std::string>& aSearchPaths)
{
    for (auto& curPath : aSearchPaths)
    {
        fs::path dir(curPath),
                 file(dir / "youtube-dl");

        if (fs::is_regular_file(file))
        {
            // TODO: Only use executable files
            m_installPath = file.string();
            return;
        }
    }
}

bool YoutubeDL::isAvailable() const
{
    return !m_installPath.empty() && fs::is_regular_file(m_installPath);
}

std::string YoutubeDL::getVersion() const
{
    std::string ver;
    const_cast<YoutubeDL*>(this)->execute("--version", ver);
    return ver.substr(0, ver.find('\n'));
}

std::string YoutubeDL::getLatestVersion() const
{
    return "";
}

void YoutubeDL::install()
{
    // TODO
}

void YoutubeDL::update()
{
    // TODO
}

bool YoutubeDL::validRequest(const YoutubeDLRequest& aRequest) const
{
    auto colon = aRequest.Url.find(':');
    if (colon == std::string::npos)
        return false;

    // Ensure protocol is in list of safe ones
    auto prot = aRequest.Url.substr(0, colon);
    if (std::find(std::cbegin(SAFE_PROTOS), std::cend(SAFE_PROTOS), prot) == std::cend(SAFE_PROTOS))
        return false;

    // Ensure audio format is a safe one
    if (!aRequest.AudioFormat.empty() && std::find(std::cbegin(SAFE_AUDIO_FORMATS), std::cend(SAFE_AUDIO_FORMATS), aRequest.AudioFormat) == std::cend(SAFE_AUDIO_FORMATS))
        return false;

    return true;
}

YoutubeDLResponse YoutubeDL::download(const YoutubeDLRequest& aRequest)
{
    if (!validRequest(aRequest))
        return { false };

    std::ostringstream oss;

    if (aRequest.Url.empty())
    {
        // TODO
        return { false };
    }
    else
    {
        oss << std::quoted(aRequest.Url, '\'') << " ";
    }

    if (aRequest.ExtractAudio)
        oss << "--extract-audio ";
    if (!aRequest.AudioFormat.empty())
        oss << "--audio-format=" << aRequest.AudioFormat << " ";

    auto cmd = oss.str();
    std::string ret;

    if (execute(cmd, ret) != 0)
        return { false };

    // TODO: Read output

    return { true };
}

YoutubeDLResponse YoutubeDL::request(const YoutubeDLRequest& aRequest)
{
    if (!validRequest(aRequest))
        return { false };

    std::ostringstream oss;

    if (aRequest.Url.empty())
    {
        // TODO
        return { false };
    }
    else
    {
        oss << "-q -s -j " << std::quoted(aRequest.Url, '\'') << " ";
    }

    if (aRequest.ExtractAudio)
        oss << "--extract-audio ";
    if (!aRequest.AudioFormat.empty())
        oss << "--audio-format=" << aRequest.AudioFormat << " ";

    auto cmd = oss.str();
    std::string ret;

    int result = execute(cmd, ret);
    if (result != 0)
        return { false };

    Util::Log(Util::Log_Debug) << "> \"" << cmd << "\" returned (" << result << "|" << ret.size() << "B) \"" << ret << "\"";

    auto data = nlohmann::json::parse(ret);

    return { true, data["formats"][0]["url"], data["title"], data["formats"][0]["http_headers"] };
}

int YoutubeDL::execute(const std::string& args, std::string& out)
{
    std::ostringstream oss;
    oss << m_installPath << " " << args;

    std::array<char, 128> buffer;
    std::shared_ptr<FILE> pipe(popen(oss.str().c_str(), "r"));
    if (!pipe)
        throw std::runtime_error("popen() failed!");

    oss = std::ostringstream();
    while (!feof(pipe.get())) {
        if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
            oss << buffer.data();
    }

    out = oss.str();

    return WEXITSTATUS(pclose(pipe.get()));
}
