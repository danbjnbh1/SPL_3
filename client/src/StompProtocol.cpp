#include "StompProtocol.h"
#include <sstream>
#include <vector>

StompProtocol::StompProtocol() : subscriptionIdCounter(0), receiptIdCounter(0), receiptMap(), subscriptionMap(), messageList(), username() {}

string StompProtocol::createConnectFrame(const string &host, const string &username, const string &password)
{
    int receiptId = getNextReceiptId();
    ostringstream oss;
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

string StompProtocol::createSendFrame(const Event &event, const string &channel_name, const string &user_name)
{
    ostringstream frame;

    // Construct the SEND frame for the current event
    frame << "SEND\n"
          << "destination:/" << channel_name << "\n"
          << "receipt:" << getNextReceiptId() << "\n\n"
          << "user: " << user_name << "\n" // Assume a global `username` variable
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

string StompProtocol::generateSummary(const string &channelName, const string &clientName)
{
    ostringstream summaryStream;

    // Check if the channel exists
    int subscriptionId = getSubscriptionIdByChannel(channelName);
    if (subscriptionId == -1)
    {
        return "Channel " + channelName + " not found.";
    }

    // Collect and format events
    summaryStream << "Channel " << channelName << "\n";
    summaryStream << "Stats: \n";
    summaryStream << "Total: " << getMessages(clientName).size() << "\n";
    summaryStream << "Active: " << numOfActive(clientName) << "\n";
    summaryStream << "Forces arrival at scene: " << numOfForcesArrival(clientName) << "\n\n";
    summaryStream << "Event Report: " << "\n\n";

    int counter = 1;
    for (const string &message : getMessages(clientName))
    {
        
        cout << message << endl;

        map<string, string> headers = parseFrame(message);
        string body = headers["body"];
        map<string, string> bodyParsed = parseEventBody(body);
        summaryStream << "Report_" << counter << ":\n";
        summaryStream << addDetails(bodyParsed);

        summaryStream << "\n";
        counter++;
    }
    summaryStream << '\0';
    return summaryStream.str();
}
string StompProtocol::addDetails(map<string, string> &bodyParsed)
{
    ostringstream oss;
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

int StompProtocol::numOfActive(const string &clientName)
{
    int count = 0;
    for (string message : getMessages(clientName))
    {
        if (message.find("active: true") != string::npos)
        {
            count++;
        }
    }
    return count;
}

int StompProtocol::numOfForcesArrival(const string &clientName)
{
    int count = 0;
    for (string message : getMessages(clientName))
    {
        if (message.find("forces_arrival_at_scene: true") != string::npos)
        {
            count++;
        }
    }
    return count;
}

string StompProtocol::createSubscribeFrame(const string &destination)
{
    int receiptId = getNextReceiptId();
    ostringstream oss;
    oss << "SUBSCRIBE\n"
        << "receipt:" << receiptId << "\n"
        << "destination:" << destination << "\n"
        << "id:" << getNextSubscriptionId() << "\n"
        << '\0';

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
}

string StompProtocol::createUnsubscribeFrame(const string &channel)
{
    int subsctiptionId = getSubscriptionIdByChannel(channel); //! handle -1 id
    int receiptId = getNextReceiptId();
    ostringstream oss;
    oss << "UNSUBSCRIBE\n"
        << "receipt:" << receiptId << "\n"
        << "id:" << subsctiptionId << "\n\n"
        << '\0';

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
}

string StompProtocol::createDisconnectFrame()
{
    int receiptId = getNextReceiptId();
    ostringstream oss;
    oss << "DISCONNECT\n"
        << "receipt:" << receiptId << "\n"
        << '\0';

    string frame = oss.str();
    addReceipt(receiptId, frame);

    return frame;
}

map<string, string> StompProtocol::parseFrame(const string &frame)
{
    map<string, string> headers;
    istringstream stream(frame);
    string line;

    // Get the command
    getline(stream, line);
    headers["command"] = line;

    // Get the headers
    while (getline(stream, line) && !line.empty())
    {
        auto colonPos = line.find(':');
        if (colonPos != string::npos)
        {
            headers[line.substr(0, colonPos)] = line.substr(colonPos + 1);
        }
    }

    // Get the body (if any)
    string body;
    while (getline(stream, line))
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

#include "StompProtocol.h"
#include <sstream>
#include <map>
#include <string>

map<string, string> StompProtocol::parseEventBody(const string &body)
{
    map<string, string> eventDetails;
    istringstream stream(body);
    string line;

    bool inGeneralInfo = false;
    bool nextLineIsDescription = false;
    while (getline(stream, line))
    {
        auto colonPos = line.find(':');
        if (colonPos != string::npos)
        {
            string key = line.substr(0, colonPos);
            string value = line.substr(colonPos + 1);

            if (key == "general information")
            {
                inGeneralInfo = true;
                continue;
            }

            if (inGeneralInfo)
            {
                // Skip lines in the general information section
                if (line[0] != ' ')
                {
                    inGeneralInfo = false;
                }
                else
                {
                    cout << "key: " << key << " value: " << value << endl;
                    continue;
                }
            }

            if (key == "description")
            {
                nextLineIsDescription = true;
                eventDetails[key] = ""; // Initialize the description key
                continue;
            }

            eventDetails[key] = value;
        }

        else if (nextLineIsDescription)
        {
            eventDetails["description"] = line;
            nextLineIsDescription = false;
        }
    }

    return eventDetails;
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

vector<string> StompProtocol::getMessages(const string &clientName)
{
    int counter = 0;
    vector<string> messages;
    for (const string &message : messageList)
    {
        map<string, string> headers = parseFrame(message);
        if (headers["user"] == clientName)
        {
            messages.push_back(message);
        }
    }
    
    return messageList;
}

void StompProtocol::setUsername(const string &username)
{
    this->username = username;
}

const string &StompProtocol::getUsername() const
{
    return username;
}