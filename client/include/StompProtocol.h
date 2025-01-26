#pragma once
#include <string>
#include <map>
#include "../include/ConnectionHandler.h"
#include "../include/event.h"

using namespace std;

class StompProtocol
{
private:
    int subscriptionIdCounter;
    int receiptIdCounter;
    map<int, string> receiptMap;      // Map to store receipt IDs and their corresponding requests
    map<int, string> subscriptionMap; // Map to store subscription IDs and their corresponding requests
    vector<string> messageList;
    string username;

public:
    StompProtocol();

    string createConnectFrame(const string &host, const string &login, const string &passcode);
    string createSendFrame(const Event &event, const std::string &channel_name, const std::string &user_name);
    string createSubscribeFrame(const string &destination);
    string createUnsubscribeFrame(const string &id);
    string createDisconnectFrame();
    map<string, string> parseFrame(const string &frame);

    int getNextSubscriptionId();
    int getNextReceiptId();
    void addReceipt(int receiptId, const std::string &request);
    void addSubscription(int subscriptionId, const string &destination);
    void removeSubscription(int subscriptionId);
    string getRequestByReceipt(int receiptId);
    int getSubscriptionIdByChannel(const string &channel);
    string getChannelById(int id);
    string generateSummary(const std::string &channelName, const std::string &clientName);
    void addMessage(const string &message);
    vector<string> getMessages(const string &clientName);
    int numOfActive(const string &clientName);
    int numOfForcesArrival(const string &clientName);
    string addDetails(std::map<std::string, std::string> &bodyParsed);
    map<string, string> parseEventBody(const string &body);
    void setUsername(const string &username);
    const string &getUsername() const;
    string converTimestampToString(const std::string &timestampStr);
    string truncateString(const std::string &str);
};
