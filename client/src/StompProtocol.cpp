#include "StompProtocol.h"
#include <sstream>

std::string StompProtocol::createConnectFrame(const std::string &host, const std::string &username, const std::string &password)
{
    std::ostringstream oss;
    oss << "CONNECT\n"
        << "accept-version:1.2\n"
        << "host:" << host << "\n"
        << "login:" << username << "\n"
        << "passcode:" << password << "\n\n"
        << "\u0000";
    return oss.str();
}

std::string StompProtocol::createSendFrame(const std::string &destination, const std::string &message)
{
    std::ostringstream oss;
    oss << "SEND\n"
        << "destination:" << destination << "\n\n"
        << message << "\n"
        << "\u0000";
    return oss.str();
}

std::string StompProtocol::createSubscribeFrame(const std::string &destination, const std::string &id)
{
    std::ostringstream oss;
    oss << "SUBSCRIBE\n"
        << "destination:" << destination << "\n"
        << "id:" << id << "\n"
        << "ack:auto\n\n"
        << "\u0000";
    return oss.str();
}

std::string StompProtocol::createUnsubscribeFrame(const std::string &id)
{
    std::ostringstream oss;
    oss << "UNSUBSCRIBE\n"
        << "id:" << id << "\n\n"
        << "\u0000";
    return oss.str();
}

std::string StompProtocol::createDisconnectFrame()
{
    std::ostringstream oss;
    oss << "DISCONNECT\n\n"
        << "\u0000";
    return oss.str();
}
