#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
#include "ConnectionHandler.h"
#include "StompProtocol.h"
#include "event.h"
#include "FileUtils.h"

using namespace std;
mutex mtx;
condition_variable cv;

void keyboardReader(ConnectionHandler *&connectionHandler, StompProtocol *&stompProtocol)
{
    while (true)
    {
        const short bufsize = 1024;
        char buf[bufsize];
        cin.getline(buf, bufsize);
        string line(buf);
        istringstream iss(line);
        string command;
        iss >> command;

        if (command == "login")
        {
            if (connectionHandler->isConnected())
            {
                cout << "Please logout first" << endl;
                continue;
            }
            string hostPort, username, password;
            iss >> hostPort >> username >> password;

            string host = hostPort.substr(0, hostPort.find(':'));
            string portStr = hostPort.substr(hostPort.find(':') + 1);
            short port = stoi(portStr);

            if (!connectionHandler->connect(host, port))
            {
                cout << "Cannot connect to " << host << ":" << port << endl;
                connectionHandler->close();
                continue;
            }
            cv.notify_all(); // Notify the socketReader thread

            string connectFrame = stompProtocol->createConnectFrame("stomp.cs.bgu.ac.il", username, password);
            if (!connectionHandler->sendLine(connectFrame))
            {
                cout << "Disconnected. Exiting1...\n"
                     << endl;
                break;
            }
            stompProtocol->setUsername(username);
            continue;
        }
        if (!connectionHandler || !connectionHandler->isConnected())
        {
            cout << "Please login first" << endl;
            continue;
        }
        else if (command == "report")
        {
            // Extract the file path from the input
            string filePath;
            iss >> filePath;

            // Parse events file using the given `parseEventsFile` function
            names_and_events parsedData = parseEventsFile(filePath);

            // Extract the channel name and events
            string channel_name = parsedData.get_channel_name();
            if (stompProtocol->getSubscriptionIdByChannel(channel_name) == -1)
            {
                cout << "You are not subscribed to this channel" << endl;
                continue;
            }
            vector<Event> events = parsedData.get_events();

            // Sort events by `date_time`
            sort(events.begin(), events.end(), [](const Event &a, const Event &b)
                 { return a.get_date_time() < b.get_date_time(); });

            // Create and send frames for each event
            for (const Event &event : events)
            {
                // Call the `createSendFrame` function to construct the frame
                string user_name = stompProtocol->getUsername();
                std::string frameStr = stompProtocol->createSendFrame(event, channel_name, user_name);

                // Send the frame to the server
                if (!connectionHandler->sendLine(frameStr))
                {
                    cerr << "Failed to send frame for event: " << event.get_name() << endl;
                    break;
                }
                cout << "Sent frame for event: " << event.get_name() << endl;
            }
        }
        else if (command == "join")
        {
            string channel;
            iss >> channel;
            if (stompProtocol->getSubscriptionIdByChannel(channel) != -1)
            {
                cout << "You are already subscribed to this channel" << endl;
                continue;
            }
            string subscribeFrame = stompProtocol->createSubscribeFrame(channel);
            if (!connectionHandler->sendLine(subscribeFrame))
            {
                cout << "Disconnected. Exiting3...\n"
                     << endl;
                break;
            }
        }
        else if (command == "exit")
        {
            string channel;
            iss >> channel;
            int id = stompProtocol->getSubscriptionIdByChannel(channel);
            if (id == -1)
            {
                cout << "You are not subscribed to this channel" << endl;
                continue;
            }
            string unsubscribeFrame = stompProtocol->createUnsubscribeFrame(channel);
            if (!connectionHandler->sendLine(unsubscribeFrame))
            {
                cout << "Disconnected. Exiting4...\n"
                     << endl;
                break;
            }
        }
        else if (command == "logout")
        {
            string disconnectFrame = stompProtocol->createDisconnectFrame();
            if (!connectionHandler->sendLine(disconnectFrame))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
        }
        else if (command == "summary")
        {
            string channelName, clientName, summaryFile;

            iss >> channelName >> clientName;
            getline(iss, summaryFile);

            if (!summaryFile.empty() && summaryFile[0] == ' ')
            {
                summaryFile = summaryFile.substr(1);
            }

            // Generate the summary for the channel
            string summary = stompProtocol->generateSummary(channelName, clientName);

            // Write the summary to the file
            if (writeSummaryToFile(summaryFile, summary))
            {
                cout << "Summary written to " << summaryFile << endl;
            }
            else
            {
                cerr << "Failed to write summary to " << summaryFile << endl;
            }
        }

        else
        {
            cout << "Invalid command" << endl;
        }
    }
}

void socketReader(ConnectionHandler *&connectionHandler, StompProtocol *&stompProtocol)
{
    unique_lock<mutex> lock(mtx);
    while (true)
    {
        cv.wait(lock, [&connectionHandler]
                { return connectionHandler != nullptr && connectionHandler->isConnected(); });

        string answer;
        if (!connectionHandler->getLine(answer))
        {
            cout << "Disconnected. Exiting7...\n"
                 << endl;
            break;
        }
        cout << answer << endl;
        map<string, string> headers = stompProtocol->parseFrame(answer);
        string command = headers["command"];
        if (command == "RECEIPT")
        {
            int receiptId = stoi(headers["receipt-id"]);
            string requestedFrame = stompProtocol->getRequestByReceipt(receiptId);
            map<string, string> parsedRequestedFrame = stompProtocol->parseFrame(requestedFrame);
            string requestedCommand = parsedRequestedFrame["command"];
            if (requestedCommand == "DISCONNECT")
            {
                cout << "Logged out" << endl;
                connectionHandler->close();
                if (stompProtocol != nullptr)
                {
                    delete stompProtocol;
                    stompProtocol = new StompProtocol();
                }
                continue;
            }
            if (requestedCommand == "SUBSCRIBE")
            {
                cout << "Joined channel " << parsedRequestedFrame["destination"] << endl;
                stompProtocol->addSubscription(stoi(parsedRequestedFrame["id"]), parsedRequestedFrame["destination"]);
                continue;
            }
            if (requestedCommand == "UNSUBSCRIBE")
            {
                cout << "Exited channel " << stompProtocol->getChannelById(stoi(parsedRequestedFrame["id"])) << endl;
                stompProtocol->removeSubscription(stoi(parsedRequestedFrame["id"]));
                continue;
            }
        }

        if (command == "ERROR")
        {
            cout << "ERROR FROM THE SERVER: " << endl;
            cout << answer << endl;
            connectionHandler->close();
            if (stompProtocol != nullptr)
            {
                delete stompProtocol;
                stompProtocol = new StompProtocol();
            }
        }

        if (command == "CONNECTED")
        {
            cout << "Login successful" << endl;
        }

        if (command == "MESSAGE")
        {
            stompProtocol->addMessage(answer);
        }
    }
}

int main(int argc, char *argv[])
{
    cout << "Starting client" << endl;
    ConnectionHandler *connectionHandler = new ConnectionHandler();
    StompProtocol *stompProtocol = new StompProtocol();

    thread socketThread(socketReader, ref(connectionHandler), ref(stompProtocol));
    keyboardReader(connectionHandler, stompProtocol);

    socketThread.join();

    delete connectionHandler;
    delete stompProtocol;

    return 0;
}