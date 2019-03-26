#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct YoutubeDLRequest
{
    std::string Url;

    bool ExtractAudio;
    std::string AudioFormat;
    std::string VideoFormat;
};

struct YoutubeDLResponse
{
    bool Success;

    uint32_t Duration;
    std::string Title;
    std::string ThumbnailUrl;
    std::string DownloadUrl;
    std::unordered_map<std::string, std::string> DownloadHeaders;
};

class YoutubeDL
{
public:
    YoutubeDL();
    YoutubeDL(const YoutubeDL&) = default;
    YoutubeDL(YoutubeDL&&) = default;
    ~YoutubeDL();

    YoutubeDL& operator=(const YoutubeDL&) = default;
    YoutubeDL& operator=(YoutubeDL&&) = default;

    static YoutubeDL& getSingleton();

    void findInstall();
    void findInstall(const std::vector<std::string>& aSearchPaths);

    bool isAvailable() const;
    std::string getVersion() const;
    std::string getLatestVersion() const;

    void install();
    void update();

    bool validRequest(const YoutubeDLRequest& aRequest) const;

    YoutubeDLResponse download(const YoutubeDLRequest& aRequest);
    YoutubeDLResponse request(const YoutubeDLRequest& aRequest);

private:
    int execute(const std::string& args, std::string& out);

    std::string m_installPath;
};
