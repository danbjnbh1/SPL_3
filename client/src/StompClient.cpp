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
        else if (command == "subscribe")
        {
            string destination, id;
            iss >> destination >> id;
            string subscribeFrame = stompProtocol.createSubscribeFrame(destination, id);
            if (!connectionHandler->sendLine(subscribeFrame))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent SUBSCRIBE frame to server" << endl;
        }
        else if (command == "unsubscribe")
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
        else if (command == "disconnect")
        {
            string disconnectFrame = stompProtocol.createDisconnectFrame();
            if (!connectionHandler->sendLine(disconnectFrame))
            {
                cout << "Disconnected. Exiting...\n"
                     << endl;
                break;
            }
            cout << "Sent DISCONNECT frame to server" << endl;
            break;
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
        int len = answer.length();
        answer.resize(len - 1); // Remove the newline character
        cout << "Reply: " << answer << " " << len << " bytes " << endl
             << endl;
        // auto headers = stompProtocol.parseFrame(answer);
        // // Process the headers as needed
        // if (headers["command"] == "bye")
        // {
        //     break;
        // }
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