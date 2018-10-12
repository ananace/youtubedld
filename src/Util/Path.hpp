#pragma once

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

std::filesystem::path ExpandPath(const std::filesystem::path& aPath);

}

