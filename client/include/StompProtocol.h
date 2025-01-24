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

public:
    StompProtocol();

    string createConnectFrame(const string &host, const string &login, const string &passcode);
    string createSendFrame(const Event &event, const std::string &channel_name);
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
    std::string generateSummary(const std::string &channelName);
};
