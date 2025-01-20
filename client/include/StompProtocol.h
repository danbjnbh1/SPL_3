#pragma once
#pragma once
#include <string>
#include <map>
#include "../include/ConnectionHandler.h"

// TODO: implement the STOMP protocol
class StompProtocol
{
private:
public:
    std::string createConnectFrame(const std::string &host, const std::string &login, const std::string &passcode);
    std::string createSendFrame(const std::string &destination, const std::string &message);
    std::string createSubscribeFrame(const std::string &destination, const std::string &id);
    std::string createUnsubscribeFrame(const std::string &id);
    std::string createDisconnectFrame();
    std::map<std::string, std::string> parseFrame(const std::string &frame);
};
