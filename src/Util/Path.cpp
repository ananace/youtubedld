#include "Path.hpp"

#include <wordexp.h>

std::filesystem::path Util::ExpandPath(const std::filesystem::path& aPath)
{
    wordexp_t p;
    wordexp(aPath.c_str(), &p, 0);

    std::filesystem::path ret = { p.we_wordv[0] };

    wordfree(&p);

    return ret;
}
