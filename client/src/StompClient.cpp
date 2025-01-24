#include <iostream>
#include <thread>
#include <mutex>
#include "ConnectionHandler.h"
#include "StompProtocol.h"
using namespace std;
mutex mtx;

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
        else if (command == "send")
        {
            string destination, message;
            iss >> destination;
            getline(iss, message);
            string sendFrame = stompProtocol.createSendFrame(destination, message);
            if (!connectionHandler->sendLine(sendFrame))
            {
                cout << "Disconnected. Exiting2...\n"
                     << endl;
                break;
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
        else
        {
            cout << "Invalid command" << endl;
        }
    }
}

void socketReader(ConnectionHandler *&connectionHandler, StompProtocol &stompProtocol)
{
    while (true)
    {
        if (connectionHandler == nullptr || !connectionHandler->isConnected())
        {
            cout << "waiting for connectionHandler to be initialized" << endl;

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

    thread keyboardThread(keyboardReader, ref(connectionHandler), ref(stompProtocol));
    thread socketThread(socketReader, ref(connectionHandler), ref(stompProtocol));

    keyboardThread.join();
    socketThread.join();

    return 0;
}