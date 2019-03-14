#pragma once

#include <stdexcept>

namespace Protocols
{

namespace MPD
{

enum Acks {
    ACK_OK_SILENT = -1,
    ACK_OK = 0,

    ACK_ERROR_NOT_LIST = 1,
    ACK_ERROR_ARG = 2,
    ACK_ERROR_PASSWORD = 3,
    ACK_ERROR_PERMISSION = 4,
    ACK_ERROR_UNKNOWN = 5,

    ACK_ERROR_NO_EXIST = 50,
    ACK_ERROR_PLAYLIST_MAX = 51,
    ACK_ERROR_SYSTEM = 52,
    ACK_ERROR_PLAYLIST_LOAD = 53,
    ACK_ERROR_UPDATE_ALREADY = 54,
    ACK_ERROR_PLAYER_SYNC = 55,
    ACK_ERROR_EXIST = 56,
};

class MPDError : public std::exception
{
public:
    MPDError(Acks aError, const std::string& aErrMsg);
    MPDError(Acks aError, const std::string& aCommand, const std::string& aErrMsg);
    MPDError(int aCommandListIndex, Acks aError, const std::string& aErrMsg);
    MPDError(int aCommandListIndex, Acks aError, const std::string& aCommand, const std::string& aErrMsg);

    const char* what() const noexcept;
    const char* whatCommand() const noexcept;
    int whatAck() const noexcept;
    int whatCmdListIndex() const noexcept;
    void setCmdListIndex(int aIndex);

private:
    std::string generateErrMsg(const std::string& aShortMsg);

    int m_cmdListIndex;
    Acks m_error;
    std::string m_command;
    std::string m_errMsg;
};

}

}
