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
        cout << "Command: " << command << endl;

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
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent CONNECT frame to server" << endl;
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
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent SEND frame to server" << endl;
        }
        else if (command == "join")
        {
            string channel;
            iss >> channel;
            string subscribeFrame = stompProtocol.createSubscribeFrame(channel);
            if (!connectionHandler->sendLine(subscribeFrame))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent SUBSCRIBE frame to server" << endl;
        }
        else if (command == "exit")
        {
            string id;
            iss >> id;
            string unsubscribeFrame = stompProtocol.createUnsubscribeFrame(id);
            if (!connectionHandler->sendLine(unsubscribeFrame))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent UNSUBSCRIBE frame to server" << endl;
        }
        else if (command == "logout")
        {
            string disconnectFrame = stompProtocol.createDisconnectFrame();
            if (!connectionHandler->sendLine(disconnectFrame))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent DISCONNECT frame to server" << endl;
        }
        else
        {
            int len = line.length();
            if (!connectionHandler->sendLine(line))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent " << len + 1 << " bytes to server" << endl;
            if (line == "bye")
            {
                break;
            }
        }
    }
}

void socketReader(ConnectionHandler *&connectionHandler, StompProtocol &stompProtocol)
{
    while (true)
    {
        if (connectionHandler == nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Wait for connectionHandler to be initialized
            continue;
        }
        string answer;
        if (!connectionHandler->getLine(answer))
        {
            cout << "Disconnected. Exiting...\n"
                 << endl;
            break;
        }
        cout << "Reply: " << answer << endl;
        map<string, string> headers = stompProtocol.parseFrame(answer);
        string command = headers["command"];
        if (command == "RECEIPT")
        {
            int receiptId = stoi(headers["receipt-id"]);
            string request = stompProtocol.getRequestByReceipt(receiptId);
            if (request == "DISCONNECT")
            {
                cout << "Logged out" << endl;
                connectionHandler->close();
                break;
            }
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