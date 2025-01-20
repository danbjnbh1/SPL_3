#include <iostream>
#include <thread>
#include <mutex>
#include "client/include/ConnectionHandler.h"
#include "client/include/StompProtocol.h"

std::mutex mtx;

void keyboardReader(ConnectionHandler &connectionHandler, StompProtocol &stompProtocol) {
    while (true) {
        const short bufsize = 1024;
        char buf[bufsize];
        std::cin.getline(buf, bufsize);
        std::string line(buf);
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "login") {
            std::string hostPort, username, password;
            iss >> hostPort >> username >> password;

            std::string host = hostPort.substr(0, hostPort.find(':'));
            std::string portStr = hostPort.substr(hostPort.find(':') + 1);
            short port = std::stoi(portStr);

            std::string connectFrame = stompProtocol.createConnectFrame("stomp.cs.bgu.ac.il", username, password);
            if (!connectionHandler.sendLine(connectFrame)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::cout << "Sent CONNECT frame to server" << std::endl;
        } else if (command == "send") {
            std::string destination, message;
            iss >> destination;
            std::getline(iss, message);
            std::string sendFrame = stompProtocol.createSendFrame(destination, message);
            if (!connectionHandler.sendLine(sendFrame)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::cout << "Sent SEND frame to server" << std::endl;
        } else if (command == "subscribe") {
            std::string destination, id;
            iss >> destination >> id;
            std::string subscribeFrame = stompProtocol.createSubscribeFrame(destination, id);
            if (!connectionHandler.sendLine(subscribeFrame)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::cout << "Sent SUBSCRIBE frame to server" << std::endl;
        } else if (command == "unsubscribe") {
            std::string id;
            iss >> id;
            std::string unsubscribeFrame = stompProtocol.createUnsubscribeFrame(id);
            if (!connectionHandler.sendLine(unsubscribeFrame)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::cout << "Sent UNSUBSCRIBE frame to server" << std::endl;
        } else if (command == "disconnect") {
            std::string disconnectFrame = stompProtocol.createDisconnectFrame();
            if (!connectionHandler.sendLine(disconnectFrame)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::cout << "Sent DISCONNECT frame to server" << std::endl;
            break;
        } else {
            int len = line.length();
            if (!connectionHandler.sendLine(line)) {
                std::cout << "Disconnected. Exiting...\n" << std::endl;
                break;
            }
            std::cout << "Sent " << len + 1 << " bytes to server" << std::endl;
            if (line == "bye") {
                break;
            }
        }
    }
}

void socketReader(ConnectionHandler &connectionHandler, StompProtocol &stompProtocol) {
    while (true) {
        std::string answer;
        if (!connectionHandler.getLine(answer)) {
            std::cout << "Disconnected. Exiting...\n" << std::endl;
            break;
        }
        int len = answer.length();
        answer.resize(len - 1); // Remove the newline character
        std::cout << "Reply: " << answer << " " << len << " bytes " << std::endl << std::endl;
        auto headers = stompProtocol.parseFrame(answer);
        // Process the headers as needed
        if (headers["command"] == "bye") {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " host port" << std::endl << std::endl;
        return -1;
    }
    std::string host = argv[1];
    short port = atoi(argv[2]);

    ConnectionHandler connectionHandler(host, port);
    StompProtocol stompProtocol;

    if (!connectionHandler.connect()) {
        std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
        return 1;
    }

    std::thread keyboardThread(keyboardReader, std::ref(connectionHandler), std::ref(stompProtocol));
    std::thread socketThread(socketReader, std::ref(connectionHandler), std::ref(stompProtocol));

    keyboardThread.join();
    socketThread.join();

    return 0;
}