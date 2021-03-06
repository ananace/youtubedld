#include "YoutubeDL.hpp"
#include "Logging.hpp"

#include "../External/json.hpp"

#include <algorithm>
#include <experimental/filesystem>
#include <iomanip>
#include <fstream>
#include <random>
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

YoutubeDL s_youtubeDL;
YoutubeDL& YoutubeDL::getSingleton()
{
    return s_youtubeDL;
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

    tokens.push_back(fs::current_path());

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

bool YoutubeDL::canLocalInstall() const
{
    auto curPath = fs::current_path();
    auto binaryPath = curPath / "youtube-dl";
    if (fs::is_regular_file(binaryPath))
        return true;

    auto ofs = std::ofstream(binaryPath);
    if (ofs.good())
    {
        fs::remove(binaryPath);
        return true;
    }

    return false;
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
    // https://youtube-dl.org/downloads/latest/
    //
    // Generates redirect like; Location: https://youtube-dl.org/downloads/2020.11.18/
    return "";
}

bool YoutubeDL::install()
{
    // TODO
    return false;
}

bool YoutubeDL::update()
{
    // TODO: Proper error handling
    if (!isAvailable())
        return false;

    std::string out;
    execute("--update", out);

    if (out.find("Please use that to update.") != std::string::npos)
        return false;

    return true;
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
        oss << std::quoted(aRequest.Url, '\'') << " ";

    if (!aRequest.VideoFormat.empty())
        oss << "--format=" << aRequest.VideoFormat << " ";
    else
        oss << "--format=bestaudio ";

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
        oss << "-C -q -s -j " << std::quoted(aRequest.Url, '\'') << " ";
    }

    if (aRequest.ExtractAudio)
        oss << "--extract-audio ";
    if (!aRequest.AudioFormat.empty())
        oss << "--audio-format=" << aRequest.AudioFormat << " ";

    auto cmd = oss.str();
    std::string ret;

    int result = execute(cmd, ret);
    if (result != 0)
        throw std::runtime_error(ret);

    Util::Log(Util::Log_Debug) << "[YDL] > \"" << cmd << "\" returned (" << result << "|" << ret.size() << "B)";

    try
    {
        auto data = nlohmann::json::parse(ret);
        if (data["duration"].is_null())
            data["duration"] = 0;

        if (data["thumbnail"].is_null())
        {
            auto thumbnails = data["thumbnails"];
            if (thumbnails.empty())
                data["thumbnail"] = "";
            else
                data["thumbnail"] = thumbnails.front();
        }

        auto response = YoutubeDLResponse{ true, data["duration"], data["title"], data["thumbnail"] };

        if (data.count("artist") > 0 && !data["artist"].is_null())
            response.Artist = data["artist"];
        else if (data.count("creator") > 0 && !data["creator"].is_null())
            response.Artist = data["creator"];
        else if (data.count("uploader") > 0 && !data["uploader"].is_null())
            response.Artist = data["uploader"];

        if (data.count("extractor_key") > 0 && !data["extractor_key"].is_null())
            response.Extractor = data["extractor_key"];
        else if (data.count("extractor") > 0 && !data["extractor"].is_null())
            response.Extractor = data["extractor"];

        nlohmann::json chosenFormat = { { "abr", -1.0 } };

        // Direct format match
        if (data.count("url") > 0)
        {
            chosenFormat = data;
        }
        else
        {
            nlohmann::json formats;
            // Multiple format matches
            if (data.count("requested_formats") > 0)
            {
                formats = data["requested_formats"];
            }
            // No format match, iterate all available formats
            else
            {
                formats = data["formats"];
            }

            for (auto& format : formats)
            {
                if (format["acodec"] == "none")
                    continue;
                if (format["vcodec"] != "none")
                    continue;

                if (format["abr"].is_null())
                    format["abr"] = 1.0;

                if (format["abr"].get<double>() > chosenFormat["abr"].get<double>())
                {
                    chosenFormat = format;
                    Util::Log(Util::Log_Debug) << "Using format " << chosenFormat.dump();
                }
            }

            // Unable to find a suitable format, fall back to first one
            if (chosenFormat["url"].is_null())
            {
                chosenFormat = *std::find_if(formats.begin(), formats.end(), [](auto& format) { return format["acodec"] != "none"; });

                Util::Log(Util::Log_Debug) << "Falling back to format " << chosenFormat.dump();
            }
        }

        response.DownloadUrl = chosenFormat["url"];
        response.DownloadHeaders = chosenFormat["http_headers"].get<std::unordered_map<std::string, std::string>>();

        return response;
    }
    catch(const std::exception& ex)
    {
        Util::Log(Util::Log_Error) << ex.what();
        throw ex;
    }
}

int YoutubeDL::execute(const std::string& args, std::string& out)
{
    std::ostringstream oss;
    oss << m_installPath << " " << args;

    std::random_device dev;
    std::uniform_int_distribution<int> dist(100000, 999999);
    int val = dist(dev);

    auto tmpdir = fs::temp_directory_path();
    auto outname = tmpdir / ("ydl_out" + std::to_string(val));
    auto errname = tmpdir / ("ydl_err" + std::to_string(val));

    Util::Log(Util::Log_Debug) << "[YDL] < " << oss.str();

    int ret = 255;
    try
    {
        std::string scommand = oss.str();
        std::string cmd = scommand + " 1> " + outname.string() + " 2> " + errname.string();
        ret = std::system(cmd.c_str());

        std::string filename;
        if (ret == 0)
            filename = outname;
        else
            filename = errname;

        std::ifstream file(filename, std::ios::in | std::ios::binary);
        std::string line;
        while (file)
        {
            std::getline(file, line);
            out.append(line);
        }
        file.close();
    }
    catch(...) { }

    fs::remove(errname);
    fs::remove(outname);

    return ret;
}
