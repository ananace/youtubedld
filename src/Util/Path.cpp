#include "Path.hpp"
#include "Logging.hpp"

#include <wordexp.h>

Util::PathExpandError::PathExpandError(const std::string& aMsg)
    : mMsg(aMsg)
{ }

std::filesystem::path Util::ExpandPath(const std::filesystem::path& aPath, bool throws)
{
    wordexp_t p;
    int err = wordexp(aPath.c_str(), &p, WRDE_NOCMD | WRDE_UNDEF);

    if (err != 0 && throws)
    {
        if (err == WRDE_BADCHAR)
            throw PathExpandError("Bad character in substitution");
        if (err == WRDE_BADVAL)
            throw PathExpandError("Undefined variable in path substitution");
        if (err == WRDE_CMDSUB)
            throw PathExpandError("Attempted command execution");
        if (err == WRDE_NOSPACE)
            throw PathExpandError("Memory error");
        if (err == WRDE_SYNTAX)
            throw PathExpandError("Syntax error");
        throw PathExpandError("Unknown error");
    }

    std::filesystem::path ret = { p.we_wordv[0] };
    for (size_t i = 1; i < p.we_wordc; ++i)
    {
        ret.append(" ");
        ret.append(p.we_wordv[i]);
    }

    wordfree(&p);

    Log(Log_Debug) << "[Path] \"" << aPath.string() << "\" expanded to \"" << ret.string() << "\"";

    return ret;
}
