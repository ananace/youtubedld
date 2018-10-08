#include "YoutubeDL.hpp"

#include <experimental/filesystem>
#include <sstream>

namespace fs = std::experimental::filesystem;

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
    return fs::is_regular_file(m_installPath);
}

std::string YoutubeDL::getVersion() const
{
    std::string ver;
    execute("--version", ver);
    return ver;
}

std::string YoutubeDL::getLatestVersion() const
{
    return "";
}

void YoutubeDL::install()
{
}

void YoutubeDL::update()
{
}

YoutubeDLResponse YoutubeDL::request(const YoutubeDLRequest& aRequest)
{
    return {};
}
