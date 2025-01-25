#include "StompProtocol.h"
#include <sstream>
#include <vector>

StompProtocol::StompProtocol() : subscriptionIdCounter(0), receiptIdCounter(0), receiptMap(), subscriptionMap(), messageList() {}

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

    // Collect and format events
    summaryStream << "Channel " << channelName << "\n";
    summaryStream << "Stats: \n";
    summaryStream << "Total: " << getMessages().size() << "\n";
    summaryStream << "Active: " << numOfActive() << "\n";
    summaryStream << "Forces arrival at scene: " << numOfForcesArrival() << "\n\n";
    summaryStream << "Event Report: " << "\n\n";

    int counter = 1;
    for (const std::string &message : getMessages())
    {

        std::map<std::string, std::string> headers = parseFrame(message);

        std::string body = headers["body"];
        std::map<std::string, std::string> bodyParsed = parseFrame(body);
        summaryStream << "Report_" << counter << ":\n";
        summaryStream << addDetails(bodyParsed);

        summaryStream << "\n";
        counter++;
    }
    summaryStream << '\0';
    return summaryStream.str();
}
std::string StompProtocol::addDetails(std::map<std::string, std::string> &bodyParsed)
{

    std::ostringstream oss;
    for (const auto &[key, value] : bodyParsed)
    {
        if (key == "city")
        {
            oss << "  city:" << value << "\n";
            continue;
        }
        if (key == "date time")
        {
            oss << "  date time:" << value << "\n";
            continue;
        }
        if (key == "event name")
        {
            oss << "  event name:" << value << "\n";
            continue;
        }
        if (key == "description")
        {
            oss << "  summary:" << value << "\n";
            continue;
        }
    }
    return oss.str();
}

int StompProtocol::numOfActive()
{
    int count = 0;
    for (string message : getMessages())
    {
        if (message.find("active: true") != string::npos)
        {
            count++;
        }
    }
    return count;
}

int StompProtocol::numOfForcesArrival()
{
    int count = 0;
    for (string message : getMessages())
    {
        if (message.find("forces_arrival_at_scene: true") != string::npos)
        {
            count++;
        }
    }
    return count;
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

void StompProtocol::addMessage(const string &message)
{
    messageList.push_back(message);
}

vector<string> StompProtocol::getMessages()
{
    return messageList;
}