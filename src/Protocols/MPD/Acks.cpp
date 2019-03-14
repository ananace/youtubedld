#include "Acks.hpp"

using namespace Protocols::MPD;

MPDError::MPDError(Acks aError, const std::string& aErrMsg)
    : m_cmdListIndex(0)
    , m_error(aError)
    , m_errMsg(generateErrMsg(aErrMsg))
{
}

MPDError::MPDError(Acks aError, const std::string& aCommand, const std::string& aErrMsg)
    : m_cmdListIndex(0)
    , m_error(aError)
    , m_command(aCommand)
    , m_errMsg(generateErrMsg(aErrMsg))
{
}

MPDError::MPDError(int aCmdListIndex, Acks aError, const std::string& aErrMsg)
    : m_cmdListIndex(aCmdListIndex)
    , m_error(aError)
    , m_errMsg(generateErrMsg(aErrMsg))
{
}

MPDError::MPDError(int aCmdListIndex, Acks aError, const std::string& aCommand, const std::string& aErrMsg)
    : m_cmdListIndex(aCmdListIndex)
    , m_error(aError)
    , m_command(aCommand)
    , m_errMsg(generateErrMsg(aErrMsg))
{
}

std::string MPDError::generateErrMsg(const std::string& aShortMsg)
{
    return "ACK ["+std::to_string(m_error)+"@"+std::to_string(m_cmdListIndex)+"] {"+m_command+"} "+aShortMsg+"\n";
}

const char* MPDError::what() const noexcept
{
    return m_errMsg.c_str();
}
const char* MPDError::whatCommand() const noexcept
{
    return m_command.c_str();
}
int MPDError::whatAck() const noexcept
{
    return m_error;
}
int MPDError::whatCmdListIndex() const noexcept
{
    return m_cmdListIndex;
}
void MPDError::setCmdListIndex(int aIndex)
{
    m_cmdListIndex = aIndex;
}
