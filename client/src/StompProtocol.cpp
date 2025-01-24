#include "StompProtocol.h"
#include <sstream>

StompProtocol::StompProtocol() : subscriptionIdCounter(0), receiptIdCounter(0), receiptMap() {}

std::string StompProtocol::createConnectFrame(const std::string &host, const std::string &username, const std::string &password)
{
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "CONNECT\n"
        << "receipt:" << receiptId << "\n"
        << "accept-version:1.2\n"
        << "host:" << host << "\n"
        << "login:" << username << "\n"
        << "passcode:" << password << "\n\n"
        << '\0';

    addReceipt(receiptId, "CONNECT");

    return oss.str();
}

std::string StompProtocol::createSendFrame(const std::string &destination, const std::string &message)
{
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "SEND\n"
        << "receipt:" << receiptId << "\n"
        << "destination:" << destination << "\n\n"
        << message << "\n"
        << '\0';

    addReceipt(receiptId, "SEND");

    return oss.str();
}

std::string StompProtocol::createSubscribeFrame(const std::string &destination)
{
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "SUBSCRIBE\n"
        << "receipt:" << receiptId << "\n"
        << "destination:" << destination << "\n"
        << "id:" << getNextSubscriptionId() << "\n"
        << '\0';

    addReceipt(receiptId, "SUBSCRIBE");

    return oss.str();
}

std::string StompProtocol::createUnsubscribeFrame(const std::string &id)
{
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "UNSUBSCRIBE\n"
        << "receipt:" << receiptId << "\n"
        << "id:" << id << "\n\n"
        << '\0';

    addReceipt(receiptId, "UNSUBSCRIBE");

    return oss.str();
}

std::string StompProtocol::createDisconnectFrame()
{
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "DISCONNECT\n"
        << "receipt:" << receiptId << "\n"
        << '\0';

    addReceipt(receiptId, "DISCONNECT");

    return oss.str();
}

std::map<std::string, std::string> StompProtocol::parseFrame(const std::string &frame)
{
    std::map<std::string, std::string> headers;
    std::istringstream stream(frame);
    std::string line;

    // Get the command
    std::getline(stream, line);
    headers["command"] = line;

    // Get the headers
    while (std::getline(stream, line) && !line.empty())
    {
        auto colonPos = line.find(':');
        if (colonPos != std::string::npos)
        {
            headers[line.substr(0, colonPos)] = line.substr(colonPos + 1);
        }
    }

    // Get the body (if any)
    std::string body;
    while (std::getline(stream, line))
    {
        body += line + "\n";
    }
    if (!body.empty())
    {
        // Remove the last newline character
        body.pop_back();
        headers["body"] = body;
    }

    return headers;
}

int StompProtocol::getNextSubscriptionId()
{
    return subscriptionIdCounter++;
}

int StompProtocol::getNextReceiptId()
{
    return receiptIdCounter++;
}

void StompProtocol::addReceipt(int receiptId, const string &request)
{
    receiptMap[receiptId] = request;
}

string StompProtocol::getRequestByReceipt(int receiptId)
{
    if (receiptMap.find(receiptId) != receiptMap.end())
    {
        return receiptMap[receiptId];
    }
    return "";
}