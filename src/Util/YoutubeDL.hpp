#pragma once

#include <string>
#include <vector>

struct YoutubeDLRequest
{

};

struct YoutubeDLResponse
{
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

    void findInstall();
    void findInstall(const std::vector<std::string>& aSearchPaths);

    bool isAvailable() const;
    std::string getVersion() const;
    std::string getLatestVersion() const;

    void install();
    void update();

    YoutubeDLResponse request(const YoutubeDLRequest& aRequest);

private:
    int execute(const std::string& args, std::string& out) const;

    std::string m_installPath;
};
