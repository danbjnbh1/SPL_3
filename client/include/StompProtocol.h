#pragma once
#include <string>
#include <map>
#include "../include/ConnectionHandler.h"

using namespace std;

class StompProtocol
{
private:
    int subscriptionIdCounter;
    int receiptIdCounter;
    map<int, string> receiptMap; // Map to store receipt IDs and their corresponding requests

public:
    StompProtocol();

    string createConnectFrame(const string &host, const string &login, const string &passcode);
    string createSendFrame(const string &destination, const string &message);
    string createSubscribeFrame(const string &destination);
    string createUnsubscribeFrame(const string &id);
    string createDisconnectFrame();
    map<string, string> parseFrame(const string &frame);

    int getNextSubscriptionId();
    int getNextReceiptId();
    void addReceipt(int receiptId, const std::string &request);
    string getRequestByReceipt(int receiptId);
};
