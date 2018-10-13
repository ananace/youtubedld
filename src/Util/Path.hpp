#pragma once

#include <stdexcept>
#if __has_include(<filesystem>)
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
    namespace filesystem = std::experimental::filesystem;
}
#endif

namespace Util
{

class PathExpandError : public std::exception
{
public:
    PathExpandError(const std::string& aMsg);

    const char* what() const noexcept {
        return mMsg.c_str();
    }

private:
    std::string mMsg;
};

std::filesystem::path ExpandPath(const std::filesystem::path& aPath, bool throws = false);

}

