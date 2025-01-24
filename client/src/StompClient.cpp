#include <iostream>
#include <thread>
#include <fstream>
#include <mutex>
#include "ConnectionHandler.h"
#include "StompProtocol.h"
#include "event.h"

using namespace std;
mutex mtx;
condition_variable cv;

void keyboardReader(ConnectionHandler *&connectionHandler, StompProtocol &stompProtocol)
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
            if (connectionHandler && connectionHandler->isConnected())
            {
                cout << "Please logout first" << endl;
                continue;
            }
            string hostPort, username, password;
            iss >> hostPort >> username >> password;

            string host = hostPort.substr(0, hostPort.find(':'));
            string portStr = hostPort.substr(hostPort.find(':') + 1);
            short port = stoi(portStr);

            if (connectionHandler != nullptr)
            {
                delete connectionHandler;
            }
            connectionHandler = new ConnectionHandler(host, port);

            if (!connectionHandler->connect())
            {
                cout << "Cannot connect to " << host << ":" << port << endl;
                continue;
            }
            cv.notify_all(); // Notify the socketReader thread

            string connectFrame = stompProtocol.createConnectFrame("stomp.cs.bgu.ac.il", username, password);
            if (!connectionHandler->sendLine(connectFrame))
            {
                cout << "Disconnected. Exiting1...\n"
                     << endl;
                break;
            }
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
            std::string filePath;
            iss >> filePath;

            // Parse events file using the given `parseEventsFile` function
            names_and_events parsedData = parseEventsFile(filePath);

            // Extract the channel name and events
            std::string channel_name = parsedData.get_channel_name();
            std::vector<Event> events = parsedData.get_events();

            // Sort events by `date_time`
            std::sort(events.begin(), events.end(), [](const Event &a, const Event &b)
                      { return a.get_date_time() < b.get_date_time(); });

            // Create and send frames for each event
            for (const Event &event : events)
            {
                // Call the `createSendFrame` function to construct the frame
                std::string frameStr = stompProtocol.createSendFrame(event, channel_name);

                // Send the frame to the server
                if (!connectionHandler->sendLine(frameStr))
                {
                    std::cerr << "Failed to send frame for event: " << event.get_name() << std::endl;
                    break;
                }
                std::cout << "Sent frame for event: " << event.get_name() << std::endl;
            }
        }

        else if (command == "join")
        {
            string channel;
            iss >> channel;
            string subscribeFrame = stompProtocol.createSubscribeFrame(channel);
            if (!connectionHandler->sendLine(subscribeFrame))
            {
                cout << "Disconnected. Exiting3...\n"
                     << endl;
                break;
            }
        }
        else if (command == "exit")
        {
            string id;
            iss >> id;
            string unsubscribeFrame = stompProtocol.createUnsubscribeFrame(id);
            if (!connectionHandler->sendLine(unsubscribeFrame))
            {
                cout << "Disconnected. Exiting4...\n"
                     << endl;
                break;
            }
        }
        else if (command == "logout")
        {
            string disconnectFrame = stompProtocol.createDisconnectFrame();
            if (!connectionHandler->sendLine(disconnectFrame))
            {
                cout << "Disconnected. Exiting5...\n"
                     << endl;
                break;
            }
        }
        else if (command == "summary")
        {
        }
        else
        {
            cout << "Invalid command" << endl;
        }
    }
}

void socketReader(ConnectionHandler *&connectionHandler, StompProtocol &stompProtocol)
{
    unique_lock<mutex> lock(mtx);
    while (true)
    {
        cout << "Waiting for connection..." << endl;
        cv.wait(lock, [&connectionHandler]
                { return connectionHandler != nullptr && connectionHandler->isConnected(); });
        cout << "Connected!" << endl;
        if (connectionHandler == nullptr || !connectionHandler->isConnected())
        {
            this_thread::sleep_for(chrono::milliseconds(100)); // Wait for connectionHandler to be initialized
            continue;
        }
        string answer;
        if (!connectionHandler->getLine(answer))
        {
            cout << "Disconnected. Exiting7...\n"
                 << endl;
            break;
        }
        cout << answer << endl;
        map<string, string> headers = stompProtocol.parseFrame(answer);
        string command = headers["command"];
        if (command == "RECEIPT")
        {
            int receiptId = stoi(headers["receipt-id"]);
            string requestedFrame = stompProtocol.getRequestByReceipt(receiptId);
            map<string, string> parsedRequestedFrame = stompProtocol.parseFrame(requestedFrame);
            string requestedCommand = parsedRequestedFrame["command"];
            cout << requestedFrame << endl;
            if (requestedCommand == "DISCONNECT")
            {
                cout << "Logged out" << endl;
                connectionHandler->close();
                continue;
            }
            if (requestedCommand == "SUBSCRIBE")
            {
                cout << "Joined channel " << parsedRequestedFrame["destination"] << endl;
                stompProtocol.addSubscription(stoi(parsedRequestedFrame["id"]), parsedRequestedFrame["destination"]);
                continue;
            }
            if (requestedCommand == "UNSUBSCRIBE")
            {
                cout << "Exited channel " << stompProtocol.getChannelById(stoi(parsedRequestedFrame["id"])) << endl;
                stompProtocol.removeSubscription(stoi(parsedRequestedFrame["id"]));
                continue;
            }
        }

        if (command == "ERROR")
        {
            cout << "ERROR FROM THE SERVER: " << endl;
            cout << answer << endl;
            connectionHandler->close();
        }

        if (command == "CONNECTED")
        {
            cout << "Login successful" << endl;
        }
    }
}

int main(int argc, char *argv[])
{
    cout << "Starting client" << endl;
    ConnectionHandler *connectionHandler = nullptr;
    StompProtocol stompProtocol;

    thread socketThread(socketReader, ref(connectionHandler), ref(stompProtocol));
    keyboardReader(connectionHandler, stompProtocol);

    socketThread.join();

    return 0;
}