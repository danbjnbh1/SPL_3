#include "StompProtocol.h"
#include <sstream>

StompProtocol::StompProtocol() : subscriptionIdCounter(0), receiptIdCounter(0), receiptMap(), subscriptionMap() {}

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

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
}

std::string StompProtocol::createSendFrame(const Event &event, const std::string &channel_name)
{
    std::ostringstream frame;

    // Construct the SEND frame for the current event
    frame << "SEND\n"
          << "destination:/" << channel_name << "\n"
          << "receipt:" << getNextReceiptId() << "\n\n"
          << "user: " << channel_name << "\n" // Assume a global `username` variable
          << "city: " << event.get_city() << "\n"
          << "event name: " << event.get_name() << "\n"
          << "date time: " << event.get_date_time() << "\n";

    // Add general information
    frame << "general information:\n";
    for (const auto &info : event.get_general_information())
    {
        frame << "  " << info.first << ": " << info.second << "\n";
    }

    // Add description
    frame << "description:\n"
          << event.get_description() << "\n";

    // Terminate the frame with the end character
    frame << '\0';

    return frame.str();
}

std::string StompProtocol::generateSummary(const std::string &channelName)
{
    std::ostringstream summaryStream;

    // Check if the channel exists
    int subscriptionId = getSubscriptionIdByChannel(channelName);
    if (subscriptionId == -1)
    {
        return "Channel " + channelName + " not found.";
    }

    // Collect and format events (placeholder logic for event collection)
    summaryStream << "Summary for channel: " << channelName << "\n";
    summaryStream << "-----------------------------------\n";
    summaryStream << "Event Name | Date-Time | Description\n";
    summaryStream << "-----------------------------------\n";

    // Example event details (replace with real event logic if applicable)
    summaryStream << "Event1 | 2025-01-01 10:00 | Example description 1\n";
    summaryStream << "Event2 | 2025-01-02 14:00 | Example description 2\n";

    return summaryStream.str();
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

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
}

std::string StompProtocol::createUnsubscribeFrame(const string &channel)
{
    int subsctiptionId = getSubscriptionIdByChannel(channel); //! handle -1 id
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "UNSUBSCRIBE\n"
        << "receipt:" << receiptId << "\n"
        << "id:" << subsctiptionId << "\n\n"
        << '\0';

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
}

std::string StompProtocol::createDisconnectFrame()
{
    int receiptId = getNextReceiptId();
    std::ostringstream oss;
    oss << "DISCONNECT\n"
        << "receipt:" << receiptId << "\n"
        << '\0';

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
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

void StompProtocol::addSubscription(int subscriptionId, const string &destination)
{
    subscriptionMap[subscriptionId] = destination;
}

void StompProtocol::removeSubscription(int subscriptionId)
{
    subscriptionMap.erase(subscriptionId);
}

string StompProtocol::getRequestByReceipt(int receiptId)
{
    return receiptMap[receiptId];
}

int StompProtocol::getSubscriptionIdByChannel(const string &channel)
{
    std::cout << "exit " << channel << endl;

    for (const pair<const int, string> &pair : subscriptionMap)
    {
        if (pair.second == channel)
        {
            return pair.first;
        }
    }
    return -1; // Return -1 if the subscription ID is not found
}

string StompProtocol::getChannelById(int id)
{
    return subscriptionMap[id];
}